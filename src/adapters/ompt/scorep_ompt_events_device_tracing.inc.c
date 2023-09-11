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

#include "scorep_ompt_device.h"
#include "scorep_ompt_callbacks_device.h"
#include "scorep_ompt_confvars.h"


/* Those functions are implemented in scorep_ompt_events_device_[x].inc.c, but are
 * required to be able to compile the code below */
/* *INDENT-OFF* */
static void device_tracing_handle_target_data_op_record( ompt_record_target_data_op_t, const scorep_ompt_target_data_t*, scorep_ompt_device_stream_t*, uint64_t );
static void device_tracing_handle_submit_record( ompt_record_target_kernel_t, const scorep_ompt_target_data_t*, scorep_ompt_device_stream_t*, uint64_t );
static void cb_buffer_request( int, ompt_buffer_t**, size_t* );
static void cb_buffer_complete( int, ompt_buffer_t*, size_t, ompt_buffer_cursor_t, int );
/* *INDENT-ON* */


static inline void
device_tracing_register_callback( scorep_ompt_target_device_t* device,
                                  ompt_callbacks_t             callback,
                                  const char*                  name )
{
    ompt_set_result_t result;
    result = device->device_functions.set_trace_ompt( device->address, true, callback );
    /* Do no allow ompt_set_sometimes or ompt_set_sometimes paired,
     * as not allowed per spec. */
    if ( result != ompt_set_always )
    {
        device->device_functions.set_trace_ompt( device->address, false, callback );
        UTILS_WARNING( "[OMPT] Failed to register callback %s for device %s. "
                       "Corresponding events will not be recorded for this accelerator.",
                       name,
                       device->name );
    }
}


