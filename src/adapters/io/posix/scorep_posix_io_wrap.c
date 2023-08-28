/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016-2020, 2023, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2021,
 * Deutsches Zentrum fuer Luft- und Raumfahrt, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 * @ingroup    POSIX_IO_Wrapper
 *
 * @brief C interface wrappers for POSIX I/O routines
 */

#ifdef __PGI
#define restrict
#endif

#include <config.h>

#include "scorep_posix_io.h"

#include <unistd.h>
#include <limits.h>

#include <SCOREP_Events.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_IoManagement.h>
#include <SCOREP_Libwrap_Internal.h>

#define SCOREP_DEBUG_MODULE_NAME IO
#include <UTILS_Debug.h>

/**
 * @brief Issues one IoOperationBegin event per vector entry.
 */
static inline void
posix_io_vector_begin( SCOREP_IoFileHandle    handle,
                       SCOREP_IoOperationMode mode,
                       const struct iovec*    iov,
                       int                    iovCnt,
                       uint64_t               offset )
{
    for ( int i = 0; i < iovCnt; i++ )
    {
        SCOREP_IoOperationBegin( handle,
                                 mode,
                                 SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                 iov[ i ].iov_len,
                                 SCOREP_INVALID_BLOCKING_IO_OPERATION_MATCHING_ID + i + 1,
                                 offset );

        if ( offset != SCOREP_IO_UNKNOWN_OFFSET )
        {
            offset += iov[ i ].iov_len;
        }
    }
}

/**
 * @brief Issues one IoOperationComplete event per vector entry.
 *        The transfer size per vector is decompsosed from the total
 *        number of transferred bytes.
 */
static inline void
posix_io_vector_complete( SCOREP_IoFileHandle    handle,
                          SCOREP_IoOperationMode mode,
                          const struct iovec*    iov,
                          int                    iovCnt,
                          ssize_t                transferredBytes )
{
    for ( int i = 0; i < iovCnt; i++ )
    {
        ssize_t request_size = iov[ i ].iov_len;

        if ( request_size > transferredBytes )
        {
            request_size = transferredBytes;
        }

        SCOREP_IoOperationComplete( handle,
                                    mode,
                                    ( uint64_t )request_size,
                                    SCOREP_INVALID_BLOCKING_IO_OPERATION_MATCHING_ID + i + 1 );

        transferredBytes -= request_size;
    }
}

/* *******************************************************************
 * Function wrappers
 * ******************************************************************/

/*
 * Synchronous I/O
 */

