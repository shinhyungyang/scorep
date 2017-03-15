## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2014-2015,
## Technische Universitaet Dresden, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##

# SCOREP_THREAD_LOCAL_STORAGE
# -----------------
# Checks whether the compiler supports thread-local storage.
# Sets scorep_have_thread_local_storage to yes if the compiler does,
# otherwise scorep_have_thread_local_storage is set to no.
AC_DEFUN([SCOREP_THREAD_LOCAL_STORAGE], [

scorep_have_thread_local_storage="no"
thread_local_storage_summary="no"
thread_local_storage_cflags=""

_SCOREP_CHECK__THREAD(scorep_have_thread_local_storage,thread_local_storage_summary)

AS_IF([test "x${scorep_have_thread_local_storage}" = "xno"],
      [_SCOREP_CHECK_C11_THREAD_LOCAL(scorep_have_thread_local_storage,thread_local_storage_summary)])

AS_IF([test "x${scorep_have_thread_local_storage}" = "xno"],
      [thread_local_storage_cflags="-c11"
       _SCOREP_CHECK_C11_THREAD_LOCAL(scorep_have_thread_local_storage,thread_local_storage_summary,$thread_local_storage_cflags)
       AS_IF([test "x${scorep_have_thread_local_storage}" = "xyes"],
             [CC="$CC $thread_local_storage_cflags"])])

need_tls_model_macro=yes
AC_SCOREP_COND_HAVE([THREAD_LOCAL_STORAGE],
                    [test "x${scorep_have_thread_local_storage}" = "xyes"],
                    [Defined if thread local storage support is available.],
                    [AS_IF([test "x${enable_shared}" = "xyes"], [
                         _SCOREP_CHECK_TLS_MODEL([initial-exec], [
                             AC_DEFINE([SCOREP_THREAD_LOCAL_STORAGE_MODEL( tls_model_arg )],
                                       [__attribute__(( tls_model( tls_model_arg ) ))],
                                       [Variable attribute to select a specific TLS model.])
                             need_tls_model_macro=no
                             thread_local_storage_summary="${thread_local_storage_summary} and the initial-exec model"
                         ], [
                             thread_local_storage_summary="${thread_local_storage_summary} and the default model"
                         ])
                     ])])
AS_IF([test "x${need_tls_model_macro}" = "xyes"],
    [AC_DEFINE([SCOREP_THREAD_LOCAL_STORAGE_MODEL( tls_model_arg )],
        [/* not supported tls_model_arg */],
        [Variable attribute to select a specific TLS model.])])
AS_UNSET([need_tls_model_macro])

AFS_SUMMARY([TLS support], [${thread_local_storage_summary}])
])


# _SCOREP_CHECK__THREAD( RESULT,
#                        SUMMARY )
# -----------------
# Performs checks whether the compiler supports '__thread'.
AC_DEFUN([_SCOREP_CHECK__THREAD], [

AC_LANG_PUSH([C])

dnl There is a bug in gcc < 4.1.2 involving TLS and -fPIC on x86:
dnl http://gcc.gnu.org/ml/gcc-bugs/2006-09/msg02275.html
dnl
dnl And mingw also does compile __thread but resultant code actually
dnl fails to work correctly at least in some not so ancient version:
dnl http://mingw-users.1079350.n2.nabble.com/gcc-4-4-multi-threaded-exception-handling-amp-thread-specifier-not-working-td3440749.html
dnl
dnl Also it was reported that earlier gcc versions for mips compile
dnl __thread but it does not really work

AC_MSG_CHECKING([for __thread])
AC_LINK_IFELSE([AC_LANG_PROGRAM([#if defined(__GNUC__) && ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 1) || (__GNUC__ == 4 && __GNUC_MINOR__ == 1 && __GNUC_PATCHLEVEL__ < 2))
#ups_unsupported_gcc_version gcc has this bug: http://gcc.gnu.org/ml/gcc-bugs/2006-09/msg02275.html
#elif defined(__MINGW32__)
#ups_unsupported_mingw_version mingw doesnt really support thread local storage
#elif defined(__APPLE__)
#ups_unsupported_platform OSX __thread support is known to call malloc which makes it unsafe to use from malloc replacement
#endif

__thread int global_thread_private_var = 1;
], [static __thread int my_thread_private_var = 0])],
   [$1="yes"
    $2="yes, using __thread"
    AC_DEFINE([SCOREP_THREAD_LOCAL_STORAGE_SPECIFIER],
              [__thread],
              [Set specifier to mark a variable as thread-local storage (TLS)])],
   [$1="no"
    $2="no"])
AC_MSG_RESULT([$][$1])

AC_LANG_POP([C])

])


