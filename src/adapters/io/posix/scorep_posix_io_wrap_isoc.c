/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016-2020, 2023, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2022,
 * Forschungszentrum Juelich GmbH, Germany
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

#include <SCOREP_Events.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_IoManagement.h>
#include <SCOREP_Libwrap_Internal.h>

#define SCOREP_DEBUG_MODULE_NAME IO
#include <UTILS_Debug.h>

/*
 * Annex K of the C11 standard specifies bounds-checking interfaces.
 * Only a few implementations exist, e.g. Microsoft Visual Studio.
 * Neither BSD nor Linux distributions provide an implementation.
 * It's unlikely that the glibc will provide an implementation of
 * these APIs in the near future. We decided to do NOT wrap the
 * following functions as we can't test them yet:
 *
 * errno_t tmpfile_s( FILE * restrict * restrict streamptr );
 * errno_t tmpnam_s( char *s, rsize_t maxsize );
 * errno_t fopen_s( FILE * restrict * restrict streamptr,
 *                  const char * restrict filename,
 *                  const char * restrict mode );
 * errno_t freopen_s( FILE * restrict * restrict newstreamptr,
 *                    const char * restrict filename,
 *                    const char * restrict mode,
 *                    FILE * restrict stream );
 * int fprintf_s( FILE * restrict stream,
 *                const char * restrict format, ... );
 * int fscanf_s( FILE * restrict stream,
 *               const char * restrict format, ... );
 * int printf_s( const char * restrict format, ... );
 * int scanf_s(const char * restrict format, ...);
 * int snprintf_s( char * restrict s, rsize_t n,
 *                 const char * restrict format, ... );
 * int sprintf_s( char * restrict s, rsize_t n,
 *                const char * restrict format, ... );
 * int sscanf_s( const char * restrict s,
 *               const char * restrict format, ... );
 * int vfprintf_s( FILE * restrict stream,
 *                 const char * restrict format,
 *                 va_list arg );
 * int vfscanf_s( FILE * restrict stream,
 *                const char * restrict format,
 *                va_list arg );
 * int vprintf_s( const char * restrict format,
 *                va_list arg );
 * int vscanf_s( const char * restrict format,
 *               va_list arg );
 * int vsnprintf_s( char * restrict s, rsize_t n,
 *                  const char * restrict format,
 *                  va_list arg );
 * int vsprintf_s( char * restrict s, rsize_t n,
 *                 const char * restrict format,
 *                 va_list arg );
 * int vsscanf_s( const char * restrict s,
 *                const char * restrict format,
 *                va_list arg );
 * char *gets_s( char *s, rsize_t n );
 */


/* *******************************************************************
 * Translate ISO C I/O types to Score-P representative
 * ******************************************************************/

/**
 * Translate the POSIX mode of an open operation to its Score-P equivalent.
 * The mode is specified by a string starting with character(s) determining
 * the mode (e.g., 'r', 'r+', 'w', 'w+', 'a', 'a+').
 *
 * @param mode      String specifying the POSIX mode of the open operation as string.
 *
 * @return Score-P equivalent of the POSIX mode.
 */
