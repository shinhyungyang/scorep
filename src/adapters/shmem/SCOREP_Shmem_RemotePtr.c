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
 * @brief C interface wrappers for remote pointer routines
 */

#include <config.h>

#include "scorep_shmem_internal.h"
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>


#if SHMEM_HAVE_DECL( SHMEM_PTR ) && defined( SCOREP_SHMEM_PTR_PROTO_ARGS )
void*
shmem_ptr SCOREP_SHMEM_PTR_PROTO_ARGS
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    void* ret;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmem_ptr );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    ret = pshmem_ptr( ptr, pe );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_shmem_region__shmem_ptr );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif
