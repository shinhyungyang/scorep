/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */


/**
 * @file  SCOREP_Mpi_Rma.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 * @status     alpha
 * @ingroup    MPI_Wrapper
 *
 * @brief C interface wrappers for one-sided communication
 */

#include <config.h>
#include "SCOREP_Mpi.h"

/** internal id counter for rma operations */
static int scorep_rma_id = 0;

/** current rma id to use in event generation */
#define SCOREP_CURR_RMA_ID   scorep_rma_id
/** increment and get rma id to use in event generation */
#define SCOREP_NEXT_RMA_ID ++scorep_rma_id

/**
 * @name C wrappers for access functions
 * @{
 */
#if HAVE( DECL_PMPI_ACCUMULATE ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Accumulate )
/**
 * Measurement wrapper for MPI_Accumulate
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_RmaPut.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Accumulate( void*        origin_addr,
                int          origin_count,
                MPI_Datatype origin_datatype,
                int          target_rank,
                MPI_Aint     target_disp,
                int          target_count,
                MPI_Datatype target_datatype,
                MPI_Op       op,
                MPI_Win      win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
/* One-sided communication not handled in first version
    int     sendsz;
    elg_ui4 dpid;
    elg_ui4 wid;
 */
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_ACCUMULATE ] );

/* One-sided communication not handled in first version
    dpid = scorep_mpi_win_rank_to_pe( target_rank, win );
    wid = scorep_mpi_win_rank_id( win );

    PMPI_Type_size(origin_datatype, &sendsz);
    esd_mpi_put_1ts( dpid, wid, SCOREP_NEXT_RMA_ID, origin_count * sendsz);
 */
        return_val = PMPI_Accumulate( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win );

/* One-sided communication not handled in first version
    esd_mpi_put_1te_remote(dpid, wid, SCOREP_CURR_RMA_ID);
 */

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_ACCUMULATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Accumulate( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, op, win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GET ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Get
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Get( void*        origin_addr,
         int          origin_count,
         MPI_Datatype origin_datatype,
         int          target_rank,
         MPI_Aint     target_disp,
         int          target_count,
         MPI_Datatype target_datatype,
         MPI_Win      win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
/* One-sided communication not handled in first version
    int32_t              sendsz;
    SCOREP_Mpi_Rank        dest_proc_rank;
    SCOREP_MPIWindowHandle win_handle;
 */

        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GET ] );

/* One-sided communication not handled in first version
    dest_proc_rank = scorep_mpi_win_rank_to_pe( target_rank, win );
    win_handle = scorep_mpi_win_id( win );
 */
        /* in MPI_GET the target buffer is remote */
/* One-sided communication not handled in first version
    PMPI_Type_size(target_datatype, &sendsz);
    esd_mpi_get_1ts_remote(dest_proc_rank, win_handle, SCOREP_NEXT_RMA_ID, target_count * sendsz);
 */
        return_val = PMPI_Get( origin_addr, origin_count,
                               origin_datatype, target_rank, target_disp,
                               target_count, target_datatype, win );

        /* in MPI_GET the origin buffer is local */
/* One-sided communication not handled in first version
    esd_mpi_get_1te( dest_proc_rank, win_handle, SCOREP_CURR_RMA_ID);
 */
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GET ] );

        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Get( origin_addr, origin_count,
                               origin_datatype, target_rank, target_disp,
                               target_count, target_datatype, win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_PUT ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( MPI_Put )
/**
 * Measurement wrapper for MPI_Put
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_RmaPut.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Put( void*        origin_addr,
         int          origin_count,
         MPI_Datatype origin_datatype,
         int          target_rank,
         MPI_Aint     target_disp,
         int          target_count,
         MPI_Datatype target_datatype,
         MPI_Win      win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
/* One-sided communication not handled in first version
    int     sendsz;
    elg_ui4 dpid;
    elg_ui4 wid;
 */
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_PUT ] );

/* One-sided communication not handled in first version
    dpid = scorep_mpi_win_rank_to_pe( target_rank, win );
    wid = scorep_mpi_win_rank_id( win );

    PMPI_Type_size(origin_datatype, &sendsz);
    esd_mpi_put_1ts( dpid, wid, SCOREP_NEXT_RMA_ID, origin_count * sendsz);
 */
        return_val = PMPI_Put( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win );