static inline SCOREP_IoAccessMode
get_scorep_io_access_mode_from_string( const char* mode )
{
#define R_MODE             "r"
#define R_MODE_STRLEN      strlen( R_MODE )
#define R_PLUS_MODE        "r+"
#define R_PLUS_MODE_STRLEN strlen( R_PLUS_MODE )

#define W_MODE             "w"
#define W_MODE_STRLEN      strlen( W_MODE )
#define W_PLUS_MODE        "w+"
#define W_PLUS_MODE_STRLEN strlen( W_PLUS_MODE )

#define A_MODE             "a"
#define A_MODE_STRLEN      strlen( A_MODE )
#define A_PLUS_MODE        "a+"
#define A_PLUS_MODE_STRLEN strlen( A_PLUS_MODE )

    if ( strncmp( R_PLUS_MODE, mode, strlen( R_PLUS_MODE ) ) == 0 )
    {
        return SCOREP_IO_ACCESS_MODE_READ_WRITE;
    }
    else if ( strncmp( W_PLUS_MODE, mode, strlen( W_PLUS_MODE ) ) == 0 )
    {
        return SCOREP_IO_ACCESS_MODE_READ_WRITE;
    }
    else if ( strncmp( A_PLUS_MODE, mode, strlen( A_PLUS_MODE ) ) == 0 )
    {
        return SCOREP_IO_ACCESS_MODE_READ_WRITE;
    }
    else if ( strncmp( R_MODE, mode, strlen( R_MODE ) ) == 0 )
    {
        return SCOREP_IO_ACCESS_MODE_READ_ONLY;
    }
    else if ( strncmp( W_MODE, mode, strlen( W_MODE ) ) == 0 )
    {
        return SCOREP_IO_ACCESS_MODE_WRITE_ONLY;
    }
    else if ( strncmp( A_MODE, mode, strlen( A_MODE ) ) == 0 )
    {
        return SCOREP_IO_ACCESS_MODE_WRITE_ONLY;
    }
    else
    {
        UTILS_BUG( "Invalid IO open mode." );
        return SCOREP_IO_ACCESS_MODE_NONE;
    }

#undef R_MODE
#undef R_MODE_STRLEN
#undef R_PLUS_MODE
#undef R_PLUS_MODE_STRLEN

#undef W_MODE
#undef W_MODE_STRLEN
#undef W_PLUS_MODE
#undef W_PLUS_MODE_STRLEN

#undef A_MODE
#undef A_MODE_STRLEN
#undef A_PLUS_MODE
#undef A_PLUS_MODE_STRLEN
}

/* *******************************************************************
 * Function wrappers
 * ******************************************************************/

int
SCOREP_LIBWRAP_WRAPPER( fclose )( FILE* fp )
{
    /*
     * fclose manpage:
     *
     * int fclose(FILE *stream);
     *
     * The fclose() function flushes the stream pointed to by stream
     * (writing any buffered output data using fflush(3)) and closes
     * the underlying file descriptor.
     */
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fclose );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_ISOC,
                                                                   &fp );
        #if HAVE_BACKEND( POSIX_FILENO )
        int                   fd        = fileno( fp );
        SCOREP_IoHandleHandle fd_handle = SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
        #endif

        SCOREP_IoMgmt_PushHandle( handle );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fclose )( fp );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_IoMgmt_PopHandle( handle );
        /*
         * As any further access to the stream results in undefined behavior
         * destroy the corresponding handle in the Score-P internal I/O handle
         * management unconditionally.
         */
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( ret != 0 && errno != EBADF )
            {
                SCOREP_IoMgmt_ReinsertHandle( SCOREP_IO_PARADIGM_ISOC, handle );

                #if HAVE_BACKEND( POSIX_FILENO )
                if ( fd_handle != SCOREP_INVALID_IO_HANDLE )
                {
                    SCOREP_IoMgmt_ReinsertHandle( SCOREP_IO_PARADIGM_POSIX, fd_handle );
                }
                #endif
            }
            else
            {
                #if HAVE_BACKEND( POSIX_FILENO )
                if ( fd_handle != SCOREP_INVALID_IO_HANDLE )
                {
                    SCOREP_IoMgmt_DestroyHandle( fd_handle );
                    SCOREP_IoDestroyHandle( fd_handle );
                }
                #endif
                SCOREP_IoMgmt_DestroyHandle( handle );
            }
            SCOREP_IoDestroyHandle( handle );
        }

        SCOREP_ExitRegion( scorep_posix_io_region_fclose );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fclose )( fp );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

