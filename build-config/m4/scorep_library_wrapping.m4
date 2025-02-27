## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2014, 2017, 2025,
## Technische Universitaet Dresden, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license. See the COPYING file in the package base
## directory for details.
##

## file build-config/m4/scorep_library_wrapping.m4


AC_DEFUN([SCOREP_CHECK_LIBRARY_WRAPPING], [
AC_REQUIRE([SCOREP_CHECK_DLFCN])dnl
AC_REQUIRE([SCOREP_CHECK_LIBGOTCHA])dnl

dnl Do not check for prerequisite of library wrapping on the frontend.
AS_IF([test "x$ac_scorep_backend" = xno], [AC_MSG_ERROR([cannot check for library wrapping support on frontend.])])

scorep_libwrap_support="yes"
scorep_libwrap_summary_reason=""

AS_IF([test "x${scorep_gotcha_support}" != "xyes"],
    [scorep_libwrap_support="no"
     scorep_libwrap_summary_reason=", missing libgotcha support"])

AC_SCOREP_COND_HAVE([LIBWRAP_SUPPORT],
    [test "x${scorep_libwrap_support}" = "xyes"],
    [Define if library wrapping is supported.])

scorep_libwrap_plugin_support=$scorep_libwrap_support
scorep_libwrap_plugin_summary_reason=""

AM_COND_IF([HAVE_DLFCN_SUPPORT],
    [],
    [scorep_libwrap_plugin_support="no"
     scorep_libwrap_plugin_summary_reason+=", missing dlopen support"])

AC_SCOREP_COND_HAVE([LIBWRAP_PLUGIN_SUPPORT],
    [test "x${scorep_libwrap_plugin_support}" = "xyes"],
    [Define if library wrapping plug-ins are supported.])

])

dnl ----------------------------------------------------------------------------

AC_DEFUN([SCOREP_CHECK_LIBRARY_WRAPPING_GENERATOR], [

# The genrator workflow (`scorep-libwrap-init`) needs to be able to build
# loadable objects with this `libtool`, hence it needs to have `--enabled-shared`
# A runtime check is also in `scorep-libwrap-init`.
AFS_AM_CONDITIONAL([HAVE_LIBRARY_WRAPPING_GENERATOR],
    [test x"$enable_shared" = x"yes" &&
     test -e ../build-libwrap/library_wrapping_supported],
    [false])

AM_COND_IF([HAVE_LIBRARY_WRAPPING_GENERATOR],
    [scorep_have_libwrap_generator="yes"],
    [scorep_have_libwrap_generator="no"])

])

AC_DEFUN([SCOREP_LIBRARY_WRAPPING_GENERATOR], [
AC_REQUIRE([SCOREP_CHECK_LIBRARY_WRAPPING_GENERATOR])dnl

AM_COND_IF([HAVE_LIBRARY_WRAPPING_GENERATOR], [$1], [$2])

])

dnl ----------------------------------------------------------------------------

AC_DEFUN([SCOREP_LIBRARY_WRAPPING_SUMMARY], [
AC_REQUIRE([SCOREP_CHECK_LIBRARY_WRAPPING])dnl
AC_REQUIRE([SCOREP_CHECK_LIBRARY_WRAPPING_GENERATOR])dnl

AFS_SUMMARY_PUSH

SCOREP_GOTCHA

AFS_SUMMARY([Loading user wrappers], [${scorep_libwrap_plugin_support}${scorep_libwrap_plugin_summary_reason}])
AFS_SUMMARY([Building user wrappers], [${scorep_have_libwrap_generator}])
AFS_SUMMARY_POP([Library wrapping support], [${scorep_gotcha_support}])

])
