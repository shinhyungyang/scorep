/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2014,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
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
 * @brief Fortran interface wrappers for one-sided communication
 */

#include <config.h>
#include "SCOREP_Fmpi.h"

/* uppercase defines */
/** @def MPI_Accumulate_U
    Exchange MPI_Accumulate_U by MPI_ACCUMULATE.
    It is used for the Fortran wrappers of MPI_Accumulate.
 */
#define MPI_Accumulate_U MPI_ACCUMULATE

/** @def MPI_Get_U
    Exchange MPI_Get_U by MPI_GET.
    It is used for the Fortran wrappers of MPI_Get.
 */
#define MPI_Get_U MPI_GET

/** @def MPI_Put_U
    Exchange MPI_Put_U by MPI_PUT.
    It is used for the Fortran wrappers of MPI_Put.
 */
#define MPI_Put_U MPI_PUT

/** @def MPI_Win_complete_U
    Exchange MPI_Win_complete_U by MPI_WIN_COMPLETE.
    It is used for the Fortran wrappers of MPI_Win_complete.
 */
#define MPI_Win_complete_U MPI_WIN_COMPLETE

/** @def MPI_Win_create_U
    Exchange MPI_Win_create_U by MPI_WIN_CREATE.
    It is used for the Fortran wrappers of MPI_Win_create.
 */
#define MPI_Win_create_U MPI_WIN_CREATE

/** @def MPI_Win_fence_U
    Exchange MPI_Win_fence_U by MPI_WIN_FENCE.
    It is used for the Fortran wrappers of MPI_Win_fence.
 */
#define MPI_Win_fence_U MPI_WIN_FENCE

/** @def MPI_Win_free_U
    Exchange MPI_Win_free_U by MPI_WIN_FREE.
    It is used for the Fortran wrappers of MPI_Win_free.
 */
#define MPI_Win_free_U MPI_WIN_FREE

/** @def MPI_Win_get_group_U
    Exchange MPI_Win_get_group_U by MPI_WIN_GET_GROUP.
    It is used for the Fortran wrappers of MPI_Win_get_group.
 */
#define MPI_Win_get_group_U MPI_WIN_GET_GROUP

/** @def MPI_Win_lock_U
    Exchange MPI_Win_lock_U by MPI_WIN_LOCK.
    It is used for the Fortran wrappers of MPI_Win_lock.
 */
#define MPI_Win_lock_U MPI_WIN_LOCK

/** @def MPI_Win_post_U
    Exchange MPI_Win_post_U by MPI_WIN_POST.
    It is used for the Fortran wrappers of MPI_Win_post.
 */
#define MPI_Win_post_U MPI_WIN_POST

/** @def MPI_Win_start_U
    Exchange MPI_Win_start_U by MPI_WIN_START.
    It is used for the Fortran wrappers of MPI_Win_start.
 */
#define MPI_Win_start_U MPI_WIN_START

/** @def MPI_Win_test_U
    Exchange MPI_Win_test_U by MPI_WIN_TEST.
    It is used for the Fortran wrappers of MPI_Win_test.
 */
#define MPI_Win_test_U MPI_WIN_TEST

/** @def MPI_Win_unlock_U
    Exchange MPI_Win_unlock_U by MPI_WIN_UNLOCK.
    It is used for the Fortran wrappers of MPI_Win_unlock.
 */
#define MPI_Win_unlock_U MPI_WIN_UNLOCK

/** @def MPI_Win_wait_U
    Exchange MPI_Win_wait_U by MPI_WIN_WAIT.
    It is used for the Fortran wrappers of MPI_Win_wait.
 */
#define MPI_Win_wait_U MPI_WIN_WAIT


/** @def MPI_Win_create_keyval_U
    Exchange MPI_Win_create_keyval_U by MPI_WIN_CREATE_KEYVAL.
    It is used for the Fortran wrappers of MPI_Win_create_keyval.
 */
#define MPI_Win_create_keyval_U MPI_WIN_CREATE_KEYVAL

/** @def MPI_Win_delete_attr_U
    Exchange MPI_Win_delete_attr_U by MPI_WIN_DELETE_ATTR.
    It is used for the Fortran wrappers of MPI_Win_delete_attr.
 */
#define MPI_Win_delete_attr_U MPI_WIN_DELETE_ATTR

/** @def MPI_Win_free_keyval_U
    Exchange MPI_Win_free_keyval_U by MPI_WIN_FREE_KEYVAL.
    It is used for the Fortran wrappers of MPI_Win_free_keyval.
 */
