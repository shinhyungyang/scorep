/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2021
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#include <config.h>

#include "scorep_mpi_logical_timer.h"

#include <services/include/SCOREP_Timer_Ticks.h>
#include <services/include/SCOREP_Timer_Avail.h>
#include <services/include/SCOREP_Timer_Utils.h>

#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <stddef.h> /* For size_t */

#ifdef SCOREP_MPI_LTIMER_CUSTOM_DATATYPE

MPI_Datatype scorep_mpi_ltimer_datatype;
MPI_Op       scorep_mpi_ltimer_op_max;

void
scorep_mpi_ltimer_op_max_fun( void* invec, void* inoutvec, int* len, MPI_Datatype* datatype )
{
    scorep_mpi_ltimer* in    = ( scorep_mpi_ltimer* )invec;
    scorep_mpi_ltimer* inout = ( scorep_mpi_ltimer* )inoutvec;

    for ( int k = 0; k < *len; ++k )
    {
        if ( *in > *inout )
        {
            *inout = *in;
        }
        in++;
        inout++;
    }
}
#else
#define scorep_mpi_ltimer_op_max MPI_MAX
#endif

void
scorep_mpi_ltimer_init()
{
#ifdef SCOREP_MPI_LTIMER_CUSTOM_DATATYPE
    PMPI_Type_dup( MPI_UINT64_T, &scorep_mpi_ltimer_datatype );
    PMPI_Type_commit( &scorep_mpi_ltimer_datatype );
    PMPI_Op_create( &scorep_mpi_ltimer_op_max_fun, 1, &scorep_mpi_ltimer_op_max );
#endif
}

void
scorep_mpi_ltimer_init_thread()
{
#ifdef SCOREP_MPI_LTIMER_CUSTOM_DATATYPE
    /* TODO:
     * Find out wether we need to do something special here
     */
    scorep_mpi_ltimer_init();
#endif
}


bool
scorep_mpi_ltimer_enabled()
{
    extern timer_type scorep_timer;

    if (  scorep_timer == TIMER_LOGICAL
       || scorep_timer == TIMER_LOGICAL_HWCTR_INSTR )
    {
        return true;
    }
    return false;
}


scorep_mpi_ltimer
scorep_mpi_get_ltimer()
{
    return SCOREP_Timer_GetLogical();
}


/**
 * @brief Use with caution: Advance the timer to `max(@p remote_timer, @p cached_timer)`
 *
 * This function exists to avoid duplicate calls to @ref scorep_mpi_get_ltimer when
 * the local time is known already.
 * Do not use if there is any possibility that the local timer could have been updated
 * in the meantime.
 *
 * @param cached_timer Assumed to hold the value of the local timer obtained previously.
 */
scorep_mpi_ltimer
scorep_mpi_forward_ltimer_cached( scorep_mpi_ltimer remote_time,
                                  scorep_mpi_ltimer cached_time )
{
    if ( remote_time > cached_time )
    {
        SCOREP_Timer_SetLogical( remote_time );
        return remote_time;
    }
    return cached_time;
}


scorep_mpi_ltimer
scorep_mpi_forward_ltimer( scorep_mpi_ltimer remote_time )
{
    const scorep_mpi_ltimer local_timer = scorep_mpi_get_ltimer();
    return scorep_mpi_forward_ltimer_cached( remote_time, local_timer );
}


void
scorep_mpi_ltimer_send( int      dest,
                        MPI_Comm comm )
{
    const scorep_mpi_ltimer local_time = scorep_mpi_get_ltimer();
    PMPI_Send( &local_time, 1, scorep_mpi_ltimer_datatype, dest, scorep_mpi_ltimer_tag, comm );
}


scorep_mpi_ltimer
scorep_mpi_ltimer_recv( int      source,
                        MPI_Comm comm )
{
    scorep_mpi_ltimer remote_time = 0;
    PMPI_Recv( &remote_time, 1, scorep_mpi_ltimer_datatype, source, scorep_mpi_ltimer_tag, comm, MPI_STATUS_IGNORE );
    return scorep_mpi_forward_ltimer( remote_time );
}


scorep_mpi_ltimer
scorep_mpi_ltimer_bcast( int      root,
                         MPI_Comm comm )
{
    const scorep_mpi_ltimer local_time  = scorep_mpi_get_ltimer();
    scorep_mpi_ltimer       remote_time = local_time;
    PMPI_Bcast( &remote_time, 1, scorep_mpi_ltimer_datatype, root, comm );
    return scorep_mpi_forward_ltimer_cached( remote_time, local_time );
}


scorep_mpi_ltimer
scorep_mpi_ltimer_reduce( int      root,
                          MPI_Comm comm )
{
    const scorep_mpi_ltimer local_time = scorep_mpi_get_ltimer();
    scorep_mpi_ltimer       max_time   = 0;
    PMPI_Reduce( &local_time, &max_time, 1, scorep_mpi_ltimer_datatype, scorep_mpi_ltimer_op_max, root, comm );

    int rank;
    PMPI_Comm_rank( comm, &rank );
    if ( rank == root )
    {
        return scorep_mpi_forward_ltimer_cached( max_time, local_time );
    }
    return local_time;
}

scorep_mpi_ltimer
scorep_mpi_ltimer_allreduce( MPI_Comm comm )
{
    const scorep_mpi_ltimer local_time = scorep_mpi_get_ltimer();
    scorep_mpi_ltimer       max_time   = 0;
    PMPI_Allreduce( &local_time, &max_time, 1, scorep_mpi_ltimer_datatype, scorep_mpi_ltimer_op_max, comm );
    return scorep_mpi_forward_ltimer_cached( max_time, local_time );
}
