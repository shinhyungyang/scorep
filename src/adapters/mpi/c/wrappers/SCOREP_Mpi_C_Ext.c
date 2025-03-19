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
 * Copyright (c) 2009-2015, 2025,
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
 * a BSD-style license. See the COPYING file in the package base
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
 * @brief C interface wrappers for external interface functions
 */

#include <config.h>
#include <SCOREP_Mpi_Reg.h>
#include <scorep_mpi_groups.h>
#include <scorep_mpi_c.h>
#include <UTILS_Error.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>

/**
 * @name C wrappers
 * @{
 */

#if HAVE( MPI_1_0_SYMBOL_PMPI_ABORT )
/**
 * Measurement wrapper for MPI_Abort
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-1.0
 * @ingroup ext
 */
int
MPI_Abort( MPI_Comm comm,
           int      errorcode )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    UTILS_WARNING( "Explicit MPI_Abort call abandoning the SCOREP measurement." );
    SCOREP_SetAbortFlag();

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ABORT ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ABORT ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Abort( comm, errorcode );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ABORT ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ABORT ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif


#if HAVE( MPI_GREQUEST_COMPLETE_COMPLIANT )
#if HAVE( MPI_2_0_SYMBOL_PMPI_GREQUEST_COMPLETE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Grequest_complete )
/**
 * Declaration of PMPI-symbol for MPI_Grequest_complete
 */
int
PMPI_Grequest_complete( MPI_Request request );

/**
 * Measurement wrapper for MPI_Grequest_complete
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdCompl.w
 * @note C interface
 * @note Introduced with MPI-2.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Grequest_complete call with enter and exit events.
 */
int
MPI_Grequest_complete( MPI_Request request )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_COMPLETE ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_COMPLETE ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Grequest_complete( request );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_COMPLETE ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_COMPLETE ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#endif

#if HAVE( MPI_1_0_SYMBOL_PMPI_GET_COUNT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Get_count )
/**
 * Declaration of PMPI-symbol for MPI_Get_count
 */
int
PMPI_Get_count( SCOREP_MPI_CONST_DECL MPI_Status* status,
                MPI_Datatype                      datatype,
                int*                              count );

/**
 * Measurement wrapper for MPI_Get_count
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_count call with enter and exit events.
 */
int
MPI_Get_count( SCOREP_MPI_CONST_DECL MPI_Status* status, MPI_Datatype datatype, int* count )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_COUNT ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_COUNT ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Get_count( status, datatype, count );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_COUNT ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_COUNT ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_1_0_SYMBOL_PMPI_GET_ELEMENTS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Get_elements )
/**
 * Declaration of PMPI-symbol for MPI_Get_elements
 */
int
PMPI_Get_elements( SCOREP_MPI_CONST_DECL MPI_Status* status,
                   MPI_Datatype                      datatype,
                   int*                              count );

/**
 * Measurement wrapper for MPI_Get_elements
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_elements call with enter and exit events.
 */
int
MPI_Get_elements( SCOREP_MPI_CONST_DECL MPI_Status* status, MPI_Datatype datatype, int* count )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Get_elements( status, datatype, count );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_3_0_SYMBOL_PMPI_GET_ELEMENTS_X ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Get_elements_x )
/**
 * Declaration of PMPI-symbol for MPI_Get_elements_x
 */
int
PMPI_Get_elements_x( SCOREP_MPI_CONST_DECL MPI_Status* status,
                     MPI_Datatype                      datatype,
                     MPI_Count*                        count );

/**
 * Measurement wrapper for MPI_Get_elements_x
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-3.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_elements_x call with enter and exit events.
 */
int
MPI_Get_elements_x( SCOREP_MPI_CONST_DECL MPI_Status* status, MPI_Datatype datatype, MPI_Count* count )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS_X ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS_X ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Get_elements_x( status, datatype, count );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS_X ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ELEMENTS_X ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_1_0_SYMBOL_PMPI_GET_PROCESSOR_NAME ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Get_processor_name )
/**
 * Declaration of PMPI-symbol for MPI_Get_processor_name
 */