FILE*
SCOREP_LIBWRAP_WRAPPER( fdopen )( int fd, const char* mode )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    FILE* ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fdopen );

        SCOREP_IoHandleHandle fd_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_ISOC,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fdopen )( fd, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != NULL && fd_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoFileHandle   file          = SCOREP_IoHandleHandle_GetIoFile( fd_handle );
            SCOREP_IoHandleHandle stream_handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_ISOC, file, fd + 1, &ret );
            if ( stream_handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreateHandle( stream_handle,                                  /* I/O handle */
                                       get_scorep_io_access_mode_from_string( mode ),  /* open mode */
                                       SCOREP_IO_CREATION_FLAG_NONE,                   /* additional flags */
                                       SCOREP_IO_STATUS_FLAG_NONE );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }


        SCOREP_ExitRegion( scorep_posix_io_region_fdopen );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fdopen )( fd, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fflush )( FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fflush );

        SCOREP_IoHandleHandle io_handle;
        if ( stream == NULL )
        {
            io_handle = scorep_posix_io_flush_all_handle;
        }
        else
        {
            io_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        }
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( io_handle,
                                     SCOREP_IO_OPERATION_MODE_FLUSH,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_IoMgmt_PushHandle( io_handle );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fflush )( stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( io_handle,
                                        SCOREP_IO_OPERATION_MODE_FLUSH,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( io_handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fflush );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fflush )( stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fgetc )( FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fgetc );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( 1 /* one character == one byte */ ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fgetc )( stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )( ret != EOF ? 1 : 0 ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fgetc );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fgetc )( stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fgetpos )( FILE* stream, fpos_t* pos )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fgetpos );
        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fgetpos )( stream, pos );
        SCOREP_EXIT_WRAPPED_REGION();
        SCOREP_IoMgmt_PopHandle( handle );
        SCOREP_ExitRegion( scorep_posix_io_region_fgetpos );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fgetpos )( stream, pos );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

char*
SCOREP_LIBWRAP_WRAPPER( fgets )( char* s, int size, FILE* stream )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    char* ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fgets );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( size ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fgets )( s, size, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )( ret != NULL ? strlen( s ) + 1 /* terminating null byte */ : 0 ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fgets );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fgets )( s, size, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

void
SCOREP_LIBWRAP_WRAPPER( flockfile )( FILE* filehandle )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_flockfile );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &filehandle );

        SCOREP_ENTER_WRAPPED_REGION();
        SCOREP_LIBWRAP_ORIGINAL( flockfile )( filehandle );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoAcquireLock( handle, SCOREP_LOCK_EXCLUSIVE );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_flockfile );
    }
    else
    {
        SCOREP_LIBWRAP_ORIGINAL( flockfile )( filehandle );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

static inline void
create_posix_handle( int fd, const char* path, SCOREP_IoAccessMode access_mode )
{
    SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_POSIX,
                                       SCOREP_IO_HANDLE_FLAG_NONE,
                                       SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                       "" );

    SCOREP_IoFileHandle   file      = SCOREP_IoMgmt_GetIoFileHandle( path );
    SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_CompleteHandleCreation(
        SCOREP_IO_PARADIGM_POSIX, file, fd + 1, &fd );

    if ( io_handle != SCOREP_INVALID_IO_HANDLE )
    {
        SCOREP_IoCreateHandle( io_handle,
                               access_mode,
                               SCOREP_IO_CREATION_FLAG_NONE,
                               SCOREP_IO_STATUS_FLAG_NONE );
    }
}

