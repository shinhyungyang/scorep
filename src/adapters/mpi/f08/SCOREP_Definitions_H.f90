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
module SCOREP_Definitions_H
    use :: SCOREP_PublicHandles_H, only:SCOREP_IoFileHandle, &
        SCOREP_IoHandleHandle

    implicit none

    interface
        function SCOREP_IoHandleHandle_GetIoFile(handle) result(file_handle) &
            bind(c, name='SCOREP_IoHandleHandle_GetIoFile_fromF08')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
            integer(SCOREP_IoFileHandle) :: file_handle
        end function
    end interface
end module