int
PMPI_Get_processor_name( char* name,
                         int*  resultlen );

/**
 * Measurement wrapper for MPI_Get_processor_name
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_processor_name call with enter and exit events.
 */
int
MPI_Get_processor_name( char* name, int* resultlen )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_PROCESSOR_NAME ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_PROCESSOR_NAME ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Get_processor_name( name, resultlen );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_PROCESSOR_NAME ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_PROCESSOR_NAME ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_2_0_SYMBOL_PMPI_GREQUEST_START ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Grequest_start )
/**
 * Declaration of PMPI-symbol for MPI_Grequest_start
 */
int
PMPI_Grequest_start( MPI_Grequest_query_function*  query_fn,
                     MPI_Grequest_free_function*   free_fn,
                     MPI_Grequest_cancel_function* cancel_fn,
                     void*                         extra_state,
                     MPI_Request*                  request );

/**
 * Measurement wrapper for MPI_Grequest_start
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Grequest_start call with enter and exit events.
 */
int
MPI_Grequest_start( MPI_Grequest_query_function* query_fn, MPI_Grequest_free_function* free_fn, MPI_Grequest_cancel_function* cancel_fn, void* extra_state, MPI_Request* request )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_START ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_START ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Grequest_start( query_fn, free_fn, cancel_fn, extra_state, request );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_START ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GREQUEST_START ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_2_0_SYMBOL_PMPI_STATUS_SET_CANCELLED ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Status_set_cancelled )
/**
 * Declaration of PMPI-symbol for MPI_Status_set_cancelled
 */
int
PMPI_Status_set_cancelled( MPI_Status* status,
                           int         flag );

/**
 * Measurement wrapper for MPI_Status_set_cancelled
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Status_set_cancelled call with enter and exit events.
 */
int
MPI_Status_set_cancelled( MPI_Status* status, int flag )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_CANCELLED ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_CANCELLED ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Status_set_cancelled( status, flag );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_CANCELLED ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_CANCELLED ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_2_0_SYMBOL_PMPI_STATUS_SET_ELEMENTS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Status_set_elements )
/**
 * Declaration of PMPI-symbol for MPI_Status_set_elements
 */
int
PMPI_Status_set_elements( MPI_Status*  status,
                          MPI_Datatype datatype,
                          int          count );

/**
 * Measurement wrapper for MPI_Status_set_elements
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Status_set_elements call with enter and exit events.
 */
int
MPI_Status_set_elements( MPI_Status* status, MPI_Datatype datatype, int count )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Status_set_elements( status, datatype, count );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS ] );
        }
        SCOREP_MPI_EVENT_GEN_ON();
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return return_val;
}
#endif
#if HAVE( MPI_3_0_SYMBOL_PMPI_STATUS_SET_ELEMENTS_X ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Status_set_elements_x )
/**
 * Declaration of PMPI-symbol for MPI_Status_set_elements_x
 */
int
PMPI_Status_set_elements_x( MPI_Status*  status,
                            MPI_Datatype datatype,
                            MPI_Count    count );

/**
 * Measurement wrapper for MPI_Status_set_elements_x
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-3.0
 * @ingroup ext
 * Triggers an enter and exit event.
 * It wraps the MPI_Status_set_elements_x call with enter and exit events.
 */
int
MPI_Status_set_elements_x( MPI_Status* status, MPI_Datatype datatype, MPI_Count count )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    const int event_gen_active           = SCOREP_MPI_IS_EVENT_GEN_ON;
    const int event_gen_active_for_group = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_EXT );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        if ( event_gen_active_for_group )
        {
            SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS_X ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_EnterWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS_X ] );
        }
    }

    SCOREP_ENTER_WRAPPED_REGION();
    return_val = PMPI_Status_set_elements_x( status, datatype, count );
    SCOREP_EXIT_WRAPPED_REGION();

    if ( event_gen_active )
    {
        if ( event_gen_active_for_group )
        {
            SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS_X ] );
        }
        else if ( SCOREP_IsUnwindingEnabled() )
        {
            SCOREP_ExitWrapper( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_STATUS_SET_ELEMENTS_X ] );
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
