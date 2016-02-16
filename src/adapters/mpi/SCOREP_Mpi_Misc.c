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
 * @brief C interface wrappers for miscelaneous and handler conversion
 *        functions
 */

#include <config.h>
/* We do wrap deprecated functions here, but we don't want warnings */
#define OMPI_WANT_MPI_INTERFACE_WARNING 0
#include "SCOREP_Mpi.h"
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>

/**
 * @name C wrappers
 * @{
 */

#if HAVE( DECL_PMPI_ADDRESS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Address ) && defined( SCOREP_MPI_ADDRESS_PROTO_ARGS )
/**
 * Measurement wrapper for MPI_Address
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdWithProto.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Address call with enter and exit events.
 */
int
MPI_Address SCOREP_MPI_ADDRESS_PROTO_ARGS
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ADDRESS ],
                                   ( intptr_t )PMPI_Address );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Address( location, address );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ADDRESS ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Address( location, address );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif


#if HAVE( DECL_PMPI_ALLOC_MEM ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Alloc_mem )
/**
 * Measurement wrapper for MPI_Alloc_mem
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Alloc_mem call with enter and exit events.
 */
int
MPI_Alloc_mem( MPI_Aint size, MPI_Info info, void* baseptr )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        if ( scorep_mpi_memory_recording )
        {
            uint64_t size_as_uint64 = size;
            SCOREP_AddAttribute( scorep_mpi_memory_alloc_size_attribute,
                                 &size_as_uint64 );
        }
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ALLOC_MEM ],
                                   ( intptr_t )PMPI_Alloc_mem );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Alloc_mem( size, info, baseptr );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( scorep_mpi_memory_recording && size > 0 && MPI_SUCCESS == return_val )
        {
            SCOREP_AllocMetric_HandleAlloc( scorep_mpi_allocations_metric,
                                            ( uint64_t )( *( void** )baseptr ),
                                            size );
        }

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_ALLOC_MEM ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Alloc_mem( size, info, baseptr );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_FREE_MEM ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Free_mem )
/**
 * Measurement wrapper for MPI_Free_mem
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Free_mem call with enter and exit events.
 */
int
MPI_Free_mem( void* base )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FREE_MEM ],
                                   ( intptr_t )PMPI_Free_mem );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Free_mem( base );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( scorep_mpi_memory_recording && base && MPI_SUCCESS == return_val )
        {
            uint64_t dealloc_size;
            SCOREP_AllocMetric_HandleFree( scorep_mpi_allocations_metric,
                                           ( uint64_t )base, &dealloc_size );
            SCOREP_AddAttribute( scorep_mpi_memory_dealloc_size_attribute,
                                 &dealloc_size );
        }

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_FREE_MEM ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Free_mem( base );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GET_ADDRESS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Get_address )
/**
 * Measurement wrapper for MPI_Get_address
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_address call with enter and exit events.
 */
int
MPI_Get_address( SCOREP_MPI_CONST_DECL void* location, MPI_Aint* address )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ADDRESS ],
                                   ( intptr_t )PMPI_Get_address );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Get_address( location, address );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_ADDRESS ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Get_address( location, address );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GET_VERSION ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Get_version )
/**
 * Measurement wrapper for MPI_Get_version
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Get_version call with enter and exit events.
 */
int
MPI_Get_version( int* version, int* subversion )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_VERSION ],
                                   ( intptr_t )PMPI_Get_version );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Get_version( version, subversion );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_GET_VERSION ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Get_version( version, subversion );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_OP_COMMUTATIVE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Op_commutative )
/**
 * Measurement wrapper for MPI_Op_commutative
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Op_commutative call with enter and exit events.
 */
int
MPI_Op_commutative( MPI_Op op, int* commute )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_OP_COMMUTATIVE ],
                                   ( intptr_t )PMPI_Op_commutative );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Op_commutative( op, commute );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_OP_COMMUTATIVE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Op_commutative( op, commute );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_OP_CREATE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Op_create )
/**
 * Measurement wrapper for MPI_Op_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Op_create call with enter and exit events.
 */
int
MPI_Op_create( MPI_User_function* function, int commute, MPI_Op* op )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_OP_CREATE ],
                                   ( intptr_t )PMPI_Op_create );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Op_create( function, commute, op );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_OP_CREATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Op_create( function, commute, op );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_OP_FREE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Op_free )
/**
 * Measurement wrapper for MPI_Op_free
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Op_free call with enter and exit events.
 */
int
MPI_Op_free( MPI_Op* op )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_OP_FREE ],
                                   ( intptr_t )PMPI_Op_free );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Op_free( op );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_OP_FREE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Op_free( op );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_REQUEST_GET_STATUS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Request_get_status )
/**
 * Measurement wrapper for MPI_Request_get_status
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Request_get_status call with enter and exit events.
 */
