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
module scorep_mpi_coll_h_default
    use, intrinsic :: iso_fortran_env, only: count_kind => int32

    use, intrinsic :: iso_c_binding, only: c_int64_t, c_bool
    use :: mpi_f08, only:MPI_Comm, MPI_Datatype, PMPI_Comm_size

    use scorep_mpi_coll_h_bindc

    implicit none

    private

    include 'scorep_mpi_coll_h.inc.f90'
end module

module scorep_mpi_coll_h_large
    use :: mpi_f08, only:count_kind => MPI_Count_kind

    use, intrinsic :: iso_c_binding, only: c_int64_t, c_bool
    use :: mpi_f08, only:MPI_Comm, MPI_Datatype, PMPI_Comm_size

    use scorep_mpi_coll_h_bindc

    implicit none

    private

    include 'scorep_mpi_coll_h.inc.f90'
end module

module scorep_mpi_coll_h
    use, intrinsic :: iso_c_binding, only: c_int64_t, c_bool
    use :: mpi_f08, only:MPI_Comm, MPI_Datatype, MPI_Count_kind

    !
    ! Bind(C) Interfaces:
    !
    use scorep_mpi_coll_h_bindc

    !
    ! Interface to be called from F08 wrappers
    ! These functions
    ! 1. convert 'use mpi_f08' handle types(e.g. type(MPI_Comm) :: comm) to the 'use mpi' handle integer (e.g. comm%MPI_VAL)
    ! 2. call the bind(c) interface
    !
    use scorep_mpi_coll_h_default
    use scorep_mpi_coll_h_large

    implicit none

    private :: &
        c_int64_t, &
        c_bool, &
        MPI_Comm, &
        MPI_Datatype, &
        MPI_Count_kind

    interface scorep_mpi_coll_bytes_barrier
        module procedure scorep_mpi_coll_bytes_barrier_impl
    end interface

contains

    subroutine scorep_mpi_coll_bytes_barrier_impl(comm, sendbytes, recvbytes)
        type(MPI_Comm), intent(in) :: comm
        integer(c_int64_t), intent(out) :: sendbytes, recvbytes

        call scorep_mpi_coll_bytes_barrier_toC(comm%MPI_VAL, sendbytes, recvbytes)
    end subroutine
end module
