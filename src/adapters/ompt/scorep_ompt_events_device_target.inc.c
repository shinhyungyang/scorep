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

/* Callback-related functions */

static inline bool
record_target_region( ompt_target_t kind )
{
    static bool initialized = false;
    static bool feature_kernel;
    static bool feature_memory;
    if ( !initialized )
    {
        feature_kernel = scorep_ompt_target_features & SCOREP_OMPT_TARGET_FEATURE_KERNEL;
        feature_memory = scorep_ompt_target_features & SCOREP_OMPT_TARGET_FEATURE_MEMORY;
        initialized    = true;
    }

    /* Only record target event if corresponding confvar is active */
    switch ( kind )
    {
        case ompt_target:
        case ompt_target_nowait:
            if ( !feature_kernel )
            {
                return false;
            }
            break;
        case ompt_target_enter_data:
        case ompt_target_enter_data_nowait:
        case ompt_target_exit_data:
        case ompt_target_exit_data_nowait:
        case ompt_target_update:
        case ompt_target_update_nowait:
            if ( !feature_memory )
            {
                return false;
            }
            break;
        default:
            return false;
    }

    return true;
}


static inline void
target_emi_host_only( ompt_target_t         kind,
                      ompt_scope_endpoint_t endpoint,
                      int                   deviceNum,
                      ompt_data_t*          taskData,
                      ompt_data_t*          targetTaskData,
                      ompt_data_t*          targetData,
                      const void*           codeptrRa )
{
    switch ( endpoint )
    {
        case ompt_scope_begin:
        {
            scorep_ompt_target_data_t* data = SCOREP_Memory_AlignedMalloc( SCOREP_ALLOCATOR_ALIGNMENT, sizeof( *data ) );
            memset( data, 0, sizeof( *data ) );
            data->codeptr_ra              = codeptrRa;
            data->target_id               = scorep_ompt_get_unique_id();
            data->supports_device_tracing = false;
            set_target_data( &targetData->value, data );

            SCOREP_EnterRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_HOST ) );
            SCOREP_TriggerParameterString( parameters.target_type, target_kind2string( kind ) );
            break;
        }
        case ompt_scope_end:
        {
            scorep_ompt_target_data_t* data = get_target_data( targetData->value );
            SCOREP_ExitRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_HOST ) );
            free_target_data( &targetData->value );
            break;
        }
        default:
            break;
    }
}


static inline void
target_emi_device_tracing( ompt_target_t         kind,
                           ompt_scope_endpoint_t endpoint,
                           int                   deviceNum,
                           ompt_data_t*          taskData,
                           ompt_data_t*          targetTaskData,
                           ompt_data_t*          targetData,
                           const void*           codeptrRa )
{
    switch ( endpoint )
    {
        case ompt_scope_begin:
        {
            scorep_ompt_target_data_t* data = SCOREP_Memory_AlignedMalloc( SCOREP_ALLOCATOR_ALIGNMENT, sizeof( *data ) );
            memset( data, 0, sizeof( *data ) );
            data->codeptr_ra              = codeptrRa;
            data->target_id               = scorep_ompt_get_unique_id();
            data->supports_device_tracing = true;
            set_target_data( &targetData->value, data );

            SCOREP_EnterRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_HOST ) );
            SCOREP_TriggerParameterString( parameters.target_type, target_kind2string( kind ) );
            break;
        }
        case ompt_scope_end:
        {
            scorep_ompt_target_data_t* data = get_target_data( targetData->value );
            SCOREP_ExitRegion( get_region( data->codeptr_ra, TOOL_EVENT_TARGET_HOST ) );
            break;
        }
        default:
            break;
    }
}


void
scorep_ompt_cb_target_emi( ompt_target_t         kind,
                           ompt_scope_endpoint_t endpoint,
                           int                   device_num,
                           ompt_data_t*          task_data,
                           ompt_data_t*          target_task_data,
                           ompt_data_t*          target_data,
                           const void*           codeptr_ra )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "atid = %d | kind = %s | endpoint = %s | "
                       "device_num = %d | task_data = %p | target_task_data = %p | "
                       "target_data = %p | target_data->value = %lx | codeptr_ra = %p",
                       adapter_tid,
                       target_kind2string( kind ),
                       scope_endpoint2string( endpoint ),
                       device_num,
                       task_data,
                       target_task_data,
                       target_data,
                       target_data ? target_data->value : 0,
                       codeptr_ra );
    SCOREP_OMPT_ENSURE_INITIALIZED();
    SCOREP_OMPT_RETURN_ON_INVALID_EVENT();
    if ( !record_target_region( kind ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }
    SCOREP_OMPT_ENSURE_TARGET_DATA_NON_NULL( "target_emi" );

    /* We only want to record events for devices where tracing is available. */
    scorep_ompt_target_device_t* device = get_device( device_num );
    if ( !device )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    if ( device->device_tracing_available )
    {
        target_emi_device_tracing(
            kind,
            endpoint,
            device_num,
            task_data,
            target_task_data,
            target_data,
            codeptr_ra );
    }
    else
    {
        target_emi_host_only(
            kind,
            endpoint,
            device_num,
            task_data,
            target_task_data,
            target_data,
            codeptr_ra );
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}
