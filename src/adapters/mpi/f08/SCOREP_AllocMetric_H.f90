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
module SCOREP_AllocMetric_H
    use, intrinsic :: iso_c_binding, only: c_ptr, c_int64_t, c_size_t

    implicit none

    private
    public :: &
        SCOREP_AllocMetric_HandleAlloc

    interface SCOREP_AllocMetric_HandleAlloc
        subroutine SCOREP_AllocMetric_HandleAlloc(allocMetric, resultAddr, allocSize) &
            bind(c, name='SCOREP_AllocMetric_HandleAlloc_fromF08')
            import
            implicit none
            type(c_ptr), value :: allocMetric
            type(c_ptr), value :: resultAddr
            integer(c_size_t), value :: allocSize
        end subroutine
    end interface

end module
