!
!  This file is part of the Score-P software (http://www.score-p.org)
!
!  Copyright (c) 2020,
!  Technische Universitaet Dresden, Germany
!
!  Copyright (c) 2020-2021,
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
    write (*, *) "[Score-P] 'USE mpi_f08' not supported yet,"
    write (*, *) "           consider switching to 'USE mpi'."
    call abort
end subroutine

subroutine MPI_Init_thread_f08(required, provided, ierror)
    integer, intent(in) :: required
    integer, intent(out) :: provided
    integer, optional, intent(out) :: ierror
    write (*, *) "[Score-P] 'USE mpi_f08' not supported yet,"
    write (*, *) "           consider switching to 'USE mpi'."
    call abort
end subroutine
