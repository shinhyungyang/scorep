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
module SCOREP_IoManagement_H
    use, intrinsic :: iso_c_binding, only: c_char, c_ptr, c_int32_t
    use :: mpi_f08, only:MPI_File
    use :: scorep_mpi_f08_utils, only:c_string

    use :: SCOREP_Types_H, only:SCOREP_IoHandleFlag
    use :: SCOREP_PublicTypes_H, only:SCOREP_IoParadigmType
    use :: SCOREP_PublicHandles_H, only: &
        SCOREP_InterimCommunicatorHandle, &
        SCOREP_IoFileHandle, &
        SCOREP_IoHandleHandle

    implicit none

    private

    public SCOREP_IoMgmt_GetIoFileHandle, &
        SCOREP_IoMgmt_BeginHandleCreation, &
        SCOREP_IoMgmt_CompleteHandleCreation, &
        SCOREP_IoMgmt_DropIncompleteHandle, &
        SCOREP_IoMgmt_DestroyHandle, &
        SCOREP_IoMgmt_RemoveHandle, &
        SCOREP_IoMgmt_ReinsertHandle, &
        SCOREP_IoMgmt_GetAndPushHandle, &
        SCOREP_IoMgmt_PushHandle, &
        SCOREP_IoMgmt_PopHandle

    interface SCOREP_IoMgmt_GetIoFileHandle
        module procedure SCOREP_IoMgmt_GetIoFileHandle
    end interface

    interface SCOREP_IoMgmt_BeginHandleCreation
        module procedure SCOREP_IoMgmt_BeginHandleCreation
    end interface

    interface SCOREP_IoMgmt_CompleteHandleCreation
        module procedure SCOREP_IoMgmt_CompleteHandleCreation
    end interface

    interface
        subroutine SCOREP_IoMgmt_DropIncompleteHandle() &
            bind(c, name='SCOREP_IoMgmt_DropIncompleteHandle')
            implicit none
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoMgmt_DestroyHandle(handle) &
            bind(c, name='SCOREP_IoMgmt_DestroyHandle')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
        end subroutine
    end interface

    interface SCOREP_IoMgmt_RemoveHandle
        module procedure SCOREP_IoMgmt_RemoveHandle
    end interface

    interface
        subroutine SCOREP_IoMgmt_ReinsertHandle(paradigm, handle) &
            bind(c, name='SCOREP_IoMgmt_ReinsertHandle')
            import
            implicit none
            integer(SCOREP_IoParadigmType), intent(in), value :: paradigm
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
        end subroutine
    end interface

    interface SCOREP_IoMgmt_GetAndPushHandle
        module procedure SCOREP_IoMgmt_GetAndPushHandle
    end interface

    interface
        subroutine SCOREP_IoMgmt_PushHandle(handle) &
            bind(c, name='SCOREP_IoMgmt_PushHandle')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoMgmt_PopHandle(handle) &
            bind(c, name='SCOREP_IoMgmt_PopHandle')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
        end subroutine
    end interface

    interface
        function SCOREP_IoMgmt_GetIoFileHandle_toC(pathname) result(fileHandle) &
            bind(c, name='SCOREP_IoMgmt_GetIoFileHandle')
            import
            implicit none
            character(kind=c_char), intent(in) :: pathname(*)
            integer(SCOREP_IoFileHandle) :: fileHandle
        end function
    end interface

    interface
        subroutine SCOREP_IoMgmt_BeginHandleCreation_toC(paradigm, flags, scope, name) &
            bind(c, name='SCOREP_IoMgmt_BeginHandleCreation')
            import
            implicit none
            integer(SCOREP_IoParadigmType), intent(in), value :: paradigm
            integer(SCOREP_IoHandleFlag), intent(in), value :: flags
            integer(SCOREP_InterimCommunicatorHandle), value :: scope
            character(kind=c_char), intent(in) :: name(*)
        end subroutine
    end interface

    interface
        function SCOREP_IoMgmt_CompleteHandleCreation_toC(paradigm, file, unifyKey, ioHandle) result(handle) &
            bind(c, name='SCOREP_IoMgmt_CompleteHandleCreation_fromF08')
            import
            implicit none
            integer(SCOREP_IoParadigmType), intent(in) :: paradigm
            integer(SCOREP_IoFileHandle), intent(in) :: file
            integer(c_int32_t), intent(in) :: unifyKey
            integer, intent(in) :: ioHandle

            integer(SCOREP_IoHandleHandle) :: handle
        end function
    end interface

    interface
        function SCOREP_IoMgmt_RemoveHandle_toC(paradigm, ioHandle) result(handle) &
            bind(c, name='SCOREP_IoMgmt_RemoveHandle_fromF08')
            import
            implicit none
            integer(SCOREP_IoParadigmType), intent(in) :: paradigm
            integer, intent(in) :: ioHandle

            integer(SCOREP_IoHandleHandle) :: handle
        end function
    end interface

    interface
        function SCOREP_IoMgmt_GetAndPushHandle_toC(paradigm, ioHandle) result(handle) &
            bind(c, name='SCOREP_IoMgmt_GetAndPushHandle_fromF08')
            import
            implicit none
            integer(SCOREP_IoParadigmType), intent(in) :: paradigm
            integer, intent(in) :: ioHandle

            integer(SCOREP_IoHandleHandle) :: handle
        end function
    end interface