int
MPI_Request_get_status( MPI_Request request, int* flag, MPI_Status* status )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_REQUEST_GET_STATUS ],
                                   ( intptr_t )PMPI_Request_get_status );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Request_get_status( request, flag, status );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_REQUEST_GET_STATUS ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Request_get_status( request, flag, status );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif


#if HAVE( DECL_PMPI_INFO_C2F ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_c2f )
/**
 * Measurement wrapper for MPI_Info_c2f
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_c2f call with enter and exit events.
 */
MPI_Fint
MPI_Info_c2f( MPI_Info info )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    MPI_Fint return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_C2F ],
                                   ( intptr_t )PMPI_Info_c2f );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_c2f( info );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_C2F ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_c2f( info );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_INFO_CREATE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_create )
/**
 * Measurement wrapper for MPI_Info_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_create call with enter and exit events.
 */
int
MPI_Info_create( MPI_Info* info )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_CREATE ],
                                   ( intptr_t )PMPI_Info_create );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_create( info );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_CREATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_create( info );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_INFO_DUP ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_dup )
/**
 * Measurement wrapper for MPI_Info_dup
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_dup call with enter and exit events.
 */
int
MPI_Info_dup( MPI_Info info, MPI_Info* newinfo )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_DUP ],
                                   ( intptr_t )PMPI_Info_dup );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_dup( info, newinfo );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_DUP ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_dup( info, newinfo );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_INFO_F2C ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_f2c )
/**
 * Measurement wrapper for MPI_Info_f2c
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_f2c call with enter and exit events.
 */
MPI_Info
MPI_Info_f2c( MPI_Fint info )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    MPI_Info return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_F2C ],
                                   ( intptr_t )PMPI_Info_f2c );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_f2c( info );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_F2C ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_f2c( info );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_INFO_FREE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_free )
/**
 * Measurement wrapper for MPI_Info_free
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_free call with enter and exit events.
 */
int
MPI_Info_free( MPI_Info* info )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_FREE ],
                                   ( intptr_t )PMPI_Info_free );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_free( info );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_FREE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_free( info );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif


#if HAVE( MPI_INFO_DELETE_COMPLIANT )
#if HAVE( DECL_PMPI_INFO_DELETE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_delete )
/**
 * Measurement wrapper for MPI_Info_delete
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdCompl.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_delete call with enter and exit events.
 */
int
MPI_Info_delete( MPI_Info info, SCOREP_MPI_CONST_DECL char* key )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_DELETE ],
                                   ( intptr_t )PMPI_Info_delete );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_delete( info, key );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_DELETE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_delete( info, key );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif
#endif


#if HAVE( MPI_INFO_GET_COMPLIANT )
#if HAVE( DECL_PMPI_INFO_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_get )
/**
 * Measurement wrapper for MPI_Info_get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdCompl.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_get call with enter and exit events.
 */
int
MPI_Info_get( MPI_Info info, SCOREP_MPI_CONST_DECL char* key, int valuelen, char* value, int* flag )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_GET ],
                                   ( intptr_t )PMPI_Info_get );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_get( info, key, valuelen, value, flag );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_GET ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_get( info, key, valuelen, value, flag );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif
#endif


#if HAVE( MPI_INFO_GET_VALUELEN_COMPLIANT )
#if HAVE( DECL_PMPI_INFO_GET_VALUELEN ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_get_valuelen )
/**
 * Measurement wrapper for MPI_Info_get_valuelen
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdCompl.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_get_valuelen call with enter and exit events.
 */
int
MPI_Info_get_valuelen( MPI_Info info, SCOREP_MPI_CONST_DECL char* key, int* valuelen, int* flag )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_GET_VALUELEN ],
                                   ( intptr_t )PMPI_Info_get_valuelen );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_get_valuelen( info, key, valuelen, flag );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_GET_VALUELEN ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_get_valuelen( info, key, valuelen, flag );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif
#endif


#if HAVE( MPI_INFO_SET_COMPLIANT )
#if HAVE( DECL_PMPI_INFO_SET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( MPI_Info_set )
/**
 * Measurement wrapper for MPI_Info_set
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_StdCompl.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup misc
 * Triggers an enter and exit event.
 * It wraps the MPI_Info_set call with enter and exit events.
 */
int
MPI_Info_set( MPI_Info info, SCOREP_MPI_CONST_DECL char* key, SCOREP_MPI_CONST_DECL char* value )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterWrappedRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_SET ],
                                   ( intptr_t )PMPI_Info_set );

        SCOREP_ENTER_WRAPPED_REGION();
        return_val = PMPI_Info_set( info, key, value );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_ExitRegion( scorep_mpi_regions[ SCOREP_MPI_REGION__MPI_INFO_SET ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Info_set( info, key, value );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();

    return return_val;
}
#endif
#endif


/**
 * @}
 */
