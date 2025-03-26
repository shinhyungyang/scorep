/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
/*
 * Functions to be called from Fortran (scorep_mpi_coll_h.F90) via the bind(c) interface
 * 1. convert F90 integer handles to C handle types ( via e.g. PMPI_Comm_f2c )
 * 2. call the C implementation
 */

void
COUNT_FUN( scorep_mpi_coll_bytes_barrier_fromF08 )( MPI_Fint* comm,
                                                    uint64_t* sendbytes,
                                                    uint64_t* recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_barrier )( PMPI_Comm_f2c( *comm ),
                                                sendbytes,
                                                recvbytes );
}


void
COUNT_FUN( scorep_mpi_coll_bytes_gather_fromF08 )(  const COUNT_T*  sendcount,
                                                    const MPI_Fint* sendtype,
                                                    const COUNT_T*  recvcount,
                                                    const MPI_Fint* recvtype,
                                                    const MPI_Fint* root,
                                                    const bool*     inplace,
                                                    const MPI_Fint* comm,
                                                    uint64_t*       sendbytes,
                                                    uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_gather )( *sendcount,
                                               PMPI_Type_f2c( *sendtype ),
                                               *recvcount,
                                               PMPI_Type_f2c( *recvtype ),
                                               *root,
                                               *inplace,
                                               PMPI_Comm_f2c( *comm ),
                                               sendbytes,
                                               recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_reduce_fromF08 )( const COUNT_T*  count,
                                                   const MPI_Fint* datatype,
                                                   const MPI_Fint* root,
                                                   const bool*     inplace,
                                                   const MPI_Fint* comm,
                                                   uint64_t*       sendbytes,
                                                   uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_reduce )( *count,
                                               PMPI_Type_f2c( *datatype ),
                                               *root,
                                               *inplace,
                                               PMPI_Comm_f2c( *comm ),
                                               sendbytes,
                                               recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_gatherv_fromF08 )( const COUNT_T*  sendcount,
                                                    const MPI_Fint* sendtype,
                                                    const COUNT_T*  recvcounts,
                                                    const MPI_Fint* recvtype,
                                                    const MPI_Fint* root,
                                                    const bool*     inplace,
                                                    const MPI_Fint* comm,
                                                    uint64_t*       sendbytes,
                                                    uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_gatherv )( *sendcount,
                                                PMPI_Type_f2c( *sendtype ),
                                                recvcounts,
                                                PMPI_Type_f2c( *recvtype ),
                                                *root,
                                                *inplace,
                                                PMPI_Comm_f2c( *comm ),
                                                sendbytes,
                                                recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_bcast_fromF08 )( const COUNT_T*  count,
                                                  const MPI_Fint* datatype,
                                                  const MPI_Fint* root,
                                                  const MPI_Fint* comm,
                                                  uint64_t*       sendbytes,
                                                  uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_bcast )( *count,
                                              PMPI_Type_f2c( *datatype ),
                                              *root,
                                              PMPI_Comm_f2c( *comm ),
                                              sendbytes,
                                              recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_scatter_fromF08 )(  const COUNT_T*  sendcount,
                                                     const MPI_Fint* sendtype,
                                                     const COUNT_T*  recvcount,
                                                     const MPI_Fint* recvtype,
                                                     const MPI_Fint* root,
                                                     const bool*     inplace,
                                                     const MPI_Fint* comm,
                                                     uint64_t*       sendbytes,
                                                     uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_scatter )( *sendcount,
                                                PMPI_Type_f2c( *sendtype ),
                                                *recvcount,
                                                PMPI_Type_f2c( *recvtype ),
                                                *root,
                                                *inplace,
                                                PMPI_Comm_f2c( *comm ),
                                                sendbytes,
                                                recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_scatterv_fromF08 )( const COUNT_T*  sendcounts,
                                                     const MPI_Fint* sendtype,
                                                     const COUNT_T*  recvcount,
                                                     const MPI_Fint* recvtype,
                                                     const MPI_Fint* root,
                                                     const bool*     inplace,
                                                     const MPI_Fint* comm,
                                                     uint64_t*       sendbytes,
                                                     uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_scatterv )( sendcounts,
                                                 PMPI_Type_f2c( *sendtype ),
                                                 *recvcount,
                                                 PMPI_Type_f2c( *recvtype ),
                                                 *root,
                                                 *inplace,
                                                 PMPI_Comm_f2c( *comm ),
                                                 sendbytes,
                                                 recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_alltoall_fromF08 )( const COUNT_T*  sendcount,
                                                     const MPI_Fint* sendtype,
                                                     const COUNT_T*  recvcount,
                                                     const MPI_Fint* recvtype,
                                                     const bool*     inplace,
                                                     const MPI_Fint* comm,
                                                     uint64_t*       sendbytes,
                                                     uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_alltoall )( *sendcount,
                                                 PMPI_Type_f2c( *sendtype ),
                                                 *recvcount,
                                                 PMPI_Type_f2c( *recvtype ),
                                                 *inplace,
                                                 PMPI_Comm_f2c( *comm ),
                                                 sendbytes,
                                                 recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_alltoallv_fromF08 )( const COUNT_T*  sendcounts,
                                                      const MPI_Fint* sendtype,
                                                      const COUNT_T*  recvcounts,
                                                      const MPI_Fint* recvtype,
                                                      const bool*     inplace,
                                                      const MPI_Fint* comm,
                                                      uint64_t*       sendbytes,
                                                      uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_alltoallv )( sendcounts,
                                                  PMPI_Type_f2c( *sendtype ),
                                                  recvcounts,
                                                  PMPI_Type_f2c( *recvtype ),
                                                  *inplace,
                                                  PMPI_Comm_f2c( *comm ),
                                                  sendbytes,
                                                  recvbytes );
}