FILE*
SCOREP_LIBWRAP_WRAPPER( fopen )( const char* path, const char* mode )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    FILE* ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fopen );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_ISOC,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fopen )( path, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        /*
         * Argument 'mode' is a string. It begins with one of the following sequences.
         *
         * r   Open file for reading.
         *     The stream is positioned at the beginning of the file.
         *
         * r+  Open file for reading and writing.
         *     The stream is positioned at the beginning of the file.
         *
         * w   Truncate file to zero length or create text file for writing.
         *     The stream is positioned at the beginning of the file.
         *
         * w+  Open for reading and writing.
         *     The file is created if it does not exist, otherwise it is truncated.
         *     The stream is positioned at the beginning of the file.
         *
         * a   Open for appending (writing at end of file).
         *     The file is created if it does not exist.
         *     The stream is positioned at the end of the file.
         *
         * a+  Open for reading and appending (writing at end of file).
         *     The file is created if it does not exist.
         *     The initial file position for reading is at the
         *     beginning of the file, but output is always appended to
         *     the end of the file.
         *
         * Additional characters may follow.
         *
         */

        if ( ret != NULL )
        {
            SCOREP_IoAccessMode access_mode = get_scorep_io_access_mode_from_string( mode );

            int fd = -1; // +1 also used as unify key
            #if HAVE_BACKEND( POSIX_FILENO )
            fd = fileno( ret );

            SCOREP_IoHandleHandle fd_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &fd );

            if ( fd_handle == SCOREP_INVALID_IO_HANDLE )
            {
                create_posix_handle( fd, path, access_mode );
            }
            #endif /* HAVE_BACKEND( POSIX_FILENO ) */
            SCOREP_IoFileHandle   file      = SCOREP_IoMgmt_GetIoFileHandle( path );
            SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_ISOC, file, fd + 1, &ret );

            if ( io_handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreateHandle( io_handle,
                                       get_scorep_io_access_mode_from_string( mode ),
                                       SCOREP_IO_CREATION_FLAG_NONE,
                                       SCOREP_IO_STATUS_FLAG_NONE );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }


        SCOREP_ExitRegion( scorep_posix_io_region_fopen );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fopen )( path, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

FILE*
SCOREP_LIBWRAP_WRAPPER( fopen64 )( const char* path, const char* mode )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    FILE* ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fopen64 );

        SCOREP_IoMgmt_BeginHandleCreation( SCOREP_IO_PARADIGM_ISOC,
                                           SCOREP_IO_HANDLE_FLAG_NONE,
                                           SCOREP_INVALID_INTERIM_COMMUNICATOR,
                                           "" );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fopen64 )( path, mode );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != NULL )
        {
            SCOREP_IoAccessMode access_mode = get_scorep_io_access_mode_from_string( mode );

            int fd = -1; // +1 also used as unify key
            #if HAVE_BACKEND( POSIX_FILENO )
            fd = fileno( ret );

            SCOREP_IoHandleHandle fd_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_POSIX, &fd );

            if ( fd_handle == SCOREP_INVALID_IO_HANDLE )
            {
                create_posix_handle( fd, path, access_mode );
            }
            #endif /* HAVE_BACKEND( POSIX_FILENO ) */

            SCOREP_IoFileHandle file = SCOREP_IoMgmt_GetIoFileHandle( path );

            SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_CompleteHandleCreation(
                SCOREP_IO_PARADIGM_ISOC, file, fd + 1, &ret );

            if ( io_handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreateHandle( io_handle,
                                       access_mode,
                                       SCOREP_IO_CREATION_FLAG_NONE,
                                       SCOREP_IO_STATUS_FLAG_NONE );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }


        SCOREP_ExitRegion( scorep_posix_io_region_fopen64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fopen64 )( path, mode );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fputc )( int c, FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fputc );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( 1 /* one character == one byte */ ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fputc )( c, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ( uint64_t )( 1 ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fputc );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fputc )( c, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fprintf )( FILE* stream, const char* format, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fprintf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        va_list args;
        va_start( args, format );
        SCOREP_ENTER_WRAPPED_REGION();
        ret = vfprintf( stream, format, args );
        SCOREP_EXIT_WRAPPED_REGION();
        va_end( args );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fprintf );
    }
    else
    {
        va_list args;
        va_start( args, format );
        ret = vfprintf( stream, format, args );
        va_end( args );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fputs )( const char* s, FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    /* fputs() writes the string s to stream, WITHOUT its terminating null byte ('\0'). */

    int ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fputs );

        uint64_t              length = ( uint64_t )strlen( s );
        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     length,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fputs )( s, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        length,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fputs );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fputs )( s, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

size_t
SCOREP_LIBWRAP_WRAPPER( fread )( void* ptr, size_t size, size_t nmemb, FILE* stream )
{
    bool   trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    size_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fread );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( size * nmemb ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fread )( ptr, size, nmemb, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )( size * ret ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fread );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fread )( ptr, size, nmemb, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

FILE*
SCOREP_LIBWRAP_WRAPPER( freopen )( const char* path, const char* mode, FILE* stream )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    FILE* ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_freopen );

        // Case: stream will be closed by freopen
        SCOREP_IoHandleHandle old_handle = SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        int fd = -1; // +1 also used as unify key
        #if HAVE_BACKEND( POSIX_FILENO )
        fd = fileno( stream );
        SCOREP_IoHandleHandle fd_handle = SCOREP_IoMgmt_RemoveHandle( SCOREP_IO_PARADIGM_POSIX, &fd );
        #endif

        SCOREP_IoMgmt_BeginHandleDuplication( SCOREP_IO_PARADIGM_ISOC,
                                              old_handle );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( freopen )( path, mode, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( ret != NULL )
        {
            if ( old_handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoDestroyHandle( old_handle );
                SCOREP_IoMgmt_DestroyHandle( old_handle );
            }
            SCOREP_IoAccessMode access_mode = get_scorep_io_access_mode_from_string( mode );

            #if HAVE_BACKEND( POSIX_FILENO )
            fd = fileno( ret );
            if ( fd_handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoDestroyHandle( fd_handle );
                SCOREP_IoMgmt_DestroyHandle( fd_handle );
            }
            create_posix_handle( fd, path, access_mode );
            #endif /* HAVE_BACKEND( POSIX_FILENO ) */

            SCOREP_IoHandleHandle new_handle = SCOREP_IoMgmt_CompleteHandleDuplication(
                SCOREP_IO_PARADIGM_ISOC, SCOREP_IoMgmt_GetIoFileHandle( path ), fd + 1, &ret );
            if ( new_handle != SCOREP_INVALID_IO_HANDLE )
            {
                SCOREP_IoCreateHandle( new_handle,
                                       access_mode,
                                       SCOREP_IO_CREATION_FLAG_NONE,
                                       SCOREP_IO_STATUS_FLAG_NONE );
            }
        }
        else
        {
            SCOREP_IoMgmt_DropIncompleteHandle();
        }



        SCOREP_ExitRegion( scorep_posix_io_region_freopen );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( freopen )( path, mode, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}


int
SCOREP_LIBWRAP_WRAPPER( fscanf )( FILE* stream, const char* format, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fscanf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();

        va_list args;
        va_start( args, format );
        ret = vfscanf( stream, format, args );
        va_end( args );

        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fscanf );
    }
    else
    {
        va_list args;
        va_start( args, format );
        ret = vfscanf( stream, format, args );
        va_end( args );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fseek )( FILE* stream, long offset, int whence )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fseek );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fseek )( stream, offset, whence );
        SCOREP_EXIT_WRAPPED_REGION();

        long fp_offset = SCOREP_LIBWRAP_ORIGINAL( ftell )( stream );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           ( int64_t )offset,
                           scorep_posix_io_get_scorep_io_seek_option( whence ),
                           ( uint64_t )fp_offset );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fseek );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fseek )( stream, offset, whence );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fseeko )( FILE* stream, off_t offset, int whence )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fseeko );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fseeko )( stream, offset, whence );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           ( int64_t )offset,
                           scorep_posix_io_get_scorep_io_seek_option( whence ),
                           ( uint64_t )( SCOREP_LIBWRAP_ORIGINAL( ftello )( stream ) ) );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fseeko );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fseeko )( stream, offset, whence );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( fseeko64 )( FILE* stream, scorep_off64_t offset, int whence )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fseeko64 );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fseeko64 )( stream, offset, whence );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           ( int64_t )offset,
                           scorep_posix_io_get_scorep_io_seek_option( whence ),
                           ( uint64_t )( SCOREP_LIBWRAP_ORIGINAL( ftello )( stream ) ) );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fseeko64 );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fseeko64 )( stream, offset, whence );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

