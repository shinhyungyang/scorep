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

!> This module exports comparison operators for the mpi_f08 handle types.
!>
!> This works around a bug in gfortran <= 11
!>   - OpenMPI's mpi_f08 exports only operator(.eq.), operator(.ne.), while
!>     MPICH's mpi_f08 exports only operator(==), operator(/=)
!>   - gfortran <= 11 rejects `use :: mpi_f08, only: operator(.eq.)` with MPICH
!>     and rejects `use :: mpi_f08, only: operator(==)` with OpenMPI.
!>     This is a bug, because the Fortran standard requires these to be synonyms,
!>     i.e, overloading one automatically provides the other.
!>     Other compilers and newer versions accept the code.
!> This module conflicts with an unrestricted `use :: mpi_f08`, because
!> (at least one of) the exported operators is already defined there.
module scorep_mpi_f08_handles
    use :: mpi_f08, only:MPI_Comm, MPI_Group, MPI_Request, MPI_Win

    implicit none

    private

    public :: operator(.eq.), operator(.ne.)

    interface operator(.eq.)
        module procedure comm_eq
        module procedure group_eq
        module procedure request_eq
        module procedure win_eq
    end interface

    interface operator(.ne.)
        module procedure comm_ne
        module procedure group_ne
        module procedure request_ne
        module procedure win_ne
    end interface
contains

    elemental logical function comm_eq(x, y) result(cmp)
        type(MPI_Comm), intent(in) :: x, y
        cmp = (x%MPI_VAL .eq. y%MPI_VAL)
    end function

    elemental logical function group_eq(x, y) result(cmp)
        type(MPI_Group), intent(in) :: x, y
        cmp = (x%MPI_VAL .eq. y%MPI_VAL)
    end function

    elemental logical function request_eq(x, y) result(cmp)
        type(MPI_Request), intent(in) :: x, y
        cmp = (x%MPI_VAL .eq. y%MPI_VAL)
    end function

    elemental logical function win_eq(x, y) result(cmp)
        type(MPI_Win), intent(in) :: x, y
        cmp = (x%MPI_VAL .eq. y%MPI_VAL)
    end function

    elemental logical function comm_ne(x, y) result(cmp)
        type(MPI_Comm), intent(in) :: x, y
        cmp = (x%MPI_VAL .ne. y%MPI_VAL)
    end function

    elemental logical function group_ne(x, y) result(cmp)
        type(MPI_Group), intent(in) :: x, y
        cmp = (x%MPI_VAL .ne. y%MPI_VAL)
    end function

    elemental logical function request_ne(x, y) result(cmp)
        type(MPI_Request), intent(in) :: x, y
        cmp = (x%MPI_VAL .ne. y%MPI_VAL)
    end function

    elemental logical function win_ne(x, y) result(cmp)
        type(MPI_Win), intent(in) :: x, y
        cmp = (x%MPI_VAL .ne. y%MPI_VAL)
    end function

end module