# _SCOREP_CHECK_C11_THREAD_LOCAL( RESULT,
#                                 SUMMARY,
#                                 [ADDITIONAL_CFLAGS] )
# -----------------
# Performs checks whether the compiler supports the C11 '_Thread_local' feature.
AC_DEFUN([_SCOREP_CHECK_C11_THREAD_LOCAL], [

AC_LANG_PUSH([C])

cflags_save="$CFLAGS"
CFLAGS="$CFLAGS $3"

AC_MSG_CHECKING([for _Thread_local])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
_Thread_local int global_thread_private_var = 1;
], [static _Thread_local int my_thread_private_var = 0])],
   [$1="yes"
    $2="yes, using _Thread_local"
    AC_DEFINE([SCOREP_THREAD_LOCAL_STORAGE_SPECIFIER],
              [_Thread_local],
              [Set specifier to mark a variable as thread-local storage (TLS)])],
   [$1="no"
    $2="no"])
AC_MSG_RESULT([$][$1])

CFLAGS="$cflags_save"

AC_LANG_POP([C])

])

# _SCOREP_CHECK_TLS_MODEL( TLS_MODEL,
#                          ACTION_FOUND,
#                          ACTION_NOT_FOUND )
# -----------------------------------
# Performs checks whether the compiler supports '__attribute__(( tls_model( "TLS_MODEL" ) ))'.
AC_DEFUN([_SCOREP_CHECK_TLS_MODEL], [
AC_REQUIRE([LT_OUTPUT])

AC_LANG_PUSH([C])

AC_LANG_CONFTEST([
    AC_LANG_SOURCE([[
extern SCOREP_THREAD_LOCAL_STORAGE_SPECIFIER volatile int confvar __attribute__(( tls_model( "$1" ) ));

void conffunc(void)
{
    confvar++;
    --confvar;
}
]])])

tls_model_compile='$SHELL ./libtool --mode=compile --tag=_AC_CC [$]_AC_CC $CPPFLAGS [$]_AC_LANG_PREFIX[FLAGS] -c -o conftest.lo conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
tls_model_link='$SHELL ./libtool --mode=link --tag=_AC_CC [$]_AC_CC [$]_AC_LANG_PREFIX[FLAGS] -rpath $PWD/lib -o libconftest.la conftest.lo >&AS_MESSAGE_LOG_FD'
tls_model_clean='$SHELL ./libtool --mode=clean $RM conftest.lo libconftest.la >&AS_MESSAGE_LOG_FD'

AC_MSG_CHECKING([for tls_model("$1")])
AS_IF([_AC_DO_VAR([tls_model_compile]) &&
       _AC_DO_VAR([tls_model_link])],
   [AC_MSG_RESULT([yes])
    $2
    :],
   [_AC_MSG_LOG_CONFTEST
    AC_MSG_RESULT([no])
    $3
    :])

_AC_DO_VAR([tls_model_clean])
$RM conftest.$ac_ext

AS_UNSET([tls_model_compile])
AS_UNSET([tls_model_link])
AS_UNSET([tls_model_compile])

AC_LANG_POP([C])

])