#if 0
int
SCOREP_LIBWRAP_WRAPPER( fsetpos )( FILE* stream, const fpos_t* pos )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fsetpos );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fsetpos )( stream, pos );
        SCOREP_EXIT_WRAPPED_REGION();

        long fp_offset = SCOREP_LIBWRAP_ORIGINAL( ftell )( stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           0,
                           scorep_posix_io_get_scorep_io_seek_option( SEEK_SET ),
                           fp_offset );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fsetpos );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fsetpos )( stream, pos );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
#endif

long
SCOREP_LIBWRAP_WRAPPER( ftell )( FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    long ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_ftell );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( ftell )( stream );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_ftell );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( ftell )( stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

off_t
SCOREP_LIBWRAP_WRAPPER( ftello )( FILE* stream )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    off_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_ftello );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( ftello )( stream );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_ftello );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( ftello )( stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( ftrylockfile )( FILE* filehandle )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_ftrylockfile );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &filehandle );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( ftrylockfile )( filehandle );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            if ( ret == 0 )
            {
                SCOREP_IoAcquireLock( handle, SCOREP_LOCK_EXCLUSIVE );
            }
            else
            {
                SCOREP_IoTryLock( handle, SCOREP_LOCK_EXCLUSIVE );
            }
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_ftrylockfile );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( ftrylockfile )( filehandle );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