int
SCOREP_LIBWRAP_WRAPPER( close )( int fd )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_close );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                       &fd );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_POSIX,
                                        &fd );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( close )( fd );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoMgmt_PopHandle( handle );

            if ( ret != 0 )
            {
                SCOREP_IoMgmt_ReinsertHandle( SCOREP_IO_PARADIGM_POSIX, handle );
            }
            else
            {
                SCOREP_IoMgmt_DestroyHandle( handle );
            }
            SCOREP_IoDestroyHandle( handle );
        }

        SCOREP_ExitRegion( scorep_posix_io_region_close );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( close )( fd );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( closedir )( DIR* dirp )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_closedir );

        SCOREP_IoHandleHandle handle = SCOREP_INVALID_IO_HANDLE;
        int                   fd     = dirfd( dirp );

        if ( fd != -1 )
        {
            handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                     &fd );

            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_POSIX,
                                            &fd );
            }
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( closedir )( dirp );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoMgmt_PopHandle( handle );

            if ( ret != 0 )
            {
                SCOREP_IoMgmt_ReinsertHandle( SCOREP_IO_PARADIGM_POSIX, handle );
            }
            else
            {
                SCOREP_IoMgmt_DestroyHandle( handle );
            }
            SCOREP_IoDestroyHandle( handle );
        }

        SCOREP_ExitRegion( scorep_posix_io_region_closedir );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( closedir )( dirp );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( creat )( const char* pathname, mode_t mode )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_creat );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_POSIX,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( creat )( pathname, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != -1 )
        {
            SCOREP_IoFileHandle   file   = SCOREP_IoMgmt_GetIoFileHandle( pathname );
            SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_POSIX, file, ret + 1, &ret );

            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                int                   flags = O_CREAT | O_WRONLY | O_TRUNC;
                SCOREP_IoCreationFlag creation_flags;
                SCOREP_IoStatusFlag   status_flags;
                scorep_posix_io_get_scorep_io_flags( flags, &creation_flags, &status_flags );

                SCOREP_IoCreateHandle( handle,
                                       scorep_posix_io_get_scorep_io_access_mode( flags ),
                                       creation_flags,
                                       status_flags );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }


        SCOREP_ExitRegion( scorep_posix_io_region_creat );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( creat )( pathname, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( creat64 )( const char* pathname, mode_t mode )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_creat64 );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_POSIX,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( creat64 )( pathname, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != -1 )
        {
            SCOREP_IoFileHandle   file   = SCOREP_IoMgmt_GetIoFileHandle( pathname );
            SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_POSIX, file, ret + 1, &ret );

            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                int                   flags = O_CREAT | O_WRONLY | O_TRUNC;
                SCOREP_IoCreationFlag creation_flags;
                SCOREP_IoStatusFlag   status_flags;
                scorep_posix_io_get_scorep_io_flags( flags, &creation_flags, &status_flags );

                SCOREP_IoCreateHandle( handle,
                                       scorep_posix_io_get_scorep_io_access_mode( flags ),
                                       creation_flags,
                                       status_flags );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }

        SCOREP_ExitRegion( scorep_posix_io_region_creat64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( creat64 )( pathname, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( dup )( int oldfd )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_dup );

        SCOREP_IoHandleHandle old_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &oldfd );

        if ( old_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_POSIX,
                                                  old_handle );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( dup )( oldfd );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( old_handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( ret != -1 )
            {
                SCOREP_IoHandleHandle new_handle = SCOREP_IoMgmt_CompleteHandleDuplication(
                    SCOREP_IO_PARADIGM_POSIX, SCOREP_INVALID_IO_FILE, ret + 1, &ret );
                if ( new_handle != SCOREP_INVALID_IO_HANDLE )
                {
                    SCOREP_IoStatusFlag flags = SCOREP_IO_STATUS_FLAG_NONE;
                    if ( !scorep_posix_io_get_scorep_io_flags_from_fd( oldfd, NULL, NULL, &flags ) )
                    {
                        UTILS_WARNING( "Flags for oldfd are not determined" );
                    }

                    SCOREP_IoDuplicateHandle( old_handle,
                                              new_handle,
                                              flags );
                }
            }
            else
            {
                SCOREP_IoMgmt_DropIncompleteHandle();
            }
        }


        SCOREP_ExitRegion( scorep_posix_io_region_dup );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( dup )( oldfd );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}


