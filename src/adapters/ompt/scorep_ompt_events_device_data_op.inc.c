/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023-2025,
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

#include <SCOREP_AllocMetric.h>


/* General fuctions */

static inline int
get_source_device_num( ompt_target_data_op_t optype,
                       int                   srcDeviceNum,
                       int                   destDeviceNum )
{
    /* Depending on the action, the source device may be different.
     * Return the correct identifier for target_data_op callbacks and
     * trace records */
    switch ( optype )
    {
        case ompt_target_data_transfer_to_device:
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC )
        case ompt_target_data_transfer_to_device_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC ) */
        case ompt_target_data_alloc:
        #if HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC )
        case ompt_target_data_alloc_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC ) */
            return destDeviceNum;
        case ompt_target_data_transfer_from_device:
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC )
        case ompt_target_data_transfer_from_device_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC ) */
        case ompt_target_data_delete:
        #if HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC )
        case ompt_target_data_delete_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC ) */
            return srcDeviceNum;
        default:
            return -1;
    }
}

/* Device tracing functions */

static void
device_tracing_handle_target_data_op_record( const ompt_record_target_data_op_t callbackRecord,
                                             const scorep_ompt_target_data_t*   targetRegionInformation,
                                             scorep_ompt_device_stream_t*       stream,
                                             uint64_t                           startTime )
{
    UTILS_DEBUG_ENTRY(
        "host_op_id = %lx | optype = %s | src_device_num = %" PRId32 " | "
        "src_addr = %p | dest_device_num = %" PRId32 " | dest_addr = %p | "
        "bytes = %" PRIu64 " | codeptr_ra = %p | startTime = %" PRIu64 " | "
        "end_time = %" PRIu64,
        callbackRecord.host_op_id,
        target_data_optype2string( callbackRecord.optype ),
        callbackRecord.src_device_num,
        callbackRecord.src_addr,
        callbackRecord.dest_device_num,
        callbackRecord.dest_addr,
        callbackRecord.bytes,
        callbackRecord.codeptr_ra,
        startTime,
        callbackRecord.end_time );

    int device_num = get_source_device_num( callbackRecord.optype,
                                            callbackRecord.src_device_num,
                                            callbackRecord.dest_device_num );
    /* Get matching ID for RMA operation based on the host_op_id set during the initial callback */
    uint64_t rma_matching_id = mapped_id_get_host_op_id( callbackRecord.host_op_id );
    /* When looking at transfers from host to device, we should see at least one device which is not registered.
     * Since we use a hash table, we can just ask for the device number and check if the device exists. */
    scorep_ompt_target_device_t* src_device  = get_device( callbackRecord.src_device_num );
    scorep_ompt_target_device_t* dest_device = get_device( callbackRecord.dest_device_num );
    uint64_t                     remote_local_rank;

    if ( !src_device || !dest_device )
    {
        remote_local_rank = mapped_id_get_local_rank( callbackRecord.host_op_id );
    }
    else
    {
        /* Device to device */
        int other_device_num = device_num == callbackRecord.src_device_num ?
                               callbackRecord.dest_device_num : callbackRecord.src_device_num;
        scorep_ompt_device_stream_t* other_device_stream;
        if ( other_device_num == callbackRecord.src_device_num )
        {
            other_device_stream = stream;
        }
        else
        {
            other_device_stream = get_or_add_device_stream( get_device( other_device_num ), startTime );
        }
        UTILS_BUG_ON( other_device_stream == NULL );
        remote_local_rank = other_device_stream->local_rank;
    }

    switch ( callbackRecord.optype )
    {
        case ompt_target_data_transfer_to_device:
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC )
        case ompt_target_data_transfer_to_device_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC ) */
            SCOREP_Location_RmaGet(
                stream->scorep_location,
                startTime,
                scorep_ompt_rma_window_handle,
                remote_local_rank,
                callbackRecord.bytes,
                rma_matching_id );
            SCOREP_Location_RmaOpCompleteBlocking(
                stream->scorep_location,
                callbackRecord.end_time,
                scorep_ompt_rma_window_handle,
                rma_matching_id );
            /* Explicitly set the last timestamp on that location to prevent OTF2 errors being
             * reported when a region with a lower start/end timestamp is written to the location
             * after the data transfer is written */
            SCOREP_Location_SetLastTimestamp( stream->scorep_location, callbackRecord.end_time );
            break;
        case ompt_target_data_transfer_from_device:
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC )
        case ompt_target_data_transfer_from_device_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC ) */
            SCOREP_Location_RmaPut(
                stream->scorep_location,
                startTime,
                scorep_ompt_rma_window_handle,
                remote_local_rank,
                callbackRecord.bytes,
                rma_matching_id );
            SCOREP_Location_RmaOpCompleteBlocking(
                stream->scorep_location,
                callbackRecord.end_time,
                scorep_ompt_rma_window_handle,
                rma_matching_id );
            /* See above */
            SCOREP_Location_SetLastTimestamp( stream->scorep_location, callbackRecord.end_time );
            break;
        default:
            break;
    }
}