/* One-sided communication not handled in first version
    esd_mpi_put_1te_remote(dpid, wid, SCOREP_CURR_RMA_ID);
 */

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_PUT ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Put( origin_addr, origin_count, origin_datatype, target_rank, target_disp, target_count, target_datatype, win );
    }

    return return_val;
}
#endif

/**
 * @}
 * @name C wrappers for window management functions
 * @{
 */

#if HAVE( DECL_PMPI_WIN_CREATE ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_create
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_create( void*    base,
                MPI_Aint size,
                int      disp_unit,
                MPI_Info info,
                MPI_Comm comm,
                MPI_Win* win )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CREATE ] );
    }

    return_val = PMPI_Win_create( base, size, disp_unit,
                                  info, comm, win );

/* One-sided communication not handled in first version
   if (*win != MPI_WIN_NULL)
   {
    scorep_mpi_win_rank_create( *win, comm );
   }
 */

    if ( event_gen_active )
    {
/* One-sided communication not handled in first version
    esd_mpi_wincollexit(scorep_mpi_regid[SCOREP__MPI_WIN_CREATE], scorep_mpi_win_rank_id(*win));
 */
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_FREE ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_free
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_free( MPI_Win* win )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA );
    int       return_val;
/* One-sided communication not handled in first version
   SCOREP_MPIWindowHanle win_handle;
 */

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_FREE ] );
/* One-sided communication not handled in first version
    win_handle = scorep_mpi_win_rank_id(*win);
 */
    }

/* One-sided communication not handled in first version
   scorep_mpi_win_rank_free(*win);
 */

    return_val = PMPI_Win_free( win );

    if ( event_gen_active )
    {
/* One-sided communication not handled in first version
    esd_mpi_wincollexit(scorep_mpi_regid[SCOREP__MPI_WIN_FREE], win_handle);
 */

        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif

/**
 * @}
 * @name C wrappers for synchonization functions
 * @{
 */

#if HAVE( DECL_PMPI_WIN_COMPLETE ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_complete
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_complete( MPI_Win win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_COMPLETE ] );

        return_val = PMPI_Win_complete( win );

/* One-sided communication not handled in first version
    esd_mpi_winexit(scorep_mpi_regid[SCOREP__MPI_WIN_COMPLETE],
                    scorep_mpi_win_rank_id(win), scorep_mpi_winacc_get_gid(win, 1), 1);
    scorep_mpi_winacc_end(win, 1);
 */
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_complete( win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_FENCE ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_fence
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_fence( int     assert,
               MPI_Win win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_FENCE ] );

        return_val = PMPI_Win_fence( assert, win );

/* One-sided communication not handled in first version
    esd_mpi_wincollexit(scorep_mpi_regid[SCOREP__MPI_WIN_FENCE], scorep_mpi_win_rank_id(win));
 */
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_fence( assert, win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_LOCK ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_lock
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_lock( int     lock_type,
              int     rank,
              int     assert,
              MPI_Win win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_LOCK ] );

/* One-sided communication not handled in first version
    esd_mpi_win_lock( rank, scorep_mpi_win_rank_id(win), lock_type==MPI_LOCK_EXCLUSIVE);
 */
        return_val = PMPI_Win_lock( lock_type, rank,
                                    assert, win );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_LOCK ] );

        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_lock( lock_type, rank,
                                    assert, win );
    }
    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_POST ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_post
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_post( MPI_Group group,
              int       assert,
              MPI_Win   win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_POST ] );

/* One-sided communication not handled in first version
    scorep_mpi_winacc_start(win, group, 0);
 */
        return_val = PMPI_Win_post( group, assert, win );

/* One-sided communication not handled in first version
    esd_mpi_winexit(scorep_mpi_regid[SCOREP__MPI_WIN_POST],
                    scorep_mpi_win_rank_id(win), scorep_mpi_winacc_get_gid(win, 0), 0);
 */
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_post( group, assert, win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_START ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_start
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_start( MPI_Group group,
               int       assert,
               MPI_Win   win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_START ] );

/* One-sided communication not handled in first version
    scorep_mpi_winacc_start(win, group, 1);
 */
        return_val = PMPI_Win_start( group, assert, win );

