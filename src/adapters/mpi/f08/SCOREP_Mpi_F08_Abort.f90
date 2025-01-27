!
!  This file is part of the Score-P software (http://www.score-p.org)
!
!  Copyright (c) 2020,
!  Technische Universitaet Dresden, Germany
!
!  Copyright (c) 2020-2021, 2025,
!  Forschungszentrum Juelich GmbH, Germany
!
! This software may be modified and distributed under the terms of
! a BSD-style license. See the COPYING file in the package base
! directory for details.
!

!>
!! @file
!! @brief      Abort on usage of mpi_f08
!!
!! @ingroup    MPI_Wrapper
!<

subroutine MPI_Init_f08(ierror)
    integer, optional, intent(out) :: ierror
    write (*, *) "[Score-P] Support for 'USE mpi_f08' is disabled in", &
        " this Score-P installation."
    write (*, *) "          Please view 'scorep.summary' for details. "
    call abort
end subroutine

subroutine MPI_Init_thread_f08(required, provided, ierror)
    integer, intent(in) :: required
    integer, intent(out) :: provided
    integer, optional, intent(out) :: ierror
    write (*, *) "[Score-P] Support for 'USE mpi_f08' is disabled in", &
        " this Score-P installation."
    write (*, *) "          Please view 'scorep.summary' for details. "
    call abort
end subroutine

subroutine MPI_Session_init_f08(info, errhandler, session, ierror)
    ! The real mpi_f08 interface is:
    ! type(MPI_Info), intent(in) :: info
    ! type(MPI_Errhandler), intent(in) :: errhandler
    ! type(MPI_Session), intent(in) :: session
    ! integer, optional, intent(out) :: ierror
    !
    ! But this function is linked to the MPI adapter if mpi_f08 is not supported
    ! in Score-P. Therefore we cannot 'use mpi_f08' without additional configure
    ! checks. It is easier to lie about the interface, if we never actually
    ! use any of the arguments.
    integer :: info, errhandler, session
    integer, optional, intent(out) :: ierror

    write (*, *) "[Score-P] The MPI Sessions model is currently not", &
        " supported by Score-P."
    write (*, *) "[Score-P] Support for 'USE mpi_f08' is disabled in", &
        " this Score-P installation."
    write (*, *) "          Please view 'scorep.summary' for details. "
    call abort
end subroutine