#define MPI_Win_free_keyval_U MPI_WIN_FREE_KEYVAL

/** @def MPI_Win_get_attr_U
    Exchange MPI_Win_get_attr_U by MPI_WIN_GET_ATTR.
    It is used for the Fortran wrappers of MPI_Win_get_attr.
 */
#define MPI_Win_get_attr_U MPI_WIN_GET_ATTR

/** @def MPI_Win_get_name_U
    Exchange MPI_Win_get_name_U by MPI_WIN_GET_NAME.
    It is used for the Fortran wrappers of MPI_Win_get_name.
 */
#define MPI_Win_get_name_U MPI_WIN_GET_NAME

/** @def MPI_Win_set_attr_U
    Exchange MPI_Win_set_attr_U by MPI_WIN_SET_ATTR.
    It is used for the Fortran wrappers of MPI_Win_set_attr.
 */
#define MPI_Win_set_attr_U MPI_WIN_SET_ATTR

/** @def MPI_Win_set_name_U
    Exchange MPI_Win_set_name_U by MPI_WIN_SET_NAME.
    It is used for the Fortran wrappers of MPI_Win_set_name.
 */
#define MPI_Win_set_name_U MPI_WIN_SET_NAME



/* lowercase defines */
/** @def MPI_Accumulate_L
    Exchanges MPI_Accumulate_L by mpi_accumulate.
    It is used for the Fortran wrappers of MPI_Accumulate.
 */
#define MPI_Accumulate_L mpi_accumulate

/** @def MPI_Get_L
    Exchanges MPI_Get_L by mpi_get.
    It is used for the Fortran wrappers of MPI_Get.
 */
#define MPI_Get_L mpi_get

/** @def MPI_Put_L
    Exchanges MPI_Put_L by mpi_put.
    It is used for the Fortran wrappers of MPI_Put.
 */
#define MPI_Put_L mpi_put

/** @def MPI_Win_complete_L
    Exchanges MPI_Win_complete_L by mpi_win_complete.
    It is used for the Fortran wrappers of MPI_Win_complete.
 */
#define MPI_Win_complete_L mpi_win_complete

/** @def MPI_Win_create_L
    Exchanges MPI_Win_create_L by mpi_win_create.
    It is used for the Fortran wrappers of MPI_Win_create.
 */
#define MPI_Win_create_L mpi_win_create

/** @def MPI_Win_fence_L
    Exchanges MPI_Win_fence_L by mpi_win_fence.
    It is used for the Fortran wrappers of MPI_Win_fence.
 */
#define MPI_Win_fence_L mpi_win_fence

/** @def MPI_Win_free_L
    Exchanges MPI_Win_free_L by mpi_win_free.
    It is used for the Fortran wrappers of MPI_Win_free.
 */
#define MPI_Win_free_L mpi_win_free

/** @def MPI_Win_get_group_L
    Exchanges MPI_Win_get_group_L by mpi_win_get_group.
    It is used for the Fortran wrappers of MPI_Win_get_group.
 */
#define MPI_Win_get_group_L mpi_win_get_group

/** @def MPI_Win_lock_L
    Exchanges MPI_Win_lock_L by mpi_win_lock.
    It is used for the Fortran wrappers of MPI_Win_lock.
 */
#define MPI_Win_lock_L mpi_win_lock

/** @def MPI_Win_post_L
    Exchanges MPI_Win_post_L by mpi_win_post.
    It is used for the Fortran wrappers of MPI_Win_post.
 */
#define MPI_Win_post_L mpi_win_post

/** @def MPI_Win_start_L
    Exchanges MPI_Win_start_L by mpi_win_start.
    It is used for the Fortran wrappers of MPI_Win_start.
 */
#define MPI_Win_start_L mpi_win_start

/** @def MPI_Win_test_L
    Exchanges MPI_Win_test_L by mpi_win_test.
    It is used for the Fortran wrappers of MPI_Win_test.
 */
#define MPI_Win_test_L mpi_win_test

/** @def MPI_Win_unlock_L
    Exchanges MPI_Win_unlock_L by mpi_win_unlock.
    It is used for the Fortran wrappers of MPI_Win_unlock.
 */
#define MPI_Win_unlock_L mpi_win_unlock

