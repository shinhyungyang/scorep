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

#include "scorep_ompt_callbacks_device.h"

/* Device tracing functions */

static void
device_tracing_handle_submit_record( const ompt_record_target_kernel_t callbackRecord,
                                     const scorep_ompt_target_data_t*  targetRegionInformation,
                                     scorep_ompt_device_stream_t*      stream,
                                     uint64_t                          startTime )
{
    UTILS_DEBUG_ENTRY( "host_op_id = %" PRIu64 " | requested_num_teams = %" PRIu32
                       " | granted_num_teams = %" PRIu32 " | "
                       "start_time = %" PRIu64 " | end_time = %" PRIu64,
                       callbackRecord.host_op_id,
                       callbackRecord.requested_num_teams,
                       callbackRecord.granted_num_teams,
                       startTime,
                       callbackRecord.end_time );

    SCOREP_RegionHandle region = get_region(
        targetRegionInformation->codeptr_ra,
        TOOL_EVENT_TARGET_KERNEL );
    SCOREP_Location_EnterRegion(
        stream->scorep_location,
        startTime,
        region );
    SCOREP_Location_TriggerParameterUint64(
        stream->scorep_location,
        startTime,
        parameters.callsite_id,
        ( uintptr_t )targetRegionInformation->codeptr_ra );
    SCOREP_Location_TriggerParameterUint64(
        stream->scorep_location,
        startTime,
        parameters.requested_num_teams,
        ( uint64_t )callbackRecord.requested_num_teams );
    SCOREP_Location_TriggerParameterUint64(
        stream->scorep_location,
        startTime,
        parameters.granted_num_teams,
        ( uint64_t )callbackRecord.granted_num_teams );
    SCOREP_Location_ExitRegion(
        stream->scorep_location,
        callbackRecord.end_time,
        region );
}

/* Callback-related functions */

static inline void
target_submit_emi_host_only( ompt_scope_endpoint_t endpoint,
                             ompt_data_t*          targetData,
                             ompt_id_t*            hostOpId,
                             unsigned int          requestedNumTeams )
{
    scorep_ompt_target_data_t* data = get_target_data( targetData->value );
    switch ( endpoint )
    {
        case ompt_scope_begin:
            /* Even though we do not use host_op_id, set a unique value to ensure that
             * the value differs from data transfers as the runtime might reuse pointers. */
            *hostOpId = scorep_ompt_get_unique_id();
            SCOREP_EnterRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_KERNEL_LAUNCH ) );
            break;
        case ompt_scope_end:
            SCOREP_ExitRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_KERNEL_LAUNCH ) );
            break;
        default:
            break;
    }
}


static inline void
target_submit_emi_device_tracing( ompt_scope_endpoint_t endpoint,
                                  ompt_data_t*          targetData,
                                  ompt_id_t*            hostOpId,
                                  unsigned int          requestedNumTeams )
{
    scorep_ompt_target_data_t* data = get_target_data( targetData->value );
    switch ( endpoint )
    {
        case ompt_scope_begin:
        {
            /* Even though we do not use host_op_id, set a unique value to ensure that
             * the value differs from data transfers as the runtime might reuse pointers. */
            *hostOpId = scorep_ompt_get_unique_id();
            SCOREP_EnterRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_KERNEL_LAUNCH ) );
            SCOREP_TriggerParameterUint64( parameters.callsite_id, ( uintptr_t )data->codeptr_ra );
            break;
        }
        case ompt_scope_end:
            SCOREP_ExitRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_KERNEL_LAUNCH ) );
            break;
        default:
            break;
    }
}


void
scorep_ompt_cb_target_submit_emi( ompt_scope_endpoint_t endpoint,
                                  ompt_data_t*          target_data,
                                  ompt_id_t*            host_op_id,
                                  unsigned int          requested_num_teams )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "endpoint = %s | target_data = %p | "
                       "target_data->value = %lx | host_op_id = %p (%lx) | "
                       "requested_num_teams = %d",
                       scope_endpoint2string( endpoint ),
                       target_data,
                       target_data ? target_data->value : 0,
                       host_op_id,
                       host_op_id ? *host_op_id : 0,
                       requested_num_teams );
    SCOREP_OMPT_ENSURE_INITIALIZED();
    SCOREP_OMPT_RETURN_ON_INVALID_EVENT();
    SCOREP_OMPT_ENSURE_TARGET_DATA_NON_NULL( "target_submit_emi" );

    scorep_ompt_target_data_t* data = get_target_data( target_data->value );
    /* Submit callback is invoked even though no callback has set the target_data field.
     * This means that the other callbacks did not finish correctly */
    if ( !data )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    if ( !data->supports_device_tracing )
    {
        target_submit_emi_host_only( endpoint,
                                     target_data,
                                     host_op_id,
                                     requested_num_teams );
    }
    else
    {
        target_submit_emi_device_tracing( endpoint,
                                          target_data,
                                          host_op_id,
                                          requested_num_teams );
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}
