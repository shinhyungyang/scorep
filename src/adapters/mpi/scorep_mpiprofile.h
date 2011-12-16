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


#ifndef SCOREP_MPIPROFILE_H
#define SCOREP_MPIPROFILE_H


/**
 * @file        scorep_mpiprofile.h
 * @maintainer  Yury Oleynik <oleynik@in.tum.de>
 *
 * @brief   Declaration of mpi profiling functions
 *
 * @status ALPHA
 */


#include <SCOREP_Types.h>

#define MPIPROFILER_TIMEPACK_BUFSIZE ( sizeof( long long ) + 1 * sizeof( int ) )

/**
 * MPI asynchronous profiling request tracking addition datastructure
 */

typedef struct scorep_wait_state_tracking_struct
{
    int         send_tp_position_in_pool;
    MPI_Request tp_request;
    int         tp_comm_partner;
    int         tp_tag;
    int         tp_comm_partner_wc;
    int         tp_tag_wc;
}scorep_wait_state_request_tracking;


extern int myrank;

/**
 * @internal
 * Structure to hold the \a MPI_COMM_WORLD duplication used in mpi profiling.
 */
typedef struct scorep_mpiprofile_world_comm_dup_struct
{
    MPI_Group group;                            /** Associated MPI group */
    MPI_Comm  comm;
}scorep_mpiprofile_world_comm_dup;

/**
 * Contains the data of the MPI_COMM_WORLD definition.
 */
extern scorep_mpiprofile_world_comm_dup world_comm_dup;

int
scorep_mpiprofile_get_timepack_from_pool
(
    void** free_buffer,
    int*   index
);

void
scorep_mpiprofile_store_timepack_request_in_pool
(
    MPI_Request request,
    int         position
);

void
scorep_mpiprofile_free_timepack_pool();

void
scorep_mpiprofile_init_timepack
(
    void*    buf,
    uint64_t time
);

int
scorep_mpiprofiling_rank_to_pe
(
    int      rank,
    MPI_Comm comm,
    int*     global_rank
);

void
scorep_mpiprofile_init
(
);

void
scorep_mpiprofile_init_metrics
(
);

void
scorep_mpiprofile_finalize
(
);

void*
scorep_mpiprofile_get_remote_time_packs
(
    int size
);

void*
scorep_mpiprofile_get_remote_time_pack
(
);

void
scorep_mpiprofile_release_local_time_pack
(
    void* local_time_pack
);

void
scorep_mpiprofile_release_remote_time_pack
(
    void* remote_time_pack
);

void
scorep_mpiprofile_release_remote_time_packs
(
    void* remote_time_packs
);

/**
 * Creates time pack buffer containing rank and time stamp
 *
 * @param time time stamp value to be packed.
 * @return pointer to the MPI_PACKED buffer containing time stamp and mpi rank.
 */
void*
scorep_mpiprofile_get_time_pack
(
    uint64_t time
);

/**
 * Evaluates two time packs for p2p communications
 *
 * @param srcTimePack time pack of the sending process.
 * @param dstTimePack time pack of the receiving process.
 */
void
scorep_mpiprofile_eval_1x1_time_packs
(
    void* srcTimePack,
    void* dstTimePack
);

/**
 * Evaluates multiple time packs for Nx1 communications
 *
 * @param timePacks array of time packs.
 * @param size size of the array.
 */
void
scorep_mpiprofile_eval_nx1_time_packs
(
    void* timePacks,
    int   size
);

/**
 * Evaluates multiple time packs for NxN communications
 *
 * @param srcTimePacks array of sorce time packs.
 * @param dstTimePack time pack of the receiving process.
 * @param size size of the srcTimePacks array.
 */
void
scorep_mpiprofile_eval_multi_time_packs
(
    void* srcTimePacks,
    void* dstTimePack,
    int   size
);

/**
 * Evaluates two time stamps of the communicating processes to determine waiting states
 *
 * @param src Rrank of the receive side.
 * @param dst Rank of the destination side.
 * @param sendTime Time stamp on the send side.
 * @param recvTime Time stamp on the receive side.
 */
void
scorep_mpiprofile_eval_time_stamps
(
    int      src,
    int      dst,
    uint64_t sendTime,
    uint64_t recvTime
);

/**
 * Gets threshold value for determining late process wait states for mpi profiling.
 *
 * @return Threshold value.
 */
int64_t
mpiprofiling_get_late_threshold
(
);

/**
 * Sets threshold value for determining late process wait states for mpi profiling.
 *
 * @param newThreshold New threshold value.
 */
void
mpiprofiling_set_late_threshold
(
    int64_t newThreshold
);

#endif /* SCOREP_MPIPROFILE_H */
