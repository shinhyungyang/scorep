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
module SCOREP_PublicTypes_H
    use, intrinsic :: iso_c_binding, only: c_int64_t
    !
    ! typedefs
    !

    ! Handles are usually typedef'd to
    !   SCOREP_AnyHandle := SCOREP_Allocator_MovableMemory := uint32_t
    ! HACK: We are using a signed type here,
    ! as Fortran does not have unsigned types
    use, intrinsic :: iso_c_binding, only: SCOREP_Allocator_MovableMemory => c_int32_t
    use, intrinsic :: iso_c_binding, only: SCOREP_AnyHandle => c_int32_t
    use, intrinsic :: iso_c_binding, only: SCOREP_RegionHandle => c_int32_t

    use, intrinsic :: iso_c_binding, only: SCOREP_MpiRank => c_int

    ! HACK: This is a uint64_t in C
    use, intrinsic :: iso_c_binding, only: SCOREP_MpiRequestId => c_int64_t

    ! Enums
    use, intrinsic :: iso_c_binding, only: SCOREP_CollectiveType => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_LockType => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_RmaSyncType => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_RmaSyncLevel => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_RmaAtomicType => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoParadigmType => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoAccessMode => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoCreationFlag => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoStatusFlag => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoSeekOption => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoOperationMode => c_int
    use, intrinsic :: iso_c_binding, only: SCOREP_IoOperationFlag => c_int

    implicit none

    private :: c_int64_t

    ! enum SCOREP_CollectiveType
    !&<
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_BARRIER                      = 0
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_BROADCAST                    = 1
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_GATHER                       = 2
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_GATHERV                      = 3
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_SCATTER                      = 4
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_SCATTERV                     = 5
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLGATHER                    = 6
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLGATHERV                   = 7
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLTOALL                     = 8
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLTOALLV                    = 9
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLTOALLW                    = 10
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLREDUCE                    = 11
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_REDUCE                       = 12
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_REDUCE_SCATTER               = 13
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_REDUCE_SCATTER_BLOCK         = 14
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_SCAN                         = 15
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_EXSCAN                       = 16
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_CREATE_HANDLE                = 17
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_DESTROY_HANDLE               = 18
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_ALLOCATE                     = 19
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_DEALLOCATE                   = 20
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_CREATE_HANDLE_AND_ALLOCATE   = 21
    integer(SCOREP_CollectiveType), parameter :: SCOREP_COLLECTIVE_DESTROY_HANDLE_AND_DEALLOCATE= 22
    !&>

    ! enum SCOREP_LockType
    !&<
    integer(SCOREP_LockType), parameter :: SCOREP_LOCK_EXCLUSIVE    = 0
    integer(SCOREP_LockType), parameter :: SCOREP_LOCK_SHARED       = 1
    integer(SCOREP_LockType), parameter :: SCOREP_INVALID_LOCK_TYPE = 2
    !&>

    ! enum SCOREP_RmaSyncType
    !&<
    integer(SCOREP_RmaSyncType), parameter :: SCOREP_RMA_SYNC_TYPE_MEMORY      = 0
    integer(SCOREP_RmaSyncType), parameter :: SCOREP_RMA_SYNC_TYPE_NOTIFY_IN   = 1
    integer(SCOREP_RmaSyncType), parameter :: SCOREP_RMA_SYNC_TYPE_NOTIFY_OUT  = 2
    integer(SCOREP_RmaSyncType), parameter :: SCOREP_INVALID_RMA_SYNC_TYPE     = 3
    !&>

    ! enum SCOREP_RmaSyncLevel
    !&<
    integer(SCOREP_RmaSyncLevel), parameter :: SCOREP_RMA_SYNC_LEVEL_NONE    = 0
    integer(SCOREP_RmaSyncLevel), parameter :: SCOREP_RMA_SYNC_LEVEL_PROCESS = ibset(0, 0)
    integer(SCOREP_RmaSyncLevel), parameter :: SCOREP_RMA_SYNC_LEVEL_MEMORY  = ibset(0, 1)
    !&>

    ! enum SCOREP_RmaAtomicType
    !&<
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_ACCUMULATE           = 0
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_INCREMENT            = 1
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_TEST_AND_SET         = 2
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_COMPARE_AND_SWAP     = 3
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_SWAP                 = 4
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_FETCH_AND_ADD        = 5
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_FETCH_AND_INCREMENT  = 6
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_ADD                  = 7
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_RMA_ATOMIC_TYPE_FETCH_AND_ACCUMULATE = 8
    integer(SCOREP_RmaAtomicType), parameter :: SCOREP_INVALID_RMA_ATOMIC_TYPE              = 9
    !&>

    ! enum SCOREP_IoParadigmType
    !&<
    integer(SCOREP_IoParadigmType), parameter :: SCOREP_IO_PARADIGM_POSIX        = 0
    integer(SCOREP_IoParadigmType), parameter :: SCOREP_IO_PARADIGM_ISOC         = 1
    integer(SCOREP_IoParadigmType), parameter :: SCOREP_IO_PARADIGM_MPI          = 2
    integer(SCOREP_IoParadigmType), parameter :: SCOREP_INVALID_IO_PARADIGM_TYPE = 3
    !&>

    ! enum SCOREP_IoAccessMode
    !&<
    integer(SCOREP_IoAccessMode), parameter :: SCOREP_IO_ACCESS_MODE_NONE         = 0
    integer(SCOREP_IoAccessMode), parameter :: SCOREP_IO_ACCESS_MODE_READ_ONLY    = 1
    integer(SCOREP_IoAccessMode), parameter :: SCOREP_IO_ACCESS_MODE_WRITE_ONLY   = 2
    integer(SCOREP_IoAccessMode), parameter :: SCOREP_IO_ACCESS_MODE_READ_WRITE   = 3
    integer(SCOREP_IoAccessMode), parameter :: SCOREP_IO_ACCESS_MODE_EXECUTE_ONLY = 4
    integer(SCOREP_IoAccessMode), parameter :: SCOREP_IO_ACCESS_MODE_SEARCH_ONLY  = 5
    !&>

    ! enum SCOREP_IoCreationFlag
    !&<
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_NONE                    = 0
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_CREATE                  = ibset(0,0)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_TRUNCATE                = ibset(0,1)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_DIRECTORY               = ibset(0,2)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_EXCLUSIVE               = ibset(0,3)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_NO_CONTROLLING_TERMINAL = ibset(0,4)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_NO_FOLLOW               = ibset(0,5)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_PATH                    = ibset(0,6)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_TEMPORARY_FILE          = ibset(0,7)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_LARGEFILE               = ibset(0,8)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_NO_SEEK                 = ibset(0,9)
    integer(SCOREP_IoCreationFlag), parameter :: SCOREP_IO_CREATION_FLAG_UNIQUE                  = ibset(0,10)
    !&>

    ! enum SCOREP_IoStatusFlag
    !&<
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_NONE            = 0
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_CLOSE_ON_EXEC   = ibset(0,0)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_APPEND          = ibset(0,1)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_NON_BLOCKING    = ibset(0,2)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_ASYNC           = ibset(0,3)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_SYNC            = ibset(0,4)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_DATA_SYNC       = ibset(0,5)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_AVOID_CACHING   = ibset(0,6)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_NO_ACCESS_TIME  = ibset(0,7)
    integer(SCOREP_IoStatusFlag), parameter :: SCOREP_IO_STATUS_FLAG_DELETE_ON_CLOSE = ibset(0,8)
    !&>

    ! enum SCOREP_IoSeekOption
    !&<
    integer(SCOREP_IoSeekOption), parameter :: SCOREP_IO_SEEK_FROM_START   = 0
    integer(SCOREP_IoSeekOption), parameter :: SCOREP_IO_SEEK_FROM_CURRENT = 1
    integer(SCOREP_IoSeekOption), parameter :: SCOREP_IO_SEEK_FROM_END     = 2
    integer(SCOREP_IoSeekOption), parameter :: SCOREP_IO_SEEK_DATA         = 3
    integer(SCOREP_IoSeekOption), parameter :: SCOREP_IO_SEEK_HOLE         = 4
    integer(SCOREP_IoSeekOption), parameter :: SCOREP_IO_SEEK_INVALID      = 5
    !&>

    ! enum SCOREP_IoOperationMode
    !&<
    integer(SCOREP_IoOperationMode), parameter :: SCOREP_IO_OPERATION_MODE_READ  = 0
    integer(SCOREP_IoOperationMode), parameter :: SCOREP_IO_OPERATION_MODE_WRITE = 1
    integer(SCOREP_IoOperationMode), parameter :: SCOREP_IO_OPERATION_MODE_FLUSH = 2
    !&>

    ! enum SCOREP_IoOperationFlag
    !&<
    integer(SCOREP_IoOperationFlag), parameter :: SCOREP_IO_OPERATION_FLAG_NONE = 0
    integer(SCOREP_IoOperationFlag), parameter :: SCOREP_IO_OPERATION_FLAG_BLOCKING = 0
    integer(SCOREP_IoOperationFlag), parameter :: SCOREP_IO_OPERATION_FLAG_NON_BLOCKING = ibset(0,0)
    integer(SCOREP_IoOperationFlag), parameter :: SCOREP_IO_OPERATION_FLAG_COLLECTIVE = ibset(0,1)
    integer(SCOREP_IoOperationFlag), parameter :: SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE = 0
    !&>

    ! #define SCOREP_IO_UNKNOWN_OFFSET UINT64_MAX
    ! HACK: Fortran only has signed integer types, this relies on the assumption
    ! that the bit representation of -1(signed) = UINT64_MAX(unsigned)
    integer(c_int64_t), parameter :: SCOREP_IO_UNKNOWN_OFFSET = -1

    ! #define SCOREP_ALL_TARGET_RANKS
    integer(SCOREP_MpiRank), parameter :: SCOREP_ALL_TARGET_RANKS = -1

    ! #define SCOREP_INVALID_ROOT_RANK
    integer(SCOREP_MpiRank), parameter :: SCOREP_INVALID_ROOT_RANK = -1

    ! #define SCOREP_MPI_ROOT
    integer(SCOREP_MpiRank), parameter :: SCOREP_MPI_ROOT = -2

    ! #define SCOREP_MPI_PROC_NULL
    integer(SCOREP_MpiRank), parameter :: SCOREP_MPI_PROC_NULL = -3

    integer(SCOREP_Allocator_MovableMemory), parameter :: SCOREP_MOVABLE_NULL = 0

end module
