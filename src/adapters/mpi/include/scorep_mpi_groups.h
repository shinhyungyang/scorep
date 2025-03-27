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
 * Copyright (c) 2009-2019, 2023,
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
 * Copyright (c) 2022,
 * Deutsches Zentrum fuer Luft- und Raumfahrt, Germany
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

#ifndef SCOREP_MPI_GROUPS_H
#define SCOREP_MPI_GROUPS_H


#include <stdint.h>

/** function type is not point-to-point */
#define SCOREP_MPI_TYPE__NONE                          0
/** function type is receive operation */
#define SCOREP_MPI_TYPE__RECV                          1
/** function type is send operation */
#define SCOREP_MPI_TYPE__SEND                          2
/** function type is send and receive operation */
#define SCOREP_MPI_TYPE__SENDRECV                      3
/** function type is collective */
#define SCOREP_MPI_TYPE__COLLECTIVE                    4

/** function has is unknown communication pattern */
#define SCOREP_COLL_TYPE__UNKNOWN                      1
/** function is barrier-like operation */
#define SCOREP_COLL_TYPE__BARRIER                      2
/** function has 1:n communication pattern */
#define SCOREP_COLL_TYPE__ONE2ALL                      3
/** function has n:1 communication pattern */
#define SCOREP_COLL_TYPE__ALL2ONE                      4
/** function has n:n communication pattern */
#define SCOREP_COLL_TYPE__ALL2ALL                      5
/** function may be partially synchronizing */
#define SCOREP_COLL_TYPE__PARTIAL                      6
/** function is implicitly synchronizing */
#define SCOREP_COLL_TYPE__IMPLIED                      7

/** Mappings from MPI collective names to Score-P @{ */
#define SCOREP_MPI_COLLECTIVE__MPI_BARRIER              SCOREP_COLLECTIVE_BARRIER
#define SCOREP_MPI_COLLECTIVE__MPI_BCAST                SCOREP_COLLECTIVE_BROADCAST
#define SCOREP_MPI_COLLECTIVE__MPI_GATHER               SCOREP_COLLECTIVE_GATHER
#define SCOREP_MPI_COLLECTIVE__MPI_GATHERV              SCOREP_COLLECTIVE_GATHERV
#define SCOREP_MPI_COLLECTIVE__MPI_SCATTER              SCOREP_COLLECTIVE_SCATTER
#define SCOREP_MPI_COLLECTIVE__MPI_SCATTERV             SCOREP_COLLECTIVE_SCATTERV
#define SCOREP_MPI_COLLECTIVE__MPI_ALLGATHER            SCOREP_COLLECTIVE_ALLGATHER
#define SCOREP_MPI_COLLECTIVE__MPI_ALLGATHERV           SCOREP_COLLECTIVE_ALLGATHERV
#define SCOREP_MPI_COLLECTIVE__MPI_ALLTOALL             SCOREP_COLLECTIVE_ALLTOALL
#define SCOREP_MPI_COLLECTIVE__MPI_ALLTOALLV            SCOREP_COLLECTIVE_ALLTOALLV
#define SCOREP_MPI_COLLECTIVE__MPI_ALLTOALLW            SCOREP_COLLECTIVE_ALLTOALLW
#define SCOREP_MPI_COLLECTIVE__MPI_ALLREDUCE            SCOREP_COLLECTIVE_ALLREDUCE
#define SCOREP_MPI_COLLECTIVE__MPI_REDUCE               SCOREP_COLLECTIVE_REDUCE
#define SCOREP_MPI_COLLECTIVE__MPI_REDUCE_SCATTER       SCOREP_COLLECTIVE_REDUCE_SCATTER
#define SCOREP_MPI_COLLECTIVE__MPI_REDUCE_SCATTER_BLOCK SCOREP_COLLECTIVE_REDUCE_SCATTER_BLOCK
#define SCOREP_MPI_COLLECTIVE__MPI_SCAN                 SCOREP_COLLECTIVE_SCAN
#define SCOREP_MPI_COLLECTIVE__MPI_EXSCAN               SCOREP_COLLECTIVE_EXSCAN

