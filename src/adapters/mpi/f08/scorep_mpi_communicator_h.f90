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
module scorep_mpi_communicator_h
    use iso_c_binding, only: c_char
    use :: mpi_f08, only:MPI_Comm, MPI_Group, MPI_Win
    use :: scorep_mpi_f08_utils, only:c_string
    use :: SCOREP_PublicHandles_H, only:SCOREP_RmaWindowHandle
    use :: SCOREP_PublicTypes_H, only:SCOREP_CollectiveType
    use :: scorep_mpi_communicator_mgmt_h, only:Scorep_Mpi_EpochType, SCOREP_Mpi_GroupHandle

    implicit none

    private

    ! enum SCOREP_Mpi_EpochType_enum
    ! WARNING: In scorep_mpi_communicator.h, the enum values are normal 32-bit integers
    !          but Scorep_Mpi_EpochType is a 8-bit integer
    !&<
    integer(Scorep_Mpi_EpochType), parameter, public :: SCOREP_MPI_RMA_ACCESS_EPOCH   = 0
    integer(Scorep_Mpi_EpochType), parameter, public :: SCOREP_MPI_RMA_EXPOSURE_EPOCH = 1
    !&>

    public :: scorep_mpi_comm_free, &
              scorep_mpi_group_create, &
              scorep_mpi_group_free, &
              scorep_mpi_group_handle, &
              scorep_mpi_comm_set_name, &
              scorep_mpi_comm_set_default_names, &
              scorep_mpi_win_create, &
              scorep_mpi_win_free, &
              scorep_mpi_win_handle, &
              scorep_mpi_win_collective_type, &
              scorep_mpi_win_set_name, &
              scorep_mpi_epoch_start, &
              scorep_mpi_epoch_end, &
              scorep_mpi_epoch_get_group_handle

    interface scorep_mpi_comm_free
        module procedure scorep_mpi_comm_free
    end interface

    interface scorep_mpi_group_create
        module procedure scorep_mpi_group_create
    end interface

    interface scorep_mpi_group_free
        module procedure scorep_mpi_group_free
    end interface

    interface scorep_mpi_group_handle
        module procedure scorep_mpi_group_handle
    end interface

    interface scorep_mpi_comm_set_name
        module procedure scorep_mpi_comm_set_name
    end interface

    interface
        subroutine scorep_mpi_comm_set_default_names() &
            bind(c, name="scorep_mpi_comm_set_default_names")
            implicit none
        end subroutine
    end interface

    interface scorep_mpi_win_create
        module procedure scorep_mpi_win_create
    end interface

    interface scorep_mpi_win_free
        module procedure scorep_mpi_win_free
    end interface

    interface scorep_mpi_win_handle
        module procedure scorep_mpi_win_handle
    end interface

    interface scorep_mpi_win_collective_type
        module procedure scorep_mpi_win_collective_type
    end interface

    interface scorep_mpi_win_set_name
        module procedure scorep_mpi_win_set_name
    end interface

    interface scorep_mpi_epoch_start
        module procedure scorep_mpi_epoch_start
    end interface

    interface scorep_mpi_epoch_end
        module procedure scorep_mpi_epoch_end
    end interface

    interface scorep_mpi_epoch_get_group_handle
        module procedure scorep_mpi_epoch_get_group_handle
    end interface

    interface
        subroutine scorep_mpi_comm_free_toC(comm) &
            bind(c, name='scorep_mpi_comm_free_fromF08')
            implicit none
            integer, intent(in) :: comm
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_group_create_toC(group) &
            bind(c, name='scorep_mpi_group_create_fromF08')
            implicit none
            integer, intent(in) :: group
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_group_free_toC(group) &
            bind(c, name='scorep_mpi_group_free_fromF08')
            implicit none
            integer, intent(in) :: group
        end subroutine
    end interface

    interface
        function scorep_mpi_group_handle_toC(group) result(groupHandle) &
            bind(c, name='scorep_mpi_group_handle_fromF08')
            import
            implicit none
            integer, intent(in) :: group
            integer(SCOREP_Mpi_GroupHandle) :: groupHandle
        end function
    end interface

    interface
        subroutine scorep_mpi_comm_set_name_toC(comm, name) &
            bind(c, name='scorep_mpi_comm_set_name_fromF08')
            import
            implicit none
            integer, intent(in) :: comm
            character(kind=c_char) :: name(*)
        end subroutine
    end interface

    interface
        function scorep_mpi_win_create_toC(name, win, comm) result(windowHandle) &
            bind(c, name='scorep_mpi_win_create_fromF08')
            import
            implicit none
            character(kind=c_char) :: name(*)
            integer, intent(in) :: win
            integer, intent(in) :: comm
            integer(SCOREP_RmaWindowHandle) :: windowHandle
        end function
    end interface

    interface
        subroutine scorep_mpi_win_free_toC(win) &
            bind(c, name='scorep_mpi_win_free_fromF08')
            implicit none
            integer, intent(in) :: win
        end subroutine
    end interface

    interface
        function scorep_mpi_win_handle_toC(win) result(windowHandle) &
            bind(c, name='scorep_mpi_win_handle_fromF08')
            import
            implicit none
            integer, intent(in) :: win
            integer(SCOREP_RmaWindowHandle) :: windowHandle
        end function
    end interface

    interface
        function scorep_mpi_win_collective_type_toC(win) result(collectiveType) &
            bind(c, name='scorep_mpi_win_collective_type_fromF08')
            import
            implicit none
            integer, intent(in) :: win
            integer(SCOREP_CollectiveType) :: collectiveType
        end function
    end interface

    interface
        subroutine scorep_mpi_win_set_name_toC(win, name) &
            bind(c, name='scorep_mpi_win_set_name_fromF08')
            import
            implicit none
            integer, intent(in) :: win
            character(kind=c_char) :: name(*)
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_epoch_start_toC(win, group, epochType) &
            bind(c, name='scorep_mpi_epoch_start_fromF08')
            import
            implicit none
            integer, intent(in) :: win
            integer, intent(in) :: group
            integer(Scorep_Mpi_EpochType) :: epochType
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_epoch_end_toC(win, epochType) &
            bind(c, name='scorep_mpi_epoch_end_fromF08')
            import
            implicit none
            integer, intent(in) :: win
            integer(Scorep_Mpi_EpochType) :: epochType
        end subroutine
    end interface

    interface
        function scorep_mpi_epoch_get_group_handle_toC(win, epochType) result(groupHandle) &
            bind(c, name='scorep_mpi_epoch_get_group_handle_fromF08')
            import
            implicit none
            integer, intent(in) :: win
            integer(Scorep_Mpi_EpochType) :: epochType
            integer(SCOREP_Mpi_GroupHandle) :: groupHandle
        end function
    end interface