static inline void
device_tracing_initialize( scorep_ompt_target_device_t* device,
                           ompt_function_lookup_t       lookup )
{
#define TRACING_INIT_WARN( part_failed )                                                \
    UTILS_WARNING( "[OMPT] Device Tracing interface could not be initialized "          \
                   "and will be disabled for device %s. This will lead to accelerator " \
                   "events not being recorded by the OMPT adapter. %s.", device->name, ( part_failed ) )

#define LOOKUP_FUNCTION( name, lookup, storage )                      \
    {                                                                 \
        storage.name = ( ompt_##name##_t )( lookup( "ompt_"#name ) ); \
    }

    #define LOOKUP_FUNCTION_REQUIRED( name, lookup, storage )               \
    {                                                                       \
        LOOKUP_FUNCTION( name, lookup, storage );                           \
        if ( storage.name == NULL )                                         \
        {                                                                   \
            TRACING_INIT_WARN( "Function " #name " could not be enabled" ); \
            return;                                                         \
        }                                                                   \
    }

    if ( lookup == NULL )
    {
        TRACING_INIT_WARN( "Lookup function is NULL." );
        return;
    }

    LOOKUP_FUNCTION_REQUIRED( get_device_time, lookup, device->device_functions );
    LOOKUP_FUNCTION_REQUIRED( set_trace_ompt, lookup, device->device_functions );
    LOOKUP_FUNCTION_REQUIRED( start_trace, lookup, device->device_functions );
    LOOKUP_FUNCTION_REQUIRED( flush_trace, lookup, device->device_functions );
    LOOKUP_FUNCTION_REQUIRED( advance_buffer_cursor, lookup, device->device_functions );
    LOOKUP_FUNCTION_REQUIRED( get_record_ompt, lookup, device->device_functions );
    LOOKUP_FUNCTION( stop_trace, lookup, device->device_functions );
    LOOKUP_FUNCTION( pause_trace, lookup, device->device_functions );

    /* These are the events being present in ROCm 5.6.1 which give us information about the target regions.
     * We register ompt_callback_target_emi only to correctly free memory when we know that the respective
     * target region has finished on the device. ompt_callback_target_map is missing both because it isn't
     * implemented and because it doesn't provide useful information for Score-P specifically. */
    #define REGISTER( callback ) device_tracing_register_callback( device, callback, #callback )
    if ( scorep_ompt_target_features > 0 )
    {
        REGISTER( ompt_callback_target_emi );
    }
    if ( scorep_ompt_target_features & SCOREP_OMPT_TARGET_FEATURE_KERNEL )
    {
        REGISTER( ompt_callback_target_submit_emi );
    }
    if ( scorep_ompt_target_features & SCOREP_OMPT_TARGET_FEATURE_MEMORY )
    {
        REGISTER( ompt_callback_target_data_op_emi );
    }
    #undef REGISTER

    bool start_trace_result = device->device_functions.start_trace( device->address,
                                                                    cb_buffer_request,
                                                                    cb_buffer_complete );
    if ( !start_trace_result )
    {
        TRACING_INIT_WARN( "Start of device tracing interface failed after enabling events!" );
        return;
    }

    device->device_tracing_available = true;
    UTILS_DEBUG_EXIT( "Enabling device tracing for device %s", device->name );

    #undef LOOKUP_FUNCTION
    #undef LOOKUP_FUNCTION_REQUIRED
    #undef TRACING_INIT_WARN
}


static inline void
device_tracing_evaluate_buffer( scorep_ompt_target_device_t* bufferDevice,
                                ompt_buffer_t*               buffer,
                                size_t                       bytes,
                                ompt_buffer_cursor_t         begin )
{
    // TODO: Buffers might not be sorted by beginning time. We just experience this with AOMP right now.
    // TODO: We should sort the buffer by time before processing it.
    // TODO: We may also want to move data transfers to separate locations for easier post-processing.
    do
    {
        ompt_record_ompt_t*        record      = bufferDevice->device_functions.get_record_ompt( buffer, begin );
        scorep_ompt_target_data_t* target_data = ( scorep_ompt_target_data_t* )record->target_id;

        /* If we reach the end of a target region, we can free our allocated structure to transfer information */
        if ( ( record->type == ompt_callback_target_emi || record->type == ompt_callback_target ) &&
             record->record.target.endpoint == ompt_scope_end )
        {
            SCOREP_Memory_AlignedFree( target_data );
            continue;
        }
        /* Ignore events without a valid timestamp as they will cause issues. */
        if ( record->time == ompt_time_none )
        {
            continue;
        }

        record->time = translate_to_host_time( bufferDevice, record->time );
        scorep_ompt_device_stream_t* current_stream = get_or_add_device_stream( bufferDevice, record->time );
        UTILS_MutexLock( &current_stream->stream_lock );
        switch ( record->type )
        {
            case ompt_callback_target_data_op:
            case ompt_callback_target_data_op_emi:
            {
                /* Allocations and deletions should be recorded as device-level metrics and are therefore not included here.
                 * Associate and disassociate only offer information for non-performance tools and are ignored as well */
                ompt_target_data_op_t record_optype = record->record.target_data_op.optype;
                if ( record_optype != ompt_target_data_transfer_to_device &&
                     record_optype != ompt_target_data_transfer_to_device_async &&
                     record_optype != ompt_target_data_transfer_from_device &&
                     record_optype != ompt_target_data_transfer_from_device_async )
                {
                    break;
                }
                record->record.target_data_op.end_time = translate_to_host_time(
                    bufferDevice,
                    record->record.target_data_op.end_time );
                device_tracing_handle_target_data_op_record( record->record.target_data_op,
                                                             target_data,
                                                             current_stream,
                                                             record->time );
                if ( target_data->target_id == ompt_id_none )
                {
                    SCOREP_Memory_AlignedFree( target_data );
                }
            }
            break;
            case ompt_callback_target_submit:
            case ompt_callback_target_submit_emi:
                record->record.target_kernel.end_time = translate_to_host_time(
                    bufferDevice,
                    record->record.target_kernel.end_time );
                device_tracing_handle_submit_record( record->record.target_kernel,
                                                     target_data,
                                                     current_stream,
                                                     record->time );
                break;
            default:
                break;
        }
        UTILS_MutexUnlock( &current_stream->stream_lock );
    }
    while ( bufferDevice->device_functions.advance_buffer_cursor(
                bufferDevice->address,
                buffer,
                bytes,
                begin,
                &begin ) );
}
