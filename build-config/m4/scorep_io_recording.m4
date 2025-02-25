## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2015-2017, 2020, 2023, 2025,
## Technische Universitaet Dresden, Germany
##
## Copyright (c) 2022,
## Forschungszentrum Juelich GmbH, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license. See the COPYING file in the package base
## directory for details.
##

## file build-config/m4/scorep_io_recording.m4

AC_DEFUN([SCOREP_IO_RECORDING], [
AFS_SUMMARY_SECTION_BEGIN([I/O Recording features])
    _SCOREP_IO_RECORDING_POSIX
AFS_SUMMARY_SECTION_END
]) # AC_DEFUN(SCOREP_IO_RECORDING)

dnl ----------------------------------------------------------------------------

dnl ----------------------------------------------------------------------------

AC_DEFUN([_SCOREP_IO_RECORDING_POSIX], [
AC_REQUIRE([AFS_CHECK_THREAD_LOCAL_STORAGE])dnl
AC_REQUIRE([SCOREP_CHECK_LIBRARY_WRAPPING])dnl

dnl Check for POSIX synchronous I/O

AFS_SUMMARY_PUSH

scorep_posix_io_support="yes"
scorep_posix_io_summary_reason=

# check for library wrapping support
AM_COND_IF([HAVE_LIBWRAP_SUPPORT],
    [],
    [scorep_posix_io_support="no"
     AS_VAR_APPEND([scorep_posix_io_summary_reason], [", missing library wrapping support"])])

# check result of TLS
AS_IF([test x"${scorep_posix_io_support}" = x"yes"],
      [AS_IF([test x"${afs_have_thread_local_storage}" != x"yes"],
             [scorep_posix_io_support="no"
              AS_VAR_APPEND([scorep_posix_io_summary_reason], [", missing TLS support"])])])

AC_SCOREP_COND_HAVE([POSIX_IO_SUPPORT],
                    [test x"${scorep_posix_io_support}" = x"yes"],
                    [Defined if recording calls to POSIX I/O is possible.])

AFS_SUMMARY_POP([POSIX I/O support], [${scorep_posix_io_support}${scorep_posix_io_summary_reason}])

dnl Check for POSIX asynchronous I/O
AFS_SUMMARY_PUSH

scorep_posix_aio_support=${scorep_posix_io_support}
scorep_posix_aio_summary_reason=

# check result of TLS
AS_IF([test x"${afs_have_thread_local_storage}" != x"yes"],
      [scorep_posix_aio_support="no"
       AS_VAR_APPEND([scorep_posix_aio_summary_reason], [", missing TLS support"])])

AS_IF([test x"${scorep_posix_aio_support}" = x"yes"],
      [AC_CHECK_HEADER([aio.h],
                       [],
                       [AC_MSG_NOTICE([no aio.h file found])
                        scorep_posix_aio_support="no"
                        AS_VAR_APPEND([scorep_posix_aio_summary_reason], [", missing aio.h header"])])])

AS_IF([test x"${scorep_posix_aio_support}" = x"yes"],
      [AC_SEARCH_LIBS([aio_read],
                      [rt],
                      [AS_CASE([${ac_cv_search_aio_read}],
                               [no*],  [with_posix_aio_libs=""],
                               [with_posix_aio_libs="${ac_cv_search_aio_read}"])],
                      [AC_MSG_NOTICE([no library found providing aio_read])
                       scorep_posix_aio_support="no"
                       AS_VAR_APPEND([scorep_posix_aio_summary_reason], [", missing aio library"])])])

AS_IF([test x"${scorep_posix_aio_support}" = x"yes"],
      [AC_CHECK_MEMBER([struct aiocb.__return_value], [],
                       [AC_MSG_NOTICE([we need 'struct aiocb.__return_value' member])
                        scorep_posix_aio_support="no"
                        AS_VAR_APPEND([scorep_posix_aio_summary_reason], [", missing struct aiocb.__return_value member"])],
                       [[#include <aio.h>]])])

AC_SCOREP_COND_HAVE([POSIX_AIO_SUPPORT],
                    [test x"${scorep_posix_aio_support}" = x"yes"],
                    [Defined if recording calls to POSIX asynchronous I/O is possible.],
                    [scorep_posix_aio_summary_reason="${with_posix_aio_libs:+, using ${with_posix_aio_libs}}"])

AFS_SUMMARY_POP([POSIX asynchronous I/O support], [${scorep_posix_aio_support}${scorep_posix_aio_summary_reason}])
])
