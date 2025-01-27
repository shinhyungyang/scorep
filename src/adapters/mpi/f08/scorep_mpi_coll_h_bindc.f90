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
module scorep_mpi_coll_h_bindc
    use, intrinsic :: iso_c_binding, only: c_bool, c_int64_t
    use :: mpi_f08, only:MPI_Count_kind

    implicit none

    private :: &
        c_bool, &
        c_int64_t, &
        MPI_Count_kind

    interface scorep_mpi_coll_bytes_barrier_toC
        subroutine scorep_mpi_coll_bytes_barrier_toC_default(comm, sendbytes, recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_barrier_fromF08")
            import
            implicit none
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_gather_toC
        subroutine scorep_mpi_coll_bytes_gather_toC_default(sendcount, &
                                                            sendtype, &
                                                            recvcount, &
                                                            recvtype, &
                                                            root, &
                                                            inplace, &
                                                            comm, &
                                                            sendbytes, &
                                                            recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_gather_fromF08")
            import
            implicit none
            integer :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine

        subroutine scorep_mpi_coll_bytes_gather_toC_large(sendcount, &
                                                          sendtype, &
                                                          recvcount, &
                                                          recvtype, &
                                                          root, &
                                                          inplace, &
                                                          comm, &
                                                          sendbytes, &
                                                          recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_gather_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_reduce_toC
        subroutine scorep_mpi_coll_bytes_reduce_toC_default(count, &
                                                            datatype, &
                                                            root, &
                                                            inplace, &
                                                            comm, &
                                                            sendbytes, &
                                                            recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_reduce_fromF08")
            import
            implicit none
            integer :: count
            integer :: datatype, root, comm
            logical(c_bool) :: inplace
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine

        subroutine scorep_mpi_coll_bytes_reduce_toC_large(count, &
                                                          datatype, &
                                                          root, &
                                                          inplace, &
                                                          comm, &
                                                          sendbytes, &
                                                          recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_reduce_fromF08_c")
            import
            implicit none
            integer(kind=MPI_Count_kind) :: count
            integer :: datatype, root, comm
            logical(c_bool) :: inplace
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_gatherv_toC
        subroutine scorep_mpi_coll_bytes_gatherv_toC_default(sendcount, &
                                                             sendtype, &
                                                             recvcounts, &
                                                             recvtype, &
                                                             root, &
                                                             inplace, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_gatherv_fromF08")
            import
            implicit none
            integer :: sendcount, recvcounts(*)
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine

        subroutine scorep_mpi_coll_bytes_gatherv_toC_large(sendcount, &
                                                           sendtype, &
                                                           recvcounts, &
                                                           recvtype, &
                                                           root, &
                                                           inplace, &
                                                           comm, &
                                                           sendbytes, &
                                                           recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_gatherv_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcounts(*)
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_bcast_toC
        subroutine scorep_mpi_coll_bytes_bcast_toC_default(count, &
                                                           datatype, &
                                                           root, &
                                                           comm, &
                                                           sendbytes, &
                                                           recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_bcast_fromF08")
            import
            implicit none
            integer :: count
            integer :: datatype, root, comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine

        subroutine scorep_mpi_coll_bytes_bcast_toC_large(count, &
                                                         datatype, &
                                                         root, &
                                                         comm, &
                                                         sendbytes, &
                                                         recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_bcast_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: count
            integer :: datatype, root, comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_scatter_toC
        subroutine scorep_mpi_coll_bytes_scatter_toC_default(sendcount, &
                                                             sendtype, &
                                                             recvcount, &
                                                             recvtype, &
                                                             root, &
                                                             inplace, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_scatter_fromF08")
            import
            implicit none
            integer :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine

        subroutine scorep_mpi_coll_bytes_scatter_toC_large(sendcount, &
                                                           sendtype, &
                                                           recvcount, &
                                                           recvtype, &
                                                           root, &
                                                           inplace, &
                                                           comm, &
                                                           sendbytes, &
                                                           recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_scatter_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_scatterv_toC
        subroutine scorep_mpi_coll_bytes_scatterv_toC_default(sendcounts, &
                                                              sendtype, &
                                                              recvcount, &
                                                              recvtype, &
                                                              root, &
                                                              inplace, &
                                                              comm, &
                                                              sendbytes, &
                                                              recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_scatterv_fromF08")
            import
            implicit none
            integer :: sendcounts(*), recvcount
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine

        subroutine scorep_mpi_coll_bytes_scatterv_toC_large(sendcounts, &
                                                            sendtype, &
                                                            recvcount, &
                                                            recvtype, &
                                                            root, &
                                                            inplace, &
                                                            comm, &
                                                            sendbytes, &
                                                            recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_scatterv_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcounts(*), recvcount
            integer :: sendtype, recvtype
            integer :: root
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_alltoall_toC
        subroutine scorep_mpi_coll_bytes_alltoall_toC_default(sendcount, &
                                                              sendtype, &
                                                              recvcount, &
                                                              recvtype, &
                                                              inplace, &
                                                              comm, &
                                                              sendbytes, &
                                                              recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_alltoall_fromF08")
            import
            implicit none
            integer :: sendcount, recvcount
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_alltoall_toC_large(sendcount, &
                                                            sendtype, &
                                                            recvcount, &
                                                            recvtype, &
                                                            inplace, &
                                                            comm, &
                                                            sendbytes, &
                                                            recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_alltoall_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcount
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_alltoallv_toC
        subroutine scorep_mpi_coll_bytes_alltoallv_toC_default(sendcounts, &
                                                               sendtype, &
                                                               recvcounts, &
                                                               recvtype, &
                                                               inplace, &
                                                               comm, &
                                                               sendbytes, &
                                                               recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_alltoallv_fromF08")
            import
            implicit none
            integer :: sendcounts(*), recvcounts(*)
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_alltoallv_toC_large(sendcounts, &
                                                             sendtype, &
                                                             recvcounts, &
                                                             recvtype, &
                                                             inplace, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_alltoallv_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcounts(*), recvcounts(*)
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_alltoallw_toC
        subroutine scorep_mpi_coll_bytes_alltoallw_toC_default(sendcounts, &
                                                               sendtypes, &
                                                               recvcounts, &
                                                               recvtypes, &
                                                               inplace, &
                                                               comm, &
                                                               sendbytes, &
                                                               recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_alltoallw_fromF08")
            import
            implicit none
            integer :: sendcounts(*), recvcounts(*)
            integer :: sendtypes(*), recvtypes(*)
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_alltoallw_toC_large(sendcounts, &
                                                             sendtypes, &
                                                             recvcounts, &
                                                             recvtypes, &
                                                             inplace, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_alltoallw_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcounts(*), recvcounts(*)
            integer :: sendtypes(*), recvtypes(*)
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_allgather_toC
        subroutine scorep_mpi_coll_bytes_allgather_toC_default(sendcount, &
                                                               sendtype, &
                                                               recvcount, &
                                                               recvtype, &
                                                               inplace, &
                                                               comm, &
                                                               sendbytes, &
                                                               recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_allgather_fromF08")
            import
            implicit none
            integer :: sendcount, recvcount
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_allgather_toC_large(sendcount, &
                                                             sendtype, &
                                                             recvcount, &
                                                             recvtype, &
                                                             inplace, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_allgather_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcount
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_allgatherv_toC
        subroutine scorep_mpi_coll_bytes_allgatherv_toC_default(sendcount, &
                                                                sendtype, &
                                                                recvcounts, &
                                                                recvtype, &
                                                                inplace, &
                                                                comm, &
                                                                sendbytes, &
                                                                recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_allgatherv_fromF08")
            import
            implicit none
            integer :: sendcount, recvcounts(*)
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_allgatherv_toC_large(sendcount, &
                                                              sendtype, &
                                                              recvcounts, &
                                                              recvtype, &
                                                              inplace, &
                                                              comm, &
                                                              sendbytes, &
                                                              recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_allgatherv_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcounts(*)
            integer :: sendtype, recvtype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_allreduce_toC
        subroutine scorep_mpi_coll_bytes_allreduce_toC_normal(count, &
                                                              datatype, &
                                                              inplace, &
                                                              comm, &
                                                              sendbytes, &
                                                              recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_allreduce_fromF08")
            import
            implicit none
            integer :: count
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_allreduce_toC_large(count, &
                                                             datatype, &
                                                             inplace, &
                                                             comm, &
                                                             sendbytes, &
                                                             recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_allreduce_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: count
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_reduce_scatter_block_toC
        subroutine scorep_mpi_coll_bytes_reduce_scatter_block_toC_normal(recvcount, &
                                                                         datatype, &
                                                                         inplace, &
                                                                         comm, &
                                                                         sendbytes, &
                                                                         recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_reduce_scatter_block_fromF08")
            import
            implicit none
            integer :: recvcount
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_reduce_scatter_block_toC_large(recvcount, &
                                                                        datatype, &
                                                                        inplace, &
                                                                        comm, &
                                                                        sendbytes, &
                                                                        recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_reduce_scatter_block_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: recvcount
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_reduce_scatter_toC
        subroutine scorep_mpi_coll_bytes_reduce_scatter_toC_normal(recvcounts, &
                                                                   datatype, &
                                                                   inplace, &
                                                                   comm, &
                                                                   sendbytes, &
                                                                   recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_reduce_scatter_fromF08")
            import
            implicit none
            integer :: recvcounts(*)
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_reduce_scatter_toC_large(recvcounts, &
                                                                  datatype, &
                                                                  inplace, &
                                                                  comm, &
                                                                  sendbytes, &
                                                                  recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_reduce_scatter_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: recvcounts(*)
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_scan_toC
        subroutine scorep_mpi_coll_bytes_scan_toC_normal(count, &
                                                         datatype, &
                                                         inplace, &
                                                         comm, &
                                                         sendbytes, &
                                                         recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_scan_fromF08")
            import
            implicit none
            integer :: count
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_scan_toC_large(count, &
                                                        datatype, &
                                                        inplace, &
                                                        comm, &
                                                        sendbytes, &
                                                        recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_scan_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: count
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_exscan_toC
        subroutine scorep_mpi_coll_bytes_exscan_toC_normal(count, &
                                                           datatype, &
                                                           inplace, &
                                                           comm, &
                                                           sendbytes, &
                                                           recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_exscan_fromF08")
            import
            implicit none
            integer :: count
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_exscan_toC_large(count, &
                                                          datatype, &
                                                          inplace, &
                                                          comm, &
                                                          sendbytes, &
                                                          recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_exscan_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: count
            integer :: datatype
            logical(c_bool) :: inplace
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_neighbor_alltoall_toC
        subroutine scorep_mpi_coll_bytes_neighbor_alltoall_toC_default(sendcount, &
                                                                       sendtype, &
                                                                       recvcount, &
                                                                       recvtype, &
                                                                       comm, &
                                                                       sendbytes, &
                                                                       recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_alltoall_fromF08")
            import
            implicit none
            integer :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_neighbor_alltoall_toC_large(sendcount, &
                                                                     sendtype, &
                                                                     recvcount, &
                                                                     recvtype, &
                                                                     comm, &
                                                                     sendbytes, &
                                                                     recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_alltoall_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_neighbor_alltoallv_toC
        subroutine scorep_mpi_coll_bytes_neighbor_alltoallv_toC_default(sendcounts, &
                                                                        sendtype, &
                                                                        recvcounts, &
                                                                        recvtype, &
                                                                        comm, &
                                                                        sendbytes, &
                                                                        recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_alltoallv_fromF08")
            import
            implicit none
            integer :: sendcounts(*), recvcounts(*)
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_neighbor_alltoallv_toC_large(sendcounts, &
                                                                      sendtype, &
                                                                      recvcounts, &
                                                                      recvtype, &
                                                                      comm, &
                                                                      sendbytes, &
                                                                      recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_alltoallv_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcounts(*), recvcounts(*)
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_neighbor_alltoallw_toC
        subroutine scorep_mpi_coll_bytes_neighbor_alltoallw_toC_default(sendcounts, &
                                                                        sendtypes, &
                                                                        recvcounts, &
                                                                        recvtypes, &
                                                                        comm, &
                                                                        sendbytes, &
                                                                        recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_alltoallw_fromF08")
            import
            implicit none
            integer :: sendcounts(*), recvcounts(*)
            integer :: sendtypes(*), recvtypes(*)
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_neighbor_alltoallw_toC_large(sendcounts, &
                                                                      sendtypes, &
                                                                      recvcounts, &
                                                                      recvtypes, &
                                                                      comm, &
                                                                      sendbytes, &
                                                                      recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_alltoallw_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcounts(*), recvcounts(*)
            integer :: sendtypes(*), recvtypes(*)
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_neighbor_allgather_toC
        subroutine scorep_mpi_coll_bytes_neighbor_allgather_toC_default(sendcount, &
                                                                        sendtype, &
                                                                        recvcount, &
                                                                        recvtype, &
                                                                        comm, &
                                                                        sendbytes, &
                                                                        recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_allgather_fromF08")
            import
            implicit none
            integer :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_neighbor_allgather_toC_large(sendcount, &
                                                                      sendtype, &
                                                                      recvcount, &
                                                                      recvtype, &
                                                                      comm, &
                                                                      sendbytes, &
                                                                      recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_allgather_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcount
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface

    interface scorep_mpi_coll_bytes_neighbor_allgatherv_toC
        subroutine scorep_mpi_coll_bytes_neighbor_allgatherv_toC_default(sendcount, &
                                                                         sendtype, &
                                                                         recvcounts, &
                                                                         recvtype, &
                                                                         comm, &
                                                                         sendbytes, &
                                                                         recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_allgatherv_fromF08")
            import
            implicit none
            integer :: sendcount, recvcounts(*)
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
        subroutine scorep_mpi_coll_bytes_neighbor_allgatherv_toC_large(sendcount, &
                                                                       sendtype, &
                                                                       recvcounts, &
                                                                       recvtype, &
                                                                       comm, &
                                                                       sendbytes, &
                                                                       recvbytes) &
            bind(c, name="scorep_mpi_coll_bytes_neighbor_allgatherv_fromF08_c")
            import
            implicit none
            integer(MPI_Count_kind) :: sendcount, recvcounts(*)
            integer :: sendtype, recvtype
            integer :: comm
            integer(c_int64_t) :: sendbytes, recvbytes
        end subroutine
    end interface
end module