/* Callback-related functions */

static inline SCOREP_RegionHandle
get_target_data_op_region( const void*           codeptrRa,
                           ompt_target_data_op_t optype )
{
    switch ( optype )
    {
        #if HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC )
        case ompt_target_data_alloc_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC ) */
        case ompt_target_data_alloc:
            return get_region( codeptrRa, TOOL_EVENT_TARGET_ALLOC );
        #if HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC )
        case ompt_target_data_delete_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC ) */
        case ompt_target_data_delete:
            return get_region( codeptrRa, TOOL_EVENT_TARGET_FREE );
        case ompt_target_data_transfer_from_device:
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC )
        case ompt_target_data_transfer_from_device_async:
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_FROM_DEVICE_ASYNC ) */
        case ompt_target_data_transfer_to_device:
        #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC )
        case ompt_target_data_transfer_to_device_async:
        #endif
            return get_region( codeptrRa, TOOL_EVENT_TARGET_DATA );
            /* Ignore the next two events, since they do not offer us anything for measurements */
        #if HAVE( DECL_OMPT_TARGET_DATA_ASSOCIATE )
        case ompt_target_data_associate:
            return SCOREP_INVALID_REGION;
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_ASSOCIATE ) */
        #if HAVE( DECL_OMPT_TARGET_DATA_DISASSOCIATE )
        case ompt_target_data_disassociate:
            return SCOREP_INVALID_REGION;
        #endif /* HAVE( DECL_OMPT_TARGET_DATA_DISASSOCIATE ) */
        default:
            UTILS_WARNING( "[OMPT] Unknown data transfer optype %d", optype );
            return SCOREP_INVALID_REGION;
    }
}


static inline void
handle_alloc( scorep_ompt_target_device_t* device,
              const void*                  ptr,
              size_t                       size )
{
    UTILS_DEBUG_ENTRY( "device = %p | ptr = %p | size = %zu", device, ptr, size );
    if ( ptr == NULL )
    {
        return;
    }
    /* In the case that we already have an allocation, we only want to realloc
     * that memory. This prevents an error message to appear during program
     * execution */
    if ( SCOREP_AllocMetric_AddrExists( device->alloc_metric, ( uint64_t )ptr ) )
    {
        void* allocation = NULL;
        SCOREP_AllocMetric_AcquireAlloc( device->alloc_metric, ( uint64_t )ptr, &allocation );
        UTILS_BUG_ON( allocation == NULL );
        SCOREP_AllocMetric_HandleRealloc(
            device->alloc_metric,
            ( uint64_t )ptr,
            size,
            allocation,
            NULL );
    }
    else
    {
        SCOREP_AllocMetric_HandleAlloc( device->alloc_metric, ( uint64_t )ptr, size );
    }
}


static inline void
handle_free( scorep_ompt_target_device_t* device,
             const void*                  ptr )
{
    UTILS_DEBUG_ENTRY( "device = %p | ptr = %p", device, ptr );
    if ( ptr == NULL )
    {
        return;
    }
    void* allocation = NULL;
    SCOREP_AllocMetric_AcquireAlloc( device->alloc_metric, ( uint64_t )ptr, &allocation );
    if ( allocation )
    {
        SCOREP_AllocMetric_HandleFree( device->alloc_metric, allocation, NULL );
        return;
    }

    UTILS_WARNING( "[OMPT] Tried to free memory of device %s, "
                   "which has not been allocated!",
                   device->name );
}


