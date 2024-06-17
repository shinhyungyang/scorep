/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023-2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 */

static inline uint64_t
get_host_local_rank( void )
{
    /* Ensure that the local rank for the host thread is set to ensure
     * that the data transfer shows up correctly. */
    scorep_ompt_cpu_location_data* data =
        SCOREP_Location_GetSubsystemData(
            SCOREP_Location_GetCurrentCPULocation(),
            scorep_ompt_get_subsystem_id() );
    if ( data->local_rank == SCOREP_OMPT_INVALID_LOCAL_RANK )
    {
        data->local_rank = UTILS_Atomic_FetchAdd_uint32(
            &scorep_ompt_global_location_count,
            1,
            UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
        UTILS_DEBUG( "[%s] Added location id %" PRIu32,
                     UTILS_FUNCTION_NAME,
                     data->local_rank );
    }

    return data->local_rank;
}


/* Use the following 64-bit layout for mapping targetIds:
 * -----------------------------------------------------------------------------
 * 00000000 00000000 | 00000000 00000000 00000000 00000000 00000000 00000000
 * hostLocationId    | hostOpId
 * -----------------------------------------------------------------------------
 * Reasoning:
 * While the OMPT adapter does transfer a lot of information into the buffer complete
 * callback, there is still some information we want to transfer ourselves.
 * To show the correct correspondent host location for data
 * transfers, we transfer the local rank used in RMA operations. The hostOpId is here
 * to identify the RMA operation by a unique ID.
 */

#define MAPPING_HOST_OP_ID_BITSIZE 48ul

static inline ompt_id_t
mapped_id_get_local_rank( ompt_id_t mappedTargetId )
{
    return mappedTargetId >> MAPPING_HOST_OP_ID_BITSIZE;
}

static inline ompt_id_t
mapped_id_get_host_op_id( ompt_id_t mappedTargetId )
{
    return mappedTargetId - ( ( mappedTargetId >> MAPPING_HOST_OP_ID_BITSIZE ) << MAPPING_HOST_OP_ID_BITSIZE );
}

static inline ompt_id_t
get_mapped_id( ompt_id_t hostLocationId,
               ompt_id_t hostOpId )
{
    return ( ompt_id_t )( ( hostLocationId << MAPPING_HOST_OP_ID_BITSIZE ) | hostOpId );
}

/* To correlate events between host-side callbacks and buffer events on the
 * target side, the OpenMP spec. offers the target_data->value field in the
 * callbacks, and the target_id field in ompt_record_ompt_t. Using
 * target_data->ptr is not an option, as it may cause undefined behavior in
 * C++ runtimes, and would violate the OpenMP 6.0 spec. (p.744, l. 30-33).
 * However, storing a single ompt_id_t in the target_id field is not sufficient
 * to store all the information we need, e.g. codeptr_ra and target_id.
 * For environments where a void* has a size of 64-bit or lower, we work around
 * this limitation by converting the pointer to ompt_id_t and store it in
 * target_data->value. ompt_id_t is defined as a 64-bt unsigned integer in the
 * OpenMP 6.0 spec. (33.18).
 * On other systems, we need to use a hash map to store the target data.
 * This approach is significantly slower than using pointers, but is unavoidable
 * if device tracing should still be supported. */
#if HAVE( SCOREP_OMPT_TARGET_NEEDS_HASHMAP )
typedef struct target_data_table_wrapper_t
{
    scorep_ompt_target_data_t* data;
} target_data_table_wrapper;


#define TARGET_DATA_TABLE_HASH_EXPONENT 5
typedef ompt_id_t                 target_data_table_key_t;
typedef target_data_table_wrapper target_data_table_value_t;


static uint32_t
target_data_table_bucket_idx( target_data_table_key_t key )
{
    uint32_t hashvalue = jenkins_hash( &key, sizeof( key ), 0 );
    return hashvalue & hashmask( TARGET_DATA_TABLE_HASH_EXPONENT );
}


static bool
target_data_table_equals( target_data_table_key_t key1,
                          target_data_table_key_t key2 )
{
    return key1 == key2;
}


static void*
target_data_table_allocate_chunk( size_t chunkSize )
{
    void* chunk = SCOREP_Memory_AlignedMalloc( SCOREP_CACHELINESIZE, chunkSize );
    UTILS_BUG_ON( chunk == 0 );
    return chunk;
}


static void
target_data_table_free_chunk( void* chunk )
{
    SCOREP_Memory_AlignedFree( chunk );
}


static target_data_table_value_t
target_data_table_value_ctor( target_data_table_key_t* key,
                              void*                    ctorData )
{
    target_data_table_wrapper return_val;
    memset( &return_val, 0, sizeof( target_data_table_wrapper ) );
    return_val.data = ctorData;
    return return_val;
}


static inline void
target_data_table_value_dtor( target_data_table_key_t   key,
                              target_data_table_value_t value )
{
    /* Nothing to do here */
}


SCOREP_HASH_TABLE_NON_MONOTONIC( target_data_table, 10, hashsize( TARGET_DATA_TABLE_HASH_EXPONENT ) );
#endif


static inline scorep_ompt_target_data_t*
get_target_data( ompt_id_t value )
{
    if ( value == ompt_id_none )
    {
        return NULL;
    }
    #if HAVE( SCOREP_OMPT_TARGET_NEEDS_HASHMAP )
    target_data_table_wrapper wrapper;
    bool                      found = target_data_table_get( value, &wrapper );
    if ( found )
    {
        return wrapper.data;
    }
    else
    {
        return NULL;
    }
    #else
    return ( scorep_ompt_target_data_t* )value;
    #endif
}


static inline void
set_target_data( ompt_id_t* value, scorep_ompt_target_data_t* data )
{
    #if HAVE( SCOREP_OMPT_TARGET_NEEDS_HASHMAP )
    *value = data->target_id;
    target_data_table_wrapper unused;
    bool                      inserted = target_data_table_get_and_insert( *value, ( void* )data, &unused );
    UTILS_BUG_ON( !inserted, "Failed to insert target data into hash map" );
    #else
    memcpy( value, &data, sizeof( scorep_ompt_target_data_t* ) );
    #endif
}


static inline void
free_target_data( ompt_id_t* value )
{
    #if HAVE( SCOREP_OMPT_TARGET_NEEDS_HASHMAP )
    target_data_table_remove( *value );
    #else
    scorep_ompt_target_data_t* data = ( scorep_ompt_target_data_t* )( *value );
    SCOREP_Memory_AlignedFree( data );
    #endif
    /* Explicitly set value to an invalid value to ensure that future lookups do not break.
     * Some runtimes (e.g. LLVM 18.1.x) re-use the target_data pointer. */
    static const ompt_id_t none_value = ompt_id_none;
    memcpy( value, &none_value, sizeof( none_value ) );
}
