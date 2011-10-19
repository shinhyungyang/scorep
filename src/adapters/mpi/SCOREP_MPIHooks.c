/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen University, Germany
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
 * @file        SCOREP_MPIHooks.c
 * @maintainer  Yury Olenyik <oleynik@in.tum.de>
 *
 * @brief   mpi hooks functions to be used by MPI profiling
 *
 * @status alpha
 *
 */

#include <config.h>

#include "SCOREP_MPIHooks.h"
#include "scorep_utility/SCOREP_Debug.h"
#include "scorep_mpiprofile.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*----------------------------------------------
 * 1x1 pre- and post- communication hooks
   -----------------------------------------------*/

bool scorep_hooks_on = false;

/**
 * Pre-communication hook for MPI_Send
 */
void
SCOREP_Hooks_Pre_MPI_Send
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    uint64_t     start_time_stamp
)
{
}

/**
 * Post-communication hook for MPI_Send
 */
void
SCOREP_Hooks_Post_MPI_Send
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_value
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    PMPI_Send(  localTimePack,
                MPIPROFILER_TIMEPACK_BUFSIZE,
                MPI_PACKED,
                dest,
                tag,
                comm );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Bsend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    PMPI_Send(  localTimePack,
                MPIPROFILER_TIMEPACK_BUFSIZE,
                MPI_PACKED,
                dest,
                tag,
                comm );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Ssend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    PMPI_Send(  localTimePack,
                MPIPROFILER_TIMEPACK_BUFSIZE,
                MPI_PACKED,
                dest,
                tag,
                comm );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Rsend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    PMPI_Send(  localTimePack,
                MPIPROFILER_TIMEPACK_BUFSIZE,
                MPI_PACKED,
                dest,
                tag,
                comm );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

/**
 * Pre-communication hook for MPI_Recv
 */
void
SCOREP_Hooks_Pre_MPI_Recv
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          source,
    int          tag,
    MPI_Comm     comm,
    MPI_Status*  status,
    uint64_t     start_time_stamp
)
{
}

/**
 * Post-communication hook for MPI_Recv
 */
void
SCOREP_Hooks_Post_MPI_Recv
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          source,
    int          tag,
    MPI_Comm     comm,
    MPI_Status*  status,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    source = status->MPI_SOURCE;
    tag    = status->MPI_TAG;
    void*      remoteTimePack = scorep_mpiprofile_get_remote_time_pack();
    MPI_Status s;
    PMPI_Recv(      remoteTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    source,
                    tag,
                    comm,
                    &s );

    scorep_mpiprofile_eval_1x1_time_packs( remoteTimePack,
                                           localTimePack );
    scorep_mpiprofile_release_remote_time_pack( remoteTimePack );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Isend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    MPI_Request* request,
    int64_t      start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Issend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    MPI_Request* request,
    int64_t      start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Ibsend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    MPI_Request* request,
    int64_t      start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Irsend