int
SCOREP_LIBWRAP_WRAPPER( dup2 )( int oldfd, int newfd )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_dup2 );

        SCOREP_IoHandleHandle new_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &newfd );
        SCOREP_IoHandleHandle old_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &oldfd );

        if ( old_handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( new_handle != SCOREP_INVALID_IO_HANDLE )
            {
                // newfd is still open
                if ( oldfd != newfd )
                {
                    // newfd will be closed
                    SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_POSIX, &newfd );
                    SCOREP_IoMgmt_DestroyHandle( new_handle );
                    SCOREP_IoDestroyHandle( new_handle );
                    SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_POSIX, old_handle );
                }
                // else
                // oldfd == newfd dup2 will return newfd and does nothing
                // the FD_CLOEXEC file descriptor flag is cleared for newfd
            }
            else
            {
                SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_POSIX, old_handle );
            }
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( dup2 )( oldfd, newfd );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( oldfd != newfd && old_handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( ret != -1 )
            {
                new_handle = SCOREP_IoMgmt_CompleteHandleDuplication(
                    SCOREP_IO_PARADIGM_POSIX, SCOREP_INVALID_IO_FILE, newfd + 1, &newfd );
                if ( new_handle != SCOREP_INVALID_IO_HANDLE )
                {
                    SCOREP_IoStatusFlag flags       = SCOREP_IO_STATUS_FLAG_NONE;
                    int                 posix_flags = SCOREP_LIBWRAP_ORIGINAL( fcntl )( ret, F_GETFL, 0 );

                    if ( posix_flags >= 0 )
                    {
                        scorep_posix_io_get_scorep_io_flags( posix_flags, NULL, &flags );
                    }
                    else
                    {
                        UTILS_WARNING( "Flags for oldfd are not determined" );
                    }
                    SCOREP_IoDuplicateHandle( old_handle,
                                              new_handle,
                                              flags );
                }
            }
            else
            {
                SCOREP_IoMgmt_DropIncompleteHandle();
            }
        }
        SCOREP_ExitRegion( scorep_posix_io_region_dup2 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( dup2 )( oldfd, newfd );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( dup3 )( int oldfd, int newfd, int flags )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_dup3 );

        SCOREP_IoHandleHandle new_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &newfd );
        SCOREP_IoHandleHandle old_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &oldfd );

        if ( old_handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( new_handle != SCOREP_INVALID_IO_HANDLE )
            {
                // If oldfd equals newfd, then dup3() fails with the error EINVAL.
                SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_POSIX, &newfd );
                SCOREP_IoMgmt_DestroyHandle( new_handle );
                SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_POSIX, old_handle );
            }
            else
            {
                SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_POSIX, old_handle );
            }
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( dup3 )( oldfd, newfd, flags );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( old_handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( ret != -1 )
            {
                new_handle = SCOREP_IoMgmt_CompleteHandleDuplication(
                    SCOREP_IO_PARADIGM_POSIX, SCOREP_INVALID_IO_FILE, newfd + 1, &newfd );
                if ( new_handle != SCOREP_INVALID_IO_HANDLE )
                {
                    SCOREP_IoStatusFlag status_flags = SCOREP_IO_STATUS_FLAG_NONE;

                    scorep_posix_io_get_scorep_io_flags( flags, NULL, &status_flags );

                    SCOREP_IoDuplicateHandle( old_handle,
                                              new_handle,
                                              status_flags );
                }
            }
            else
            {
                SCOREP_IoMgmt_DropIncompleteHandle();
            }
        }


        SCOREP_ExitRegion( scorep_posix_io_region_dup3 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( dup3 )( oldfd, newfd, flags );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fcntl )( int fd, int cmd, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fcntl );
    }

    int                   int_arg;
    void*                 pointer_arg;
    SCOREP_IoHandleHandle new_handle;
    SCOREP_IoFileHandle   file_handle;
    SCOREP_IoHandleHandle old_handle = SCOREP_INVALID_IO_HANDLE;

    va_list args;
    va_start( args, cmd );
    switch ( cmd )
    {
        /*
         * Calls to fcntl without var arg parameter
         */
        case F_GETFD:      /* (void) */
        case F_GETFL:      /* (void) */
        case F_GETOWN:     /* (void) */
#ifdef F_GETSIG
        case F_GETSIG:     /* (void) */
#endif
#ifdef F_GETLEASE
        case F_GETLEASE:   /* (void) */
#endif
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ: /* (void) (since Linux 2.6.35) */
#endif                     /* F_GETPIPE_SZ */
#ifdef F_GET_SEALS
        case F_GET_SEALS:  /* (void) (since Linux 3.17) */
#endif                     /* F_GET_SEALS */
            {
                if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
                {
                    old_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
                    SCOREP_ENTER_WRAPPED_REGION();
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd /* no var arg */ );
                    SCOREP_EXIT_WRAPPED_REGION();
                    SCOREP_IoMgmt_PopHandle( old_handle );
                }
                else
                {
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd /* no var arg */ );
                }
                break;
            }

        /*
         * Calls to fcntl with var arg parameter of type int
         */
        case F_DUPFD:         /* (int) */
#ifdef F_DUPFD_CLOEXEC
        case F_DUPFD_CLOEXEC: /* (int) (since Linux 2.6.24) */
#endif                        /* F_DUPFD_CLOEXEC */
            {
                int_arg = va_arg( args, int );
                if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
                {
                    old_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
                    if ( fd != int_arg && old_handle != SCOREP_INVALID_IO_HANDLE )
                    {
                        SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_POSIX, old_handle );
                    }
                    SCOREP_ENTER_WRAPPED_REGION();
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd, int_arg );
                    SCOREP_EXIT_WRAPPED_REGION();
                }
                else
                {
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd, int_arg );
                }
                break;
            }
        case F_SETFD:      /* (int) */
        case F_SETFL:      /* (int) */
        case F_SETOWN:     /* (int) */
#ifdef F_SETSIG
        case F_SETSIG:     /* (int) */
#endif
#ifdef F_SETLEASE
        case F_SETLEASE:   /* (int) */
#endif
#ifdef F_NOTIFY
        case F_NOTIFY:     /* (int) */
#endif
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ: /* (int) (since Linux 2.6.35) */
#endif                     /* F_SETPIPE_SZ */
#ifdef F_ADD_SEALS
        case F_ADD_SEALS:  /* (int) (since Linux 3.17) */