contains

    function SCOREP_IoMgmt_GetIoFileHandle(pathname) result(fileHandle)
        character(len=*), intent(in) :: pathname
        integer(SCOREP_IoFileHandle) :: fileHandle

        character(kind=c_char, len=len(pathname) + 1) :: pathname_c

        call c_string(pathname, pathname_c)
        fileHandle = SCOREP_IoMgmt_GetIoFileHandle_toC(pathname_c)
    end function

    subroutine SCOREP_IoMgmt_BeginHandleCreation(paradigm, flags, scope, name)
        integer(SCOREP_IoParadigmType), intent(in) :: paradigm
        integer(SCOREP_IoHandleFlag), intent(in) :: flags
        integer(SCOREP_InterimCommunicatorHandle) :: scope
        character(len=*), intent(in) :: name

        character(kind=c_char, len=len(name) + 1) :: name_c

        call c_string(name, name_c)
        call SCOREP_IoMgmt_BeginHandleCreation_toC(paradigm, flags, scope, name_c)
    end subroutine

    function SCOREP_IoMgmt_CompleteHandleCreation(paradigm, file, unifyKey, ioHandle) result(handle)
        integer(SCOREP_IoParadigmType), intent(in) :: paradigm
        integer(SCOREP_IoFileHandle), intent(in) :: file
        integer(c_int32_t), intent(in) :: unifyKey
        type(MPI_File), intent(in) :: ioHandle

        integer(SCOREP_IoHandleHandle) :: handle

        handle = SCOREP_IoMgmt_CompleteHandleCreation_toC(paradigm, file, unifyKey, ioHandle%MPI_VAL)
    end function

    function SCOREP_IoMgmt_RemoveHandle(paradigm, ioHandle) result(handle)
        integer(SCOREP_IoParadigmType), intent(in) :: paradigm
        type(MPI_File), intent(in) :: ioHandle

        integer(SCOREP_IoHandleHandle) :: handle

        handle = SCOREP_IoMgmt_RemoveHandle_toC(paradigm, ioHandle%MPI_VAL)
    end function

    function SCOREP_IoMgmt_GetAndPushHandle(paradigm, ioHandle) result(handle)
        integer(SCOREP_IoParadigmType), intent(in) :: paradigm
        type(MPI_File), intent(in) :: ioHandle

        integer(SCOREP_IoHandleHandle) :: handle

        handle = SCOREP_IoMgmt_GetAndPushHandle_toC(paradigm, ioHandle%MPI_VAL)
    end function
end module
