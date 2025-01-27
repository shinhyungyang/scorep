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
module scorep_mpi_request_mgmt_h
    use, intrinsic :: iso_c_binding, only: &
        c_ptr, c_f_pointer, c_loc, &
        c_int, c_int64_t, c_size_t, c_bool, &
        scorep_mpi_request_type => c_int, &
        scorep_mpi_request_flag => c_int64_t

    use :: mpi_f08, only:MPI_Request, &
        MPI_Status, &
        MPI_Datatype, &
        MPI_Comm, &
        MPI_File

    use :: SCOREP_PublicTypes_H, only:SCOREP_MpiRequestId, &
        SCOREP_CollectiveType, &
        SCOREP_IoOperationMode
    use :: scorep_mpi_interop_status_h

    implicit none

    private

    public :: &
        c_size_t, &
        c_f_pointer, &
        c_loc

    public :: &
        scorep_mpi_request_is_null, &
        scorep_mpi_get_request_id, &
        scorep_mpi_unmark_request, &
        scorep_mpi_request_p2p_create, &
        scorep_mpi_request_comm_idup_create, &
        scorep_mpi_request_icoll_create, &
        scorep_mpi_request_win_create, &
        scorep_mpi_request_io_create, &
        scorep_mpi_request_get, &
        scorep_mpi_request_free, &
        scorep_mpi_check_all_or_test_all, &
        scorep_mpi_check_all_or_none, &
        scorep_mpi_check_some_test_some, &
        scorep_mpi_check_some, &
        scorep_mpi_test_all, &
        scorep_mpi_request_start, &
        scorep_mpi_request_set_completed, &
        scorep_mpi_request_set_cancel, &
        scorep_mpi_request_free_wrapper, &
        scorep_mpi_request_tested, &
        scorep_mpi_check_request, &
        scorep_mpi_check_request_interop, &
        scorep_mpi_cleanup_request, &
        scorep_mpi_sizeof_f08_status, &
        scorep_mpi_save_request_array, &
        scorep_mpi_saved_request_get, &
        scorep_mpi_get_status_array

    ! enum scorep_mpi_request_type
    !&<
    integer(scorep_mpi_request_type), parameter, public :: &
        SCOREP_MPI_REQUEST_TYPE_NONE      = 0, &
        SCOREP_MPI_REQUEST_TYPE_SEND      = 1, &
        SCOREP_MPI_REQUEST_TYPE_RECV      = 2, &
        SCOREP_MPI_REQUEST_TYPE_IO        = 3, &
        SCOREP_MPI_REQUEST_TYPE_RMA       = 4, &
        SCOREP_MPI_REQUEST_TYPE_COMM_IDUP = 5, &
        SCOREP_MPI_REQUEST_TYPE_ICOLL     = 6
    !&>

    ! enum scorep_mpi_request_flags
    !&<
    integer(scorep_mpi_request_flag), parameter, public :: &
        SCOREP_MPI_REQUEST_FLAG_NONE          = int(z'000', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT = int(z'001', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_DEALLOCATE    = int(z'002', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE     = int(z'010', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_ANY_TAG       = int(z'020', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_ANY_SRC       = int(z'040', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_CAN_CANCEL    = int(z'080', kind=scorep_mpi_request_flag), &
        SCOREP_MPI_REQUEST_FLAG_IS_COMPLETED  = int(z'100', kind=scorep_mpi_request_flag)
    !&>

    interface
        function scorep_mpi_request_is_null(scorepRequest) result(flag) &
            bind(c, name="scorep_mpi_request_is_null_fromF08")
            import
            implicit none
            type(c_ptr), intent(in), value :: scorepRequest
            logical(c_bool) :: flag
        end function
    end interface

    interface
        function scorep_mpi_get_request_id() result(id) bind(c)
            import
            implicit none
            integer(SCOREP_MpiRequestId) :: id
        end function
    end interface

    interface
        subroutine scorep_mpi_unmark_request(req) bind(c)
            import
            implicit none
            type(c_ptr), intent(in), value :: req
        end subroutine
    end interface

    interface scorep_mpi_request_p2p_create
        module procedure scorep_mpi_request_p2p_create
    end interface

    interface scorep_mpi_request_comm_idup_create
        module procedure scorep_mpi_request_comm_idup_create
    end interface

    interface scorep_mpi_request_icoll_create
        module procedure scorep_mpi_request_icoll_create
    end interface

    interface scorep_mpi_request_win_create
        module procedure scorep_mpi_request_win_create
    end interface

    interface scorep_mpi_request_io_create
        module procedure scorep_mpi_request_io_create
    end interface

    interface scorep_mpi_request_get
        module procedure scorep_mpi_request_get
    end interface

    interface
        subroutine scorep_mpi_request_free(req) bind(c)
            import
            implicit none
            type(c_ptr), intent(in), value :: req
        end
    end interface

    interface scorep_mpi_check_all_or_test_all
        module procedure scorep_mpi_check_all_or_test_all_scalar
        module procedure scorep_mpi_check_all_or_test_all_array
    end interface

    interface scorep_mpi_check_all_or_none
        module procedure scorep_mpi_check_all_or_none_scalar
        module procedure scorep_mpi_check_all_or_none_array
    end interface

    interface scorep_mpi_check_some_test_some
        module procedure scorep_mpi_check_some_test_some_scalar
        module procedure scorep_mpi_check_some_test_some_array
    end interface

    interface scorep_mpi_check_some
        module procedure scorep_mpi_check_some_scalar
        module procedure scorep_mpi_check_some_array
    end interface

    interface scorep_mpi_test_all
        module procedure scorep_mpi_test_all_array
    end interface

    interface
        subroutine scorep_mpi_request_tested(req) bind(c)
            import
            implicit none
            type(c_ptr), intent(in), value :: req
        end subroutine
    end interface

    interface scorep_mpi_check_request
        module procedure scorep_mpi_check_request
    end interface

    interface
        subroutine scorep_mpi_check_request_interop(req, interopStatus) bind(c)
            import
            implicit none
            type(c_ptr), intent(in), value :: req
            type(scorep_mpi_interop_status), intent(in) :: interopStatus
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_cleanup_request(req) bind(c)
            import
            implicit none
            type(c_ptr), intent(in), value :: req
        end subroutine
    end interface

    interface scorep_mpi_sizeof_f08_status
        module procedure scorep_mpi_sizeof_f08_status
    end interface

    interface scorep_mpi_save_request_array
        module procedure scorep_mpi_save_request_array_scalar
        module procedure scorep_mpi_save_request_array_array
    end interface

    interface
        function scorep_mpi_saved_request_get(arrayIndex) result(req) &
            bind(c, name="scorep_mpi_saved_f08_request_get_fromF08")
            import
            implicit none
            integer(c_size_t), intent(in), value :: arrayIndex
            type(c_ptr) :: req
        end function
    end interface

    interface
        function scorep_mpi_get_status_array(arraySize) result(statusArray) &
            bind(c, name="scorep_mpi_get_f08_status_array_fromF08")
            import
            implicit none
            integer(c_size_t), intent(in), value :: arraySize
            type(c_ptr) :: statusArray
        end function
    end interface

    interface
        subroutine scorep_mpi_request_p2p_create_toC( &
            request, &
            type, &
            flags, &
            tag, &
            dest, &
            bytes, &
            datatype, &
            comm, &
            id) bind(c, name="scorep_mpi_request_p2p_create_fromF08")
            import
            implicit none
            integer, intent(in) :: request
            integer(scorep_mpi_request_type), intent(in) :: type
            integer(scorep_mpi_request_flag), intent(in) :: flags
            integer, intent(in) :: tag
            integer, intent(in) :: dest
            integer(c_int64_t), intent(in) :: bytes
            integer, intent(in) :: datatype
            integer, intent(in) :: comm
            integer(SCOREP_MpiRequestId), intent(in) :: id
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_request_comm_idup_create_toC(request, parentComm, newcomm, id) &
            bind(c, name="scorep_mpi_request_comm_idup_create_fromF08")
            import
            implicit none
            integer request, parentComm
            type(c_ptr), value :: newcomm
            integer(SCOREP_MpiRequestId) :: id
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_request_icoll_create_toC( &
            request, &
            flags, &
            collectiveType, &
            rootLoc, &
            bytesSent, &
            bytesRecv, &
            comm, &
            id &
            ) bind(c, name="scorep_mpi_request_icoll_create_fromF08")
            import
            implicit none
            integer, intent(in) :: request
            integer(scorep_mpi_request_flag), intent(in) :: flags
            integer(SCOREP_CollectiveType), intent(in) :: collectiveType
            integer, intent(in) :: rootLoc
            integer(c_int64_t), intent(in) :: bytesSent, bytesRecv
            integer :: comm
            integer(SCOREP_MpiRequestId) :: id
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_request_win_create_toC(mpiRequest, rmaRequest) &
            bind(c, name='scorep_mpi_request_win_create_fromF08')
            import
            implicit none
            integer, intent(in) :: mpiRequest
            type(c_ptr), intent(in), value :: rmaRequest
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_request_io_create_toC(request, mode, bytes, datatype, fh, id) &
            bind(c, name='scorep_mpi_request_io_create_fromF08')
            import
            implicit none
            integer, intent(in) :: request
            integer(SCOREP_IoOperationMode), intent(in) :: mode
            integer(c_int64_t), intent(in) :: bytes
            integer, intent(in) :: datatype
            integer, intent(in) :: fh
            integer(SCOREP_MpiRequestId), intent(in) :: id
        end subroutine
    end interface

    interface
        function scorep_mpi_request_get_toC(request) result(scorepRequest) &
            bind(c, name="scorep_mpi_request_get_fromF08")
            import
            implicit none
            integer, intent(in) :: request
            type(c_ptr) :: scorepRequest
        end function
    end interface

    interface
        subroutine scorep_mpi_save_f08_request_array_toC(requestArray, arraySize) &
            bind(c, name="scorep_mpi_save_f08_request_array_fromF08")
            import
            implicit none
            integer, dimension(*), intent(in) :: requestArray
            integer(c_size_t), intent(in), value :: arraySize
        end subroutine
    end interface

contains

    subroutine scorep_mpi_request_p2p_create( &
        request, &
        type, &
        flags, &
        tag, &
        dest, &
        bytes, &
        datatype, &
        comm, &
        id)
        type(MPI_Request), intent(in) :: request
        integer(scorep_mpi_request_type), intent(in) :: type
        integer(scorep_mpi_request_flag), intent(in) :: flags
        integer, intent(in) :: tag
        integer, intent(in) :: dest
        integer(c_int64_t), intent(in) :: bytes
        type(MPI_Datatype), intent(in) :: datatype
        type(MPI_Comm), intent(in) :: comm
        integer(SCOREP_MpiRequestId), intent(in) :: id

        call scorep_mpi_request_p2p_create_toC( &
            request%MPI_VAL, &
            type, &
            flags, &
            tag, &
            dest, &
            bytes, &
            datatype%MPI_VAL, &
            comm%MPI_VAL, &
            id)
    end subroutine

    subroutine scorep_mpi_request_comm_idup_create(request, parentComm, newcomm, id)
        type(MPI_Request), intent(in) :: request
        type(MPI_Comm), intent(in) :: parentComm
        type(MPI_Comm), intent(in), target :: newcomm
        integer(SCOREP_MpiRequestId), intent(in) :: id

        type(c_ptr) :: newcomm_loc

        newcomm_loc = c_loc(newcomm%MPI_VAL)

        call scorep_mpi_request_comm_idup_create_toC( &
            request%MPI_VAL, &
            parentComm%MPI_VAL, &
            newcomm_loc, &
            id)
    end subroutine

    subroutine scorep_mpi_request_icoll_create( &
        request, &
        flags, &
        collectiveType, &
        rootLoc, &
        bytesSent, &
        bytesRecv, &
        comm, &
        id)
        type(MPI_Request), intent(in) :: request
        integer(scorep_mpi_request_flag), intent(in) :: flags
        integer(SCOREP_CollectiveType), intent(in) :: collectiveType
        integer, intent(in) :: rootLoc
        integer(c_int64_t), intent(in) :: bytesSent, bytesRecv
        type(MPI_Comm), intent(in) :: comm
        integer(SCOREP_MpiRequestId), intent(in) :: id

        call scorep_mpi_request_icoll_create_toC( &
            request%MPI_VAL, &
            flags, &
            collectiveType, &
            rootLoc, &
            bytesSent, &
            bytesRecv, &
            comm%MPI_VAL, &
            id)
    end subroutine

    subroutine scorep_mpi_request_win_create(mpiRequest, rmaRequest)
        type(MPI_Request), intent(in) :: mpiRequest
        type(c_ptr), intent(in), value :: rmaRequest

        call scorep_mpi_request_win_create_toC(mpiRequest%MPI_VAL, rmaRequest)
    end subroutine

    subroutine scorep_mpi_request_io_create(request, mode, bytes, datatype, fh, id)
        type(MPI_Request), intent(in) :: request
        integer(SCOREP_IoOperationMode), intent(in) :: mode
        integer(c_int64_t), intent(in) :: bytes
        type(MPI_Datatype), intent(in) :: datatype
        type(MPI_File), intent(in) :: fh
        integer(SCOREP_MpiRequestId), intent(in) :: id

        call scorep_mpi_request_io_create_toC(request%MPI_VAL, mode, bytes, datatype%MPI_VAL, fh%MPI_VAL, id)
    end subroutine

    function scorep_mpi_request_get(request) result(scorepRequest)
        type(MPI_Request), intent(in) :: request
        type(c_ptr) :: scorepRequest

        scorepRequest = scorep_mpi_request_get_toC(request%MPI_VAL)
    end function

    subroutine scorep_mpi_check_all_or_test_all_scalar(count, flag, status)
        integer, intent(in) :: count
        logical, intent(in) :: flag
        type(MPI_Status), intent(inout) :: status

        type(MPI_Status), dimension(1) :: array_of_statuses

        array_of_statuses(1) = status
        call scorep_mpi_check_all_or_test_all_array(count, flag, array_of_statuses)
    end subroutine

    subroutine scorep_mpi_check_all_or_test_all_array(count, flag, array_of_statuses)
        integer, intent(in) :: count
        logical, intent(in) :: flag
        type(MPI_Status), intent(inout), dimension(:) :: array_of_statuses

        integer(c_size_t) :: i
        type(c_ptr) :: scorep_req

        if (flag) then
            do i = 1, count
                scorep_req = scorep_mpi_saved_request_get(i - 1)
                call scorep_mpi_check_request(scorep_req, array_of_statuses(i))
                call scorep_mpi_cleanup_request(scorep_req)
                call scorep_mpi_unmark_request(scorep_req)
            end do
        else
            do i = 1, count
                scorep_req = scorep_mpi_saved_request_get(i - 1)
                call scorep_mpi_request_tested(scorep_req)
                call scorep_mpi_unmark_request(scorep_req)
            end do
        end if
    end subroutine

    subroutine scorep_mpi_check_all_or_none_scalar(count, flag, status)
        integer, intent(in) :: count
        logical, intent(in) :: flag
        type(MPI_Status), intent(inout) :: status

        type(MPI_Status), dimension(1) :: array_of_statuses

        array_of_statuses(1) = status
        call scorep_mpi_check_all_or_none_array(count, flag, array_of_statuses)
    end subroutine

    subroutine scorep_mpi_check_all_or_none_array(count, flag, array_of_statuses)
        integer, intent(in) :: count
        logical, intent(in) :: flag
        type(MPI_Status), intent(inout), dimension(:) :: array_of_statuses

        integer(c_size_t) :: i
        type(c_ptr) :: scorep_req

        if (flag) then
            do i = 1, count
                scorep_req = scorep_mpi_saved_request_get(i - 1)
                call scorep_mpi_check_request(scorep_req, array_of_statuses(i))
                call scorep_mpi_cleanup_request(scorep_req)
                call scorep_mpi_unmark_request(scorep_req)
            end do
        end if
    end subroutine

    subroutine scorep_mpi_check_some_test_some_scalar(incount, outcount, index, status)
        integer, intent(in) :: incount, outcount
        integer, intent(inout) :: index
        type(MPI_Status), intent(inout) :: status

        integer, dimension(1) :: array_of_indices
        type(MPI_Status), dimension(1) :: array_of_statuses

        array_of_indices(1) = index
        array_of_statuses(1) = status
        call scorep_mpi_check_some_test_some_array(incount, outcount, array_of_indices, array_of_statuses)
    end subroutine

    subroutine scorep_mpi_check_some_test_some_array(incount, outcount, array_of_indices, array_of_statuses)
        integer, intent(in) :: incount, outcount
        integer, intent(inout), dimension(*) :: array_of_indices
        type(MPI_Status), intent(inout), dimension(:) :: array_of_statuses

        integer :: cur, j, tmp
        integer(c_size_t) :: req_idx
        type(c_ptr) :: scorep_req
        type(MPI_Status) :: tmpstat

        cur = 1

        do req_idx = 1, incount
            scorep_req = scorep_mpi_saved_request_get(req_idx - 1)

            if (.not. scorep_mpi_request_is_null(scorep_req)) then
                j = cur
                do while (j .le. outcount .and. req_idx .ne. array_of_indices(j))
                    j = j + 1
                end do

                if (j .le. outcount) then
                    tmp = array_of_indices(cur)
                    array_of_indices(cur) = array_of_indices(j)
                    array_of_indices(j) = tmp

                    tmpstat = array_of_statuses(cur)
                    array_of_statuses(cur) = array_of_statuses(j)
                    array_of_statuses(j) = tmpstat

                    call scorep_mpi_check_request(scorep_req, array_of_statuses(cur))
                    call scorep_mpi_cleanup_request(scorep_req)

                    cur = cur + 1
                else
                    call scorep_mpi_request_tested(scorep_req)
                end if
                call scorep_mpi_unmark_request(scorep_req)
            end if
        end do
    end subroutine

    subroutine scorep_mpi_check_some_scalar(incount, outcount, index, status)
        integer, intent(in) :: incount, outcount
        integer, intent(inout) :: index
        type(MPI_Status), intent(inout) :: status

        integer, dimension(1) :: array_of_indices
        type(MPI_Status), dimension(1) :: array_of_statuses
        array_of_indices(1) = index
        array_of_statuses(1) = status

        call scorep_mpi_check_some_array(incount, outcount, array_of_indices, array_of_statuses)
    end subroutine

    subroutine scorep_mpi_check_some_array(incount, outcount, array_of_indices, array_of_statuses)
        integer, intent(in) :: incount, outcount
        integer, intent(inout), dimension(*) :: array_of_indices
        type(MPI_Status), intent(inout), dimension(:) :: array_of_statuses

        integer(c_size_t) :: j
        type(c_ptr) :: scorep_req

        do j = 1, outcount
            scorep_req = scorep_mpi_saved_request_get(int(array_of_indices(j) - 1, c_size_t))
            call scorep_mpi_check_request(scorep_req, array_of_statuses(j))
            call scorep_mpi_cleanup_request(scorep_req)
            call scorep_mpi_unmark_request(scorep_req)
        end do
    end subroutine

    subroutine scorep_mpi_test_all_array(count)
        integer, intent(in) :: count

        integer(c_size_t) :: i
        type(c_ptr) :: scorep_req

        do i = 1, count
            scorep_req = scorep_mpi_saved_request_get(i - 1)
            call scorep_mpi_request_tested(scorep_req)
            call scorep_mpi_unmark_request(scorep_req)
        end do
    end subroutine

    subroutine scorep_mpi_check_request(scorepRequest, status)
        type(c_ptr), intent(in) :: scorepRequest
        type(MPI_Status) :: status

        type(scorep_mpi_interop_status) :: interop_status

        interop_status = scorep_mpi_interop_status_create(status)
        call scorep_mpi_check_request_interop(scorepRequest, interop_status)
    end subroutine

    function scorep_mpi_sizeof_f08_status() result(bytes) bind(C, name='scorep_mpi_sizeof_f08_status_toF08')
        integer :: bytes

        type(MPI_Status) :: status

        ! storage_size returns the size of an object in bits
        bytes = storage_size(status) / 8
    end function

    subroutine scorep_mpi_save_request_array_scalar(request)
        type(MPI_Request), intent(in) :: request

        integer(c_size_t) :: arr_size = 1
        integer, dimension(1) :: f90_handle_array

        f90_handle_array = request%MPI_VAL
        call scorep_mpi_save_f08_request_array_toC(f90_handle_array, arr_size)
    end subroutine

    subroutine scorep_mpi_save_request_array_array(requestArray)
        type(MPI_Request), dimension(:), intent(in) :: requestArray

        integer(c_size_t) :: arr_size
        integer, dimension(size(requestArray, 1)) :: f90_handle_array

        arr_size = size(requestArray, 1)
        f90_handle_array = requestArray%MPI_VAL
        call scorep_mpi_save_f08_request_array_toC(f90_handle_array, arr_size)
    end subroutine

end module
