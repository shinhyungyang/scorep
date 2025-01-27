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
module scorep_mpi_io_h
    use, intrinsic :: iso_c_binding, only: c_int
    use SCOREP_PublicTypes_H, only: SCOREP_IoAccessMode, SCOREP_IoCreationFlag, SCOREP_IoStatusFlag, SCOREP_IoSeekOption

    implicit none

    private

    public :: scorep_mpi_io_get_access_mode, &
              scorep_mpi_io_get_creation_flags, &
              scorep_mpi_io_get_status_flags, &
              scorep_mpi_io_get_seek_option

    interface scorep_mpi_io_get_access_mode
        function scorep_mpi_io_get_access_mode(amode) result(res) &
            bind(c, name="scorep_mpi_io_get_access_mode")
            import
            implicit none
            integer(c_int), intent(in), value :: amode
            integer(SCOREP_IoAccessMode) :: res
        end function
    end interface

    interface scorep_mpi_io_get_creation_flags
        function scorep_mpi_io_get_creation_flags(amode) result(res) &
            bind(c, name="scorep_mpi_io_get_creation_flags")
            import
            implicit none
            integer(c_int), intent(in), value :: amode
            integer(SCOREP_IoCreationFlag) :: res
        end function
    end interface

    interface scorep_mpi_io_get_status_flags
        function scorep_mpi_io_get_status_flags(amode) result(res) &
            bind(c, name="scorep_mpi_io_get_status_flags")
            import
            implicit none
            integer(c_int), intent(in), value :: amode
            integer(SCOREP_IoStatusFlag) :: res
        end function
    end interface

    interface scorep_mpi_io_get_seek_option
        function scorep_mpi_io_get_seek_option(amode) result(res) &
            bind(c, name="scorep_mpi_io_get_seek_option")
            import
            implicit none
            integer(c_int), intent(in), value :: amode
            integer(SCOREP_IoSeekOption) :: res
        end function
    end interface

end module