static inline void
target_data_op_emi_host_only( ompt_scope_endpoint_t endpoint,
                              ompt_data_t*          targetTaskData,
                              ompt_data_t*          targetData,
                              ompt_id_t*            hostOpId,
                              ompt_target_data_op_t optype,
                              void*                 srcAddr,
                              int                   srcDeviceNum,
                              void*                 destAddr,
                              int                   destDeviceNum,
                              size_t                bytes,
                              const void*           codeptrRa )
{
    /* If the data transfer is contained within a target region, we are able to
     * receive some information from the passed target_data->ptr to record a
     * callsite id. */
    scorep_ompt_target_data_t* data = get_target_data( targetData->value );
    /* If the target_data is NULL, we are not in a target region. Allocate
    * the struct anyway to ensure that Enter and Exit work correctly.
    * If there's already a pointer without a target region, the target_data
    * was reused. In that case, overwrite the target_data->value field. */
    if ( endpoint == ompt_scope_begin && ( !data || data->target_id == ompt_id_none ) )
    {
        data = SCOREP_Memory_AlignedMalloc( SCOREP_ALLOCATOR_ALIGNMENT, sizeof( *data ) );
        memset( data, 0, sizeof( *data ) );
        data->codeptr_ra              = codeptrRa;
        data->target_id               = ompt_id_none;
        data->supports_device_tracing = false;
        set_target_data( &targetData->value, data );
    }
    UTILS_BUG_ON( !data, "[OMPT] Expected data to be non-NULL" );

    SCOREP_RegionHandle target_region = get_target_data_op_region( data->codeptr_ra, optype );
    if ( target_region == SCOREP_INVALID_REGION )
    {
        if ( data->target_id == ompt_id_none )
        {
            free_target_data( &targetData->value );
        }
        return;
    }

    switch ( endpoint )
    {
        case ompt_scope_begin:
            SCOREP_EnterRegion( target_region );
            break;
        case ompt_scope_end:
            if ( data->target_id == ompt_id_none )
            {
                free_target_data( &targetData->value );
            }
            SCOREP_ExitRegion( target_region );
            break;
        default:
            break;
    }
}


static inline void
target_data_op_emi_device_tracing( ompt_scope_endpoint_t endpoint,
                                   ompt_data_t*          targetTaskData,
                                   ompt_data_t*          targetData,
                                   ompt_id_t*            hostOpId,
                                   ompt_target_data_op_t optype,
                                   void*                 srcAddr,
                                   int                   srcDeviceNum,
                                   void*                 destAddr,
                                   int                   destDeviceNum,
                                   size_t                bytes,
                                   const void*           codeptrRa )
{
    /* If the data transfer is contained within a target region, we are able to
     * receive some information from the passed target_data->ptr to record a
     * callsite id. */
    scorep_ompt_target_data_t* data = get_target_data( targetData->value );
    /* If the target_data is NULL, we are not in a target region. Allocate
    * the struct anyway to ensure that Enter and Exit work correctly.
    * If there's already a pointer without a target region, the target_data
    * was reused. In that case, overwrite the target_data->value field. */
    if ( endpoint == ompt_scope_begin && ( !data || data->target_id == ompt_id_none ) )
    {
        data = SCOREP_Memory_AlignedMalloc( SCOREP_ALLOCATOR_ALIGNMENT, sizeof( *data ) );
        memset( data, 0, sizeof( *data ) );
        data->codeptr_ra              = codeptrRa;
        data->target_id               = ompt_id_none;
        data->supports_device_tracing = true;
        set_target_data( &targetData->value, data );
    }
    UTILS_BUG_ON( !data, "[OMPT] Expected data to be non-NULL" );

    /* Get the region handle for the target data operation */
    SCOREP_RegionHandle target_region = get_target_data_op_region( data->codeptr_ra, optype );
    if ( target_region == SCOREP_INVALID_REGION )
    {
        if ( data->target_id == ompt_id_none )
        {
            free_target_data( &targetData->value );
        }
        return;
    }

    switch ( endpoint )
    {
        case ompt_scope_begin:
        {
            /* With the external monitoring interface, the runtime does
             * not set target_id and host_op_id. Instead, this is expected
             * to be done by the tool. Therefore, we can freely set
             * values for both here. */
            ompt_id_t operation_id = scorep_ompt_get_unique_id();
            /* Do not only transfer the unique operation id for RMA matching,
             * but also the local rank of the host thread to correctly reconstruct
             * the data transfer when we're actually able to write the transfer
             * during buffer evaluation. */
            *hostOpId = get_mapped_id( get_host_local_rank(), operation_id );
            /* Write host event */
            SCOREP_EnterRegion( target_region );
            data->num_records++;
        }
        break;
        case ompt_scope_end:
        {
            int                          device_num = get_source_device_num( optype, srcDeviceNum, destDeviceNum );
            scorep_ompt_target_device_t* device     = get_device( device_num );
            switch ( optype )
            {
                #if HAVE( DECL_OMPT_TARGET_DATA_TRANSFER_TO_DEVICE_ASYNC )
                case ompt_target_data_alloc_async:
                #endif /* HAVE( DECL_OMPT_TARGET_DATA_ALLOC_ASYNC ) */
                case ompt_target_data_alloc:
                    handle_alloc( device, destAddr, bytes );
                    break;
                #if HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC )
                case ompt_target_data_delete_async:
                #endif /* HAVE( DECL_OMPT_TARGET_DATA_DELETE_ASYNC ) */
                case ompt_target_data_delete:
                    handle_free( device, srcAddr );
                    break;
                default:
                    break;
            }
            SCOREP_ExitRegion( target_region );
            break;
        }
        default:
            break;
    }
}


