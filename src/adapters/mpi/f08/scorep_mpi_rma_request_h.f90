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
module scorep_mpi_rma_request_h
    use, intrinsic :: iso_c_binding, only: &
        c_int, c_ptr, c_funptr, c_funloc, &
        SCOREP_Mpi_RmaCompletionType => c_int
    use :: mpi_f08, only:MPI_Request
    use :: SCOREP_PublicTypes_H, only:SCOREP_MpiRequestId
    use :: SCOREP_PublicHandles_H, only:SCOREP_RmaWindowHandle

    implicit none

    private

    public :: c_funloc
    public :: scorep_mpi_rma_request_create, &
              scorep_mpi_rma_request_find, &
              scorep_mpi_rma_request_remove, &
              scorep_mpi_rma_request_remove_by_ptr, &
              scorep_mpi_rma_request_foreach_on_window, &
              scorep_mpi_rma_request_foreach_to_target, &
              scorep_mpi_rma_request_write_standard_completion_and_remove, &
              scorep_mpi_rma_request_write_standard_completion, &
              scorep_mpi_rma_request_write_full_completion, &
              scorep_mpi_rma_request_matching_id

    !&<
    integer(SCOREP_Mpi_RmaCompletionType), parameter, public :: &
        SCOREP_MPI_RMA_REQUEST_SEPARATE_COMPLETION = 0, &
        SCOREP_MPI_RMA_REQUEST_COMBINED_COMPLETION = 1
    !&>

    interface scorep_mpi_rma_request_create
        module procedure scorep_mpi_rma_request_create
    end interface

    interface scorep_mpi_rma_request_find
        module procedure scorep_mpi_rma_request_find
    end interface

    interface scorep_mpi_rma_request_remove
        module procedure scorep_mpi_rma_request_remove
    end interface

    interface
        subroutine scorep_mpi_rma_request_remove_by_ptr(request) bind(c)
            import
            implicit none
            type(c_ptr), intent(in), value :: request
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_rma_request_foreach_on_window(window, callback) bind(c)
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: window
            type(c_funptr), intent(in), value :: callback
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_rma_request_foreach_to_target(window, target, callback) bind(c)
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: window
            integer(c_int), intent(in), value :: target
            type(c_funptr), intent(in), value :: callback
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_rma_request_write_standard_completion_and_remove(request) bind(c)
            use, intrinsic :: iso_c_binding, only: c_ptr
            implicit none
            type(c_ptr), value :: request
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_rma_request_write_standard_completion(request) bind(c)
            use, intrinsic :: iso_c_binding, only: c_ptr
            implicit none
            type(c_ptr), value :: request
        end subroutine
    end interface

    interface
        subroutine scorep_mpi_rma_request_write_full_completion(request) bind(c)
            use, intrinsic :: iso_c_binding, only: c_ptr
            implicit none
            type(c_ptr), value :: request
        end subroutine
    end interface

    interface
        function scorep_mpi_rma_request_matching_id(rmaRequest) result(matchingId) &
            bind(c, name='scorep_mpi_rma_request_matching_id')
            import
            implicit none
            type(c_ptr), value :: rmaRequest
            integer(SCOREP_MpiRequestId) :: matchingId
        end function
    end interface

    interface
        function scorep_mpi_rma_request_create_toC(window, targetRank, mpiHandle, completionType, matchingId) result(request) &
            bind(c, name="scorep_mpi_rma_request_create_fromF08")
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in) :: window
            integer(c_int), intent(in) :: targetRank
            integer, intent(in) :: mpiHandle
            integer(SCOREP_Mpi_RmaCompletionType), intent(in) :: completionType
            integer(SCOREP_MpiRequestId), intent(in) :: matchingId
            type(c_ptr) :: request
        end function
    end interface

    interface
        function scorep_mpi_rma_request_find_toC(window, targetRank, mpiHandle, completionType) result(request) &
            bind(c, name="scorep_mpi_rma_request_find_fromF08")
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in) :: window
            integer(c_int), intent(in) :: targetRank
            integer, intent(in) :: mpiHandle
            integer(SCOREP_Mpi_RmaCompletionType), intent(in) :: completionType
            type(c_ptr) :: request
        end function
    end interface

    interface
        subroutine scorep_mpi_rma_request_remove_toC(window, targetRank, mpiHandle, completionType) &
            bind(c, name="scorep_mpi_rma_request_remove_fromF08")
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in) :: window
            integer(c_int), intent(in) :: targetRank
            integer, intent(in) :: mpiHandle
            integer(SCOREP_Mpi_RmaCompletionType), intent(in) :: completionType
        end subroutine
    end interface

contains

    function scorep_mpi_rma_request_create(window, targetRank, mpiHandle, completionType, matchingId) result(request)
        integer(SCOREP_RmaWindowHandle), intent(in) :: window
        integer(c_int), intent(in) :: targetRank
        type(MPI_Request), intent(in) :: mpiHandle
        integer(SCOREP_Mpi_RmaCompletionType), intent(in) :: completionType
        integer(SCOREP_MpiRequestId), intent(in) :: matchingId
        type(c_ptr) :: request

        request = scorep_mpi_rma_request_create_toC(window, &
                                                    targetRank, &
                                                    mpiHandle%MPI_VAL, &
                                                    completionType, &
                                                    matchingId)
    end function

    function scorep_mpi_rma_request_find(window, targetRank, mpiHandle, completionType) result(request)
        integer(SCOREP_RmaWindowHandle), intent(in) :: window
        integer(c_int), intent(in) :: targetRank
        type(MPI_Request), intent(in) :: mpiHandle
        integer(SCOREP_Mpi_RmaCompletionType), intent(in) :: completionType
        type(c_ptr) :: request

        request = scorep_mpi_rma_request_find_toC(window, &
                                                  targetRank, &
                                                  mpiHandle%MPI_VAL, &
                                                  completionType)
    end function

    subroutine scorep_mpi_rma_request_remove(window, targetRank, mpiHandle, completionType)
        integer(SCOREP_RmaWindowHandle), intent(in) :: window
        integer(c_int), intent(in) :: targetRank
        type(MPI_Request), intent(in) :: mpiHandle
        integer(SCOREP_Mpi_RmaCompletionType), intent(in) :: completionType

        call scorep_mpi_rma_request_remove_toC(window, &
                                               targetRank, &
                                               mpiHandle%MPI_VAL, &
                                               completionType)
    end subroutine

end module
