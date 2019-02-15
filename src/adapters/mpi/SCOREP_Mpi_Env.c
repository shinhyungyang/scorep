/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2017,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2016, 2018,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * Copyright (c) 2016,
 * Technische Universitaet Darmstadt, Germany
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


#if !defined( SCOREP_MPI_NO_HOOKS )
#include "scorep_mpi_oa_profile.h"
#endif


/**
 * Stores whether the application has called MPI_Finalize to
 * return the proper value if it calls MPI_Finalized afterwards.
 */
static bool mpi_finalize_called = false;

/**
 * Issue an 'undefined reference' link error in libscorep_adapter_mpi_mgmt if
 * no symbols from SCOREP_Mpi_Env.o were linked due to missing corresponding
 * MPI symbols in the application.
 */
void
scorep_hint_No_MPI_startup_symbols_found_in_application( void )
{
}

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
 * If the measurement system is not initialized, it will initialize the measurement
 * system and enter a special region named "parallel" which is exited when MPI is
 * finalized.
 * The sequence of events generated by this wrapper is:
 * @li enter region 'MPI_Init'
 * @li define communicator 'COMM_WORLD'
 * @li exit region 'MPI_Init'
 */
int
MPI_Init( int* argc, char*** argv )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    int return_val;
    int fflag;
    int iflag;

    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* Initialize the measurement system */
        SCOREP_InitMeasurement();
    }

    /* init is deferred to later */
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        if ( event_gen_active_for_group )
        {
            /* Enter the init region */
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Init( argc, argv );
    SCOREP_EXIT_WRAPPED_REGION();

    /* XXXX should only continue if MPI initialized OK! */

    if ( ( PMPI_Initialized( &iflag ) == MPI_SUCCESS ) && ( iflag != 0 ) &&
         ( PMPI_Finalized( &fflag ) == MPI_SUCCESS ) && ( fflag == 0 ) )
    {
        /* complete initialization of measurement core and MPI event handling */
        SCOREP_InitMppMeasurement();
    }

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT ] );
        }
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
 * If the measurement system is not initialized, it will initialize the measurement
 * system and enter a special region named "parallel" which is exited when MPI is
 * finalized.
 * The sequence of events generated by this wrapper is:
 * @li enter region 'MPI_Init_thread'
 * @li define communicator 'COMM_WORLD'
 * @li exit region 'MPI_Init_thread'
 */
int
MPI_Init_thread( int* argc, char*** argv, int required, int* provided )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    int return_val;
    int fflag;
    int iflag;

    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* Initialize the measurement system */
        SCOREP_InitMeasurement();
    }

    /* init is deferred to later */
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT_THREAD ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT_THREAD ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Init_thread( argc, argv, required, provided );
    SCOREP_EXIT_WRAPPED_REGION();

    /* XXXX should only continue if MPI initialized OK! */

    if ( return_val == MPI_SUCCESS )
    {
        if ( ( required > MPI_THREAD_FUNNELED ) && ( *provided > MPI_THREAD_FUNNELED ) )
        {
            UTILS_WARNING( "MPI environment initialization request and provided level exceed MPI_THREAD_FUNNELED!" );
        }
        if ( ( required == MPI_THREAD_FUNNELED ) && ( *provided >= MPI_THREAD_FUNNELED )
             && 0 != SCOREP_Location_GetId( SCOREP_Location_GetCurrentCPULocation() ) )
        {
            UTILS_WARNING( "MPI environment initialization with MPI_THREAD_FUNNELED not on master thread!" );
        }

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
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT_THREAD ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INIT_THREAD ] );
        }
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
 * measurement system, but substitutes it with a barrier. The MPI finalization
 * will be done from the measurement system.
 */
int
MPI_Finalize( void )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZE ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZE ] );
        }
    }

    /* Be so kind and name the MPI_COMM_WORLD, if the user didn't do so already */
    scorep_mpi_comm_set_default_names();

    /* finalize MPI event handling */
    /* We need to make sure that our exit handler is called before the MPI one. */
    SCOREP_RegisterExitHandler();
  #if !defined( SCOREP_MPI_NO_HOOKS )
    scorep_mpiprofile_finalize();
  #endif

    /* fake finalization, so that MPI can be used during Score-P finalization */
    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Barrier( MPI_COMM_WORLD );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( MPI_SUCCESS == return_val )
    {
        mpi_finalize_called = true;
    }

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            /* Exit MPI_Finalize region */
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZE ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZE ] );
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
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_LIBRARY_VERSION ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_LIBRARY_VERSION ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Get_library_version( version, resultlen );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_LIBRARY_VERSION ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_LIBRARY_VERSION ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
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
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_IS_THREAD_MAIN ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_IS_THREAD_MAIN ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Is_thread_main( flag );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_IS_THREAD_MAIN ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_IS_THREAD_MAIN ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
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
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_QUERY_THREAD ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_QUERY_THREAD ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Query_thread( provided );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_QUERY_THREAD ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_QUERY_THREAD ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
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
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON && SCOREP_IS_MEASUREMENT_PHASE( WITHIN );
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZED ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZED ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Finalized( flag );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( MPI_SUCCESS == return_val && mpi_finalize_called )
    {
        *flag = 1;
    }

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZED ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FINALIZED ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
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
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON && SCOREP_IS_MEASUREMENT_PHASE( WITHIN );
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_ENV );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INITIALIZED ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INITIALIZED ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Initialized( flag );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INITIALIZED ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INITIALIZED ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif


/**
 * @}
 */