void
scorep_ompt_cb_target_data_op_emi( ompt_scope_endpoint_t endpoint,
                                   ompt_data_t*          target_task_data,
                                   ompt_data_t*          target_data,
                                   ompt_id_t*            host_op_id,
                                   ompt_target_data_op_t optype,
                                   void*                 src_addr,
                                   int                   src_device_num,
                                   void*                 dest_addr,
                                   int                   dest_device_num,
                                   size_t                bytes,
                                   const void*           codeptr_ra )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG_ENTRY( "endpoint = %s | target_task_data = %p | target_data = %p | "
                       "host_op_id = %p (%lx) | optype = %s (%d) | src_addr = %p | "
                       "src_device_num = %d | dest_addr = %p | dest_device_num = %d | "
                       "bytes = %" PRIu64 " | codeptr_ra = %p",
                       scope_endpoint2string( endpoint ),
                       target_task_data,
                       target_data,
                       host_op_id,
                       host_op_id ? *host_op_id : 0,
                       target_data_optype2string( optype ),
                       optype,
                       src_addr,
                       src_device_num,
                       dest_addr,
                       dest_device_num,
                       bytes,
                       codeptr_ra );
    SCOREP_OMPT_ENSURE_INITIALIZED();
    SCOREP_OMPT_RETURN_ON_INVALID_EVENT();
    SCOREP_OMPT_ENSURE_TARGET_DATA_NON_NULL( "target_data_op_emi" );

    int device_num = get_source_device_num( optype,
                                            src_device_num,
                                            dest_device_num );
    /* Skip event if event is triggered for initial device, i.e. the host. */
    if ( device_num == scorep_ompt_initial_device_num )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }
    scorep_ompt_target_device_t* device = get_device( device_num );
    if ( !device )
    {
        UTILS_FATAL( "Target event was triggered, but device was not initialized yet. "
                     "This might be a runtime issue. Look at `scorep-info open-issues` "
                     "for more information." );
    }

    if ( !device->device_tracing_available )
    {
        target_data_op_emi_host_only( endpoint,
                                      target_task_data,
                                      target_data,
                                      host_op_id,
                                      optype,
                                      src_addr,
                                      src_device_num,
                                      dest_addr,
                                      dest_device_num,
                                      bytes,
                                      codeptr_ra );
    }
    else
    {
        target_data_op_emi_device_tracing( endpoint,
                                           target_task_data,
                                           target_data,
                                           host_op_id,
                                           optype,
                                           src_addr,
                                           src_device_num,
                                           dest_addr,
                                           dest_device_num,
                                           bytes,
                                           codeptr_ra );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
}
