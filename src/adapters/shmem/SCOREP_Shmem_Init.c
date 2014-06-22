/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014,
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
 * @brief C interface wrappers for initialization routines
 */

#include <config.h>

#include <UTILS_Error.h>
#define SCOREP_DEBUG_MODULE_NAME SHMEM
#include <UTILS_Debug.h>

#include "scorep_shmem_internal.h"
#include <SCOREP_Events.h>
#include <SCOREP_RuntimeManagement.h>


static void
init( void )
{
    /* Determine SHMEM rank and number of PEs */
    scorep_shmem_rank_and_size();

    SCOREP_InitMppMeasurement();

    /* Define communicator and RMA window for the group of all PEs */
    scorep_shmem_setup_comm_world();
}


/* *INDENT-OFF* */

#define INIT_SHMEM( FUNCNAME )                                              \
    void                                                                    \
    SCOREP_LIBWRAP_FUNC_NAME( FUNCNAME ) ( void )                           \
    {                                                                       \
        if ( !SCOREP_IsInitialized() )                                      \
        {                                                                   \
            /* Initialize the measurement system */                         \
            SCOREP_InitMeasurement();                                       \
                                                                            \
            /* Enter global SHMEM region */                                 \
            SCOREP_EnterRegion( scorep_shmem_region__SHMEM );               \
                                                                            \
            /* Remember that SCOREP_PARALLEL__SHMEM was entered */          \
            scorep_shmem_parallel_entered = true;                           \
        }                                                                   \
        SCOREP_SHMEM_EVENT_GEN_OFF();                                       \
                                                                            \
        SCOREP_EnterRegion( scorep_shmem_region__ ## FUNCNAME );            \
                                                                            \
        SCOREP_LIBWRAP_FUNC_CALL( lw, FUNCNAME, ( ) );                      \
                                                                            \
        init();                                                             \
                                                                            \
        /* Enable SHMEM event generation */                                 \
        SCOREP_SHMEM_EVENT_GEN_ON();                                        \
                                                                            \
        SCOREP_ExitRegion( scorep_shmem_region__ ## FUNCNAME );             \
    }

/* *INDENT-ON* */

#if SHMEM_HAVE_DECL( SHMEM_INIT )
INIT_SHMEM( shmem_init )
#endif


/* *INDENT-OFF* */

#define INIT_SHMEM_WITH_ARGUMENT( FUNCNAME )                                \
    void                                                                    \
    SCOREP_LIBWRAP_FUNC_NAME( FUNCNAME ) ( int npes )                       \
    {                                                                       \
        if ( !SCOREP_IsInitialized() )                                      \
        {                                                                   \
            /* Initialize the measurement system */                         \
            SCOREP_InitMeasurement();                                       \
                                                                            \
            /* Enter global SHMEM region */                                 \
            SCOREP_EnterRegion( scorep_shmem_region__SHMEM );               \
                                                                            \
            /* Remember that SCOREP_PARALLEL__SHMEM was entered */          \
            scorep_shmem_parallel_entered = true;                           \
        }                                                                   \
        SCOREP_SHMEM_EVENT_GEN_OFF();                                       \
                                                                            \
        SCOREP_EnterRegion( scorep_shmem_region__ ## FUNCNAME );            \
                                                                            \
        SCOREP_LIBWRAP_FUNC_CALL( lw, FUNCNAME, ( npes ) );                 \
                                                                            \
        init();                                                             \
                                                                            \
        /* Enable SHMEM event generation */                                 \
        SCOREP_SHMEM_EVENT_GEN_ON();                                        \
                                                                            \
        SCOREP_ExitRegion( scorep_shmem_region__ ## FUNCNAME );             \
    }

/* *INDENT-ON* */

#if SHMEM_HAVE_DECL( START_PES )
INIT_SHMEM_WITH_ARGUMENT( start_pes )
#endif


/* *INDENT-OFF* */

#define INIT_THREAD_SHMEM( FUNCNAME )                                       \
    void                                                                    \
    SCOREP_LIBWRAP_FUNC_NAME( FUNCNAME ) ( int   required,                  \
                                           int * provided )                 \
    {                                                                       \
        if ( !SCOREP_IsInitialized() )                                      \
        {                                                                   \
            /* Initialize the measurement system */                         \
            SCOREP_InitMeasurement();                                       \
                                                                            \
            /* Enter global SHMEM region */                                 \
            SCOREP_EnterRegion( scorep_shmem_region__SHMEM );               \
                                                                            \
            /* Remember that SCOREP_PARALLEL__SHMEM was entered */          \
            scorep_shmem_parallel_entered = true;                           \
        }                                                                   \
        SCOREP_SHMEM_EVENT_GEN_OFF();                                       \
                                                                            \
        SCOREP_EnterRegion( scorep_shmem_region__ ## FUNCNAME );            \
                                                                            \
        SCOREP_LIBWRAP_FUNC_CALL( lw, FUNCNAME, ( required, provided ) );   \
                                                                            \
        init();                                                             \
                                                                            \
        /* Enable SHMEM event generation */                                 \
        SCOREP_SHMEM_EVENT_GEN_ON();                                        \
                                                                            \
        SCOREP_ExitRegion( scorep_shmem_region__ ## FUNCNAME );             \
    }

/* *INDENT-ON* */

#if SHMEM_HAVE_DECL( SHMEM_INIT_THREAD )
INIT_THREAD_SHMEM( shmem_init_thread )
#endif


#define FINALIZE_SHMEM( FUNCNAME, RETVAL )                                \
    SCOREP_LIBWRAP_FUNC_NAME( FUNCNAME ) ( void )                         \
    {                                                                     \
        UTILS_DEBUG_ENTRY();                                              \
                                                                          \
        const int event_gen_active = SCOREP_SHMEM_IS_EVENT_GEN_ON;        \
                                                                          \
        if ( event_gen_active )                                           \
        {                                                                 \
            SCOREP_SHMEM_EVENT_GEN_OFF();                                 \
            SCOREP_EnterRegion( scorep_shmem_region__ ## FUNCNAME );      \
        }                                                                 \
                                                                          \
        SCOREP_RegisterExitHandler();                                     \
                                                                          \
        if ( event_gen_active )                                           \
        {                                                                 \
            /* Exit shmem_finalize region */                              \
            SCOREP_ExitRegion( scorep_shmem_region__ ## FUNCNAME );       \
                                                                          \
            SCOREP_SHMEM_EVENT_GEN_ON();                                  \
        }                                                                 \
                                                                          \
        return RETVAL;                                                    \
    }

/*
 * Please note that 'shmem_finalize' is not part of the OpenSHMEM standard 1.0.
 * However, we need this function call to write events like RMA window destruction.
 * That's why we always wrap 'shmem_finalize'.
 * The Score-P measurement system always implements this function.
 * If the SHMEM implementation also provides 'shmem_finalize' we wrap it and
 * call the SHMEM library function later at finalization of the measurement system.
 */
#if HAVE( SHMEM_FINALIZE_COMPLIANT ) || !HAVE( DECL_SHMEM_FINALIZE )

void
FINALIZE_SHMEM( shmem_finalize, )

#elif HAVE( SHMEM_FINALIZE_OPENMPI_VARIANT )

/* The return value is equal to OSHMEM_SUCCESS. */
int
FINALIZE_SHMEM( shmem_finalize, 0 )

#endif
