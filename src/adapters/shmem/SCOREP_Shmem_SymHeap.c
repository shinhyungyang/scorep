/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file
 * @ingroup    SHMEM_Wrapper
 *
 * @brief C interface wrappers for symmetric heap routines
 */

#include <config.h>

#include "scorep_shmem_internal.h"
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>


#if SHMEM_HAVE_DECL( SHMALLOC )
void*
shmalloc( size_t size )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    void* ret;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        if ( scorep_shmem_memory_recording )
        {
            uint64_t size_as_uint64 = size;
            SCOREP_AddAttribute( scorep_shmem_memory_alloc_size_attribute,
                                 &size_as_uint64 );
        }
        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmalloc );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    ret = pshmalloc( size );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( scorep_shmem_memory_recording && size > 0 && ret )
        {
            SCOREP_AllocMetric_HandleAlloc(
                scorep_shmem_allocations_metric,
                ( uint64_t )ret, size );
        }

        SCOREP_ExitRegion( scorep_shmem_region__shmalloc );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif


#if SHMEM_HAVE_DECL( SHMEMALIGN )
void*
shmemalign( size_t alignment,
            size_t size )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    void* ret;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        if ( scorep_shmem_memory_recording )
        {
            uint64_t size_as_uint64 = size;
            SCOREP_AddAttribute( scorep_shmem_memory_alloc_size_attribute,
                                 &size_as_uint64 );
        }
        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmemalign );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    ret = pshmemalign( alignment, size );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( scorep_shmem_memory_recording && size > 0 && ret )
        {
            SCOREP_AllocMetric_HandleAlloc(
                scorep_shmem_allocations_metric,
                ( uint64_t )ret, size );
        }

        SCOREP_ExitRegion( scorep_shmem_region__shmemalign );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif


#if SHMEM_HAVE_DECL( SHREALLOC )
void*
shrealloc( void*  ptr,
           size_t size )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    void* result;
    void* allocation = NULL;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        if ( scorep_shmem_memory_recording )
        {
            uint64_t size_as_uint64 = size;
            SCOREP_AddAttribute( scorep_shmem_memory_alloc_size_attribute,
                                 &size_as_uint64 );
        }
        SCOREP_EnterWrappedRegion( scorep_shmem_region__shrealloc );

        if ( scorep_shmem_memory_recording && ptr )
        {
            SCOREP_AllocMetric_AcquireAlloc( scorep_shmem_allocations_metric,
                                             ( uint64_t )ptr, &allocation );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    result = pshrealloc( ptr, size );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( scorep_shmem_memory_recording )
        {
            /*
             * If ptr is a null pointer, then it is like malloc.
             */
            if ( ptr == NULL && result )
            {
                SCOREP_AllocMetric_HandleAlloc( scorep_shmem_allocations_metric,
                                                ( uint64_t )result,
                                                size );
            }
            /*
             * If size equals zero and ptr != NULL, then it is like free.
             */
            else if ( ptr != NULL && size == 0 )
            {
                uint64_t dealloc_size = 0;
                SCOREP_AllocMetric_HandleFree( scorep_shmem_allocations_metric,
                                               allocation, &dealloc_size );
                SCOREP_AddAttribute( scorep_shmem_memory_dealloc_size_attribute,
                                     &dealloc_size );
            }
            /* Otherwise it is a realloc, treat as realloc on success, ... */
            else if ( result )
            {
                uint64_t dealloc_size = 0;
                SCOREP_AllocMetric_HandleRealloc( scorep_shmem_allocations_metric,
                                                  ( uint64_t )result,
                                                  size,
                                                  allocation,
                                                  &dealloc_size );
                SCOREP_AddAttribute( scorep_shmem_memory_dealloc_size_attribute,
                                     &dealloc_size );
            }
            /* ... otherwise, realloc failed, ptr is not touched. *
             */
        }

        SCOREP_ExitRegion( scorep_shmem_region__shrealloc );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}
#endif


#if SHMEM_HAVE_DECL( SHFREE )
void
shfree( void* ptr )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    void* allocation = NULL;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shfree );

        if ( scorep_shmem_memory_recording && ptr )
        {
            SCOREP_AllocMetric_AcquireAlloc( scorep_shmem_allocations_metric,
                                             ( uint64_t )ptr, &allocation );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    pshfree( ptr );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( scorep_shmem_memory_recording )
        {
            uint64_t dealloc_size = 0;
            if ( ptr )
            {
                SCOREP_AllocMetric_HandleFree( scorep_shmem_allocations_metric,
                                               allocation, &dealloc_size );
            }
            SCOREP_AddAttribute( scorep_shmem_memory_dealloc_size_attribute,
                                 &dealloc_size );
        }

        SCOREP_ExitRegion( scorep_shmem_region__shfree );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}
#endif