/* One-sided communication not handled in first version
    esd_mpi_winexit(scorep_mpi_regid[SCOREP__MPI_WIN_START],
                    scorep_mpi_win_rank_id(win), scorep_mpi_winacc_get_gid(win, 1), 0);
 */
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_start( group, assert, win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_TEST ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_test
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_test( MPI_Win win,
              int*    flag )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_TEST ] );

        return_val = PMPI_Win_test( win, flag );

/* One-sided communication not handled in first version
    if (*flag != 0)
    {
      esd_mpi_winexit(scorep_mpi_regid[SCOREP__MPI_WIN_TEST],
                      scorep_mpi_win_rank_id(win), scorep_mpi_winacc_get_gid(win, 0), 1);
      scorep_mpi_winacc_end(win, 0);
    }
    else
    {
      esd_mpi_winexit(scorep_mpi_regid[SCOREP__MPI_WIN_TEST],
                      scorep_mpi_win_rank_id(win), scorep_mpi_winacc_get_gid(win, 0), 0);
    }
 */
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_test( win, flag );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_UNLOCK ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_unlock
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_unlock( int     rank,
                MPI_Win win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_UNLOCK ] );

        return_val = PMPI_Win_unlock( rank, win );
/* One-sided communication not handled in first version
    esd_mpi_win_unlock(rank, scorep_mpi_win_rank_id(win));
 */

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_UNLOCK ] );

        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_unlock( rank, win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_WAIT ) && !defined( SCOREP_MPI_NO_RMA )
/**
 * Measurement wrapper for MPI_Win_wait
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter and exit event.
 */
int
MPI_Win_wait( MPI_Win win )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();

        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_WAIT ] );

        return_val = PMPI_Win_wait( win );

/* One-sided communication not handled in first version
    esd_mpi_winexit(scorep_mpi_regid[SCOREP__MPI_WIN_WAIT],
                    scorep_mpi_win_rank_id(win), scorep_mpi_winacc_get_gid(win, 0), 1);
    scorep_mpi_winacc_end(win, 0);
 */

        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_wait( win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_GET_GROUP ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_group )
/**
 * Measurement wrapper for MPI_Win_get_group
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Group.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma
 * Triggers an enter event and an exit event. Furthermore, a communicator definition
 * event is called between enter and exit.
 */