/** @def MPI_Win_wait_L
    Exchanges MPI_Win_wait_L by mpi_win_wait.
    It is used for the Fortran wrappers of MPI_Win_wait.
 */
#define MPI_Win_wait_L mpi_win_wait


/** @def MPI_Win_create_keyval_L
    Exchanges MPI_Win_create_keyval_L by mpi_win_create_keyval.
    It is used for the Fortran wrappers of MPI_Win_create_keyval.
 */
#define MPI_Win_create_keyval_L mpi_win_create_keyval

/** @def MPI_Win_delete_attr_L
    Exchanges MPI_Win_delete_attr_L by mpi_win_delete_attr.
    It is used for the Fortran wrappers of MPI_Win_delete_attr.
 */
#define MPI_Win_delete_attr_L mpi_win_delete_attr

/** @def MPI_Win_free_keyval_L
    Exchanges MPI_Win_free_keyval_L by mpi_win_free_keyval.
    It is used for the Fortran wrappers of MPI_Win_free_keyval.
 */
#define MPI_Win_free_keyval_L mpi_win_free_keyval

/** @def MPI_Win_get_attr_L
    Exchanges MPI_Win_get_attr_L by mpi_win_get_attr.
    It is used for the Fortran wrappers of MPI_Win_get_attr.
 */
#define MPI_Win_get_attr_L mpi_win_get_attr

/** @def MPI_Win_get_name_L
    Exchanges MPI_Win_get_name_L by mpi_win_get_name.
    It is used for the Fortran wrappers of MPI_Win_get_name.
 */
#define MPI_Win_get_name_L mpi_win_get_name

/** @def MPI_Win_set_attr_L
    Exchanges MPI_Win_set_attr_L by mpi_win_set_attr.
    It is used for the Fortran wrappers of MPI_Win_set_attr.
 */
#define MPI_Win_set_attr_L mpi_win_set_attr

/** @def MPI_Win_set_name_L
    Exchanges MPI_Win_set_name_L by mpi_win_set_name.
    It is used for the Fortran wrappers of MPI_Win_set_name.
 */
#define MPI_Win_set_name_L mpi_win_set_name



#ifndef NEED_F2C_CONV

/* If MPI_Comm_f2c is HAVE as a simple macro (which typically does nothing)
 * as it would require a "real" function if it is really needed
 * => we can save the f2c and c2s conversions */

/**
 * @name Fortran wrappers for access functions
 * @{
 */
