/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2015,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2015,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2011                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2010-2011                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/


/**
 * @file
 * @ingroup    MPI_Wrapper
 *
 * @brief C interface wrappers for environmental management.
 */

#include <config.h>
/* We do wrap deprecated functions here, but we don't want warnings */
#define OMPI_WANT_MPI_INTERFACE_WARNING 0
#include "SCOREP_Mpi.h"
#include "scorep_mpi_communicator.h"
#include <UTILS_Error.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>


/** Flag set if the measurement sysem was already opened by another adapter.
    If the measurement system is not already initilized, it is assumed that
    the mpi adapter is the only active adapter. In this case, at first an
    additional region is entered MPI_Init. Thus, all regions appear as
    children of this region.
 */
static int scorep_mpi_parallel_entered = 0;

/**
 * @name C wrappers
 * @{
 */

/**
 * Measurement wrapper for MPI_Init.
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup env
 * If the measurement system is not initialized, it will iniialize the measurement
 * system and enter a special region named "parallel" which is exited when MPI is
 * finalized.
 * The sequence of events generated by this wrapper is:
 * @li enter region 'PARALLEL': Only if this adapter initializes the measurement system.
 * @li enter region 'MPI_Init'
 * @li define communicator 'COMM_WORLD'
 * @li exit region 'MPI_Init'
 */
int
MPI_Init( int* argc, char*** argv )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    int event_gen_active = 0; /* init is deferred to later */
    int return_val;
    int fflag;
    int iflag;

    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* Initialize the measurement system */
        SCOREP_InitMeasurement();

        if ( !SCOREP_IsUnwindingEnabled() )
        {
            /* Enter global MPI region */
            SCOREP_EnterRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__PARALLEL ] );

            /* Remember that SCOREP_MPI_REGION__PARALLEL was entered */
            scorep_mpi_parallel_entered = 1;
        }
    }

    event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        /* Enter the init region */
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT ],
                                   ( intptr_t )PMPI_Init );
    }

    if ( event_gen_active )
    {
        SCOREP_ENTER_WRAPPED_REGION();
    }
    return_val = PMPI_Init( argc, argv );
    if ( event_gen_active )
    {
        SCOREP_EXIT_WRAPPED_REGION();
    }

    /* XXXX should only continue if MPI initialised OK! */

    if ( ( PMPI_Initialized( &iflag ) == MPI_SUCCESS ) && ( iflag != 0 ) &&
         ( PMPI_Finalized( &fflag ) == MPI_SUCCESS ) && ( fflag == 0 ) )
    {
        /* complete initialization of measurement core and MPI event handling */
        SCOREP_InitMppMeasurement();
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}

#if HAVE( DECL_PMPI_INIT_THREAD )
/**
 * Measurement wrapper for MPI_Init_thread, the thread-capable
 * alternative to MPI_Init.
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup env
 * If the measurement system is not initialized, it will iniialize the measurement
 * system and enter a special region named "parallel" which is exited when MPI is
 * finalized.
 * The sequence of events generated by this wrapper is:
 * @li enter region 'PARALLEL': Only if this adapter initializes the measurement system.
 * @li enter region 'MPI_Init_thread'
 * @li define communicator 'COMM_WORLD'
 * @li exit region 'MPI_Init_thread'
 */
