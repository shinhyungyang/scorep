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
module scorep_mpi_io_mgmt_h
    use, intrinsic :: iso_c_binding, only: c_int64_t
    use :: mpi_f08, only:MPI_Datatype
    use :: SCOREP_PublicHandles_H, only:SCOREP_IoHandleHandle

    implicit none

    private

    public :: scorep_mpi_io_split_begin, &
              scorep_mpi_io_split_end

    interface
        subroutine scorep_mpi_io_split_begin_toC(ioHandle, matchingId, datatype) &
            bind(c, name='scorep_mpi_io_split_begin_fromF08')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in) :: ioHandle
            integer(c_int64_t), intent(in) :: matchingId
            integer, intent(in) :: datatype
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_io_split_end_toC(ioHandle, matchingId, datatype) &
            bind(c, name='scorep_mpi_io_split_end_fromF08')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in) :: ioHandle
            integer(c_int64_t), intent(out) :: matchingId
            integer, intent(out) :: datatype
        end subroutine
    end interface

contains

    subroutine scorep_mpi_io_split_begin(ioHandle, matchingId, datatype)
        integer(SCOREP_IoHandleHandle), intent(in) :: ioHandle
        integer(c_int64_t), intent(in) :: matchingId
        type(MPI_Datatype), intent(in) :: datatype

        call scorep_mpi_io_split_begin_toC(ioHandle, matchingId, datatype%MPI_VAL)
    end subroutine

    subroutine scorep_mpi_io_split_end(ioHandle, matchingId, datatype)
        integer(SCOREP_IoHandleHandle), intent(in) :: ioHandle
        integer(c_int64_t), intent(out) :: matchingId
        type(MPI_Datatype), intent(out) :: datatype

        integer :: datatype_val

        call scorep_mpi_io_split_end_toC(ioHandle, matchingId, datatype_val)
        datatype%MPI_VAL = datatype_val
    end subroutine
end module
