/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023,
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
