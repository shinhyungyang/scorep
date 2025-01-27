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
module SCOREP_Events_H
    use, intrinsic :: iso_c_binding, only: c_int, c_int32_t, c_int64_t, c_ptr
    use :: SCOREP_PublicTypes_H, only: &
        SCOREP_RegionHandle, &
        SCOREP_MpiRequestId, &
        SCOREP_CollectiveType, &
        SCOREP_LockType, &
        SCOREP_RmaSyncLevel, &
        SCOREP_RmaAtomicType, &
        SCOREP_IoAccessMode, &
        SCOREP_IoCreationFlag, &
        SCOREP_IoStatusFlag, &
        SCOREP_IoSeekOption, &
        SCOREP_IoParadigmType, &
        SCOREP_IoOperationMode, &
        SCOREP_IoOperationFlag
    use :: SCOREP_PublicHandles_H, only: &
        SCOREP_GroupHandle, &
        SCOREP_InterimCommunicatorHandle, &
        SCOREP_RmaWindowHandle, &
        SCOREP_IoFileHandle, &
        SCOREP_IoHandleHandle
    use :: SCOREP_DefinitionHandles_H, only:SCOREP_AttributeHandle

    implicit none

    private

    public :: &
        SCOREP_AddAttribute, &
        SCOREP_EnterWrapper, &
        SCOREP_ExitWrapper, &
        SCOREP_EnterWrappedRegion, &
        SCOREP_ExitRegion, &
        SCOREP_MpiSend, &
        SCOREP_MpiRecv, &
        SCOREP_MpiIsend, &
        SCOREP_MpiIrecvRequest, &
        SCOREP_MpiRequestTested, &
        SCOREP_CommCreate, &
        SCOREP_CommDestroy, &
        SCOREP_MpiCollectiveBegin, &
        SCOREP_MpiCollectiveEnd, &
        SCOREP_RmaRequestLock, &
        SCOREP_RmaReleaseLock, &
        SCOREP_RmaGroupSync, &
        SCOREP_MpiNonBlockingCollectiveRequest, &
        SCOREP_RmaWinCreate, &
        SCOREP_RmaWinDestroy, &
        SCOREP_RmaCollectiveBegin, &
        SCOREP_RmaCollectiveEnd, &
        SCOREP_RmaGet, &
        SCOREP_RmaPut, &
        SCOREP_RmaAtomic, &
        SCOREP_IoCreateHandle, &
        SCOREP_IoDestroyHandle, &
        SCOREP_IoSeek, &
        SCOREP_IoDeleteFile, &
        SCOREP_IoOperationBegin, &
        SCOREP_IoOperationIssued, &
        SCOREP_IoOperationComplete

    interface
        subroutine SCOREP_AddAttribute(attributeHandle, value) &
            bind(c, name="SCOREP_AddAttribute")
            import
            implicit none
            integer(SCOREP_AttributeHandle), intent(in), value :: attributeHandle
            type(c_ptr), intent(in), value :: value
        end subroutine
    end interface

    interface
        subroutine SCOREP_EnterWrapper(regionHandle) bind(c, name="SCOREP_EnterWrapper")
            import
            implicit none
            integer(SCOREP_RegionHandle), value, intent(in) :: regionHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_ExitWrapper(regionHandle) bind(c, name="SCOREP_ExitWrapper")
            import
            implicit none
            integer(SCOREP_RegionHandle), value, intent(in) :: regionHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_EnterWrappedRegion(regionHandle) bind(c, name="SCOREP_EnterWrappedRegion")
            import
            implicit none
            integer(SCOREP_RegionHandle), value, intent(in) :: regionHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_ExitRegion(regionHandle) bind(c, name="SCOREP_ExitRegion")
            import
            implicit none
            integer(SCOREP_RegionHandle), value, intent(in) :: regionHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiSend(destinationRank, communicatorHandle, tag, bytesSent) bind(c, name="SCOREP_MpiSend")
            import
            implicit none
            integer(c_int), intent(in), value :: destinationRank
            integer(SCOREP_InterimCommunicatorHandle), intent(in), value :: communicatorHandle
            ! WARNING: These are unsigned on the C-side
            integer(c_int32_t), intent(in), value :: tag
            integer(c_int64_t), intent(in), value :: bytesSent
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiRecv(sourceRank, communicatorHandle, tag, bytesReceived) bind(c, name="SCOREP_MpiRecv")
            import
            implicit none
            integer(c_int), intent(in), value :: sourceRank
            integer(SCOREP_InterimCommunicatorHandle), intent(in), value :: communicatorHandle
            ! WARNING: These are unsigned on the C-side
            integer(c_int32_t), intent(in), value :: tag
            integer(c_int64_t), intent(in), value :: bytesReceived
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiIsend(destinationRank, communicatorHandle, tag, bytesSent, requestId) bind(c, name="SCOREP_MpiIsend")
            import
            implicit none
            integer(c_int), intent(in), value :: destinationRank
            integer(SCOREP_InterimCommunicatorHandle), intent(in), value :: communicatorHandle
            ! WARNING: These are unsigned on the C-side
            integer(c_int32_t), intent(in), value :: tag
            integer(c_int64_t), intent(in), value :: bytesSent
            integer(SCOREP_MpiRequestId), intent(in), value :: requestId
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiIrecvRequest(requestId) bind(c, name="SCOREP_MpiIrecvRequest")
            import
            implicit none
            integer(SCOREP_MpiRequestId), intent(in), value :: requestId
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiRequestTested(requestId) bind(c, name="SCOREP_MpiRequestTested")
            import
            implicit none
            integer(SCOREP_MpiRequestId), intent(in), value :: requestId
        end subroutine
    end interface

    interface
        subroutine SCOREP_CommCreate(communicatorHandle) bind(c, name="SCOREP_CommCreate")
            import
            implicit none
            integer(SCOREP_InterimCommunicatorHandle), intent(in), value :: communicatorHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_CommDestroy(communicatorHandle) bind(c, name="SCOREP_CommDestroy")
            import
            implicit none
            integer(SCOREP_InterimCommunicatorHandle), intent(in), value :: communicatorHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiCollectiveBegin() bind(c, name="SCOREP_MpiCollectiveBegin")
            implicit none
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiCollectiveEnd(communicatorHandle, rootRank, collectiveType, bytesSent, bytesReceived) &
            bind(c, name="SCOREP_MpiCollectiveEnd")
            import
            implicit none
            integer(SCOREP_InterimCommunicatorHandle), intent(in), value :: communicatorHandle
            integer(c_int), intent(in), value :: rootRank
            integer(SCOREP_CollectiveType), intent(in), value :: collectiveType
            integer(c_int64_t), intent(in), value :: bytesSent, bytesReceived
        end subroutine
    end interface

    interface
        subroutine SCOREP_MpiNonBlockingCollectiveRequest(requestId) &
            bind(c, name="SCOREP_MpiNonBlockingCollectiveRequest")
            import
            implicit none
            integer(SCOREP_MpiRequestId), intent(in), value :: requestId
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaWinCreate(windowHandle) &
            bind(c, name="SCOREP_RmaWinCreate")
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaWinDestroy(windowHandle) &
            bind(c, name="SCOREP_RmaWinDestroy")
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaCollectiveBegin(syncLevel) &
            bind(c, name="SCOREP_RmaCollectiveBegin")
            import
            implicit none
            integer(SCOREP_RmaSyncLevel), intent(in), value :: syncLevel
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaCollectiveEnd(collectiveOp, syncLevel, windowHandle, root, bytesSent, bytesReceived) &
            bind(c, name="SCOREP_RmaCollectiveEnd")
            import
            implicit none
            integer(SCOREP_CollectiveType), intent(in), value :: collectiveOp
            integer(SCOREP_RmaSyncLevel), intent(in), value :: syncLevel
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(c_int32_t), intent(in), value :: root
            integer(c_int64_t), intent(in), value :: bytesSent
            integer(c_int64_t), intent(in), value :: bytesReceived
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaRequestLock(windowHandle, remote, lockId, lockType) &
            bind(c, name='SCOREP_RmaRequestLock')
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(c_int32_t), intent(in), value :: remote
            integer(c_int64_t), intent(in), value :: lockId
            integer(SCOREP_LockType), intent(in), value :: lockType
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaReleaseLock(windowHandle, remote, lockId) &
            bind(c, name='SCOREP_RmaReleaseLock')
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(c_int32_t), intent(in), value :: remote
            integer(c_int64_t), intent(in), value :: lockId
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaGroupSync(syncLevel, windowHandle, groupHandle) &
            bind(c, name='SCOREP_RmaGroupSync')
            import
            implicit none
            integer(SCOREP_RmaSyncLevel), intent(in), value :: syncLevel
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(SCOREP_GroupHandle), intent(in), value :: groupHandle
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaPut(windowHandle, remote, bytes, matchingId) &
            bind(c, name='SCOREP_RmaPut')
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(c_int32_t), intent(in), value :: remote
            integer(c_int64_t), intent(in), value :: bytes
            integer(c_int64_t), intent(in), value :: matchingId
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaGet(windowHandle, remote, bytes, matchingId) &
            bind(c, name='SCOREP_RmaGet')
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(c_int32_t), intent(in), value :: remote
            integer(c_int64_t), intent(in), value :: bytes
            integer(c_int64_t), intent(in), value :: matchingId
        end subroutine
    end interface

    interface
        subroutine SCOREP_RmaAtomic(windowHandle, remote, atomicType, bytesSent, bytesReceived, matchingId) &
            bind(c, name='SCOREP_RmaAtomic')
            import
            implicit none
            integer(SCOREP_RmaWindowHandle), intent(in), value :: windowHandle
            integer(c_int32_t), intent(in), value :: remote
            integer(SCOREP_RmaAtomicType), intent(in), value :: atomicType
            integer(c_int64_t), intent(in), value :: bytesSent
            integer(c_int64_t), intent(in), value :: bytesReceived
            integer(c_int64_t), intent(in), value :: matchingId
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoCreateHandle(handle, mode, creationFlags, statusFlags) bind(c, name='SCOREP_IoCreateHandle')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
            integer(SCOREP_IoAccessMode), intent(in), value :: mode
            integer(SCOREP_IoCreationFlag), intent(in), value :: creationFlags
            integer(SCOREP_IoStatusFlag), intent(in), value :: statusFlags
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoDestroyHandle(handle) bind(c, name='SCOREP_IoDestroyHandle')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoSeek(handle, offsetRequest, whence, offsetResult) bind(c, name='SCOREP_IoSeek')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
            integer(c_int64_t), intent(in), value :: offsetRequest
            integer(SCOREP_IoSeekOption), intent(in), value :: whence
            integer(c_int64_t), intent(in), value :: offsetResult
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoDeleteFile(ioParadigm, ioFile) bind(c, name='SCOREP_IoDeleteFile')
            import
            implicit none
            integer(SCOREP_IoParadigmType), intent(in), value :: ioParadigm
            integer(SCOREP_IoFileHandle), intent(in), value :: ioFile
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoOperationBegin(handle, mode, operationFlags, bytesRequest, matchingId, offset) &
            bind(c, name='SCOREP_IoOperationBegin')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
            integer(SCOREP_IoOperationMode), intent(in), value :: mode
            integer(SCOREP_IoOperationFlag), intent(in), value :: operationFlags
            integer(c_int64_t), intent(in), value :: bytesRequest, matchingId, offset
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoOperationIssued(handle, matchingId) bind(c, name='SCOREP_IoOperationIssued')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
            integer(c_int64_t), intent(in), value :: matchingId
        end subroutine
    end interface

    interface
        subroutine SCOREP_IoOperationComplete(handle, mode, bytesResult, matchingId) &
            bind(c, name='SCOREP_IoOperationComplete')
            import
            implicit none
            integer(SCOREP_IoHandleHandle), intent(in), value :: handle
            integer(SCOREP_IoOperationMode), intent(in), value :: mode
            integer(c_int64_t), intent(in), value :: bytesResult, matchingId
        end subroutine
    end interface
end module
