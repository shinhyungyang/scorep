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
module scorep_mpi_f08_utils
    implicit none

    private
    public :: c_string

    interface c_string
        module procedure c_string
    end interface

contains
    !
    ! Convert the Fortran character into a C-compatible \0-terminated character
    !
    ! For len(str) >= len(str_c), only the len(str_c)-1 first
    ! chars in str are copied to str_c. The last position in str_c is always a \0 char.
    !
    ! For len(str) < len(str_c), the entire str is copied to str_c and the rest is
    ! filled up with \0.
    !
    subroutine c_string(str, str_c)
        use iso_c_binding, only: c_char, C_NULL_CHAR
        character(len=*), intent(in) :: str
        character(len=*, kind=C_CHAR), intent(out) :: str_c

        integer :: i
        integer :: n

        n = min(len(str_c) - 1, len(str))
        str_c(1:n) = str(1:n)
        do i = n + 1, len(str_c)
            str_c(i:i) = C_NULL_CHAR
        end do
    end subroutine
end module
