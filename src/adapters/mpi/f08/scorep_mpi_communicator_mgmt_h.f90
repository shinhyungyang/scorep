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
module scorep_mpi_communicator_mgmt_h
    use, intrinsic :: iso_c_binding, only: SCOREP_Mpi_EpochType => c_int8_t
    use :: mpi_f08, only:MPI_Comm
    use :: SCOREP_PublicHandles_H, only:SCOREP_InterimCommunicatorHandle, &
        SCOREP_Mpi_GroupHandle => SCOREP_GroupHandle

    implicit none

    private

    public :: SCOREP_Mpi_EpochType
    public :: SCOREP_Mpi_GroupHandle
    public :: scorep_mpi_comm_handle, &
              scorep_mpi_comm_create, &
              scorep_mpi_get_scorep_mpi_rank

    interface scorep_mpi_comm_handle
        module procedure scorep_mpi_comm_handle
    end interface

    interface scorep_mpi_comm_create
        module procedure scorep_mpi_comm_create
    end interface

    interface scorep_mpi_get_scorep_mpi_rank
        module procedure scorep_mpi_get_scorep_mpi_rank
    end interface

    interface
        function scorep_mpi_comm_handle_toC(comm) result(commHandle) &
            bind(c, name="scorep_mpi_comm_handle_fromF08")
            import
            implicit none
            integer, intent(in) :: comm
            integer(SCOREP_InterimCommunicatorHandle) :: commHandle
        end function
    end interface

    interface
        function scorep_mpi_comm_create_toC(comm, parentComm) result(commHandle) &
            bind(c, name="scorep_mpi_comm_create_fromF08")
            import
            implicit none
            integer :: comm, parentComm
            integer(SCOREP_InterimCommunicatorHandle) :: commHandle
        end function
    end interface

contains

    function scorep_mpi_comm_handle(comm) result(commHandle)
        type(MPI_Comm), intent(in) :: comm
        integer(SCOREP_InterimCommunicatorHandle) :: commHandle

        commHandle = scorep_mpi_comm_handle_toC(comm%MPI_VAL)
    end function

    function scorep_mpi_comm_create(comm, parentComm) result(commHandle)
        type(MPI_Comm), intent(in) :: comm, parentComm
        integer(SCOREP_InterimCommunicatorHandle) :: commHandle

        commHandle = scorep_mpi_comm_create_toC(comm%MPI_VAL, parentComm%MPI_VAL)
    end function

    ! This is an inline function on the C-side.
    ! Thus, we cannot use an interface and have to re-implement it here
    function scorep_mpi_get_scorep_mpi_rank(rank) result(scorepRank)
        use :: mpi_f08, only:MPI_ROOT, MPI_PROC_NULL
        use :: SCOREP_PublicTypes_H, only:SCOREP_MpiRank, SCOREP_MPI_ROOT, SCOREP_MPI_PROC_NULL
        integer, intent(in) :: rank
        integer(SCOREP_MpiRank) :: scorepRank

        if (rank .eq. MPI_ROOT) then
            scorepRank = SCOREP_MPI_ROOT
        else if (rank .eq. MPI_PROC_NULL) then
            scorepRank = SCOREP_MPI_PROC_NULL
        else
            scorepRank = rank
        end if
    end function
end module
