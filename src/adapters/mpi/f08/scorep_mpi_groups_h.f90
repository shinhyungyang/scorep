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

module scorep_mpi_groups_h
    use, intrinsic :: iso_c_binding, only: c_int64_t

    implicit none

    private :: &
        c_int64_t

    ! pure groups, which can be specified in conf
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_CG = ibset(0, 0)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_COLL = ibset(0, 1)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_ENV = ibset(0, 2)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_ERR = ibset(0, 3)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_EXT = ibset(0, 4)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_IO = ibset(0, 5)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_MISC = ibset(0, 6)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_P2P = ibset(0, 7)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_RMA = ibset(0, 8)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_SPAWN = ibset(0, 9)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_TOPO = ibset(0, 10)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_TYPE = ibset(0, 11)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_PERF = ibset(0, 12)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_XNONBLOCK = ibset(0, 13)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_XREQTEST = ibset(0, 14)
! derived groups, which are a combination of existing groups
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_CG_ERR = ibset(0, 15)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_CG_EXT = ibset(0, 16)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_CG_MISC = ibset(0, 17)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_IO_ERR = ibset(0, 18)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_IO_MISC = ibset(0, 19)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_RMA_ERR = ibset(0, 20)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_RMA_EXT = ibset(0, 21)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_RMA_MISC = ibset(0, 22)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_TYPE_EXT = ibset(0, 23)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_TYPE_MISC = ibset(0, 24)
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_REQUEST = ibset(0, 25)
! The Fortran 2008 'iany' intrinsic is not used here, because it is
! not supported by some compilers based on flang/classic:
! NVHPC <= 24.11 and ROCm <= 6.2.0 are known examples.
! '+' works, because all operands have exactly one bit set to one.
! ALL comprises all pure groups
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_ALL = &
                                     SCOREP_MPI_ENABLED_CG + &
                                     SCOREP_MPI_ENABLED_COLL + &
                                     SCOREP_MPI_ENABLED_ENV + &
                                     SCOREP_MPI_ENABLED_ERR + &
                                     SCOREP_MPI_ENABLED_EXT + &
                                     SCOREP_MPI_ENABLED_IO + &
                                     SCOREP_MPI_ENABLED_MISC + &
                                     SCOREP_MPI_ENABLED_P2P + &
                                     SCOREP_MPI_ENABLED_RMA + &
                                     SCOREP_MPI_ENABLED_SPAWN + &
                                     SCOREP_MPI_ENABLED_TOPO + &
                                     SCOREP_MPI_ENABLED_TYPE + &
                                     SCOREP_MPI_ENABLED_PERF + &
                                     SCOREP_MPI_ENABLED_XREQTEST
! DEFAULT groups
    integer(c_int64_t), parameter :: SCOREP_MPI_ENABLED_DEFAULT = &
                                     SCOREP_MPI_ENABLED_CG + &
                                     SCOREP_MPI_ENABLED_COLL + &
                                     SCOREP_MPI_ENABLED_ENV + &
                                     SCOREP_MPI_ENABLED_IO + &
                                     SCOREP_MPI_ENABLED_P2P + &
                                     SCOREP_MPI_ENABLED_RMA + &
                                     SCOREP_MPI_ENABLED_TOPO

    integer(c_int64_t), bind(c, name="scorep_mpi_enabled") :: scorep_mpi_enabled
end module
