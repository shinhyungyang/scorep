/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2022-2024,
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

#include "scorep_ompt.h"
#include "scorep_ompt_device.h"
#include "scorep_ompt_confvars.h"

#include <SCOREP_AcceleratorManagement.h>

#include <scorep_system_tree.h>


/* forward declarations */
/* *INDENT-OFF* */
static inline void device_tracing_initialize( scorep_ompt_target_device_t*, ompt_function_lookup_t );
/* *INDENT-ON* */


/* conveniece */
#define SCOREP_OMPT_ENSURE_TARGET_DATA_NON_NULL( CALLBACK )                      \
    do {                                                                         \
        if ( target_data == NULL )                                               \
        {                                                                        \
            UTILS_FATAL( "Encountered OpenMP %s callback without target_data "   \
                         "pointer. This is a runtime issue. Please disable the " \
                         "measurement of OpenMP target directives in the "       \
                         "OMPT adapter.", ( CALLBACK ) );                        \
        }                                                                        \
    } while ( 0 )

void
scorep_ompt_device_tracing_finalize( scorep_ompt_target_device_t* device )
{
    if ( !device )
    {
        return;
    }
    if ( !device->device_tracing_available )
    {
        return;
    }
    device->device_functions.flush_trace( device->address );
    if ( device->device_functions.pause_trace )
    {
        device->device_functions.pause_trace( device->address, true );
    }
    if ( device->device_functions.stop_trace )
    {
        device->device_functions.stop_trace( device->address );
    }
}


static inline scorep_ompt_device_stream_t*
get_device_stream( scorep_ompt_target_device_t* device,
                   uint64_t                     timestamp )
{
    UTILS_MutexLock( &device->stream_lock );
    for ( scorep_ompt_device_stream_t* ptr = device->streams; ptr != NULL; ptr = ptr->next )
    {
        UTILS_MutexWait( &ptr->stream_lock, UTILS_ATOMIC_RELAXED );
        if ( SCOREP_Location_GetLastTimestamp( ptr->scorep_location ) <= timestamp )
        {
            UTILS_MutexUnlock( &device->stream_lock );
            return ptr;
        }
    }
    UTILS_MutexUnlock( &device->stream_lock );
    return NULL;
}