(
    void*        buf,
    int          count,
    MPI_Datatype datatype,
    int          dest,
    int          tag,
    MPI_Comm     comm,
    MPI_Request* request,
    int64_t      start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Exscan
(
    void*        sendbuf,
    void*        recvbuf,
    int          count,
    MPI_Datatype datatype,
    MPI_Op       op,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Alltoallw
(
    void*        sendbuf,
    int          sendcounts[],
    int          sdispls[],
    MPI_Datatype sendtypes[],
    void*        recvbuf,
    int          recvcounts[],
    int          rdispls[],
    MPI_Datatype recvtypes[],
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Reduce_scatter
(
    void*        sendbuf,
    void*        recvbuf,
    int*         recvcounts,
    MPI_Datatype datatype,
    MPI_Op       op,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Reduce_scatter_block
(
    void*        sendbuf,
    void*        recvbuf,
    int          recvcount,
    MPI_Datatype datatype,
    MPI_Op       op,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Scan
(
    void*        sendbuf,
    void*        recvbuf,
    int          count,
    MPI_Datatype datatype,
    MPI_Op       op,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
}

void
SCOREP_Hooks_Post_MPI_Scatterv
(
    void*        sendbuf,
    int*         sendcounts,
    int*         displs,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int          recvcount,
    MPI_Datatype recvtype,
    int          root,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val

)
{
}
/*----------------------------------------------
 * NxN pre- and post- communication hooks
   -----------------------------------------------*/

void
SCOREP_Hooks_Post_MPI_Alltoall
(
    void*        sendbuf,
    int          sendcount,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int          recvcount,
    MPI_Datatype recvtype,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    MPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );
    PMPI_Allgather( localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE, \
                    MPI_PACKED,
                    comm );
    scorep_mpiprofile_eval_multi_time_packs(  remoteTimePacks,
                                              localTimePack,
                                              commSize );

    scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Alltoallv
(
    void*        sendbuf,
    int*         sendcounts,
    int*         sdispls,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int*         recvcounts,
    int*         rdispls,
    MPI_Datatype recvtype,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    MPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );
    PMPI_Allgather( localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE, \
                    MPI_PACKED,
                    comm );
    scorep_mpiprofile_eval_multi_time_packs(  remoteTimePacks,
                                              localTimePack,
                                              commSize );

    scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Barrier
(
    MPI_Comm comm,
    uint64_t start_time_stamp,
    int      return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    MPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );
    PMPI_Allgather( localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE, \
                    MPI_PACKED,
                    comm );
    scorep_mpiprofile_eval_multi_time_packs(  remoteTimePacks,
                                              localTimePack,
                                              commSize );
    scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

/*----------------------------------------------
 * Nx1 pre- and post- communication hooks
   -----------------------------------------------*/

/**
 * Post-communication hook for MPI_Recv
 */
void
SCOREP_Hooks_Post_MPI_Gather
(
    void*        sendbuf,
    int          sendcount,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int          recvcount,
    MPI_Datatype recvtype,
    int          root,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    if ( myrank == root )
    {
        PMPI_Comm_size( comm, &commSize );
        remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );
    }
    PMPI_Gather(    localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    root,
                    comm );

    if ( myrank == root )
    {
        scorep_mpiprofile_eval_nx1_time_packs( remoteTimePacks,
                                               commSize );
        scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    }
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Gatherv
(
    void*        sendbuf,
    int          sendcount,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int*         recvcounts,
    int*         displs,
    MPI_Datatype recvtype,
    int          root,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    if ( myrank == root )
    {
        PMPI_Comm_size( comm, &commSize );
        remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );
    }
    PMPI_Gather(    localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    root,
                    comm );

    if ( myrank == root )
    {
        scorep_mpiprofile_eval_nx1_time_packs( remoteTimePacks,
                                               commSize );
        scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    }
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Reduce
(
    void*        sendbuf,
    void*        recvbuf,
    int          count,
    MPI_Datatype datatype,
    MPI_Op       op,
    int          root,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    PMPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );

    PMPI_Gather(    localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    root,
                    comm );

    if ( myrank == root )
    {
        scorep_mpiprofile_eval_nx1_time_packs( remoteTimePacks,
                                               commSize );
        scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    }
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Allreduce
(
    void*        sendbuf,
    void*        recvbuf,
    int          count,
    MPI_Datatype datatype,
    MPI_Op       op,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    PMPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );

    PMPI_Allgather( localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    comm );


    scorep_mpiprofile_eval_multi_time_packs(  remoteTimePacks,
                                              localTimePack,
                                              commSize );
    scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );

    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Allgather
(
    void*        sendbuf,
    int          sendcount,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int          recvcount,
    MPI_Datatype recvtype,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    PMPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );

    PMPI_Allgather( localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    comm );


    scorep_mpiprofile_eval_multi_time_packs(  remoteTimePacks,
                                              localTimePack,
                                              commSize );
    scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );

    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Allgatherv
(
    void*        sendbuf,
    int          sendcount,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int*         recvcounts,
    int*         displs,
    MPI_Datatype recvtype,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePacks;
    int   commSize;
    PMPI_Comm_size( comm, &commSize );
    remoteTimePacks = scorep_mpiprofile_get_remote_time_packs( commSize );

    PMPI_Allgather( localTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    remoteTimePacks,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    comm );


    scorep_mpiprofile_eval_multi_time_packs(  remoteTimePacks,
                                              localTimePack,
                                              commSize );
    scorep_mpiprofile_release_remote_time_packs( remoteTimePacks );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

/*----------------------------------------------
 * 1xN pre- and post- communication hooks
   -----------------------------------------------*/

void
SCOREP_Hooks_Post_MPI_Bcast
(
    void*        buffer,
    int          count,
    MPI_Datatype datatype,
    int          root,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );

    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePack;
    remoteTimePack = scorep_mpiprofile_get_remote_time_pack();
    memcpy( remoteTimePack, localTimePack, MPIPROFILER_TIMEPACK_BUFSIZE );
    PMPI_Bcast(     remoteTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    root,
                    comm );


    if ( myrank != root )
    {
        scorep_mpiprofile_eval_1x1_time_packs( remoteTimePack,
                                               localTimePack );
    }
    scorep_mpiprofile_release_remote_time_pack( remoteTimePack );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}

void
SCOREP_Hooks_Post_MPI_Scatter
(
    void*        sendbuf,
    int          sendcount,
    MPI_Datatype sendtype,
    void*        recvbuf,
    int          recvcount,
    MPI_Datatype recvtype,
    int          root,
    MPI_Comm     comm,
    uint64_t     start_time_stamp,
    int          return_val
)
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPIPROFILING, "HOOK : myrank = %d,%s", myrank, __FUNCTION__ );
    void* localTimePack = scorep_mpiprofile_get_time_pack( start_time_stamp );
    void* remoteTimePack;
    remoteTimePack = scorep_mpiprofile_get_remote_time_pack();
    memcpy( remoteTimePack, localTimePack, MPIPROFILER_TIMEPACK_BUFSIZE );
    PMPI_Bcast(     remoteTimePack,
                    MPIPROFILER_TIMEPACK_BUFSIZE,
                    MPI_PACKED,
                    root,
                    comm );


    if ( myrank != root )
    {
        scorep_mpiprofile_eval_1x1_time_packs( remoteTimePack,
                                               localTimePack );
    }
    scorep_mpiprofile_release_remote_time_pack( remoteTimePack );
    scorep_mpiprofile_release_local_time_pack( localTimePack );
}
