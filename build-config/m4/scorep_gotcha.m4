## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2025,
## Technische Universitaet Dresden, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##

## file build-config/m4/scorep_gotcha.m4


AC_DEFUN([SCOREP_CHECK_LIBGOTCHA], [
AC_REQUIRE([AC_PROG_SED])dnl

AC_ARG_VAR([CMAKE], [CMake to use for building external dependencies])
AS_IF([test "x${ac_cv_env_CMAKE_set}" != "xset"],
    [AC_PATH_PROG([CMAKE], [cmake], [:])])

scorep_gotcha_support="yes"
scorep_gotcha_summary_reason=

AC_LANG_PUSH([C])
AFS_EXTERNAL_LIB([gotcha], [_LIBGOTCHA_CHECK], [gotcha/gotcha.h], [_LIBGOTCHA_DOWNLOAD])dnl
AC_LANG_POP([C])

AS_IF([test "x${have_libgotcha}" != "xyes"],
    [scorep_gotcha_support="no"])

])dnl SCOREP_CHECK_LIBGOTCHA

dnl ----------------------------------------------------------------------------

# _LIBGOTCHA_DOWNLOAD()
# ---------------------
# Generate a Makefile.libgotcha to download libgotcha and install libgotcha
# at make time. In addition, set automake conditional
# HAVE_SCOREP_LIBGOTCHA_MAKEFILE to trigger this process from
# build-backend/Makefile.
#
m4_define([_LIBGOTCHA_DOWNLOAD], [
_afs_lib_PREFIX="${libdir}${backend_suffix}/${PACKAGE}/[]_afs_lib_name[]/${libgotcha_version}"
_afs_lib_MAKEFILE="Makefile.[]_afs_lib_name"
_afs_lib_LDFLAGS="-L$[]_afs_lib_PREFIX[]/lib -R$[]_afs_lib_PREFIX[]/lib"
_afs_lib_CPPFLAGS="-I$[]_afs_lib_PREFIX/include"
dnl
scorep_cmake_version=invalid
AS_IF([test "x${CMAKE}" != "x:"],
    [scorep_cmake_version=$($CMAKE --version | $SED -n -e 's/^cmake version //p')
     AS_CASE([${scorep_cmake_version}],
        [1.*|2.*], [AC_MSG_ERROR([${CMAKE} ${scorep_cmake_version} to old, need at least version 3+ to build libgotcha via '--with-libgotcha=download'])],
        [*.*], [: legit CMake 3+],
        [CMAKE=:])])
AS_CASE(["${CMAKE},${ac_cv_env_CMAKE_set}"],
    [:,set],
        [AC_MSG_ERROR([CMake '${ac_cv_env_CMAKE_value}' not functional, but required to build libgotcha via '--with-libgotcha=download'])],
    [:,*],
        [AC_MSG_ERROR([No CMake found, but required to build libgotcha via '--with-libgotcha=download'])])
AFS_AM_CONDITIONAL(HAVE_[]_afs_lib_MAKEFILE, [:], [false])dnl
scorep_gotcha_summary_reason=", from downloaded $libgotcha_url building with ${CMAKE} ${scorep_cmake_version}"
have_libgotcha="yes"
dnl
AC_SUBST([libgotcha_package])
AC_SUBST([libgotcha_url])
AC_CONFIG_FILES([Makefile.libgotcha:../build-backend/Makefile.libgotcha.in])
])dnl _LIBGOTCHA_DOWNLOAD

dnl ----------------------------------------------------------------------------

m4_define([_LIBGOTCHA_CHECK], [
AS_IF([test "x${_afs_lib_prevent_check}" = xyes], [
    AS_IF([test "x${_afs_lib_prevent_check_reason}" = xdisabled],
        [AS_CASE([${ac_scorep_platform}],
            [mac|mingw], [:],
            [AC_MSG_ERROR([A working _afs_lib_name installation is required, --without-_afs_lib_name is not a valid option. See --with-_afs_lib_name in INSTALL.])])],
    [test "x${_afs_lib_prevent_check_reason}" = xcrosscompile],
        [AC_MSG_ERROR([A working _afs_lib_name installation is required. Either provide a path or use the download option, see --with-_afs_lib_name in INSTALL.])],
    [: else],
        [AC_MSG_ERROR([internal: Unknown _afs_lib_prevent_check_reason="${_afs_lib_prevent_check_reason}".])])])

have_libgotcha="no"

CPPFLAGS=$_afs_lib_CPPFLAGS
AC_CHECK_HEADER([gotcha/gotcha.h],
    [LTLDFLAGS=$_afs_lib_LDFLAGS
     LTLIBS=$_afs_lib_LIBS
     AFS_LTLINK_LA_IFELSE([_LIBGOTCHA_MAIN], [_LIBGOTCHA_LA],
        [have_libgotcha="yes"
         scorep_gotcha_summary_reason="${_afs_lib_LDFLAGS:+, using $_afs_lib_LDFLAGS}${_afs_lib_CPPFLAGS:+ and $_afs_lib_CPPFLAGS}"],
        [scorep_gotcha_summary_reason=", cannot link against $_afs_lib_LIBS, try --with-libgotcha=download"])],
    [scorep_gotcha_summary_reason=", missing gotcha/gotcha.h header, try --with-libgotcha=download"])
])dnl _LIBGOTCHA_CHECK

AC_DEFUN([SCOREP_GOTCHA], [
AC_REQUIRE([SCOREP_CHECK_LIBGOTCHA])dnl
AFS_SUMMARY([libgotcha found], [${scorep_gotcha_support}${scorep_gotcha_summary_reason}])
])dnl _LIBGOTCHA_CHECK

# _LIBGOTCHA_LA()
# ---------------
# The source code for the libtool archive.
#
m4_define([_LIBGOTCHA_LA], [
AC_LANG_SOURCE([[
#include <gotcha/gotcha.h>
#include <link.h>
static gotcha_wrappee_handle_t wrappee_puts_handle;
static int puts_wrapper(const char* str) {
    typeof(&puts_wrapper) wrappee_puts = gotcha_get_wrappee(wrappee_puts_handle);
    return wrappee_puts(str);
};
static const struct gotcha_binding_t wrap_actions[] = {
    { "puts", puts_wrapper, &wrappee_puts_handle },
};
static int exclude_filter(struct link_map* target)
{
    return 1;
}
char init_wrap ()
{
    gotcha_set_library_filter_func(exclude_filter);
    gotcha_wrap(wrap_actions, sizeof(wrap_actions)/sizeof(struct gotcha_binding_t), "my_tool_name");
}
]])])dnl _LIBGOTCHA_LA


# _LIBGOTCHA_MAIN()
# -----------------
# The source code using the libtool archive.
#
m4_define([_LIBGOTCHA_MAIN], [
AC_LANG_PROGRAM(dnl
[[char init_wrap ();]],
[[init_wrap ();]]
)])dnl _LIBGOTCHA_MAIN
