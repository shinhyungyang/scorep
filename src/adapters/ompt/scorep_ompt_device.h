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

#ifndef SCOREP_OMPT_DEVICE_H
#define SCOREP_OMPT_DEVICE_H

#include <SCOREP_FastHashtab.h>
#include <SCOREP_AllocMetric.h>

#include <jenkins_hash.h>


typedef struct scorep_ompt_target_data_t
{
    const void* codeptr_ra;
    ompt_id_t   target_id;
} scorep_ompt_target_data_t;


typedef struct scorep_ompt_device_stream_t
{
    struct SCOREP_Location*             scorep_location;
    UTILS_Mutex                         stream_lock;
    uint32_t                            local_rank;

    struct scorep_ompt_device_stream_t* next;
} scorep_ompt_device_stream_t;


typedef struct scorep_ompt_device_functions_t
{
    /* Required functions */
    ompt_get_device_time_t       get_device_time;
    ompt_set_trace_ompt_t        set_trace_ompt;
    ompt_start_trace_t           start_trace;
    ompt_flush_trace_t           flush_trace;
    ompt_advance_buffer_cursor_t advance_buffer_cursor;
    ompt_get_record_ompt_t       get_record_ompt;
    /* Optional functions */
    ompt_stop_trace_t            stop_trace;
    ompt_pause_trace_t           pause_trace;
} scorep_ompt_device_functions_t;


typedef struct scorep_ompt_target_device_t
{
    ompt_device_t*                 address;
    int32_t                        device_num;
    char*                          name;

    SCOREP_SystemTreeNodeHandle    system_tree_node;
    SCOREP_LocationGroupHandle     location_group;
    SCOREP_AllocMetric*            alloc_metric;

    scorep_ompt_device_stream_t*   streams;
    uint32_t                       num_streams;
    UTILS_Mutex                    stream_lock;

    bool                           device_tracing_available;
    uint8_t                        device_tracing_buffers;
    scorep_ompt_device_functions_t device_functions;

    uint64_t                       timestamp_host_ref_begin;
    uint64_t                       timestamp_device_ref_begin;
    double                         timestamp_sync_factor;

    bool                           record_events;
} scorep_ompt_target_device_t;


/* Handling of devices in the OMPT target callbacks and device tracing interface:
 * Normally, we would be able to use ompt_get_num_devices to identify the device
 * and know the number of devices present during program execution. However, some
 * runtimes, specifically LLVM runtimes, do not correctly support this function
 * and instead return a value of 1. To circumvent the issue, we can use a hash table
 * where we use the device_num as the key and get the correct device as a result. */

typedef uint32_t                     scorep_ompt_device_table_key_t;
typedef scorep_ompt_target_device_t* scorep_ompt_device_table_value_t;


#define SCOREP_OMPT_DEVICE_TABLE_HASH_EXPONENT 5

uint32_t
scorep_ompt_device_table_bucket_idx( scorep_ompt_device_table_key_t key );

bool
scorep_ompt_device_table_equals( scorep_ompt_device_table_key_t key1,
                                 scorep_ompt_device_table_key_t key2 );

void*
scorep_ompt_device_table_allocate_chunk( size_t chunkSize );

void
scorep_ompt_device_table_free_chunk( void* chunk );

scorep_ompt_device_table_value_t
scorep_ompt_device_table_value_ctor( scorep_ompt_device_table_key_t* key,
                                     void*                           ctorData );


SCOREP_HASH_TABLE_MONOTONIC_HEADER( scorep_ompt_device_table, 4, hashsize( SCOREP_OMPT_DEVICE_TABLE_HASH_EXPONENT ) );

/**
 * Finalizes, flushes and stops the device tracing interface on an initialized
 * target device. If the device is NULL or device tracing is not available, the
 * function returns immediately.
 *
 * If the device has records not yet flushed, the runtime call with issue
 * zero or more buffer_complete callbacks, flushing all available data.
 *
 * @param device The target device to finalize.
 */
void
scorep_ompt_device_tracing_finalize( scorep_ompt_target_device_t* device );

#endif /* SCOREP_OMPT_DEVICE_H */