void
COUNT_FUN( scorep_mpi_coll_bytes_alltoallw_fromF08 )( const COUNT_T*  sendcounts,
                                                      const MPI_Fint* sendtypes,
                                                      const COUNT_T*  recvcounts,
                                                      const MPI_Fint* recvtypes,
                                                      const bool*     inplace,
                                                      const MPI_Fint* comm,
                                                      uint64_t*       sendbytes,
                                                      uint64_t*       recvbytes )
{
    int size;
    PMPI_Comm_size( PMPI_Comm_f2c( *comm ), &size );
    /* TODO:
     * Make sendtypes_c, recvtypes_c global arrays (allocated with SCOREP_AllocForMisc)
     * and grow them similar to the arrays in request_mgmt.
     */
    /* TODO:
     * Detect when PMPI_Type_f2c is a do-nothing macro (i.e. MPI_Datatype is
     * equivalent to MPI_Fint already). In this case, we can just pass the type
     * arrays directly without conversion.
     */
    MPI_Datatype sendtypes_c[ size ], recvtypes_c[ size ];
    for ( int rank = 0; rank < size; ++rank )
    {
        sendtypes_c[ rank ] = PMPI_Type_f2c( sendtypes[ rank ] );
        recvtypes_c[ rank ] = PMPI_Type_f2c( recvtypes[ rank ] );
    }
    COUNT_FUN( scorep_mpi_coll_bytes_alltoallw )( sendcounts,
                                                  sendtypes_c,
                                                  recvcounts,
                                                  recvtypes_c,
                                                  *inplace,
                                                  PMPI_Comm_f2c( *comm ),
                                                  sendbytes,
                                                  recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_allgather_fromF08 )( const COUNT_T*  sendcount,
                                                      const MPI_Fint* sendtype,
                                                      const COUNT_T*  recvcount,
                                                      const MPI_Fint* recvtype,
                                                      const bool*     inplace,
                                                      const MPI_Fint* comm,
                                                      uint64_t*       sendbytes,
                                                      uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_allgather )( *sendcount,
                                                  PMPI_Type_f2c( *sendtype ),
                                                  *recvcount,
                                                  PMPI_Type_f2c( *recvtype ),
                                                  *inplace,
                                                  PMPI_Comm_f2c( *comm ),
                                                  sendbytes,
                                                  recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_allgatherv_fromF08 )( const COUNT_T*  sendcount,
                                                       const MPI_Fint* sendtype,
                                                       const COUNT_T*  recvcounts,
                                                       const MPI_Fint* recvtype,
                                                       const bool*     inplace,
                                                       const MPI_Fint* comm,
                                                       uint64_t*       sendbytes,
                                                       uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_allgatherv )( *sendcount,
                                                   PMPI_Type_f2c( *sendtype ),
                                                   recvcounts,
                                                   PMPI_Type_f2c( *recvtype ),
                                                   *inplace,
                                                   PMPI_Comm_f2c( *comm ),
                                                   sendbytes,
                                                   recvbytes );
}


