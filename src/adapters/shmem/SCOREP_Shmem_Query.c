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
 * @brief C interface wrappers for query routines
 */

#include <config.h>

#include "scorep_shmem_internal.h"
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>


/* *INDENT-OFF* */

#define QUERY_PE( FUNCNAME ) \
    int \
    FUNCNAME( void ) \
    { \
        SCOREP_IN_MEASUREMENT_INCREMENT(); \
        int ret; \
 \
        const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON; \
 \
        if ( event_gen_active ) \
        { \
            SCOREP_SHMEM_EVENT_GEN_OFF(); \
 \
            SCOREP_EnterWrappedRegion( scorep_shmem_region__ ## FUNCNAME ); \
        } \
 \
        SCOREP_ENTER_WRAPPED_REGION(); \
        ret = p ## FUNCNAME(); \
        SCOREP_EXIT_WRAPPED_REGION(); \
 \
        if ( event_gen_active ) \
        { \
            SCOREP_ExitRegion( scorep_shmem_region__ ## FUNCNAME ); \
 \
            SCOREP_SHMEM_EVENT_GEN_ON(); \
        } \
 \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        return ret; \
    }

/* *INDENT-ON* */

#if SHMEM_HAVE_DECL( MY_PE )
QUERY_PE( my_pe )
#endif

#if SHMEM_HAVE_DECL( _MY_PE )
QUERY_PE( _my_pe )
#endif

#if SHMEM_HAVE_DECL( SHMEM_MY_PE )
QUERY_PE( shmem_my_pe )
#endif

#if SHMEM_HAVE_DECL( NUM_PES )
QUERY_PE( num_pes )
#endif

#if SHMEM_HAVE_DECL( _NUM_PES )
QUERY_PE( _num_pes )
#endif

#if SHMEM_HAVE_DECL( SHMEM_N_PES )
QUERY_PE( shmem_n_pes )
#endif


#if SHMEM_HAVE_DECL( SHMEM_PE_ACCESSIBLE )
int
shmem_pe_accessible( int pe )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    int ret;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmem_pe_accessible );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    ret = pshmem_pe_accessible( pe );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_shmem_region__shmem_pe_accessible );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif


#if SHMEM_HAVE_DECL( SHMEM_ADDR_ACCESSIBLE ) && defined( SCOREP_SHMEM_ADDR_ACCESSIBLE_PROTO_ARGS )
int
shmem_addr_accessible SCOREP_SHMEM_ADDR_ACCESSIBLE_PROTO_ARGS
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int ret;

    const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;

    if ( event_gen_active )
    {
        SCOREP_SHMEM_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_shmem_region__shmem_addr_accessible );
    }

    SCOREP_ENTER_WRAPPED_REGION();
    ret = pshmem_addr_accessible( addr, pe );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_shmem_region__shmem_addr_accessible );

        SCOREP_SHMEM_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif
