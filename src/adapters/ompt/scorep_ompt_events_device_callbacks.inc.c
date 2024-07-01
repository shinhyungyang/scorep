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

#include "scorep_ompt_callbacks_device.h"
#include "scorep_ompt_confvars.h"

#include <SCOREP_Subsystem.h>
#include <SCOREP_AcceleratorManagement.h>
#include <SCOREP_AllocMetric.h>

#include <scorep_system_tree.h>


void
scorep_ompt_cb_device_initialize( int                    device_num,
                                  const char*            type,
                                  ompt_device_t*         device,
                                  ompt_function_lookup_t lookup,
                                  const char*            documentation )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "atid %" PRIu32 " | device_num %d | type %s | device %p | "
                       "lookup %p | documentation %s",
                       adapter_tid,
                       device_num,
                       type,
                       device,
                       lookup,
                       documentation );
    SCOREP_OMPT_ENSURE_INITIALIZED();
    SCOREP_OMPT_RETURN_ON_INVALID_EVENT();

    if ( device == NULL )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    /* Allocate a new device and insert it into our hash table. If there's already a device with the same device
     * number, we abort, since we cannot handle this scenario */
    scorep_ompt_target_device_t* new_device;
    device_table_ctor_data_t     ctor_data = {
        .device     = device,
        .device_num = device_num,
        .type       = type,
        .lookup     = lookup
    };
    bool                         inserted = scorep_ompt_device_table_get_and_insert(
        device_num,
        &ctor_data,
        &new_device );
    UTILS_BUG_ON( !inserted,
                  "[OMPT] Device %d already exists in the device table",
                  device_num );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}


void
scorep_ompt_cb_device_finalize( int device_num )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "device_num %d", device_num );

    if ( scorep_ompt_record_event() == false )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    scorep_ompt_target_device_t* device = get_device( device_num );
    if ( device == NULL )
    {
        UTILS_WARNING( "[OMPT] Could not find device %d to finalize",
                       device_num );
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }
    if ( device->device_tracing_buffers > 0 )
    {
        UTILS_WARNING( "[OMPT] Device %s was finalized but not all buffers were freed. "
                       "Some events may not be processed!",
                       device->name );
    }
    device->finalized = true;

    SCOREP_IN_MEASUREMENT_DECREMENT();
}


static void
cb_buffer_request( int             device_num,
                   ompt_buffer_t** buffer,
                   size_t*         bytes )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "Allocating buffer for device %d with size %" PRIu64,
                       device_num,
                       scorep_ompt_target_buffer_chunk_size );

    *bytes = scorep_ompt_target_buffer_chunk_size;
    /* Use SCOREP_Memory_AlignedMalloc as this callback can be dispatched
     * from runtime helper threads, which might not have a location object */
    *buffer = SCOREP_Memory_AlignedMalloc( SCOREP_CACHELINESIZE, *bytes );

    scorep_ompt_target_device_t* device = get_device( device_num );
    UTILS_BUG_ON( device == NULL );
    device->device_tracing_buffers++;

    SCOREP_IN_MEASUREMENT_DECREMENT();
}


static void
cb_buffer_complete( int                  device_num,
                    ompt_buffer_t*       buffer,
                    size_t               bytes,
                    ompt_buffer_cursor_t begin,
                    int                  buffer_owned )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "device_num = %d | buffer = %p | bytes = %d | begin = %" PRIu64 " | buffer_owned = %d",
                       device_num,
                       buffer,
                       bytes,
                       begin,
                       buffer_owned );
    scorep_ompt_target_device_t* buffer_device = get_device( device_num );

    if ( bytes > 0 )
    {
        uint64_t host_time_ref_end, device_time_ref_end;
        get_host_and_device_time( &host_time_ref_end, buffer_device, &device_time_ref_end );
        buffer_device->timestamp_sync_factor = ( double )( host_time_ref_end - buffer_device->timestamp_host_ref_begin ) /
                                               ( double )( device_time_ref_end - buffer_device->timestamp_device_ref_begin );
        device_tracing_evaluate_buffer( buffer_device, buffer, bytes, begin );
    }
    else
    {
        /* This case may be encountered with ROCm compilers. The callback may get called twice.
        * The first time, the buffer is evaluated normally and has a value > 0 for bytes.
        * The second time however, the bytes field is 0 but buffer_owned is set to true
        * and the same buffer is passed. In that case, the runtime wants to inform that the
        * owned buffer can be freed. Just in case though, warn the user about this behavior. */
        UTILS_WARN_ONCE( "[OMPT] Zero byte buffer received on device %s.", buffer_device->name );
    }

    if ( buffer_owned )
    {
        SCOREP_Memory_AlignedFree( buffer );
        buffer_device->device_tracing_buffers--;
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
}
