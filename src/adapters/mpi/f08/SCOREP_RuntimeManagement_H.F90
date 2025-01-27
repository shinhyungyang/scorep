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
#include <config.h>
module SCOREP_RuntimeManagement_H

    use, intrinsic :: iso_c_binding, only: c_bool

    implicit none

    private

    public :: &
        scorep_is_unwinding_enabled, &
        SCOREP_InitMeasurement, &
        SCOREP_InitMppMeasurement, &
        SCOREP_RegisterExitHandler, &
        SCOREP_SetAbortFlag

#if defined(HAVE_UNWINDING_SUPPORT) && HAVE_UNWINDING_SUPPORT
    logical(c_bool), bind(c, name="scorep_is_unwinding_enabled") :: scorep_is_unwinding_enabled
#else
    logical(c_bool), parameter :: scorep_is_unwinding_enabled = .false.
#endif

    interface
        subroutine SCOREP_InitMeasurement() bind(c, name="SCOREP_InitMeasurement")
            implicit none
        end subroutine
    end interface

    interface
        subroutine SCOREP_InitMppMeasurement() bind(c, name="SCOREP_InitMppMeasurement")
            implicit none
        end subroutine
    end interface

    interface
        subroutine SCOREP_RegisterExitHandler() bind(c, name="SCOREP_RegisterExitHandler")
            implicit none
        end subroutine
    end interface

    interface
        subroutine SCOREP_SetAbortFlag() bind(c, name="SCOREP_SetAbortFlag")
            implicit none
        end subroutine
    end interface
end module