void
SCOREP_LIBWRAP_WRAPPER( funlockfile )( FILE* filehandle )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_funlockfile );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &filehandle );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoReleaseLock( handle, SCOREP_LOCK_EXCLUSIVE );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        SCOREP_LIBWRAP_ORIGINAL( funlockfile )( filehandle );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_funlockfile );
    }
    else
    {
        SCOREP_LIBWRAP_ORIGINAL( funlockfile )( filehandle );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

size_t
SCOREP_LIBWRAP_WRAPPER( fwrite )( const void* ptr, size_t size, size_t nmemb, FILE* stream )
{
    bool   trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    size_t ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_fwrite );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( size * nmemb ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( fwrite )( ptr, size, nmemb, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ( uint64_t )( size * ret ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_fwrite );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( fwrite )( ptr, size, nmemb, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

/* getc() might be implemented as a macro */
int
SCOREP_LIBWRAP_WRAPPER( getc )( FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_getc );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( 1 /* one character == one byte */ ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( getc )( stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )( ( ret != EOF ? 1 : 0 ) ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_getc );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( getc )( stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( getchar )( void )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_getchar );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdin );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( 1 /* one character == one byte */ ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( getchar )();
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )( ret != EOF ? 1 : 0 ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_getchar );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( getchar )();
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

char*
SCOREP_LIBWRAP_WRAPPER( gets )( char* s )
{
    bool  trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    char* ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_gets );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdin );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( 0 ) /* What's the requested size of transferred bytes of a gets operation? */,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( gets )( s );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        ( uint64_t )( ret != NULL ? strlen( s ) + 1 /* terminating null byte */ : 0 ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_gets );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( gets )( s );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( printf )( const char* format, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_printf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdout );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        va_list args;
        va_start( args, format );
        SCOREP_ENTER_WRAPPED_REGION();
        ret = vprintf( format, args );
        SCOREP_EXIT_WRAPPED_REGION();
        va_end( args );

        if ( handle != SCOREP_INVALID_IO_HANDLE && ret >= 0 )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_printf );
    }
    else
    {
        va_list args;
        va_start( args, format );
        ret = vprintf( format, args );
        va_end( args );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( putchar )( int c )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_putchar );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdout );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     ( uint64_t )( 1 /* one character == one byte */ ),
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( putchar )( c );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ( uint64_t )( 1 ),
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_putchar );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( putchar )( c );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( puts )( const char* s )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_puts );

        uint64_t              length = ( uint64_t )strlen( s ) + 1 /* terminating null byte */;
        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdout );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     length,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( puts )( s );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        length,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_puts );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( puts )( s );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( remove )( const char* pathname )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_remove );

        SCOREP_IoFileHandle file_handle = SCOREP_IoMgmt_GetIoFileHandle( pathname );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( remove )( pathname );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( file_handle != SCOREP_INVALID_IO_FILE )
        {
            SCOREP_IoDeleteFile( SCOREP_IO_PARADIGM_ISOC,
                                 file_handle );
        }

        SCOREP_ExitRegion( scorep_posix_io_region_remove );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( remove )( pathname );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

