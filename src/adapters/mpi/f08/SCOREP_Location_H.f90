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
module SCOREP_Location_H
    use, intrinsic :: iso_c_binding, only: c_ptr, c_int32_t

    implicit none

    private

    public :: &
        SCOREP_Location_GetCurrentCPULocation, &
        SCOREP_Location_GetId

    interface
        function SCOREP_Location_GetCurrentCPULocation() result(location) &
            bind(c, name="SCOREP_Location_GetCurrentCPULocation")
            import
            implicit none
            type(c_ptr) :: location
        end function
    end interface

    interface
        ! WARNING: The C function returns uint32_t
        function SCOREP_Location_GetId(location) result(id) &
            bind(c, name="SCOREP_Location_GetId")
            import
            implicit none
            type(c_ptr), value, intent(in) :: location
            integer(c_int32_t) :: id
        end function
    end interface
end module
