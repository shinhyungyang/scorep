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
module SCOREP_InMeasurement_H
    use, intrinsic :: iso_c_binding, only: c_int, c_bool
    use :: SCOREP_Types_H, only:SCOREP_MeasurementPhase

    implicit none

    private

    public :: &
        scorep_is_measurement_phase, &
        scorep_in_measurement_increment, &
        scorep_in_measurement_decrement, &
        scorep_enter_wrapped_region, &
        scorep_exit_wrapped_region

    interface
        function scorep_is_measurement_phase(phase) result(flag) bind(c, name="scorep_is_measurement_phase_fromF08")
            import
            implicit none
            integer(SCOREP_MeasurementPhase), intent(in) :: phase
            logical(c_bool) :: flag
        end function
    end interface

    interface
        subroutine scorep_in_measurement_increment() bind(c, name="scorep_in_measurement_increment_fromF08")
            implicit none
        end subroutine
    end interface

    interface
        subroutine scorep_in_measurement_decrement() bind(c, name="scorep_in_measurement_decrement_fromF08")
            implicit none
        end subroutine
    end interface

    interface
        subroutine scorep_enter_wrapped_region(inMeasurementSave) bind(c, name="scorep_enter_wrapped_region_fromF08")
            import
            implicit none
            integer(c_int), intent(out) :: inMeasurementSave
        end subroutine
    end interface

    interface
        subroutine scorep_exit_wrapped_region(inMeasurementSave) bind(c, name="scorep_exit_wrapped_region_fromF08")
            import
            implicit none
            integer(c_int), intent(in) :: inMeasurementSave
        end subroutine
    end interface

end module