void
COUNT_FUN( scorep_mpi_coll_bytes_allreduce_fromF08 )( const COUNT_T*  count,
                                                      const MPI_Fint* datatype,
                                                      const bool*     inplace,
                                                      const MPI_Fint* comm,
                                                      uint64_t*       sendbytes,
                                                      uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_allreduce )( *count,
                                                  PMPI_Type_f2c( *datatype ),
                                                  *inplace,
                                                  PMPI_Comm_f2c( *comm ),
                                                  sendbytes,
                                                  recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_reduce_scatter_block_fromF08 )( const COUNT_T*  recvcount,
                                                                 const MPI_Fint* datatype,
                                                                 const bool*     inplace,
                                                                 const MPI_Fint* comm,
                                                                 uint64_t*       sendbytes,
                                                                 uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_reduce_scatter_block )( *recvcount,
                                                             PMPI_Type_f2c( *datatype ),
                                                             *inplace,
                                                             PMPI_Comm_f2c( *comm ),
                                                             sendbytes,
                                                             recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_reduce_scatter_fromF08 )( const COUNT_T*  recvcounts,
                                                           const MPI_Fint* datatype,
                                                           const bool*     inplace,
                                                           const MPI_Fint* comm,
                                                           uint64_t*       sendbytes,
                                                           uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_reduce_scatter )( recvcounts,
                                                       PMPI_Type_f2c( *datatype ),
                                                       *inplace,
                                                       PMPI_Comm_f2c( *comm ),
                                                       sendbytes,
                                                       recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_scan_fromF08 )( const COUNT_T*  count,
                                                 const MPI_Fint* datatype,
                                                 const bool*     inplace,
                                                 const MPI_Fint* comm,
                                                 uint64_t*       sendbytes,
                                                 uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_scan )( *count,
                                             PMPI_Type_f2c( *datatype ),
                                             *inplace,
                                             PMPI_Comm_f2c( *comm ),
                                             sendbytes,
                                             recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_exscan_fromF08 )( const COUNT_T*  count,
                                                   const MPI_Fint* datatype,
                                                   const bool*     inplace,
                                                   const MPI_Fint* comm,
                                                   uint64_t*       sendbytes,
                                                   uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_exscan )( *count,
                                               PMPI_Type_f2c( *datatype ),
                                               *inplace,
                                               PMPI_Comm_f2c( *comm ),
                                               sendbytes,
                                               recvbytes );
}