int
MPI_Init_thread( int* argc, char*** argv, int required, int* provided )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    int event_gen_active = 0;
    int return_val;
    int fflag;
    int iflag;

    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* Initialize the measurement system */
        SCOREP_InitMeasurement();

        if ( !SCOREP_IsUnwindingEnabled() )
        {
            /* Enter global MPI region */
            SCOREP_EnterRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__PARALLEL ] );

            /* Remember that SCOREP_MPI_REGION__PARALLEL was entered */
            scorep_mpi_parallel_entered = 1;
        }
    }

    event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT_THREAD ],
                                   ( intptr_t )PMPI_Init_thread );
    }

    if ( event_gen_active )
    {
        SCOREP_ENTER_WRAPPED_REGION();
    }
    return_val = PMPI_Init_thread( argc, argv, required, provided );
    if ( event_gen_active )
    {
        SCOREP_EXIT_WRAPPED_REGION();
    }

    /* XXXX should only continue if MPI initialised OK! */

    if ( ( return_val == MPI_SUCCESS ) && ( required > MPI_THREAD_FUNNELED ) && ( *provided > MPI_THREAD_FUNNELED ) )
    {
        UTILS_WARNING( "MPI environment initialization request and provided level exceed MPI_THREAD_FUNNELED!" );
        /* XXXX continue even though not supported by analysis. We assume
         * that if the user requests <= MPI_THREAD_FUNNELED, the user code
         * will behave accordingly. */
    }

    if ( ( PMPI_Initialized( &iflag ) == MPI_SUCCESS ) && ( iflag != 0 ) &&
         ( PMPI_Finalized( &fflag ) == MPI_SUCCESS ) && ( fflag == 0 ) )
    {
        /* complete initialization of measurement core and MPI event handling */
        SCOREP_InitMppMeasurement();
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT_THREAD ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

/**
 * Measurement wrapper for MPI_Finalize
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup env
 * Generates an enter event at function start and an exit event at function end.
 * If the "parallel" region was entered in MPI_Init, it exits the "parallel" region.
 * It does not perform the MPI finalization, because MPI is still needed by the
 * measurement system, but substituts it with a barrier. The MPI finalization
 * will be done from the measurement system.
 */
int
MPI_Finalize( void )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        /* @todo hande <> adress mismatch, check, if we still get samples inside the barrier */
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZE ],
                                   ( intptr_t )PMPI_Barrier );
    }

    /* Be so kind and name the MPI_COMM_WORLD, if the user didn't do so already */
    scorep_mpi_comm_set_name( MPI_COMM_WORLD, "MPI_COMM_WORLD" );

    /* finalize MPI event handling */
    /* We need to make sure that our exit handler is called before the MPI one. */
    SCOREP_RegisterExitHandler();
  #if !defined( SCOREP_MPI_NO_HOOKS )
    scorep_mpiprofile_finalize();
  #endif

    /* fake finalization, so that MPI can be used during SCOREP finalization */
    if ( event_gen_active )
    {
        SCOREP_ENTER_WRAPPED_REGION();
    }
    return_val = PMPI_Barrier( MPI_COMM_WORLD );
    if ( event_gen_active )
    {
        SCOREP_EXIT_WRAPPED_REGION();
    }

    if ( event_gen_active )
    {
        /* Exit MPI_Finalize region */
        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZE ] );

        /* Exit the extra parallel region in case it was entered in MPI_Init */
        if ( scorep_mpi_parallel_entered )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__PARALLEL ] );
        }

        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}

#if HAVE( DECL_PMPI_GET_LIBRARY_VERSION ) && !defined( SCOREP_MPI_NO_ENV ) && !defined( MPI_Get_library_version )
/**
 * Measurement wrapper for MPI_Get_library_version
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-3
 * @ingroup env
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_library_version call with enter and exit events.
 */
int
MPI_Get_library_version( char* version, int* resultlen )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_LIBRARY_VERSION ],
                                   ( intptr_t )PMPI_Get_library_version );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Get_library_version( version, resultlen );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_LIBRARY_VERSION ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Get_library_version( version, resultlen );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_IS_THREAD_MAIN ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ENV ) && !defined( MPI_Is_thread_main )
/**
 * Measurement wrapper for MPI_Is_thread_main
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup env
 * Triggers an enter and exit event.
 * It wraps the MPI_Is_thread_main call with enter and exit events.
 */
int
MPI_Is_thread_main( int* flag )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_IS_THREAD_MAIN ],
                                   ( intptr_t )PMPI_Is_thread_main );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Is_thread_main( flag );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_IS_THREAD_MAIN ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Is_thread_main( flag );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_QUERY_THREAD ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ENV ) && !defined( MPI_Query_thread )
/**
 * Measurement wrapper for MPI_Query_thread
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup env
 * Triggers an enter and exit event.
 * It wraps the MPI_Query_thread call with enter and exit events.
 */
int
MPI_Query_thread( int* provided )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_QUERY_THREAD ],
                                   ( intptr_t )PMPI_Query_thread );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Query_thread( provided );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_QUERY_THREAD ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Query_thread( provided );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif


#if HAVE( DECL_PMPI_FINALIZED ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ENV ) && !defined( MPI_Finalized )
/**
 * Measurement wrapper for MPI_Finalized
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup env
 * Triggers an enter and exit event but only when within the measurement phase.
 * It wraps the MPI_Finalized call with enter and exit events.
 */
int
MPI_Finalized( int* flag )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV ) &&
         SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZED ],
                                   ( intptr_t )PMPI_Finalized );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Finalized( flag );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZED ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Finalized( flag );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_INITIALIZED ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ENV ) && !defined( MPI_Initialized )
/**
 * Measurement wrapper for MPI_Initialized
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup env
 * Triggers an enter and exit event but only when within the measurement phase.
 * It wraps the MPI_Initialized call with enter and exit events.
 */
int
MPI_Initialized( int* flag )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV ) &&
         SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INITIALIZED ],
                                   ( intptr_t )PMPI_Initialized );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Initialized( flag );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INITIALIZED ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Initialized( flag );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif


/**
 * @}
 */
