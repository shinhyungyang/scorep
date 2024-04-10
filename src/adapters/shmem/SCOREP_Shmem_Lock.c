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
 * @brief C interface wrappers for lock routines
 */

#include <config.h>

#include "scorep_shmem_internal.h"
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>


#if SHMEM_HAVE_DECL( SHMEM_SET_LOCK ) && defined( SCOREP_SHMEM_SET_LOCK_PROTO_ARGS )
void
shmem_set_lock SCOREP_SHMEM_SET_LOCK_PROTO_ARGS
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmem_set_lock );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    pshmem_set_lock( lock );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_RmaAcquireLock( scorep_shmem_world_window_handle,
                               NO_PROCESSING_ELEMENT,
                               ( uint64_t )lock,
                               SCOREP_LOCK_EXCLUSIVE );

        SCOREP_ExitRegion( scorep_shmem_region__shmem_set_lock );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}
#endif

#if SHMEM_HAVE_DECL( SHMEM_CLEAR_LOCK ) && defined( SCOREP_SHMEM_CLEAR_LOCK_PROTO_ARGS )
void
shmem_clear_lock SCOREP_SHMEM_CLEAR_LOCK_PROTO_ARGS
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmem_clear_lock );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    pshmem_clear_lock( lock );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_RmaReleaseLock( scorep_shmem_world_window_handle,
                               NO_PROCESSING_ELEMENT,
                               ( uint64_t )lock );

        SCOREP_ExitRegion( scorep_shmem_region__shmem_clear_lock );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
}
#endif

#if SHMEM_HAVE_DECL( SHMEM_TEST_LOCK ) && defined( SCOREP_SHMEM_TEST_LOCK_PROTO_ARGS )
int
shmem_test_lock SCOREP_SHMEM_TEST_LOCK_PROTO_ARGS
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int ret;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmem_test_lock );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    ret = pshmem_test_lock( lock );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( ret == 0 )
        {
            SCOREP_RmaRequestLock( scorep_shmem_world_window_handle,
                                   NO_PROCESSING_ELEMENT,
                                   ( uint64_t )lock,
                                   SCOREP_LOCK_EXCLUSIVE );
        }
        else
        {
            SCOREP_RmaTryLock( scorep_shmem_world_window_handle,
                               NO_PROCESSING_ELEMENT,
                               ( uint64_t )lock,
                               SCOREP_LOCK_EXCLUSIVE );
        }

        SCOREP_ExitRegion( scorep_shmem_region__shmem_test_lock );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif
