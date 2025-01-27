!
! This file is part of the Score-P software (http://www.score-p.org)
!
! Copyright (c) 2025,
! Forschungszentrum Juelich GmbH, Germany
!
! This software may be modified and distributed under the terms of
! a BSD-style license. See the COPYING file in the package base
! directory for details.
!

! This file is included twice from scorep_mpi_coll_h.F90 with differing definitions for 'count_kind'
! to provide the default and the large count versions.

    public :: scorep_mpi_coll_bytes_gather, &
              scorep_mpi_coll_bytes_reduce, &
              scorep_mpi_coll_bytes_gatherv, &
              scorep_mpi_coll_bytes_bcast, &
              scorep_mpi_coll_bytes_scatter, &
              scorep_mpi_coll_bytes_scatterv, &
              scorep_mpi_coll_bytes_alltoall, &
              scorep_mpi_coll_bytes_alltoallv, &
              scorep_mpi_coll_bytes_alltoallw, &
              scorep_mpi_coll_bytes_allgather, &
              scorep_mpi_coll_bytes_allgatherv, &
              scorep_mpi_coll_bytes_allreduce, &
              scorep_mpi_coll_bytes_reduce_scatter_block, &
              scorep_mpi_coll_bytes_reduce_scatter, &
              scorep_mpi_coll_bytes_scan, &
              scorep_mpi_coll_bytes_exscan, &
              scorep_mpi_coll_bytes_neighbor_alltoall, &
              scorep_mpi_coll_bytes_neighbor_alltoallv, &
              scorep_mpi_coll_bytes_neighbor_alltoallw, &
              scorep_mpi_coll_bytes_neighbor_allgather, &
              scorep_mpi_coll_bytes_neighbor_allgatherv

    !
    ! Interface to be called from F08 wrappers
    !
    interface scorep_mpi_coll_bytes_gather
        module procedure scorep_mpi_coll_bytes_gather_impl
    end interface

    interface scorep_mpi_coll_bytes_reduce
        module procedure scorep_mpi_coll_bytes_reduce_impl
    end interface

    interface scorep_mpi_coll_bytes_gatherv
        module procedure scorep_mpi_coll_bytes_gatherv_impl
    end interface

    interface scorep_mpi_coll_bytes_bcast
        module procedure scorep_mpi_coll_bytes_bcast_impl
    end interface

    interface scorep_mpi_coll_bytes_scatter
        module procedure scorep_mpi_coll_bytes_scatter_impl
    end interface

    interface scorep_mpi_coll_bytes_scatterv
        module procedure scorep_mpi_coll_bytes_scatterv_impl
    end interface

    interface scorep_mpi_coll_bytes_alltoall
        module procedure scorep_mpi_coll_bytes_alltoall_impl
    end interface

    interface scorep_mpi_coll_bytes_alltoallv
        module procedure scorep_mpi_coll_bytes_alltoallv_impl
    end interface

    interface scorep_mpi_coll_bytes_alltoallw
        module procedure scorep_mpi_coll_bytes_alltoallw_impl
    end interface

    interface scorep_mpi_coll_bytes_allgather
        module procedure scorep_mpi_coll_bytes_allgather_impl
    end interface

    interface scorep_mpi_coll_bytes_allgatherv
        module procedure scorep_mpi_coll_bytes_allgatherv_impl
    end interface

    interface scorep_mpi_coll_bytes_allreduce
        module procedure scorep_mpi_coll_bytes_allreduce_impl
    end interface

    interface scorep_mpi_coll_bytes_reduce_scatter_block
        module procedure scorep_mpi_coll_bytes_reduce_scatter_block_impl
    end interface

    interface scorep_mpi_coll_bytes_reduce_scatter
        module procedure scorep_mpi_coll_bytes_reduce_scatter_impl
    end interface

    interface scorep_mpi_coll_bytes_scan
        module procedure scorep_mpi_coll_bytes_scan_impl
    end interface

    interface scorep_mpi_coll_bytes_exscan
        module procedure scorep_mpi_coll_bytes_exscan_impl
    end interface

    interface scorep_mpi_coll_bytes_neighbor_alltoall
        module procedure scorep_mpi_coll_bytes_neighbor_alltoall_impl
    end interface

    interface scorep_mpi_coll_bytes_neighbor_alltoallv
        module procedure scorep_mpi_coll_bytes_neighbor_alltoallv_impl
    end interface

    interface scorep_mpi_coll_bytes_neighbor_alltoallw
        module procedure scorep_mpi_coll_bytes_neighbor_alltoallw_impl
    end interface

    interface scorep_mpi_coll_bytes_neighbor_allgather
        module procedure scorep_mpi_coll_bytes_neighbor_allgather_impl
    end interface

    interface scorep_mpi_coll_bytes_neighbor_allgatherv
        module procedure scorep_mpi_coll_bytes_neighbor_allgatherv_impl
    end interface

    !
    ! Implementation of F08 functions
    !
    contains

    subroutine scorep_mpi_coll_bytes_gather_impl(sendcount, &
                                                 sendtype, &
                                                 recvcount, &
                                                 recvtype, &
                                                 root, &
                                                 inplace, &
                                                 comm, &
                                                 sendbytes, &
                                                 recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        integer, intent(in) :: root
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_gather_toC(sendcount, &
                                              sendtype%MPI_VAL, &
                                              recvcount, &
                                              recvtype%MPI_VAL, &
                                              root, &
                                              inplace, &
                                              comm%MPI_VAL, &
                                              sendbytes, &
                                              recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_reduce_impl(count, &
                                                 datatype, &
                                                 root, &
                                                 inplace, &
                                                 comm, &
                                                 sendbytes, &
                                                 recvbytes)
        integer(count_kind) :: count
        type(MPI_Datatype) :: datatype
        integer :: root
        logical(c_bool) :: inplace
        type(MPI_Comm) :: comm
        integer(c_int64_t) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_reduce_toC(count, &
                                              datatype%MPI_VAL, &
                                              root, &
                                              inplace, &
                                              comm%MPI_VAL, &
                                              sendbytes, &
                                              recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_gatherv_impl(sendcount, &
                                                  sendtype, &
                                                  recvcounts, &
                                                  recvtype, &
                                                  root, &
                                                  inplace, &
                                                  comm, &
                                                  sendbytes, &
                                                  recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        integer, intent(in) :: root
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_gatherv_toC(sendcount, &
                                               sendtype%MPI_VAL, &
                                               recvcounts, &
                                               recvtype%MPI_VAL, &
                                               root, &
                                               inplace, &
                                               comm%MPI_VAL, &
                                               sendbytes, &
                                               recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_bcast_impl(count, &
                                                datatype, &
                                                root, &
                                                comm, &
                                                sendbytes, &
                                                recvbytes)
        integer(count_kind), intent(in) :: count
        type(MPI_Datatype) :: datatype
        integer :: root
        type(MPI_Comm) :: comm
        integer(c_int64_t) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_bcast_toC(count, &
                                             datatype%MPI_VAL, &
                                             root, &
                                             comm%MPI_VAL, &
                                             sendbytes, &
                                             recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_scatter_impl(sendcount, &
                                                  sendtype, &
                                                  recvcount, &
                                                  recvtype, &
                                                  root, &
                                                  inplace, &
                                                  comm, &
                                                  sendbytes, &
                                                  recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        integer, intent(in) :: root
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_scatter_toC(sendcount, &
                                               sendtype%MPI_VAL, &
                                               recvcount, &
                                               recvtype%MPI_VAL, &
                                               root, &
                                               inplace, &
                                               comm%MPI_VAL, &
                                               sendbytes, &
                                               recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_scatterv_impl(sendcounts, &
                                                   sendtype, &
                                                   recvcount, &
                                                   recvtype, &
                                                   root, &
                                                   inplace, &
                                                   comm, &
                                                   sendbytes, &
                                                   recvbytes)
        integer(count_kind), intent(in) :: sendcounts(*), recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        integer, intent(in) :: root
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_scatterv_toC(sendcounts, &
                                                sendtype%MPI_VAL, &
                                                recvcount, &
                                                recvtype%MPI_VAL, &
                                                root, &
                                                inplace, &
                                                comm%MPI_VAL, &
                                                sendbytes, &
                                                recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_alltoall_impl(sendcount, &
                                                   sendtype, &
                                                   recvcount, &
                                                   recvtype, &
                                                   inplace, &
                                                   comm, &
                                                   sendbytes, &
                                                   recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_alltoall_toC(sendcount, &
                                                sendtype%MPI_VAL, &
                                                recvcount, &
                                                recvtype%MPI_VAL, &
                                                inplace, &
                                                comm%MPI_VAL, &
                                                sendbytes, &
                                                recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_alltoallv_impl(sendcounts, &
                                                    sendtype, &
                                                    recvcounts, &
                                                    recvtype, &
                                                    inplace, &
                                                    comm, &
                                                    sendbytes, &
                                                    recvbytes)
        integer(count_kind), intent(in) :: sendcounts(*), recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_alltoallv_toC(sendcounts, &
                                                 sendtype%MPI_VAL, &
                                                 recvcounts, &
                                                 recvtype%MPI_VAL, &
                                                 inplace, &
                                                 comm%MPI_VAL, &
                                                 sendbytes, &
                                                 recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_alltoallw_impl(sendcounts, &
                                                    sendtypes, &
                                                    recvcounts, &
                                                    recvtypes, &
                                                    inplace, &
                                                    comm, &
                                                    sendbytes, &
                                                    recvbytes)
        integer(count_kind), intent(in) :: sendcounts(*), recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtypes(*), recvtypes(*)
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        integer :: comm_size, ierr
        call PMPI_Comm_size(comm, comm_size, ierr)
        call scorep_mpi_coll_bytes_alltoallw_toC(sendcounts, &
                                                 sendtypes(:comm_size)%MPI_VAL, &
                                                 recvcounts, &
                                                 recvtypes(:comm_size)%MPI_VAL, &
                                                 inplace, &
                                                 comm%MPI_VAL, &
                                                 sendbytes, &
                                                 recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_allgather_impl(sendcount, &
                                                    sendtype, &
                                                    recvcount, &
                                                    recvtype, &
                                                    inplace, &
                                                    comm, &
                                                    sendbytes, &
                                                    recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_allgather_toC(sendcount, &
                                                 sendtype%MPI_VAL, &
                                                 recvcount, &
                                                 recvtype%MPI_VAL, &
                                                 inplace, &
                                                 comm%MPI_VAL, &
                                                 sendbytes, &
                                                 recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_allgatherv_impl(sendcount, &
                                                     sendtype, &
                                                     recvcounts, &
                                                     recvtype, &
                                                     inplace, &
                                                     comm, &
                                                     sendbytes, &
                                                     recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_allgatherv_toC(sendcount, &
                                                  sendtype%MPI_VAL, &
                                                  recvcounts, &
                                                  recvtype%MPI_VAL, &
                                                  inplace, &
                                                  comm%MPI_VAL, &
                                                  sendbytes, &
                                                  recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_allreduce_impl(count, &
                                                    datatype, &
                                                    inplace, &
                                                    comm, &
                                                    sendbytes, &
                                                    recvbytes)
        integer(count_kind), intent(in) :: count
        type(MPI_Datatype), intent(in) :: datatype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_allreduce_toC(count, &
                                                 datatype%MPI_VAL, &
                                                 inplace, &
                                                 comm%MPI_VAL, &
                                                 sendbytes, &
                                                 recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_reduce_scatter_block_impl(recvcount, &
                                                               datatype, &
                                                               inplace, &
                                                               comm, &
                                                               sendbytes, &
                                                               recvbytes)
        integer(count_kind), intent(in) :: recvcount
        type(MPI_Datatype), intent(in) :: datatype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_reduce_scatter_block_toC(recvcount, &
                                                            datatype%MPI_VAL, &
                                                            inplace, &
                                                            comm%MPI_VAL, &
                                                            sendbytes, &
                                                            recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_reduce_scatter_impl(recvcounts, &
                                                         datatype, &
                                                         inplace, &
                                                         comm, &
                                                         sendbytes, &
                                                         recvbytes)
        integer(count_kind), intent(in) :: recvcounts(*)
        type(MPI_Datatype), intent(in) :: datatype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_reduce_scatter_toC(recvcounts, &
                                                      datatype%MPI_VAL, &
                                                      inplace, &
                                                      comm%MPI_VAL, &
                                                      sendbytes, &
                                                      recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_scan_impl(count, &
                                               datatype, &
                                               inplace, &
                                               comm, &
                                               sendbytes, &
                                               recvbytes)
        integer(count_kind), intent(in) :: count
        type(MPI_Datatype), intent(in) :: datatype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_scan_toC(count, &
                                            datatype%MPI_VAL, &
                                            inplace, &
                                            comm%MPI_VAL, &
                                            sendbytes, &
                                            recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_exscan_impl(count, &
                                                 datatype, &
                                                 inplace, &
                                                 comm, &
                                                 sendbytes, &
                                                 recvbytes)
        integer(count_kind), intent(in) :: count
        type(MPI_Datatype), intent(in) :: datatype
        logical(c_bool), intent(in) :: inplace
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_exscan_toC(count, &
                                              datatype%MPI_VAL, &
                                              inplace, &
                                              comm%MPI_VAL, &
                                              sendbytes, &
                                              recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_neighbor_alltoall_impl(sendcount, &
                                                            sendtype, &
                                                            recvcount, &
                                                            recvtype, &
                                                            comm, &
                                                            sendbytes, &
                                                            recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_neighbor_alltoall_toC(sendcount, &
                                                         sendtype%MPI_VAL, &
                                                         recvcount, &
                                                         recvtype%MPI_VAL, &
                                                         comm%MPI_VAL, &
                                                         sendbytes, &
                                                         recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_neighbor_alltoallv_impl(sendcounts, &
                                                             sendtype, &
                                                             recvcounts, &
                                                             recvtype, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes)
        integer(count_kind), intent(in) :: sendcounts(*), recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_neighbor_alltoallv_toC(sendcounts, &
                                                          sendtype%MPI_VAL, &
                                                          recvcounts, &
                                                          recvtype%MPI_VAL, &
                                                          comm%MPI_VAL, &
                                                          sendbytes, &
                                                          recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_neighbor_alltoallw_impl(sendcounts, &
                                                             sendtypes, &
                                                             recvcounts, &
                                                             recvtypes, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes)
        integer(count_kind), intent(in) :: sendcounts(*), recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtypes(*), recvtypes(*)
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        integer :: comm_size, ierr
        call PMPI_Comm_size(comm, comm_size, ierr)
        call scorep_mpi_coll_bytes_neighbor_alltoallw_toC(sendcounts, &
                                                          sendtypes(:comm_size)%MPI_VAL, &
                                                          recvcounts, &
                                                          recvtypes(:comm_size)%MPI_VAL, &
                                                          comm%MPI_VAL, &
                                                          sendbytes, &
                                                          recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_neighbor_allgather_impl(sendcount, &
                                                             sendtype, &
                                                             recvcount, &
                                                             recvtype, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcount
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_neighbor_allgather_toC(sendcount, &
                                                          sendtype%MPI_VAL, &
                                                          recvcount, &
                                                          recvtype%MPI_VAL, &
                                                          comm%MPI_VAL, &
                                                          sendbytes, &
                                                          recvbytes)
    end subroutine

    subroutine scorep_mpi_coll_bytes_neighbor_allgatherv_impl(sendcount, &
                                                              sendtype, &
                                                              recvcounts, &
                                                              recvtype, &
                                                              comm, &
                                                              sendbytes, &
                                                              recvbytes)
        integer(count_kind), intent(in) :: sendcount, recvcounts(*)
        type(MPI_Datatype), intent(in) :: sendtype, recvtype
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_neighbor_allgatherv_toC(sendcount, &
                                                           sendtype%MPI_VAL, &
                                                           recvcounts, &
                                                           recvtype%MPI_VAL, &
                                                           comm%MPI_VAL, &
                                                           sendbytes, &
                                                           recvbytes)
    end subroutine
