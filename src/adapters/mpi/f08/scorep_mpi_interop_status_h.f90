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
module scorep_mpi_interop_status_h
    use, intrinsic :: iso_c_binding, only: c_int, c_ptr, c_loc, c_f_pointer
    use, intrinsic :: iso_c_binding, only: SCOREP_LANGUAGE => c_int
    use :: mpi_f08, only:MPI_Status

    implicit none

    private

    public :: &
        SCOREP_LANGUAGE_C, SCOREP_LANGUAGE_F08, &
        scorep_mpi_interop_status, &
        scorep_mpi_interop_status_create, &
        scorep_mpi_status_source, &
        scorep_mpi_status_tag, &
        scorep_mpi_status_error, &
        scorep_mpi_get_count, &
        scorep_mpi_test_cancelled

    ! enum SCOREP_LANGUAGE
    integer(SCOREP_Language), parameter :: SCOREP_LANGUAGE_C = 0
    integer(SCOREP_Language), parameter :: SCOREP_LANGUAGE_F08 = 1

    type, bind(c) :: scorep_mpi_interop_status
        type(c_ptr) :: status
        integer(SCOREP_Language) :: origin_language
    end type

contains

    function scorep_mpi_interop_status_create(status) result(interopStatus)
        type(MPI_Status), intent(in), target :: status
        type(scorep_mpi_interop_status) :: interopStatus

        interopStatus%status = c_loc(status)
        interopStatus%origin_language = SCOREP_LANGUAGE_F08
    end function

    subroutine scorep_mpi_status_source(status, source) &
        bind(C, name="scorep_mpi_status_source_toF08")
        type(c_ptr), intent(in), value :: status
        integer(c_int), intent(out) :: source

        type(MPI_Status), pointer :: status_p

        call c_f_pointer(status, status_p)
        source = status_p%MPI_SOURCE
    end subroutine

    subroutine scorep_mpi_status_tag(status, tag) &
        bind(C, name="scorep_mpi_status_tag_toF08")
        type(c_ptr), intent(in), value :: status
        integer(c_int), intent(out) :: tag

        type(MPI_Status), pointer :: status_p

        call c_f_pointer(status, status_p)
        tag = status_p%MPI_TAG
    end subroutine

    subroutine scorep_mpi_status_error(status, error) &
        bind(C, name="scorep_mpi_status_error_toF08")
        type(c_ptr), intent(in), value :: status
        integer(c_int), intent(out) :: error

        type(MPI_Status), pointer :: status_p

        call c_f_pointer(status, status_p)
        error = status_p%MPI_ERROR
    end subroutine

    subroutine scorep_mpi_get_count(status, datatype, count) &
        bind(C, name="scorep_mpi_get_count_toF08")
        use :: mpi_f08, only:MPI_Datatype, PMPI_Get_count
        type(c_ptr), intent(in), value :: status
        integer, intent(in) :: datatype
        integer(c_int), intent(out) :: count

        type(MPI_Status), pointer :: status_p
        type(MPI_Datatype) :: datatype_f08

        call c_f_pointer(status, status_p)
        ! HACK: conversion from a use :: mpi integer handle to a use :: mpi_f08 handle of type(MPI_Datatype)
        ! The Standard only says:
        ! "With the Fortran mpi_f08 module, a Fortran handle is a
        ! BIND(C) derived type that contains an INTEGER component named MPI_VAL"
        ! This does NOT imply that we can construct a valid type(MPI_Datatype) like this:
        datatype_f08%MPI_VAL = datatype
        call PMPI_Get_count(status_p, datatype_f08, count)
    end subroutine

    subroutine scorep_mpi_test_cancelled(status, cancelled_c) &
        bind(C, name="scorep_mpi_test_cancelled_toF08")
        use :: mpi_f08, only:PMPI_Test_cancelled
        type(c_ptr), intent(in), value :: status
        integer(c_int), intent(out) :: cancelled_c

        type(MPI_Status), pointer :: status_p
        logical :: cancelled

        call c_f_pointer(status, status_p)
        call PMPI_Test_cancelled(status_p, cancelled)
        if (cancelled) then
            cancelled_c = 1
        else
            cancelled_c = 0
        end if
    end subroutine
end module