int
MPI_Win_get_group( MPI_Win    win,
                   MPI_Group* group )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_GROUP ] );
    }

    return_val = PMPI_Win_get_group( win, group );
    if ( *group != MPI_GROUP_NULL )
    {
        scorep_mpi_group_create( *group );
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_GROUP ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif

/**
 * @}
 * @name C wrappers for error handling functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_CALL_ERRHANDLER ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Win_call_errhandler )
/**
 * Measurement wrapper for MPI_Win_call_errhandler
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_err
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_call_errhandler( MPI_Win win,
                         int     errorcode )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_ERR ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CALL_ERRHANDLER ] );

        return_val = PMPI_Win_call_errhandler( win, errorcode );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CALL_ERRHANDLER ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_call_errhandler( win, errorcode );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_CREATE_ERRHANDLER ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Win_create_errhandler )
/**
 * Measurement wrapper for MPI_Win_create_errhandler
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_err
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_create_errhandler( MPI_Win_errhandler_fn* function,
                           MPI_Errhandler*        errhandler )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_ERR ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CREATE_ERRHANDLER ] );

        return_val = PMPI_Win_create_errhandler( function, errhandler );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CREATE_ERRHANDLER ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_create_errhandler( function, errhandler );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_GET_ERRHANDLER ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Win_get_errhandler )
/**
 * Measurement wrapper for MPI_Win_get_errhandler
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_err
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_get_errhandler( MPI_Win         win,
                        MPI_Errhandler* errhandler )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_ERR ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_ERRHANDLER ] );

        return_val = PMPI_Win_get_errhandler( win, errhandler );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_ERRHANDLER ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_get_errhandler( win, errhandler );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_SET_ERRHANDLER ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Win_set_errhandler )
/**
 * Measurement wrapper for MPI_Win_set_errhandler
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_err
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_set_errhandler( MPI_Win        win,
                        MPI_Errhandler errhandler )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_ERR ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_SET_ERRHANDLER ] );

        return_val = PMPI_Win_set_errhandler( win, errhandler );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_SET_ERRHANDLER ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_set_errhandler( win, errhandler );
    }

    return return_val;
}
#endif


/**
 * @}
 * @name C wrappers for external interface functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_CREATE_KEYVAL ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_create_keyval )
/**
 * Measurement wrapper for MPI_Win_create_keyval
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_create_keyval( MPI_Win_copy_attr_function*   win_copy_attr_fn,
                       MPI_Win_delete_attr_function* win_delete_attr_fn,
                       int*                          win_keyval,
                       void*                         extra_state )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CREATE_KEYVAL ] );

        return_val = PMPI_Win_create_keyval( win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_CREATE_KEYVAL ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_create_keyval( win_copy_attr_fn, win_delete_attr_fn, win_keyval, extra_state );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_DELETE_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_delete_attr )
/**
 * Measurement wrapper for MPI_Win_delete_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_delete_attr( MPI_Win win,
                     int     win_keyval )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_DELETE_ATTR ] );

        return_val = PMPI_Win_delete_attr( win, win_keyval );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_DELETE_ATTR ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_delete_attr( win, win_keyval );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_FREE_KEYVAL ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_free_keyval )
/**
 * Measurement wrapper for MPI_Win_free_keyval
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_free_keyval( int* win_keyval )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_FREE_KEYVAL ] );

        return_val = PMPI_Win_free_keyval( win_keyval );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_FREE_KEYVAL ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_free_keyval( win_keyval );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_GET_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_attr )
/**
 * Measurement wrapper for MPI_Win_get_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_get_attr( MPI_Win win,
                  int     win_keyval,
                  void*   attribute_val,
                  int*    flag )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_ATTR ] );

        return_val = PMPI_Win_get_attr( win, win_keyval, attribute_val, flag );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_ATTR ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_get_attr( win, win_keyval, attribute_val, flag );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_GET_NAME ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_get_name )
/**
 * Measurement wrapper for MPI_Win_get_name
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_get_name( MPI_Win win,
                  char*   win_name,
                  int*    resultlen )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_NAME ] );

        return_val = PMPI_Win_get_name( win, win_name, resultlen );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_GET_NAME ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_get_name( win, win_name, resultlen );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_SET_ATTR ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( MPI_Win_set_attr )
/**
 * Measurement wrapper for MPI_Win_set_attr
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_set_attr( MPI_Win win,
                  int     win_keyval,
                  void*   attribute_val )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_SET_ATTR ] );

        return_val = PMPI_Win_set_attr( win, win_keyval, attribute_val );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_SET_ATTR ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_set_attr( win, win_keyval, attribute_val );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_SET_NAME ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_EXT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_set_name )
/**
 * Measurement wrapper for MPI_Win_set_name
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_ext
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
int
MPI_Win_set_name( MPI_Win win,
                  char*   win_name )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_EXT ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_SET_NAME ] );

        return_val = PMPI_Win_set_name( win, win_name );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_SET_NAME ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_set_name( win, win_name );
    }

    return return_val;
}
#endif


/**
 * @}
 * @name C wrappers for handle conversion functions
 * @{
 */
#if HAVE( DECL_PMPI_WIN_C2F ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_c2f )
/**
 * Measurement wrapper for MPI_Win_c2f
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_misc
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
MPI_Fint
MPI_Win_c2f( MPI_Win win )
{
    MPI_Fint return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_C2F ] );

        return_val = PMPI_Win_c2f( win );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_C2F ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_c2f( win );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_WIN_F2C ) && !defined( SCOREP_MPI_NO_RMA ) && !defined( SCOREP_MPI_NO_MISC ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( MPI_Win_f2c )
/**
 * Measurement wrapper for MPI_Win_f2c
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup rma_misc
 * Triggers an enter and exit event.
 * It wraps the me) call with enter and exit events.
 */
MPI_Win
MPI_Win_f2c( MPI_Fint win )
{
    MPI_Win return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_RMA_MISC ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_F2C ] );

        return_val = PMPI_Win_f2c( win );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_WIN_F2C ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Win_f2c( win );
    }

    return return_val;
}
#endif


/**
 * @}
 */