#endif                     /* F_ADD_SEALS */
            {
                int_arg = va_arg( args, int );
                if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
                {
                    old_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
                    SCOREP_ENTER_WRAPPED_REGION();
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd, int_arg );
                    SCOREP_EXIT_WRAPPED_REGION();
                    SCOREP_IoMgmt_PopHandle( old_handle );
                }
                else
                {
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd, int_arg );
                }
                break;
            }

        /*
         * Calls to fcntl with var arg parameter of type (struct flock *)
         */
        case F_SETLK:      /* (struct flock *) */
        case F_SETLKW:     /* (struct flock *) */
        case F_GETLK:      /* (struct flock *) */
#ifdef F_OFD_SETLK
        case F_OFD_SETLK:  /* (struct flock *) */
#endif                     /* F_OFD_SETLK */
#ifdef F_OFD_SETLKW
        case F_OFD_SETLKW: /* (struct flock *) */
#endif                     /* F_OFD_SETLKW */
#ifdef F_OFD_GETLK
        case F_OFD_GETLK:  /* (struct flock *) */
#endif                     /* F_OFD_GETLK */

            /*
             * Calls to fcntl with var arg parameter of type (struct flock64 *)
             */
#if defined( F_SETLK64 ) && F_SETLK64 != F_SETLK
        case F_SETLK64:  /* (struct flock64 *) */
#endif                   /* defined( F_SETLK64 ) && F_SETLK64 != F_SETLK */
#if defined( F_SETLKW64 ) && F_SETLKW64 != F_SETLKW
        case F_SETLKW64: /* (struct flock64 *) */
#endif                   /* defined( F_SETLKW64 ) && F_SETLKW64 != F_SETLKW */
#if defined( F_GETLK64 ) && F_GETLK64 != F_GETLK
        case F_GETLK64:  /* (struct flock64 *) */
#endif                   /* defined( F_GETLK64 ) && F_GETLK64 != F_GETLK */

            /*
             * Calls to fcntl with var arg parameter of type (struct f_owner_ex *)
             */
#ifdef F_GETOWN_EX
        case F_GETOWN_EX: /* (struct f_owner_ex *) (since Linux 2.6.32) */
#endif                    /* F_GETOWN_EX */
#ifdef F_SETOWN_EX
        case F_SETOWN_EX: /* (struct f_owner_ex *) (since Linux 2.6.32) */
#endif                    /* F_SETOWN_EX */

#ifdef F_GETOWNER_UIDS
        case F_GETOWNER_UIDS: /* (uid_t *) (since Linux 3.6) */
#endif                        /* F_GETOWNER_UIDS */
            {
                pointer_arg = va_arg( args, void* );
                if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
                {
                    old_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
                    SCOREP_ENTER_WRAPPED_REGION();
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd, pointer_arg );
                    SCOREP_EXIT_WRAPPED_REGION();
                    SCOREP_IoMgmt_PopHandle( old_handle );
                }
                else
                {
                    ret = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, cmd, pointer_arg );
                }
                break;
            }

        default:
            UTILS_BUG( "Unhandled command in fcntl wrapper." );
    }

    va_end( args );

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        if ( cmd == F_DUPFD
#ifdef F_DUPFD_CLOEXEC
             || cmd == F_DUPFD_CLOEXEC
#endif  /* F_DUPFD_CLOEXEC */
             )
        {
            if ( fd != int_arg && old_handle != SCOREP_INVALID_IO_HANDLE )
            {
                if ( ret != -1 )
                {
                    new_handle = SCOREP_IoMgmt_CompleteHandleDuplication(
                        SCOREP_IO_PARADIGM_POSIX, SCOREP_INVALID_IO_FILE, ret + 1, &ret );
                    if ( new_handle != SCOREP_INVALID_IO_HANDLE )
                    {
                        SCOREP_IoStatusFlag flags     = SCOREP_IO_STATUS_FLAG_NONE;
                        int                 old_flags = SCOREP_LIBWRAP_ORIGINAL( fcntl )( fd, F_GETFL, 0 );

                        if ( old_flags >= 0 )
                        {
                            scorep_posix_io_get_scorep_io_flags( old_flags, NULL, &flags );
                        }
                        else
                        {
                            UTILS_WARNING( "Flags for oldfd are not determined" );
                        }
                        SCOREP_IoDuplicateHandle( old_handle,
                                                  new_handle,
                                                  flags );
                    }
                }
                else
                {
                    SCOREP_IoMgmt_DropIncompleteHandle();
                }
            }
        }
        SCOREP_ExitRegion( scorep_posix_io_region_fcntl );
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fdatasync )( int fd )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fdatasync );
        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_FLUSH,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fdatasync )( fd );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_FLUSH,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fdatasync );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fdatasync )( fd );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fsync )( int fd )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fsync );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_FLUSH,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fsync )( fd );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_FLUSH,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fsync );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fsync )( fd );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( lockf )( int fd, int cmd, off_t len )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_lockf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
        if ( handle != SCOREP_INVALID_IO_HANDLE && cmd == F_ULOCK )
        {
            SCOREP_IoReleaseLock( handle, SCOREP_LOCK_EXCLUSIVE );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( lockf )( fd, cmd, len );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            switch ( cmd )
            {
                case F_LOCK:
                    SCOREP_IoAcquireLock( handle, SCOREP_LOCK_EXCLUSIVE );
                    break;
                case F_TLOCK:
                    if ( ret == 0 )
                    {
                        SCOREP_IoAcquireLock( handle, SCOREP_LOCK_EXCLUSIVE );
                    }
                    else
                    {
                        SCOREP_IoTryLock( handle, SCOREP_LOCK_EXCLUSIVE );
                    }
                    break;
            }
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_lockf );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( lockf )( fd, cmd, len );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

