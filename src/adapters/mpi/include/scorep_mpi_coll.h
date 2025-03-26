/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_MPI_COLL_H
#define SCOREP_MPI_COLL_H

/**
   @file
   @ingroup    MPI_Wrapper

   @brief Computations of sent and received bytes for collective communication patterns
 */

#include <stdint.h>
#include <mpi.h>


/*
 * Count the number of ingoing and outgoing edges in the topology associated with
 * comm.
 *
 * indegree is the length of the recvcounts argument of MPI_Neighbor_Alltoallv.
 * outdegree is the length of the sendcounts argument of MPI_Neighbor_Alltoallv.
 *
 * For cartesian topologies, this is always 2*#dimensions, i.e., neighbors for
 * which MPI_Cart_shift would return MPI_PROC_NULL are included.
 */
static inline void
scorep_mpi_topo_num_neighbors( MPI_Comm comm, int* indegree, int* outdegree )
{
    int topo_type = MPI_UNDEFINED;
    PMPI_Topo_test( comm, &topo_type );

    switch ( topo_type )
    {
        case MPI_GRAPH:
        {
            int rank, neighbors;
            PMPI_Comm_rank( comm, &rank );
            PMPI_Graph_neighbors_count( comm, rank, &neighbors );
            *indegree  = neighbors;
            *outdegree = neighbors;
            break;
        }
        case MPI_CART:
        {
            int ndims;
            PMPI_Cartdim_get( comm, &ndims );
            *indegree  = 2 * ndims;
            *outdegree = 2 * ndims;
            break;
        }
        case MPI_DIST_GRAPH:
        {
            int weighted;
            PMPI_Dist_graph_neighbors_count( comm, indegree, outdegree, &weighted );
            break;
        }
        case MPI_UNDEFINED: /* fallthrough */
        default:
        {
            indegree  = 0;
            outdegree = 0;
            break;
        }
    }
}


/*
 * Patterns for the non-blocking and persistent collectives are identical to the
 * standard ones.
 */


#define COUNT_T int
#define COUNT_FUN( name ) name
#define TYPE_SIZE_FUN PMPI_Type_size

#include "scorep_mpi_coll.inc.h"

#undef TYPE_SIZE_FUN
#undef COUNT_T
#undef COUNT_FUN

/*
 * Large counts
 */
#if HAVE( MPI_4_0_SYMBOL_PMPI_TYPE_SIZE_C )
#define TYPE_SIZE_FUN PMPI_Type_size_c
#elif HAVE( MPI_3_0_SYMBOL_PMPI_TYPE_SIZE_X )
#define TYPE_SIZE_FUN PMPI_Type_size_x
#endif

#if defined( TYPE_SIZE_FUN )

#define COUNT_T MPI_Count
#define COUNT_FUN( name ) name ##_c

#include "scorep_mpi_coll.inc.h"

#undef TYPE_SIZE_FUN
#undef COUNT_T
#undef COUNT_FUN

#endif

#endif /* SCOREP_MPI_COLL_H */