#define SCOREP_MPI_COLLECTIVE__MPI_IBARRIER              SCOREP_COLLECTIVE_BARRIER
#define SCOREP_MPI_COLLECTIVE__MPI_IBCAST                SCOREP_COLLECTIVE_BROADCAST
#define SCOREP_MPI_COLLECTIVE__MPI_IGATHER               SCOREP_COLLECTIVE_GATHER
#define SCOREP_MPI_COLLECTIVE__MPI_IGATHERV              SCOREP_COLLECTIVE_GATHERV
#define SCOREP_MPI_COLLECTIVE__MPI_ISCATTER              SCOREP_COLLECTIVE_SCATTER
#define SCOREP_MPI_COLLECTIVE__MPI_ISCATTERV             SCOREP_COLLECTIVE_SCATTERV
#define SCOREP_MPI_COLLECTIVE__MPI_IALLGATHER            SCOREP_COLLECTIVE_ALLGATHER
#define SCOREP_MPI_COLLECTIVE__MPI_IALLGATHERV           SCOREP_COLLECTIVE_ALLGATHERV
#define SCOREP_MPI_COLLECTIVE__MPI_IALLTOALL             SCOREP_COLLECTIVE_ALLTOALL
#define SCOREP_MPI_COLLECTIVE__MPI_IALLTOALLV            SCOREP_COLLECTIVE_ALLTOALLV
#define SCOREP_MPI_COLLECTIVE__MPI_IALLTOALLW            SCOREP_COLLECTIVE_ALLTOALLW
#define SCOREP_MPI_COLLECTIVE__MPI_IALLREDUCE            SCOREP_COLLECTIVE_ALLREDUCE
#define SCOREP_MPI_COLLECTIVE__MPI_IREDUCE               SCOREP_COLLECTIVE_REDUCE
#define SCOREP_MPI_COLLECTIVE__MPI_IREDUCE_SCATTER       SCOREP_COLLECTIVE_REDUCE_SCATTER
#define SCOREP_MPI_COLLECTIVE__MPI_IREDUCE_SCATTER_BLOCK SCOREP_COLLECTIVE_REDUCE_SCATTER_BLOCK
#define SCOREP_MPI_COLLECTIVE__MPI_ISCAN                 SCOREP_COLLECTIVE_SCAN
#define SCOREP_MPI_COLLECTIVE__MPI_IEXSCAN               SCOREP_COLLECTIVE_EXSCAN

#define SCOREP_MPI_COLLECTIVE__MPI_NEIGHBOR_ALLGATHER    SCOREP_COLLECTIVE_ALLGATHER
#define SCOREP_MPI_COLLECTIVE__MPI_NEIGHBOR_ALLGATHERV   SCOREP_COLLECTIVE_ALLGATHERV
#define SCOREP_MPI_COLLECTIVE__MPI_NEIGHBOR_ALLTOALL     SCOREP_COLLECTIVE_ALLTOALL
#define SCOREP_MPI_COLLECTIVE__MPI_NEIGHBOR_ALLTOALLV    SCOREP_COLLECTIVE_ALLTOALLV
#define SCOREP_MPI_COLLECTIVE__MPI_NEIGHBOR_ALLTOALLW    SCOREP_COLLECTIVE_ALLTOALLW

#define SCOREP_MPI_COLLECTIVE__MPI_INEIGHBOR_ALLGATHER   SCOREP_COLLECTIVE_ALLGATHER
#define SCOREP_MPI_COLLECTIVE__MPI_INEIGHBOR_ALLGATHERV  SCOREP_COLLECTIVE_ALLGATHERV
#define SCOREP_MPI_COLLECTIVE__MPI_INEIGHBOR_ALLTOALL    SCOREP_COLLECTIVE_ALLTOALL
#define SCOREP_MPI_COLLECTIVE__MPI_INEIGHBOR_ALLTOALLV   SCOREP_COLLECTIVE_ALLTOALLV
#define SCOREP_MPI_COLLECTIVE__MPI_INEIGHBOR_ALLTOALLW   SCOREP_COLLECTIVE_ALLTOALLW
/** @} */

/**
 * Bit patterns for runtime wrapper enabling.
 * @note Adding a new derived group requires a change in enable_derived_groups().
 */