#if HAVE( DECL_PMPI_ACCUMULATE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Accumulate )
/**
 * Measurement wrapper for MPI_Accumulate
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Accumulate )( void* origin_addr, int* origin_count, MPI_Datatype* origin_datatype, int* target_rank, MPI_Aint* target_disp, int* target_count, MPI_Datatype* target_datatype, MPI_Op* op, MPI_Win* win, int* ierr )
{
    #if HAVE( MPI_BOTTOM )
    if ( origin_addr == scorep_mpi_fortran_bottom )
    {
        origin_addr = MPI_BOTTOM;
    }
    #endif


    *ierr = MPI_Accumulate( origin_addr, *origin_count, *origin_datatype, *target_rank, *target_disp, *target_count, *target_datatype, *op, *win );
}
#endif
#if HAVE( DECL_PMPI_GET ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Get )
/**
 * Measurement wrapper for MPI_Get
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Get )( void* origin_addr, int* origin_count, MPI_Datatype* origin_datatype, int* target_rank, MPI_Aint* target_disp, int* target_count, MPI_Datatype* target_datatype, MPI_Win* win, int* ierr )
{
    #if HAVE( MPI_BOTTOM )
    if ( origin_addr == scorep_mpi_fortran_bottom )
    {
        origin_addr = MPI_BOTTOM;
    }
    #endif


    *ierr = MPI_Get( origin_addr, *origin_count, *origin_datatype, *target_rank, *target_disp, *target_count, *target_datatype, *win );
}
#endif
#if HAVE( DECL_PMPI_PUT ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Put )
/**
 * Measurement wrapper for MPI_Put
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Put )( void* origin_addr, int* origin_count, MPI_Datatype* origin_datatype, int* target_rank, MPI_Aint* target_disp, int* target_count, MPI_Datatype* target_datatype, MPI_Win* win, int* ierr )
{
    #if HAVE( MPI_BOTTOM )
    if ( origin_addr == scorep_mpi_fortran_bottom )
    {
        origin_addr = MPI_BOTTOM;
    }
    #endif


    *ierr = MPI_Put( origin_addr, *origin_count, *origin_datatype, *target_rank, *target_disp, *target_count, *target_datatype, *win );
}
#endif

/**
 * @}
 * @name Fortran wrappers for window management functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_CREATE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_create )
/**
 * Measurement wrapper for MPI_Win_create
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_create )( void* base, MPI_Aint* size, int* disp_unit, MPI_Info* info, MPI_Comm* comm, MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_create( base, *size, *disp_unit, *info, *comm, win );
}
#endif
#if HAVE( DECL_PMPI_WIN_FREE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_free )
/**
 * Measurement wrapper for MPI_Win_free
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_free )( MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_free( win );
}
#endif

#if HAVE( DECL_PMPI_WIN_GET_GROUP ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_group )
/**
 * Measurement wrapper for MPI_Win_get_group
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_get_group )( MPI_Win* win, MPI_Group* group, int* ierr )
{
    *ierr = MPI_Win_get_group( *win, group );
}
#endif

/**
 * @}
 * @name Fortran wrappers for synchonization functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_COMPLETE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_complete )
/**
 * Measurement wrapper for MPI_Win_complete
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_complete )( MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_complete( *win );
}
#endif
#if HAVE( DECL_PMPI_WIN_FENCE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_fence )
/**
 * Measurement wrapper for MPI_Win_fence
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_fence )( int* assert, MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_fence( *assert, *win );
}
#endif
#if HAVE( DECL_PMPI_WIN_LOCK ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_lock )
/**
 * Measurement wrapper for MPI_Win_lock
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_lock )( int* lock_type, int* rank, int* assert, MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_lock( *lock_type, *rank, *assert, *win );
}
#endif
#if HAVE( DECL_PMPI_WIN_POST ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_post )
/**
 * Measurement wrapper for MPI_Win_post
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_post )( MPI_Group* group, int* assert, MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_post( *group, *assert, *win );
}
#endif
#if HAVE( DECL_PMPI_WIN_START ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_start )
/**
 * Measurement wrapper for MPI_Win_start
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_start )( MPI_Group* group, int* assert, MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_start( *group, *assert, *win );
}
#endif
#if HAVE( DECL_PMPI_WIN_TEST ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_test )
/**
 * Measurement wrapper for MPI_Win_test
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_test )( MPI_Win* win, int* flag, int* ierr )
{
    *ierr = MPI_Win_test( *win, flag );
}
#endif
#if HAVE( DECL_PMPI_WIN_UNLOCK ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_unlock )
/**
 * Measurement wrapper for MPI_Win_unlock
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_unlock )( int* rank, MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_unlock( *rank, *win );
}
#endif
#if HAVE( DECL_PMPI_WIN_WAIT ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_wait )
/**
 * Measurement wrapper for MPI_Win_wait
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 */
void
FSUB( MPI_Win_wait )( MPI_Win* win, int* ierr )
{
    *ierr = MPI_Win_wait( *win );
}
#endif

/**
 * @}
 * @name Fortran wrappers for error handling functions
 * @{
 */
/* DO not wrap group rma_err */