off_t
SCOREP_LIBWRAP_WRAPPER( lseek )( int fd, off_t offset, int whence )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    off_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_lseek );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( lseek )( fd, offset, whence );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           ( int64_t )offset,
                           scorep_posix_io_get_scorep_io_seek_option( whence ),
                           ( uint64_t )ret );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_lseek );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( lseek )( fd, offset, whence );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

scorep_off64_t
SCOREP_LIBWRAP_WRAPPER( lseek64 )( int fd, scorep_off64_t offset, int whence )
{
    bool           trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    scorep_off64_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_lseek64 );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX, &fd );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( lseek64 )( fd, offset, whence );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           ( int64_t )offset,
                           scorep_posix_io_get_scorep_io_seek_option( whence ),
                           ( uint64_t )ret );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_lseek64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( lseek64 )( fd, offset, whence );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( open )( const char* pathname, int flags, ... /* mode_t mode */ )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_open );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_POSIX,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );
        mode_t mode = 0;
        if ( ( flags & O_CREAT )
#ifdef O_TMPFILE
             || ( flags & O_TMPFILE )
#endif
             )
        {
            va_list va;
            va_start( va, flags );
            // smaller types are always promoted to int
            mode = ( mode_t )va_arg( va, int );
            va_end( va );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( open )( pathname, flags, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != -1 )
        {
            SCOREP_IoFileHandle   file   = SCOREP_IoMgmt_GetIoFileHandle( pathname );
            SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_POSIX, file, ret + 1, &ret );

            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreationFlag creation_flags;
                SCOREP_IoStatusFlag   status_flags;
                scorep_posix_io_get_scorep_io_flags( flags, &creation_flags, &status_flags );
                /* @todo handle mode, write as attribute? */
                SCOREP_IoCreateHandle( handle,
                                       scorep_posix_io_get_scorep_io_access_mode( flags ),
                                       creation_flags,
                                       status_flags );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }


        SCOREP_ExitRegion( scorep_posix_io_region_open );
    }
    else
    {
        mode_t mode = 0;
        if ( ( flags & O_CREAT )
#ifdef O_TMPFILE
             || ( flags & O_TMPFILE )
#endif
             )
        {
            va_list va;
            va_start( va, flags );
            // smaller types are always promoted to int
            mode = ( mode_t )va_arg( va, int );
            va_end( va );
        }
        ret = SCOREP_LIBWRAP_ORIGINAL( open )( pathname, flags, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( open64 )( const char* pathname, int flags, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_open64 );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_POSIX,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );
        mode_t mode = 0;
        if ( ( flags & O_CREAT )
#ifdef O_TMPFILE
             || ( flags & O_TMPFILE )
#endif
             )
        {
            va_list va;
            va_start( va, flags );
            // smaller types are always promoted to int
            mode = ( mode_t )va_arg( va, int );
            va_end( va );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( open64 )( pathname, flags, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != -1 )
        {
            SCOREP_IoFileHandle   file   = SCOREP_IoMgmt_GetIoFileHandle( pathname );
            SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_POSIX, file, ret + 1, &ret );

            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreationFlag creation_flags;
                SCOREP_IoStatusFlag   status_flags;
                scorep_posix_io_get_scorep_io_flags( flags, &creation_flags, &status_flags );

                SCOREP_IoCreateHandle( handle,
                                       scorep_posix_io_get_scorep_io_access_mode( flags ),
                                       creation_flags,
                                       status_flags );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }
        SCOREP_ExitRegion( scorep_posix_io_region_open64 );
    }
    else
    {
        mode_t mode = 0;
        if ( ( flags & O_CREAT )
#ifdef O_TMPFILE
             || ( flags & O_TMPFILE )
#endif
             )
        {
            va_list va;
            va_start( va, flags );
            // smaller types are always promoted to int
            mode = ( mode_t )va_arg( va, int );
            va_end( va );
        }
        ret = SCOREP_LIBWRAP_ORIGINAL( open64 )( pathname, flags, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( openat )( int dirfd, const char* pathname, int flags, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_openat );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_POSIX,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );
        mode_t mode = 0;
        if ( ( flags & O_CREAT )
#ifdef O_TMPFILE
             || ( flags & O_TMPFILE )
#endif
             )
        {
            va_list va;
            va_start( va, flags );
            // smaller types are always promoted to int
            mode = ( mode_t )va_arg( va, int );
            va_end( va );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( openat )( dirfd, pathname, flags, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != -1 )
        {
            char        fd_path[ PATH_MAX ];
            char        file_path[ PATH_MAX ];
            const char* proc_path = "/proc/self/fd/%d";

            int len = snprintf( fd_path, PATH_MAX, proc_path, ret );

            UTILS_BUG_ON( len < 0, "Executing snprintf to generate proc path failed" );

            len = readlink( fd_path, file_path, PATH_MAX );

            UTILS_BUG_ON( len < 0, "Cannot resolve link %s", fd_path );

            file_path[ len ] = '\0';

            SCOREP_IoFileHandle   file   = SCOREP_IoMgmt_GetIoFileHandle( file_path );
            SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_POSIX, file, ret + 1, &ret );

            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreationFlag creation_flags;
                SCOREP_IoStatusFlag   status_flags;
                scorep_posix_io_get_scorep_io_flags( flags, &creation_flags, &status_flags );

                SCOREP_IoCreateHandle( handle,
                                       scorep_posix_io_get_scorep_io_access_mode( flags ),
                                       creation_flags,
                                       status_flags );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }

        SCOREP_ExitRegion( scorep_posix_io_region_openat );
    }
    else
    {
        mode_t mode = 0;
        if ( ( flags & O_CREAT )
#ifdef O_TMPFILE
             || ( flags & O_TMPFILE )
#endif
             )
        {
            va_list va;
            va_start( va, flags );
            // smaller types are always promoted to int
            mode = ( mode_t )va_arg( va, int );
            va_end( va );
        }
        ret = SCOREP_LIBWRAP_ORIGINAL( openat )( dirfd, pathname, flags, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pread )( int fd, void* buf, size_t count, off_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pread );

        /*
         * Reads up to 'count' bytes from file descriptor 'fd' at offset 'offset' (from the start of the file) into the buffer starting at 'buf'.
         * The file offset is not changed.
         *
         * @todo: how to represent offset?
         */
        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )count,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pread )( fd, buf, count, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pread );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pread )( fd, buf, count, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pread64 )( int fd, void* buf, size_t count, scorep_off64_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pread64 );

        /*
         * Reads up to 'count' bytes from file descriptor 'fd' at offset 'offset' (from the start of the file) into the buffer starting at 'buf'.
         * The file offset is not changed.
         *
         * @todo: how to represent offset?
         */
        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )count,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pread64 )( fd, buf, count, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pread64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pread64 )( fd, buf, count, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( preadv )( int fd, const struct iovec* iov, int iovcnt, off_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_preadv );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_READ,
                                   iov,
                                   iovcnt,
                                   offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv )( fd, iov, iovcnt, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_READ,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_preadv );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv )( fd, iov, iovcnt, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( preadv2 )( int fd, const struct iovec* iov, int iovcnt, off_t offset, int flags )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_preadv2 );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_READ,
                                   iov,
                                   iovcnt,
                                   ( offset == -1 ) ? SCOREP_IO_UNKNOWN_OFFSET : offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv2 )( fd, iov, iovcnt, offset, flags );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_READ,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_preadv2 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv2 )( fd, iov, iovcnt, offset, flags );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( preadv64 )( int fd, const struct iovec* iov, int iovcnt, scorep_off64_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_preadv64 );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_READ,
                                   iov,
                                   iovcnt,
                                   offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv64 )( fd, iov, iovcnt, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_READ,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_preadv64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv64 )( fd, iov, iovcnt, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( preadv64v2 )( int fd, const struct iovec* iov, int iovcnt, scorep_off64_t offset, int flags )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_preadv64v2 );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_READ,
                                   iov,
                                   iovcnt,
                                   ( offset == -1 ) ? SCOREP_IO_UNKNOWN_OFFSET : offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv64v2 )( fd, iov, iovcnt, offset, flags );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_READ,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_preadv64v2 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( preadv64v2 )( fd, iov, iovcnt, offset, flags );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( pselect )( int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timespec* timeout, const sigset_t* sigmask )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pselect );
        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pselect )(
            nfds, readfds, writefds, exceptfds, timeout, sigmask );
        SCOREP_EXIT_WRAPPED_REGION();
        SCOREP_ExitRegion( scorep_posix_io_region_pselect );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pselect )(
            nfds, readfds, writefds, exceptfds, timeout, sigmask );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pwrite )( int fd, const void* buf, size_t count, off_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pwrite );

        /*
         * Writes up to 'count' bytes from the buffer starting at 'buf' to the file descriptor 'fd' at offset 'offset'.
         * The file offset is not changed.
         *
         * @todo: how to represent offset?
         */
        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )count,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pwrite )( fd, buf, count, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ( uint64_t )ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pwrite );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pwrite )( fd, buf, count, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pwrite64 )( int fd, const void* buf, size_t count, scorep_off64_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pwrite64 );

        /*
         * Writes up to 'count' bytes from the buffer starting at 'buf' to the file descriptor 'fd' at offset 'offset'.
         * The file offset is not changed.
         *
         * @todo: how to represent offset?
         */
        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )count,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pwrite64 )( fd, buf, count, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ( uint64_t )ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pwrite64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pwrite64 )( fd, buf, count, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pwritev )( int fd, const struct iovec* iov, int iovcnt, off_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pwritev );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_WRITE,
                                   iov,
                                   iovcnt,
                                   offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev )( fd, iov, iovcnt, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_WRITE,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pwritev );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev )( fd, iov, iovcnt, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pwritev2 )( int fd, const struct iovec* iov, int iovcnt, off_t offset, int flags )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pwritev2 );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            uint64_t event_offset = offset;
            #ifdef RWF_APPEND
            if ( flags & RWF_APPEND )
            {
                event_offset = SCOREP_IO_UNKNOWN_OFFSET;
            }
            #endif

            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_WRITE,
                                   iov,
                                   iovcnt,
                                   ( offset == -1 ) ? SCOREP_IO_UNKNOWN_OFFSET : event_offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev2 )( fd, iov, iovcnt, offset, flags );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_WRITE,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pwritev2 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev2 )( fd, iov, iovcnt, offset, flags );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pwritev64 )( int fd, const struct iovec* iov, int iovcnt, scorep_off64_t offset )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pwritev64 );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_WRITE,
                                   iov,
                                   iovcnt,
                                   offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev64 )( fd, iov, iovcnt, offset );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_WRITE,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pwritev64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev64 )( fd, iov, iovcnt, offset );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( pwritev64v2 )( int fd, const struct iovec* iov, int iovcnt, scorep_off64_t offset, int flags )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_pwritev64v2 );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            uint64_t event_offset = offset;
            #ifdef RWF_APPEND
            if ( flags & RWF_APPEND )
            {
                event_offset = SCOREP_IO_UNKNOWN_OFFSET;
            }
            #endif

            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_WRITE,
                                   iov,
                                   iovcnt,
                                   ( offset == -1 ) ? SCOREP_IO_UNKNOWN_OFFSET : event_offset );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev64v2 )( fd, iov, iovcnt, offset, flags );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_WRITE,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_pwritev64v2 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( pwritev64v2 )( fd, iov, iovcnt, offset, flags );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( read )( int fd, void* buf, size_t count )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_read );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )count,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( read )( fd, buf, count );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_read );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( read )( fd, buf, count );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( readv )( int fd, const struct iovec* iov, int iovcnt )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_readv );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( io_handle,
                                   SCOREP_IO_OPERATION_MODE_READ,
                                   iov,
                                   iovcnt,
                                   SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( readv )( fd, iov, iovcnt );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( io_handle,
                                      SCOREP_IO_OPERATION_MODE_READ,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_readv );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( readv )( fd, iov, iovcnt );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( select )( int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_select );
        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( select )( nfds, readfds, writefds, exceptfds, timeout );
        SCOREP_EXIT_WRAPPED_REGION();
        SCOREP_ExitRegion( scorep_posix_io_region_select );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( select )( nfds, readfds, writefds, exceptfds, timeout );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

void
SCOREP_LIBWRAP_WRAPPER( sync )( void )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_sync );

        SCOREP_IoMgmt_PushHandle( scorep_posix_io_sync_all_handle );

        SCOREP_IoOperationBegin( scorep_posix_io_sync_all_handle,
                                 SCOREP_IO_OPERATION_MODE_FLUSH,
                                 SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                 SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                 SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                 SCOREP_IO_UNKNOWN_OFFSET );

        SCOREP_ENTER_WRAPPED_REGION();
        SCOREP_LIBWRAP_ORIGINAL( sync )();
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_IoOperationComplete( scorep_posix_io_sync_all_handle,
                                    SCOREP_IO_OPERATION_MODE_FLUSH,
                                    SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                    SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );

        SCOREP_IoMgmt_PopHandle( scorep_posix_io_sync_all_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_sync );
    }
    else
    {
        SCOREP_LIBWRAP_ORIGINAL( sync )();
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

int
SCOREP_LIBWRAP_WRAPPER( syncfs )( int fd )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_syncfs );

        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                          &fd );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_FLUSH,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( syncfs )( fd );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_FLUSH,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_syncfs );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( syncfs )( fd );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( unlink )( const char* pathname )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_unlink );

        SCOREP_IoFileHandle file_handle = SCOREP_IoMgmt_GetIoFileHandle( pathname );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( unlink )( pathname );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( file_handle != SCOREP_INVALID_IO_FILE )
        {
            SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_POSIX,
                                 file_handle );
        }

        SCOREP_ExitRegion( scorep_posix_io_region_unlink );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( unlink )( pathname );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( unlinkat )( int dirfd, const char* pathname, int flags )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_unlinkat );

        char                  file_path[ PATH_MAX ];
        SCOREP_IoFileHandle   file_handle      = SCOREP_INVALID_IO_FILE;
        SCOREP_IoHandleHandle handle           = SCOREP_INVALID_IO_HANDLE;
        bool                  path_is_absolute = pathname[ 0 ] == '/';

        if ( dirfd == AT_FDCWD || path_is_absolute )
        {
            file_handle = SCOREP_IoMgmt_GetIoFileHandle( pathname );
        }
        else
        {
            handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                     &dirfd );
            if ( handle != SCOREP_INVALID_IO_HANDLE )
            {
                const char* dir_path = SCOREP_IoMgmt_GetIoFile( handle );

                int len = snprintf( file_path, PATH_MAX, "%s/%s", dir_path, pathname );
                UTILS_BUG_ON( len < 0, "Executing snprintf to generate file path failed" );

                file_handle = SCOREP_IoMgmt_GetIoFileHandle( file_path );
            }
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( unlinkat )( dirfd, pathname, flags );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( file_handle != SCOREP_INVALID_IO_FILE )
        {
            SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_POSIX, file_handle );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_unlinkat );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( unlinkat )( dirfd, pathname, flags );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( write )( int fd, const void* buf, size_t count )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_write );

        SCOREP_IoFileHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                     &fd );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )count,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }
        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( write )( fd, buf, count );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ( uint64_t )ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_POSIX /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_write );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( write )( fd, buf, count );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

ssize_t
SCOREP_LIBWRAP_WRAPPER( writev )( int fd, const struct iovec* iov, int iovcnt )
{
    bool    trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    ssize_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_writev );

        SCOREP_IoFileHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_POSIX,
                                                                     &fd );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_begin( handle,
                                   SCOREP_IO_OPERATION_MODE_WRITE,
                                   iov,
                                   iovcnt,
                                   SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( writev )( fd, iov, iovcnt );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            posix_io_vector_complete( handle,
                                      SCOREP_IO_OPERATION_MODE_WRITE,
                                      iov,
                                      iovcnt,
                                      ret );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_writev );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( writev )( fd, iov, iovcnt );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

#if HAVE( POSIX_AIO_SUPPORT )
#include "scorep_posix_io_wrap_aio.inc.c"
#endif /* HAVE( POSIX_AIO_SUPPORT ) */