void
COUNT_FUN( scorep_mpi_coll_bytes_neighbor_alltoall_fromF08 )( const COUNT_T*  sendcount,
                                                              const MPI_Fint* sendtype,
                                                              const COUNT_T*  recvcount,
                                                              const MPI_Fint* recvtype,
                                                              const MPI_Fint* comm,
                                                              uint64_t*       sendbytes,
                                                              uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_neighbor_alltoall )( *sendcount,
                                                          PMPI_Type_f2c( *sendtype ),
                                                          *recvcount,
                                                          PMPI_Type_f2c( *recvtype ),
                                                          PMPI_Comm_f2c( *comm ),
                                                          sendbytes,
                                                          recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_neighbor_alltoallv_fromF08 )( const COUNT_T*  sendcounts,
                                                               const MPI_Fint* sendtype,
                                                               const COUNT_T*  recvcounts,
                                                               const MPI_Fint* recvtype,
                                                               const MPI_Fint* comm,
                                                               uint64_t*       sendbytes,
                                                               uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_neighbor_alltoallv )( sendcounts,
                                                           PMPI_Type_f2c( *sendtype ),
                                                           recvcounts,
                                                           PMPI_Type_f2c( *recvtype ),
                                                           PMPI_Comm_f2c( *comm ),
                                                           sendbytes,
                                                           recvbytes );
}


void
COUNT_FUN( scorep_mpi_coll_bytes_neighbor_alltoallw_fromF08 )( const COUNT_T*  sendcounts,
                                                               const MPI_Fint* sendtypes,
                                                               const COUNT_T*  recvcounts,
                                                               const MPI_Fint* recvtypes,
                                                               const MPI_Fint* comm,
                                                               uint64_t*       sendbytes,
                                                               uint64_t*       recvbytes )
{
    int indegree  = 0;
    int outdegree = 0;
    scorep_mpi_topo_num_neighbors( PMPI_Comm_f2c( *comm ), &indegree, &outdegree );
    /* TODO:
     * make these global arrays (allocated with SCOREP_AllocForMisc) and grow * them similar to the arrays in request_mgmt */
    /* TODO:
     * detect when PMPI_Type_f2c is a do-nothing macro (i.e. MPI_Datatype is equivalent to MPI_Fint already).
     * In this case, we can just pass the type arrays directly without conversion
     */
    MPI_Datatype sendtypes_c[ outdegree ];
    MPI_Datatype recvtypes_c[ indegree ];
    for ( int i = 0; i < outdegree; ++i )
    {
        sendtypes_c[ i ] = PMPI_Type_f2c( sendtypes[ i ] );
    }
    for ( int i = 0; i < indegree; ++i )
    {
        recvtypes_c[ i ] = PMPI_Type_f2c( recvtypes[ i ] );
    }
    COUNT_FUN( scorep_mpi_coll_bytes_neighbor_alltoallw )( sendcounts,
                                                           sendtypes_c,
                                                           recvcounts,
                                                           recvtypes_c,
                                                           PMPI_Comm_f2c( *comm ),
                                                           sendbytes,
                                                           recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_neighbor_allgather_fromF08 )( const COUNT_T*  sendcount,
                                                               const MPI_Fint* sendtype,
                                                               const COUNT_T*  recvcount,
                                                               const MPI_Fint* recvtype,
                                                               const MPI_Fint* comm,
                                                               uint64_t*       sendbytes,
                                                               uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_neighbor_allgather )( *sendcount,
                                                           PMPI_Type_f2c( *sendtype ),
                                                           *recvcount,
                                                           PMPI_Type_f2c( *recvtype ),
                                                           PMPI_Comm_f2c( *comm ),
                                                           sendbytes,
                                                           recvbytes );
}

void
COUNT_FUN( scorep_mpi_coll_bytes_neighbor_allgatherv_fromF08 )( const COUNT_T*  sendcount,
                                                                const MPI_Fint* sendtype,
                                                                const COUNT_T*  recvcounts,
                                                                const MPI_Fint* recvtype,
                                                                const MPI_Fint* comm,
                                                                uint64_t*       sendbytes,
                                                                uint64_t*       recvbytes )
{
    COUNT_FUN( scorep_mpi_coll_bytes_neighbor_allgatherv )( *sendcount,
                                                            PMPI_Type_f2c( *sendtype ),
                                                            recvcounts,
                                                            PMPI_Type_f2c( *recvtype ),
                                                            PMPI_Comm_f2c( *comm ),
                                                            sendbytes,
                                                            recvbytes );
}