/**
 * @}
 * @name Fortran wrappers for external interface functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_CREATE_KEYVAL ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_create_keyval )
/**
 * Measurement wrapper for MPI_Win_create_keyval
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_create_keyval )( MPI_Win_copy_attr_function* win_copy_attr_fn, MPI_Win_delete_attr_function* win_delete_attr_fn, int* win_keyval, void* extra_state, int* ierr )
{
    *ierr = MPI_Win_create_keyval( win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state );
}
#endif
#if HAVE( DECL_PMPI_WIN_DELETE_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_delete_attr )
/**
 * Measurement wrapper for MPI_Win_delete_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_delete_attr )( MPI_Win* win, int* win_keyval, int* ierr )
{
    *ierr = MPI_Win_delete_attr( *win, *win_keyval );
}
#endif
#if HAVE( DECL_PMPI_WIN_FREE_KEYVAL ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_free_keyval )
/**
 * Measurement wrapper for MPI_Win_free_keyval
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_free_keyval )( int* win_keyval, int* ierr )
{
    *ierr = MPI_Win_free_keyval( win_keyval );
}
#endif
#if HAVE( DECL_PMPI_WIN_GET_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_attr )
/**
 * Measurement wrapper for MPI_Win_get_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_get_attr )( MPI_Win* win, int* win_keyval, void* attribute_val, int* flag, int* ierr )
{
    *ierr = MPI_Win_get_attr( *win, *win_keyval, attribute_val, flag );
}
#endif
#if HAVE( DECL_PMPI_WIN_GET_NAME ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_name )
/**
 * Measurement wrapper for MPI_Win_get_name
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_get_name )( MPI_Win* win, char* win_name, int* resultlen, int* ierr, int win_name_len )
{
    char* c_win_name     = NULL;
    int   c_win_name_len = 0;
    c_win_name = ( char* )malloc( ( win_name_len + 1 ) * sizeof( char ) );
    if ( !c_win_name )
    {
        exit( EXIT_FAILURE );
    }



    *ierr = MPI_Win_get_name( *win, c_win_name, resultlen );


    c_win_name_len = strlen( c_win_name );
    strncpy( win_name, c_win_name, c_win_name_len );
    memset( win_name + c_win_name_len, ' ', win_name_len - c_win_name_len );
    free( c_win_name );
}
#endif
#if HAVE( DECL_PMPI_WIN_SET_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Win_set_attr )
/**
 * Measurement wrapper for MPI_Win_set_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_set_attr )( MPI_Win* win, int* win_keyval, void* attribute_val, int* ierr )
{
    *ierr = MPI_Win_set_attr( *win, *win_keyval, attribute_val );
}
#endif
#if HAVE( DECL_PMPI_WIN_SET_NAME ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_set_name )
/**
 * Measurement wrapper for MPI_Win_set_name
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 */
void
FSUB( MPI_Win_set_name )( MPI_Win* win, char* win_name, int* ierr, int win_name_len )
{
    char* c_win_name = NULL;
    c_win_name = scorep_f2c_string( win_name, win_name_len );



    *ierr = MPI_Win_set_name( *win, c_win_name );

    free( c_win_name );
}
#endif


#else /* !NEED_F2C_CONV */

/**
 * @name Fortran wrappers for access functions
 * @{
 */
#if HAVE( DECL_PMPI_ACCUMULATE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Accumulate )
/**
 * Measurement wrapper for MPI_Accumulate
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Accumulate
 */