enum scorep_mpi_groups
{
    /* pure groups, which can be specified in conf */
    SCOREP_MPI_ENABLED_CG        = 1 << 0,
    SCOREP_MPI_ENABLED_COLL      = 1 << 1,
    SCOREP_MPI_ENABLED_ENV       = 1 << 2,
    SCOREP_MPI_ENABLED_ERR       = 1 << 3,
    SCOREP_MPI_ENABLED_EXT       = 1 << 4,
    SCOREP_MPI_ENABLED_IO        = 1 << 5,
    SCOREP_MPI_ENABLED_MISC      = 1 << 6,
    SCOREP_MPI_ENABLED_P2P       = 1 << 7,
    SCOREP_MPI_ENABLED_RMA       = 1 << 8,
    SCOREP_MPI_ENABLED_SPAWN     = 1 << 9,
    SCOREP_MPI_ENABLED_TOPO      = 1 << 10,
    SCOREP_MPI_ENABLED_TYPE      = 1 << 11,
    SCOREP_MPI_ENABLED_PERF      = 1 << 12,
    SCOREP_MPI_ENABLED_XNONBLOCK = 1 << 13, /* Deprecated in 9.0 */
    SCOREP_MPI_ENABLED_XREQTEST  = 1 << 14,
    /* derived groups, which are a combination of existing groups */
    SCOREP_MPI_ENABLED_CG_ERR    = 1 << 15,
    SCOREP_MPI_ENABLED_CG_EXT    = 1 << 16,
    SCOREP_MPI_ENABLED_CG_MISC   = 1 << 17,
    SCOREP_MPI_ENABLED_IO_ERR    = 1 << 18,
    SCOREP_MPI_ENABLED_IO_MISC   = 1 << 19,
    SCOREP_MPI_ENABLED_REQUEST   = 1 << 20,
    SCOREP_MPI_ENABLED_RMA_ERR   = 1 << 21,
    SCOREP_MPI_ENABLED_RMA_EXT   = 1 << 22,
    SCOREP_MPI_ENABLED_RMA_MISC  = 1 << 23,
    SCOREP_MPI_ENABLED_TYPE_EXT  = 1 << 24,
    SCOREP_MPI_ENABLED_TYPE_MISC = 1 << 25,
    /* NOTE: ALL should comprise all pure groups */
    SCOREP_MPI_ENABLED_ALL       = SCOREP_MPI_ENABLED_CG        |
                                   SCOREP_MPI_ENABLED_COLL      |
                                   SCOREP_MPI_ENABLED_ENV       |
                                   SCOREP_MPI_ENABLED_ERR       |
                                   SCOREP_MPI_ENABLED_EXT       |
                                   SCOREP_MPI_ENABLED_IO        |
                                   SCOREP_MPI_ENABLED_MISC      |
                                   SCOREP_MPI_ENABLED_P2P       |
                                   SCOREP_MPI_ENABLED_RMA       |
                                   SCOREP_MPI_ENABLED_SPAWN     |
                                   SCOREP_MPI_ENABLED_TOPO      |
                                   SCOREP_MPI_ENABLED_TYPE      |
                                   SCOREP_MPI_ENABLED_PERF      |
                                   SCOREP_MPI_ENABLED_XREQTEST,
    /* NOTE: DEFAULT should reflect the default set */
    SCOREP_MPI_ENABLED_DEFAULT = SCOREP_MPI_ENABLED_CG    |
                                 SCOREP_MPI_ENABLED_COLL  |
                                 SCOREP_MPI_ENABLED_ENV   |
                                 SCOREP_MPI_ENABLED_IO    |
                                 SCOREP_MPI_ENABLED_P2P   |
                                 SCOREP_MPI_ENABLED_RMA   |
                                 SCOREP_MPI_ENABLED_TOPO
};

/** Bit vector for runtime measurement wrapper enabling/disabling */
extern uint64_t scorep_mpi_enabled;

/**
 * Detect erroneous compile-time disablement of request group
 */
#if defined( SCOREP_MPI_NO_REQUEST )
    #error SCOREP_MPI_NO_REQUEST must not be set manually. Please remove define from the compile line.
#elif defined( SCOREP_MPI_NO_CG ) \
    && defined( SCOREP_MPI_NO_COLL ) \
    && defined( SCOREP_MPI_NO_EXT ) \
    && defined( SCOREP_MPI_NO_IO ) \
    && defined( SCOREP_MPI_NO_P2P ) \
    && defined( SCOREP_MPI_NO_RMA )
    #define SCOREP_MPI_NO_REQUEST
#endif

void
scorep_mpi_enable_derived_groups( void );

#endif /* SCOREP_MPI_GROUPS_H */