void
SCOREP_LIBWRAP_WRAPPER( rewind )( FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_rewind );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        SCOREP_LIBWRAP_ORIGINAL( rewind )( stream );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoSeek( handle,
                           ( int64_t )0,
                           scorep_posix_io_get_scorep_io_seek_option( SEEK_SET ),
                           ( uint64_t )0 );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_rewind );
    }
    else
    {
        SCOREP_LIBWRAP_ORIGINAL( rewind )( stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

int
SCOREP_LIBWRAP_WRAPPER( scanf )( const char* format, ... )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_scanf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, stdin );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();

        va_list args;
        va_start( args, format );
        ret = vscanf( format, args );
        va_end( args );

        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_scanf );
    }
    else
    {
        va_list args;
        va_start( args, format );
        ret = vscanf( format, args );
        va_end( args );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( ungetc )( int c, FILE* stream )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_ungetc );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( ungetc )( c, stream );
        SCOREP_EXIT_WRAPPED_REGION();

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_ungetc );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( ungetc )( c, stream );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( vfprintf )( FILE* stream, const char* format, va_list ap )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_vfprintf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( vfprintf )( stream, format, ap );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_vfprintf );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( vfprintf )( stream, format, ap );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( vfscanf )( FILE* stream, const char* format, va_list ap )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_vfscanf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stream );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( vfscanf )( stream, format, ap );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_vfscanf );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( vfscanf )( stream, format, ap );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( vscanf )( const char* format, va_list ap )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_vscanf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdin );

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_READ,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( vscanf )( format, ap );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_READ,
                                        SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_vscanf );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( vscanf )( format, ap );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}

int
SCOREP_LIBWRAP_WRAPPER( vprintf )( const char* format, va_list ap )
{
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();
    int  ret;

    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_EnterWrappedRegion( scorep_posix_io_region_vprintf );

        SCOREP_IoHandleHandle handle = SCOREP_IoMgmt_GetAndPushHandle( SCOREP_IO_PARADIGM_ISOC, &stdout );
        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationBegin( handle,
                                     SCOREP_IO_OPERATION_MODE_WRITE,
                                     SCOREP_IO_OPERATION_FLAG_NON_COLLECTIVE | SCOREP_IO_OPERATION_FLAG_BLOCKING,
                                     SCOREP_IO_UNKOWN_TRANSFER_SIZE,
                                     SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */,
                                     SCOREP_IO_UNKNOWN_OFFSET );
        }

        SCOREP_ENTER_WRAPPED_REGION();
        ret = SCOREP_LIBWRAP_ORIGINAL( vprintf )( format, ap );
        SCOREP_EXIT_WRAPPED_REGION();

        if ( handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationComplete( handle,
                                        SCOREP_IO_OPERATION_MODE_WRITE,
                                        ret,
                                        SCOREP_BLOCKING_IO_OPERATION_MATCHING_ID_ISOC /* matching id */ );
        }

        SCOREP_IoMgmt_PopHandle( handle );

        SCOREP_ExitRegion( scorep_posix_io_region_vprintf );
    }
    else
    {
        ret = SCOREP_LIBWRAP_ORIGINAL( vprintf )( format, ap );
    }
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return ret;
}