static inline scorep_ompt_device_stream_t*
add_device_stream( scorep_ompt_target_device_t* device,
                   SCOREP_Location*             parentLocation )
{
    UTILS_MutexLock( &device->stream_lock );
    /* Stream naming */
    char location_name[ 64 ];
    snprintf( location_name, 64, "OMP Target[%d:%d]", device->device_num, device->num_streams + 1 );
    /* Create a new stream. Since we may work on a helper thread, use SCOREP_Location_AllocForMisc. */
    /* Use the front of the device streams since it is certainly not used during this operation. */
    scorep_ompt_device_stream_t* stream = SCOREP_Location_AllocForMisc(
        parentLocation,
        sizeof( scorep_ompt_device_stream_t ) );
    stream->scorep_location = SCOREP_Location_CreateNonCPULocation(
        initial_task.scorep_location,
        SCOREP_LOCATION_TYPE_GPU,
        SCOREP_PARADIGM_OPENMP_TARGET,
        location_name,
        device->location_group );
    UTILS_BUG_ON( !stream->scorep_location );
    stream->local_rank = UTILS_Atomic_FetchAdd_uint32(
        &scorep_ompt_global_location_count,
        1,
        UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
    stream->next = NULL;

    if ( device->streams == NULL )
    {
        device->num_streams = 0;
        device->streams     = stream;
    }
    else
    {
        scorep_ompt_device_stream_t* current = device->streams;
        while ( current->next != NULL )
        {
            current = current->next;
        }
        current->next = stream;
    }
    device->num_streams++;

    UTILS_DEBUG_EXIT( "Added new device stream %s [%p] to %s",
                      location_name,
                      stream,
                      device->name );
    UTILS_MutexUnlock( &device->stream_lock );
    return stream;
}


static inline scorep_ompt_device_stream_t*
get_or_add_device_stream( scorep_ompt_target_device_t* device,
                          uint64_t                     timestamp )
{
    scorep_ompt_device_stream_t* return_value = get_device_stream( device, timestamp );
    if ( !return_value )
    {
        return_value = add_device_stream( device, device->streams->scorep_location );
    }
    UTILS_DEBUG_EXIT( "Returning stream %p", return_value );
    return return_value;
}


static inline scorep_ompt_target_device_t*
get_device( int deviceNum )
{
    scorep_ompt_target_device_t* return_val;
    if ( !scorep_ompt_device_table_get( deviceNum, &return_val ) )
    {
        return NULL;
    }
    return return_val;
}


static inline void
get_host_and_device_time( uint64_t*                    hostTime,
                          scorep_ompt_target_device_t* device,
                          uint64_t*                    deviceTime )
{
    UTILS_BUG_ON( !device->device_functions.get_device_time );
    uint64_t t1 = SCOREP_Timer_GetClockTicks();
    *deviceTime = device->device_functions.get_device_time( device );
    uint64_t t2 = SCOREP_Timer_GetClockTicks();
    /* Prevent overflow of time by using this calculation over ( t1 + t2 ) / 2.
     * This is also used by LLVM for the timestamp conversion. */
    *hostTime = t1 + ( t2 - t1 ) / 2;
}


static inline uint64_t
translate_to_host_time( scorep_ompt_target_device_t* device,
                        uint64_t                     deviceTimestamp )
{
    return ( uint64_t )(
        ( double )device->timestamp_host_ref_begin +
        ( double )( deviceTimestamp - device->timestamp_device_ref_begin )
        * device->timestamp_sync_factor );
    ;
}


/* Device hash table */

/* Helper struct for device table constructor */
typedef struct device_table_ctor_data_t
{
    ompt_device_t*         device;
    int32_t                device_num;
    const char*            type;
    ompt_function_lookup_t lookup;
} device_table_ctor_data_t;


uint32_t
scorep_ompt_device_table_bucket_idx( scorep_ompt_device_table_key_t key )
{
    uint32_t hashvalue = jenkins_hash( &key, sizeof( key ), 0 );
    return hashvalue & hashmask( SCOREP_OMPT_DEVICE_TABLE_HASH_EXPONENT );
}


bool
scorep_ompt_device_table_equals( scorep_ompt_device_table_key_t key1,
                                 scorep_ompt_device_table_key_t key2 )
{
    return key1 == key2;
}


void*
scorep_ompt_device_table_allocate_chunk( size_t chunkSize )
{
    return SCOREP_Memory_AlignedMalloc( SCOREP_CACHELINESIZE, chunkSize );
}



void
scorep_ompt_device_table_free_chunk( void* chunk )
{
    SCOREP_Memory_AlignedFree( chunk );
}


scorep_ompt_device_table_value_t
scorep_ompt_device_table_value_ctor( scorep_ompt_device_table_key_t* key,
                                     void*                           ctorData )
{
    scorep_ompt_target_device_t* new_device = SCOREP_Memory_AllocForMisc( sizeof( scorep_ompt_target_device_t ) );
    memset( new_device, 0, sizeof( scorep_ompt_target_device_t ) );
    device_table_ctor_data_t* data = ( device_table_ctor_data_t* )ctorData;

    size_t device_name_length = strlen( data->type ) + 6;
    new_device->name = SCOREP_Memory_AllocForMisc( device_name_length );
    snprintf( new_device->name, device_name_length, "%s (%d)", data->type, data->device_num );
    new_device->address    = data->device;
    new_device->device_num = data->device_num;
    /* Only check for device tracing if the user requests to use it */
    if ( scorep_ompt_target_use_device_tracing )
    {
        device_tracing_initialize( new_device, data->lookup );
    }
    if ( !new_device->device_tracing_available )
    {
        return new_device;
    }

    get_host_and_device_time(
        &new_device->timestamp_host_ref_begin,
        new_device,
        &new_device->timestamp_device_ref_begin );
    /* Create a system tree node */
    new_device->system_tree_node =
        SCOREP_Definitions_NewSystemTreeNode(
            SCOREP_GetSystemTreeNodeHandleForSharedMemory(),
            SCOREP_SYSTEM_TREE_DOMAIN_ACCELERATOR_DEVICE,
            "OpenMP Device",
            new_device->name );
    /* Location group */
    new_device->location_group = SCOREP_AcceleratorMgmt_CreateContext(
        new_device->system_tree_node,
        "OpenMP virtual streams" );
    /* Alloc metric */
    SCOREP_AllocMetric_NewScoped( "OpenMP Memory",
                                  new_device->location_group,
                                  &new_device->alloc_metric );

    /* Add at least a default stream to ensure that the location group is not empty */
    new_device->streams = NULL;
    add_device_stream( new_device, SCOREP_Location_GetCurrentCPULocation() );

    new_device->record_events = true;
    return new_device;
}


SCOREP_HASH_TABLE_MONOTONIC_DEFINITION( scorep_ompt_device_table, 4, hashsize( SCOREP_OMPT_DEVICE_TABLE_HASH_EXPONENT ) );