contains

    subroutine scorep_mpi_comm_free(comm)
        type(MPI_Comm), intent(in) :: comm

        call scorep_mpi_comm_free_toC(comm%MPI_VAL)
    end subroutine

    subroutine scorep_mpi_group_create(group)
        type(MPI_Group), intent(in) :: group

        call scorep_mpi_group_create_toC(group%MPI_VAL)
    end subroutine

    subroutine scorep_mpi_group_free(group)
        type(MPI_Group), intent(in) :: group

        call scorep_mpi_group_free_toC(group%MPI_VAL)
    end subroutine

    function scorep_mpi_group_handle(group) result(groupHandle)
        type(MPI_Group), intent(in) :: group
        integer(SCOREP_Mpi_GroupHandle) :: groupHandle

        groupHandle = scorep_mpi_group_handle_toC(group%MPI_VAL)
    end function

    subroutine scorep_mpi_comm_set_name(comm, name)
        type(MPI_Comm), intent(in) :: comm
        character(len=*), intent(in) :: name

        ! Trailing spaces are ignored by MPI_Comm_set_name.
        ! We therefore also trim the name provided to Score-P.
        character(kind=c_char, len=len_trim(name) + 1) :: name_c

        call c_string(trim(name), name_c)
        call scorep_mpi_comm_set_name_toC(comm%MPI_VAL, name_c)
    end subroutine

    function scorep_mpi_win_create(name, win, comm) result(windowHandle)
        character(len=*), intent(in) :: name
        type(MPI_Win), intent(in) :: win
        type(MPI_Comm), intent(in) :: comm
        integer(SCOREP_RmaWindowHandle) :: windowHandle

        character(kind=c_char, len=len_trim(name) + 1) :: name_c

        call c_string(trim(name), name_c)
        windowHandle = scorep_mpi_win_create_toC(name_c, win%MPI_VAL, comm%MPI_VAL)
    end function

    subroutine scorep_mpi_win_free(win)
        type(MPI_Win), intent(in) :: win

        call scorep_mpi_win_free_toC(win%MPI_VAL)
    end subroutine

    function scorep_mpi_win_handle(win) result(windowHandle)
        type(MPI_Win), intent(in) :: win
        integer(SCOREP_RmaWindowHandle) :: windowHandle

        windowHandle = scorep_mpi_win_handle_toC(win%MPI_VAL)
    end function

    function scorep_mpi_win_collective_type(win) result(collectiveType)
        type(MPI_Win), intent(in) :: win
        integer(SCOREP_CollectiveType) :: collectiveType

        collectiveType = scorep_mpi_win_collective_type_toC(win%MPI_VAL)
    end function

    subroutine scorep_mpi_win_set_name(win, name)
        type(MPI_Win), intent(in) :: win
        character(len=*), intent(in) :: name

        character(kind=c_char, len=len_trim(name) + 1) :: name_c

        call c_string(trim(name), name_c)
        call scorep_mpi_win_set_name_toC(win%MPI_VAL, name_c)
    end subroutine

    subroutine scorep_mpi_epoch_start(win, group, epochType)
        type(MPI_Win), intent(in) :: win
        type(MPI_Group), intent(in) :: group
        integer(Scorep_Mpi_EpochType), intent(in) :: epochType

        call scorep_mpi_epoch_start_toC(win%MPI_VAL, group%MPI_VAL, epochType)
    end subroutine

    subroutine scorep_mpi_epoch_end(win, epochType)
        type(MPI_Win), intent(in) :: win
        integer(Scorep_Mpi_EpochType), intent(in) :: epochType

        call scorep_mpi_epoch_end_toC(win%MPI_VAL, epochType)
    end subroutine

    function scorep_mpi_epoch_get_group_handle(win, epochType) result(groupHandle)
        type(MPI_Win), intent(in) :: win
        integer(Scorep_Mpi_EpochType), intent(in) :: epochType

        integer(SCOREP_Mpi_GroupHandle) :: groupHandle

        groupHandle = scorep_mpi_epoch_get_group_handle_toC(win%MPI_VAL, epochType)
    end function
end module