void
FSUB( MPI_Accumulate )( void* origin_addr, MPI_Fint* origin_count, MPI_Fint* origin_datatype, MPI_Fint* target_rank, MPI_Aint* target_disp, MPI_Fint* target_count, MPI_Fint* target_datatype, MPI_Fint* op, MPI_Fint* win, MPI_Fint* ierr )
{
    #if HAVE( MPI_BOTTOM )
    if ( origin_addr == scorep_mpi_fortran_bottom )
    {
        origin_addr = MPI_BOTTOM;
    }
    #endif


    *ierr = MPI_Accumulate( origin_addr, *origin_count, PMPI_Type_f2c( *origin_datatype ), *target_rank, *target_disp, *target_count, PMPI_Type_f2c( *target_datatype ), PMPI_Op_f2c( *op ), PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_GET ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Get )
/**
 * Measurement wrapper for MPI_Get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Get
 */
void
FSUB( MPI_Get )( void* origin_addr, MPI_Fint* origin_count, MPI_Fint* origin_datatype, MPI_Fint* target_rank, MPI_Aint* target_disp, MPI_Fint* target_count, MPI_Fint* target_datatype, MPI_Fint* win, MPI_Fint* ierr )
{
    #if HAVE( MPI_BOTTOM )
    if ( origin_addr == scorep_mpi_fortran_bottom )
    {
        origin_addr = MPI_BOTTOM;
    }
    #endif


    *ierr = MPI_Get( origin_addr, *origin_count, PMPI_Type_f2c( *origin_datatype ), *target_rank, *target_disp, *target_count, PMPI_Type_f2c( *target_datatype ), PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_PUT ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Put )
/**
 * Measurement wrapper for MPI_Put
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Put
 */
void
FSUB( MPI_Put )( void* origin_addr, MPI_Fint* origin_count, MPI_Fint* origin_datatype, MPI_Fint* target_rank, MPI_Aint* target_disp, MPI_Fint* target_count, MPI_Fint* target_datatype, MPI_Fint* win, MPI_Fint* ierr )
{
    #if HAVE( MPI_BOTTOM )
    if ( origin_addr == scorep_mpi_fortran_bottom )
    {
        origin_addr = MPI_BOTTOM;
    }
    #endif


    *ierr = MPI_Put( origin_addr, *origin_count, PMPI_Type_f2c( *origin_datatype ), *target_rank, *target_disp, *target_count, PMPI_Type_f2c( *target_datatype ), PMPI_Win_f2c( *win ) );
}
#endif

/**
 * @}
 * @name Fortran wrappers for window management functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_CREATE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_create )
/**
 * Measurement wrapper for MPI_Win_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_create
 */
void
FSUB( MPI_Win_create )( void* base, MPI_Aint* size, MPI_Fint* disp_unit, MPI_Fint* info, MPI_Fint* comm, MPI_Fint* win, MPI_Fint* ierr )
{
    MPI_Win c_win;


    *ierr = MPI_Win_create( base, *size, *disp_unit, PMPI_Info_f2c( *info ), PMPI_Comm_f2c( *comm ), &c_win );

    *win = PMPI_Win_c2f( c_win );
}
#endif
#if HAVE( DECL_PMPI_WIN_FREE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_free )
/**
 * Measurement wrapper for MPI_Win_free
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_free
 */
void
FSUB( MPI_Win_free )( MPI_Fint* win, MPI_Fint* ierr )
{
    MPI_Win c_win = PMPI_Win_f2c( *win );


    *ierr = MPI_Win_free( &c_win );

    *win = PMPI_Win_c2f( c_win );
}
#endif

#if HAVE( DECL_PMPI_WIN_GET_GROUP ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_group )
/**
 * Measurement wrapper for MPI_Win_get_group
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_get_group
 */
void
FSUB( MPI_Win_get_group )( MPI_Fint* win, MPI_Fint* group, MPI_Fint* ierr )
{
    MPI_Group c_group;


    *ierr = MPI_Win_get_group( PMPI_Win_f2c( *win ), &c_group );

    *group = PMPI_Group_c2f( c_group );
}
#endif

/**
 * @}
 * @name Fortran wrappers for synchonization functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_COMPLETE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_complete )
/**
 * Measurement wrapper for MPI_Win_complete
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_complete
 */
void
FSUB( MPI_Win_complete )( MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_complete( PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_WIN_FENCE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_fence )
/**
 * Measurement wrapper for MPI_Win_fence
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_fence
 */
void
FSUB( MPI_Win_fence )( MPI_Fint* assert, MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_fence( *assert, PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_WIN_LOCK ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_lock )
/**
 * Measurement wrapper for MPI_Win_lock
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_lock
 */
void
FSUB( MPI_Win_lock )( MPI_Fint* lock_type, MPI_Fint* rank, MPI_Fint* assert, MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_lock( *lock_type, *rank, *assert, PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_WIN_POST ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_post )
/**
 * Measurement wrapper for MPI_Win_post
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_post
 */
void
FSUB( MPI_Win_post )( MPI_Fint* group, MPI_Fint* assert, MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_post( PMPI_Group_f2c( *group ), *assert, PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_WIN_START ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_start )
/**
 * Measurement wrapper for MPI_Win_start
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_start
 */
void
FSUB( MPI_Win_start )( MPI_Fint* group, MPI_Fint* assert, MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_start( PMPI_Group_f2c( *group ), *assert, PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_WIN_TEST ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_test )
/**
 * Measurement wrapper for MPI_Win_test
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_test
 */
void
FSUB( MPI_Win_test )( MPI_Fint* win, MPI_Fint* flag, MPI_Fint* ierr )
{
    *ierr = MPI_Win_test( PMPI_Win_f2c( *win ), flag );
}
#endif
#if HAVE( DECL_PMPI_WIN_UNLOCK ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_unlock )
/**
 * Measurement wrapper for MPI_Win_unlock
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_unlock
 */
void
FSUB( MPI_Win_unlock )( MPI_Fint* rank, MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_unlock( *rank, PMPI_Win_f2c( *win ) );
}
#endif
#if HAVE( DECL_PMPI_WIN_WAIT ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Win_wait )
/**
 * Measurement wrapper for MPI_Win_wait
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * For the order of events see @ref MPI_Win_wait
 */
void
FSUB( MPI_Win_wait )( MPI_Fint* win, MPI_Fint* ierr )
{
    *ierr = MPI_Win_wait( PMPI_Win_f2c( *win ) );
}
#endif

/**
 * @}
 * @name Fortran wrappers for error handling functions
 * @{
 */
/* Do not wrap group rma_err */

/**
 * @}
 * @name Fortran wrappers for external interface functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_CREATE_KEYVAL ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_create_keyval )
/**
 * Measurement wrapper for MPI_Win_create_keyval
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_create_keyval
 */
void
FSUB( MPI_Win_create_keyval )( void* win_copy_attr_fn, void* win_delete_attr_fn, MPI_Fint* win_keyval, void* extra_state, MPI_Fint* ierr )
{
    *ierr = MPI_Win_create_keyval( ( MPI_Win_copy_attr_function* )win_copy_attr_fn, ( MPI_Win_delete_attr_function* )win_delete_attr_fn, win_keyval, extra_state );
}
#endif
#if HAVE( DECL_PMPI_WIN_DELETE_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_delete_attr )
/**
 * Measurement wrapper for MPI_Win_delete_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_delete_attr
 */
void
FSUB( MPI_Win_delete_attr )( MPI_Fint* win, MPI_Fint* win_keyval, MPI_Fint* ierr )
{
    MPI_Win c_win = PMPI_Win_f2c( *win );


    *ierr = MPI_Win_delete_attr( c_win, *win_keyval );

    *win = PMPI_Win_c2f( c_win );
}
#endif
#if HAVE( DECL_PMPI_WIN_FREE_KEYVAL ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_free_keyval )
/**
 * Measurement wrapper for MPI_Win_free_keyval
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_free_keyval
 */
void
FSUB( MPI_Win_free_keyval )( MPI_Fint* win_keyval, MPI_Fint* ierr )
{
    *ierr = MPI_Win_free_keyval( win_keyval );
}
#endif
#if HAVE( DECL_PMPI_WIN_GET_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_attr )
/**
 * Measurement wrapper for MPI_Win_get_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_get_attr
 */
void
FSUB( MPI_Win_get_attr )( MPI_Fint* win, MPI_Fint* win_keyval, void* attribute_val, MPI_Fint* flag, MPI_Fint* ierr )
{
    *ierr = MPI_Win_get_attr( PMPI_Win_f2c( *win ), *win_keyval, attribute_val, flag );
}
#endif
#if HAVE( DECL_PMPI_WIN_GET_NAME ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_name )
/**
 * Measurement wrapper for MPI_Win_get_name
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_get_name
 */
void
FSUB( MPI_Win_get_name )( MPI_Fint* win, char* win_name, MPI_Fint* resultlen, MPI_Fint* ierr, int win_name_len )
{
    char* c_win_name     = NULL;
    int   c_win_name_len = 0;
    c_win_name = ( char* )malloc( ( win_name_len + 1 ) * sizeof( char ) );
    if ( !c_win_name )
    {
        exit( EXIT_FAILURE );
    }



    *ierr = MPI_Win_get_name( PMPI_Win_f2c( *win ), c_win_name, resultlen );


    c_win_name_len = strlen( c_win_name );
    strncpy( win_name, c_win_name, c_win_name_len );
    memset( win_name + c_win_name_len, ' ', win_name_len - c_win_name_len );
    free( c_win_name );
}
#endif
#if HAVE( DECL_PMPI_WIN_SET_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Win_set_attr )
/**
 * Measurement wrapper for MPI_Win_set_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_set_attr
 */
void
FSUB( MPI_Win_set_attr )( MPI_Fint* win, MPI_Fint* win_keyval, void* attribute_val, MPI_Fint* ierr )
{
    MPI_Win c_win = PMPI_Win_f2c( *win );


    *ierr = MPI_Win_set_attr( c_win, *win_keyval, attribute_val );

    *win = PMPI_Win_c2f( c_win );
}
#endif
#if HAVE( DECL_PMPI_WIN_SET_NAME ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_set_name )
/**
 * Measurement wrapper for MPI_Win_set_name
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * For the order of events see @ref MPI_Win_set_name
 */
void
FSUB( MPI_Win_set_name )( MPI_Fint* win, char* win_name, MPI_Fint* ierr, int win_name_len )
{
    MPI_Win c_win      = PMPI_Win_f2c( *win );
    char*   c_win_name = NULL;
    c_win_name = scorep_f2c_string( win_name, win_name_len );



    *ierr = MPI_Win_set_name( c_win, c_win_name );

    *win = PMPI_Win_c2f( c_win );
    free( c_win_name );
}
#endif

/**
 * @}
 * @name Fortran wrappers for handle conversion functions
 * @{
 */

#endif /* !NEED_F2C_CONV */

/**
 * @}
 */
