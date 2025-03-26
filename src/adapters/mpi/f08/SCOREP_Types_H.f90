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
module SCOREP_Types_H
    use, intrinsic :: iso_c_binding, only: SCOREP_MeasurementPhase => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoHandleFlag => c_int
    use :: SCOREP_PublicTypes_H, only:SCOREP_Allocator_MovableMemory, SCOREP_MOVABLE_NULL

    implicit none

    !&<
    integer(SCOREP_MeasurementPhase), parameter :: SCOREP_MEASUREMENT_PHASE_PRE_C    = -1
    integer(SCOREP_MeasurementPhase), parameter :: SCOREP_MEASUREMENT_PHASE_WITHIN_C =  0
    integer(SCOREP_MeasurementPhase), parameter :: SCOREP_MEASUREMENT_PHASE_POST_C   =  1
    !&>

    !&<
    integer(SCOREP_IoHandleFlag), parameter :: SCOREP_IO_HANDLE_FLAG_NONE        = 0
    integer(SCOREP_IoHandleFlag), parameter :: SCOREP_IO_HANDLE_FLAG_PRE_CREATED = ibset(0,0)
    integer(SCOREP_IoHandleFlag), parameter :: SCOREP_IO_HANDLE_FLAG_ALL_PROXY   = ibset(0,1)
    !&>

    integer(SCOREP_Allocator_MovableMemory), parameter :: SCOREP_INVALID_IO_HANDLE = SCOREP_MOVABLE_NULL
    integer(SCOREP_Allocator_MovableMemory), parameter :: SCOREP_INVALID_IO_FILE = SCOREP_MOVABLE_NULL
    integer(SCOREP_Allocator_MovableMemory), parameter :: SCOREP_INVALID_INTERIM_COMMUNICATOR = SCOREP_MOVABLE_NULL
    integer(SCOREP_Allocator_MovableMemory), parameter :: SCOREP_INVALID_RMA_WINDOW = SCOREP_MOVABLE_NULL
end module
