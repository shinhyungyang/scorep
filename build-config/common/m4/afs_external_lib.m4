## -*- mode: autoconf -*-

##
## This file is part of the Score-P software ecosystem (http://www.score-p.org)
##
## Copyright (c) 2021-2022, 2024-2025,
## Forschungszentrum Juelich GmbH, Germany
##
## Copyright (c) 2022, 2024-2025,
## Technische Universitaet Dresden, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##


# AFS_EXTERNAL_LIB( LIBRARY-NAME, CHECK-MACRO, [HEADERS], [DOWNLOAD-MACRO], [HELP-STRING] )
# -----------------------------------------------------------------------------------------
# Provide configure options
#
# --with-libLIBRARY-NAME=(yes|no|[download|]<path>),
# --with-libLIBRARY-NAME-lib=<path>, and, if HEADERS given,
# --with-libLIBRARY-NAME-include=<path>
#
# for library libLIBRARY-NAME (e.g., LIBRARY-NAME=papi for libpapi).
# See the help string below for defaults.
#
# Use either --with-libLIBRARY-NAME or --with-libLIBRARY-NAME-lib as
# well as (if HEADERS given) --with-libLIBRARY-NAME-include.  If
# <path> is given, the former expects subdirectories 'include' (if
# HEADERS given), and '(lib64|lib)' to exist. HEADERS -- white-space
# separated -- (if given) must exist in 'include'. libLIBRARY-NAME.*
# must exist in '(lib64|lib)'.  The latter options expects the
# provided <path> to exist and to contain HEADERS and
# libLIBRARY-NAME.*, respectively.
#
# HEADERS can have boolean operations in it. "or" and "and" can be used
# to express a conditional list of headers required.
#
# Values for --with-libLIBRARY-NAME-lib and
# --with-libLIBRARY-NAME-include can be provided by precious variables
# LIB<upcase(LIBRARY_NAME)>_(LIB|INCLUDE).
#
# If these prerequisites are given, execute CHECK-MACRO. Perform any
# check you like, use (and modify) the AC_SUBST variables
# _afs_lib_LIBS (=-lLIBRARY-NAME), _afs_lib_CPPFLAGS (if HEADERS
# given), and _afs_lib_LDFLAGS. The latter two are either unset or
# point to an existing directory. Note that saving and restoring LIBS,
# CPPFLAGS, and LDFLAGS is taken care of here.
#
# If DOWNLOAD-MACRO is given, accept --with-libLIBRARY-NAME=download
# and, if selected, execute DOWNLOAD-MACRO instead of CHECK-MACRO.
# Consider generating a Makefile and assign it's name to the AC_SUBST
# variable _afs_lib_MAKEFILE and to create a AM_CONDITIONAL (or
# AFS_AM_CONDITIONAL). Trigger the Makefile to download, configure,
# build, and install LIBRARY-NAME into e.g.,
# _afs_lib_PREFIX="${libdir}${backend_suffix}/${PACKAGE}/[]_afs_lib_name[]/${version}".
# _afs_lib_PREFIX is also AC_SUBSTed for use in BUILT_SOURCES. For an example,
# see scorep_libbfd.m4.
#
# Both, CHECK-MACRO and DOWNLOAD-MACRO should honor
# _afs_lib_prevent_check=(yes|no) and, if 'yes'
# _afs_lib_prevent_check_reason=(disabled|crosscompile) and need to set
# _afs_lib_success to a non-"no" value on success. _afs_lib_success survives
# this macro and can also be used as a predicate for success. Its name is
# compiled into AFS_PACKAGE_name_libLIBRARY-NAME_success.
#
# Both, CHECK-MACRO and DOWNLOAD-MACRO are supposed to prepare a summary
# output that could be used in the calling macro.
#
# If HELP-STRING is given (as AS_HELP_STRING), replace the help output
# of --with-libLIBRARY-NAME entirely.
#
AC_DEFUN([AFS_EXTERNAL_LIB],dnl
[AC_REQUIRE([_AFS_LIB_REQUIRE_PATHS])dnl
m4_ifblank([$1], [m4_fatal([LIBRARY-NAME must be given])])dnl
m4_ifblank([$2], [m4_fatal([CHECK-MACRO must be given])])dnl
m4_ifnblank([$4], [AC_REQUIRE([_AFS_LIB_DOWNLOAD_PREREQ])])dnl
m4_pushdef([_afs_lib], $1)dnl
m4_pushdef([_afs_lib_name], [lib]$1)dnl
dnl use the same transformation as AC_ARG_WITH
m4_pushdef([_afs_lib_NAME], [m4_toupper(m4_translit(_afs_lib_name, [-+.], [___]))])dnl
m4_pushdef([_afs_lib_withval], [with_]m4_translit(_afs_lib_name, [-+.], [___]))dnl
m4_pushdef([_afs_lib_withval_lib], [with_]m4_translit(_afs_lib_name, [-+.], [___])_lib)dnl
m4_ifnblank([$3], [m4_pushdef([_afs_lib_withval_include], [with_]m4_translit(_afs_lib_name, [-+.], [___])_include)])dnl
dnl
m4_pushdef([_afs_lib_success], AFS_PACKAGE_name[]_[]m4_translit(_afs_lib_name, [-+.], [___])[]_success)dnl
m4_pushdef([_afs_lib_LDFLAGS], AFS_PACKAGE_NAME[]_[]_afs_lib_NAME[]_LDFLAGS)dnl
m4_ifnblank([$3], [m4_pushdef([_afs_lib_CPPFLAGS], AFS_PACKAGE_NAME[]_[]_afs_lib_NAME[]_CPPFLAGS)])dnl
m4_pushdef([_afs_lib_LIBS], AFS_PACKAGE_NAME[]_[]_afs_lib_NAME[]_LIBS)dnl
AC_SUBST(_afs_lib_LDFLAGS)dnl
m4_ifnblank([$3], [AC_SUBST(_afs_lib_CPPFLAGS)])dnl
AC_SUBST(_afs_lib_LIBS)dnl
m4_ifnblank([$4], dnl
m4_pushdef(_afs_lib_MAKEFILE, AFS_PACKAGE_NAME[]_[]_afs_lib_NAME[]_MAKEFILE)dnl
m4_pushdef([_afs_lib_PREFIX], AFS_PACKAGE_NAME[]_[]_afs_lib_NAME[]_PREFIX))dnl
m4_ifnblank([$4], [dnl Can't be put into ifnblank above
AC_SUBST(_afs_lib_MAKEFILE)dnl
AC_SUBST(_afs_lib_PREFIX)])dnl
dnl
dnl withval variables of the AC_ARG_WITH macros below are
dnl ${_afs_lib_withval}"
dnl ${_afs_lib_withval_lib}"
dnl ${_afs_lib_withval_include}"
dnl
_afs_lib_prevent_check=no
m4_ifnblank([$4], [_afs_lib_download=no])
AS_UNSET([_afs_lib_prevent_check_reason])
AC_ARG_WITH(_afs_lib_name,
    [m4_ifblank([$5],
         AS_HELP_STRING([--with-]_afs_lib_name[@<:@=yes|no|m4_ifnblank([$4],[download|])<path to ]_afs_lib_name[ installation>@:>@],
              [Try to use ]_afs_lib_name[. Defaults to [yes] on
               non-cross-compile systems and expects library and
               headers to be found in system locations. Defaults to
               [no] on cross-compile systems. Provide
               ]_afs_lib_name['s installation prefix [path] to
               override the defaults.
               --with-]_afs_lib_name[=<path> is a shorthand for]
               m4_ifnblank([$3], --with-]_afs_lib_name[-include=<path/include> and)
               [--with-]_afs_lib_name[-lib=<path/(lib64|lib)>. If this is not the
               case, use the explicit option]m4_ifnblank([$3], [s])
               [ directly or provide path]m4_ifnblank([$3], [s])
               [ via ]_afs_lib_NAME[_LIB]
               m4_ifnblank([$3], [ and ]_afs_lib_NAME[_INCLUDE])[.]
               m4_ifnblank([$4], [Use [[download]] to automatically obtain
               and use ]_afs_lib_name[ via external tarball. See
               --with-package-cache=<path> how to provide the tarball
               for an offline installation.])),
         [$5])])
m4_ifnblank([$3],
    [AC_ARG_WITH(_afs_lib_name[-include],
        AS_HELP_STRING([--with-_afs_lib_name-include=<Path to _afs_lib_name headers: $3>], [], [79]))])dnl
AC_ARG_WITH(_afs_lib_name[-lib],
     AS_HELP_STRING([--with-_afs_lib_name-lib=<Path to _afs_lib_name libraries>], [], [79]))
dnl
m4_ifnblank([$3],
    [AC_ARG_VAR(_afs_lib_NAME[]_INCLUDE, [Path to ]_afs_lib_name[ headers: $3. Superseded by --with-]_afs_lib_name[ variants.])])dnl
AC_ARG_VAR(_afs_lib_NAME[]_LIB, [Path to ]_afs_lib_name[ libraries. Superseded by --with-]_afs_lib_name[ variants.])dnl
m4_ifnblank([$3], [AC_ARG_VAR(_afs_lib_NAME[]_EXTRA_CPPFLAGS, [Extra C preprocessor flags required to use ]_afs_lib_name[.])])dnl
AC_ARG_VAR(_afs_lib_NAME[]_EXTRA_LIBS, [Extra libraries required to use ]_afs_lib_name[.])dnl
AC_ARG_VAR(_afs_lib_NAME[]_EXTRA_LDFLAGS, [Extra link flags required to use ]_afs_lib_name[.])dnl
dnl
# Supersede environment variables if --with-libLIBRARY-NAME option
# given. Unset env variables to prevent further processing.
AS_IF([test "${_afs_lib_withval:+set}" = set],
    [AS_IF([test "${_afs_lib_NAME[]_LIB:+set}" = set],
         [AC_MSG_WARN([_afs_lib_NAME[]_LIB=${_afs_lib_NAME[]_LIB} superseded with --with-_afs_lib_name=${_afs_lib_withval}])
          AS_UNSET([_afs_lib_NAME[]_LIB])])
     m4_ifnblank([$3], [dnl
     AS_IF([test "${_afs_lib_NAME[]_INCLUDE:+set}" = set],
         [AC_MSG_WARN([_afs_lib_NAME[]_INLUDE=${_afs_lib_NAME[]_INCLUDE} superseded with --with-_afs_lib_name=${_afs_lib_withval}])
          AS_UNSET([_afs_lib_NAME[]_INCLUDE])])])])
# Convert LIB<upcase(LIBRARY_NAME)>_(LIB|INCLUDE) env var to
# --with-libLIBRARY-NAME-(lib|include) if the latter wasn't
# provided. Otherwise, --with-libLIBRARY-NAME-(lib|include)
# supersedes.
AS_IF([test "${_afs_lib_NAME[]_LIB:+set}" = set],
    [AS_IF([test "${_afs_lib_withval_lib:+set}" = set],
         [AC_MSG_WARN([_afs_lib_NAME[]_LIB=${_afs_lib_NAME[]_LIB} superseded with --with-_afs_lib_name-lib=${_afs_lib_withval_lib}])],
         [_afs_lib_withval_lib="${_afs_lib_NAME[]_LIB}"
          AC_MSG_NOTICE([Using _afs_lib_NAME[]_LIB as --with-_afs_lib_name-lib=${_afs_lib_withval_lib}. Further mentioning of --with-_afs_lib_name-lib applies to _afs_lib_NAME[]_LIB.])])])
m4_ifnblank([$3], [dnl
AS_IF([test "${_afs_lib_NAME[]_INCLUDE:+set}" = set],
    [AS_IF([test "${_afs_lib_withval_include:+set}" = set],
         [AC_MSG_WARN([_afs_lib_NAME[]_INCLUDE=${_afs_lib_NAME[]_INCLUDE} superseded with --with-_afs_lib_name-include=${_afs_lib_withval_include}])],
         [_afs_lib_withval_include="${_afs_lib_NAME[]_INCLUDE}"
          AC_MSG_NOTICE([Using _afs_lib_NAME[]_INCLUDE as --with-_afs_lib_name-include=${_afs_lib_withval_include}. Further mentioning of --with-_afs_lib_name-include applies to _afs_lib_NAME[]_INCLUDE.])])])])
dnl
# Check valid combinations of options and if directories and libs exist if <path> was provided.
AS_CASE(["${_afs_lib_withval:+set1}${_afs_lib_withval_lib:+set2}m4_ifnblank([$3], [${_afs_lib_withval_include:+set3}])"],
    m4_ifnblank([$3], [dnl
    [set1set*],
        [AC_MSG_ERROR([Use either shorthand --with-_afs_lib_name or explicit options --with-_afs_lib_name-lib and --with-_afs_lib_name-include.])],
    [set2],
        [AC_MSG_ERROR([If --with-_afs_lib_name-lib is given, --with-_afs_lib_name-include is required.])],
    [set3],
        [AC_MSG_ERROR([If --with-_afs_lib_name-include is given, --with-_afs_lib_name-lib is required.])]], [dnl $3 blank
    [set1set2],
        [AC_MSG_ERROR([Use either shorthand --with-_afs_lib_name or explicit option --with-_afs_lib_name-lib.])],
    [set2],
        [AS_CASE([${_afs_lib_withval_lib}],
             [yes|no], [AC_MSG_ERROR([--with-_afs_lib_name-lib requires a <path>.])],
             [_LIB_CONSISTENCY_CHECKS])]]),
    [set1],
        [AS_CASE([$_afs_lib_withval],
             [yes], [AS_IF([test "x${afs_lib_require_install_paths}" = xyes],
                         [_afs_lib_withval=no; _afs_lib_withval_lib=no[]m4_ifnblank([$3], [; _afs_lib_withval_include=no])
                          _afs_lib_prevent_check=yes
                          _afs_lib_prevent_check_reason="crosscompile"
                          AC_MSG_WARN([In cross-compile mode, you need to provide a path to --with-_afs_lib_name (or --with-_afs_lib_name-lib[]m4_ifnblank([$3], [ and --with-_afs_lib_name-lib-include])) in order to activate $1 support.[]m4_ifnblank([$4], [ Alternatively, use --with-_afs_lib_name[]=download.])])],
                         [_afs_lib_withval_lib=yes[]m4_ifnblank([$3], [; _afs_lib_withval_include=yes])])],
             [no], [_afs_lib_withval_lib=no[]m4_ifnblank([$3], [; _afs_lib_withval_include=no])
                    _afs_lib_prevent_check=yes
                    _afs_lib_prevent_check_reason="disabled"],
             m4_ifnblank([$4], [[download], [_afs_lib_withval_lib=download; m4_ifnblank([$3], [_afs_lib_withval_include=download; ])_afs_lib_download=yes],])
             [AFS_CONSISTENT_DIR([_afs_lib_withval], [--with-_afs_lib_name])
              AS_IF([! test -d "$_afs_lib_withval"],
                  [AC_MSG_ERROR([Directory $_afs_lib_withval (from --with-_afs_lib_name) does not exist. Typo?])])
              m4_ifnblank([$3], [dnl
              dnl header consistency checks, differ from the checks below
              AS_IF([! test -d "$_afs_lib_withval/include"],
                  [AC_MSG_ERROR([Directory $_afs_lib_withval/include (via --with-_afs_lib_name) does not exist. Consider using --with-_afs_lib_name-lib and --with-_afs_lib_name-include.])])
              _HEADER_EXISTS_CHECK([$3], [$_afs_lib_withval/include], [header_consistent])
              AS_IF([test ${header_consistent} -eq 0],
                  [AC_MSG_ERROR([Header(s) check failed, see WARNING(s) above.])])
              _afs_lib_withval_include="$_afs_lib_withval/include"])
              dnl lib consistency checks, differ from LIB_CONSISTENCY_CHECKS
              AS_IF([test -d "${_afs_lib_withval}/lib64"],
                  [AS_IF([test "$(echo $_afs_lib_withval/lib64/_afs_lib_name.*)" != "$_afs_lib_withval/lib64/_afs_lib_name.*"],
                      [_afs_lib_withval_lib="${_afs_lib_withval}/lib64"])],
                  [test -d "${_afs_lib_withval}/lib"],
                  [AS_IF([test "$(echo $_afs_lib_withval/lib/_afs_lib_name.*)" != "$_afs_lib_withval/lib/_afs_lib_name.*"],
                      [_afs_lib_withval_lib="${_afs_lib_withval}/lib"])],
                  [AC_MSG_ERROR([Neither ${_afs_lib_withval}/(lib64|lib) (via --with-_afs_lib_name) exist. Consider using --with-_afs_lib_name-lib and --with-_afs_lib_name-include.])])
              AS_IF([test "${_afs_lib_withval_lib:+set}" != set],
                  [AC_MSG_ERROR([Neither ${_afs_lib_withval}/(lib64|lib) contain _afs_lib_name.*. Consider using --with-_afs_lib_name-lib and --with-_afs_lib_name-include.])])])],
    m4_ifnblank([$3], [dnl
    [set2set3],
        [AS_CASE([${_afs_lib_withval_lib},${_afs_lib_withval_include}],
             [yes,*|no,*|*,yes|*,no], [AC_MSG_ERROR([Both, --with-_afs_lib_name-lib and --with-_afs_lib_name-include require a <path>.])],
             [dnl header consistency checks, differ from the checks above
              AFS_CONSISTENT_DIR([_afs_lib_withval_include], [--with-_afs_lib_name-include])
              AS_IF([! test -d "$_afs_lib_withval_include"],
                  [AC_MSG_ERROR([Directory $_afs_lib_withval_include (from --with-_afs_lib_name-include) does not exist.])])
              _HEADER_EXISTS_CHECK([$3], [$_afs_lib_withval_include], [header_consistent])
              AS_IF([test ${header_consistent} -eq 0],
                  [AC_MSG_ERROR([Header(s) check failed, see WARNING(s) above.])])
              _LIB_CONSISTENCY_CHECKS])],])
    [""],
        [AS_IF([test "x${afs_lib_require_install_paths}" = xyes],
            [_afs_lib_withval=no; _afs_lib_withval_lib=no[]m4_ifnblank([$3], [; _afs_lib_withval_include=no])
             _afs_lib_prevent_check=yes
             _afs_lib_prevent_check_reason="crosscompile"
             AC_MSG_WARN([In cross-compile mode, you need to provide a path to --with-_afs_lib_name (or --with-_afs_lib_name-lib[]m4_ifnblank([$3], [ and --with-_afs_lib_name-lib-include])) in order to activate $1 support.[]m4_ifnblank([$4], [ Alternatively, use --with-_afs_lib_name[]=download.])])],
            [_afs_lib_withval=yes; _afs_lib_withval_lib=yes[]m4_ifnblank([$3], [; _afs_lib_withval_include=yes])])]
)
dnl
AS_IF([test "${_afs_lib_withval_lib:+set}" != set],
    [AC_MSG_ERROR([internal: _afs_lib_withval_lib not set])])
m4_ifnblank([$3], [dnl
AS_IF([test "${_afs_lib_withval_include:+set}" != set],
    [AC_MSG_ERROR([internal: _afs_lib_withval_include not set])])])
dnl
dnl Execute either CHECK-MACRO or DOWNLOAD-MACRO
_afs_lib_success=no
_afs_lib_LIBS="-l[]_afs_lib ${_afs_lib_NAME[]_EXTRA_LIBS}"
m4_ifnblank([$4], [
dnl CF: Cannot get AS_IF to work here
if test "x${_afs_lib_download}" = xyes; then :
    dnl
    dnl DOWNLOAD-MACRO needs to set the AC_SUBST variables
    dnl _afs_lib_LDFLAGS, [_afs_lib_CPPFLAGS,] _afs_lib_MAKEFILE, and
    dnl _afs_lib_PREFIX. afs_lib_LIBS is already set. DOWNLOAD-MACRO
    dnl is supposed to prepare summary output. On success it needs to set
    dnl _afs_lib_success to a non-"no" value.
    dnl
    #--- Delegated download for lib$1: begin
    $4
    #--- Delegated download for lib$1: end
    AS_UNSET([_afs_lib_download])
else])
    m4_ifnblank([$3], [AS_UNSET([_afs_lib_CPPFLAGS])])
    AS_UNSET([_afs_lib_LDFLAGS])
    AS_IF([test "x${_afs_lib_prevent_check}" = xno],
        [AS_IF([test "x${_afs_lib_withval_lib}" != xyes],
             [dnl Search _afs_lib_withval_lib for LIBRARY-NAME (-Ldir).
              dnl Add _afs_lib_withval_lib to the run-time path of a program that
              dnl links LIBRARY-NAME either directly or via a libtool library (-Rdir).
              _afs_lib_LDFLAGS="-L${_afs_lib_withval_lib} -R${_afs_lib_withval_lib} ${_afs_lib_NAME[]_EXTRA_LDFLAGS}"])
         m4_ifnblank([$3], [AS_IF([test "x${_afs_lib_withval_include}" != xyes],
             [_afs_lib_CPPFLAGS="-I${_afs_lib_withval_include} ${_afs_lib_NAME[]_EXTRA_CPPFLAGS}"])])])
    dnl
    dnl CHECK-MACRO can use and modify the AC_SUBST variables
    dnl _afs_lib_LIBS, _afs_lib_LDFLAGS, [_afs_lib_CPPFLAGS,] and is
    dnl supposed to honor _afs_lib_prevent_check. No need to use
    dnl _afs_lib_withval*. CHECK-MACRO is supposed to prepare summary
    dnl output. For summary potentially make use of
    dnl _afs_lib_prevent_check_reason. On success it needs to set
    dnl _afs_lib_success to a non-"no" value.
    dnl
    m4_ifnblank([$3], [_afs_lib_cppflags_save="${CPPFLAGS}"])
    _afs_lib_ldflags_save="${LDFLAGS}"
    _afs_lib_libs_save="${LIBS}"
    #--- Delegated check for lib$1: begin
    $2
    #--- Delegated check for lib$1: end
    m4_ifnblank([$3], [CPPFLAGS="${_afs_lib_cppflags_save}"])
    LDFLAGS="${_afs_lib_ldflags_save}"
    LIBS="${_afs_lib_libs_save}"
m4_ifnblank([$4], [fi])
dnl
dnl Unset AC_SUBST variables in case of unsuccess
AS_IF([test x"${_afs_lib_success}" = x"no"],
    [AS_UNSET([_afs_lib_LIBS])
     AS_UNSET([_afs_lib_LDFLAGS])
     m4_ifnblank([$3], [AS_UNSET([_afs_lib_CPPFLAGS])])
     m4_ifnblank([$4], [AS_UNSET([_afs_lib_MAKEFILE])])
     m4_ifnblank([$4], [AS_UNSET([_afs_lib_PREFIX])])])
dnl
AS_UNSET([_afs_lib_prevent_check])
AS_UNSET([_afs_lib_prevent_check_reason])
dnl
m4_popdef([_afs_lib])dnl
m4_popdef([_afs_lib_name])dnl
m4_popdef([_afs_lib_NAME])dnl
m4_popdef([_afs_lib_withval])dnl
m4_popdef([_afs_lib_withval_lib])dnl
m4_ifnblank([$3], [m4_popdef([_afs_lib_withval_include])])dnl
dnl
m4_popdef([_afs_lib_success])dnl
m4_popdef([_afs_lib_LDFLAGS])dnl
m4_ifnblank([$3], [m4_popdef([_afs_lib_CPPFLAGS])])dnl
m4_popdef([_afs_lib_LIBS])dnl
m4_ifnblank([$4],
m4_popdef([_afs_lib_MAKEFILE])dnl
m4_popdef([_afs_lib_PREFIX])dnl
)
])# AFS_EXTERNAL_LIB


# _LIB_CONSISTENCY_CHECKS()
# -------------------------
#
m4_define([_LIB_CONSISTENCY_CHECKS], [dnl
dnl lib consistency checks
AFS_CONSISTENT_DIR([_afs_lib_withval_lib], [--with-_afs_lib_name-lib])
AS_IF([! test -d "$_afs_lib_withval_lib"],
    [AC_MSG_ERROR([Directory $_afs_lib_withval_lib (from --with-_afs_lib_name-lib) does not exist.])])
AS_IF([test "$(echo $_afs_lib_withval_lib/_afs_lib_name.*)" = "$_afs_lib_withval_lib/_afs_lib_name.*"],
    [AC_MSG_ERROR([Directory $_afs_lib_withval_lib (from --with-_afs_lib_name-lib) does not contain _afs_lib_name.*.])])dnl
])# _LIB_CONSISTENCY_CHECKS


# _AFS_LIB_REQUIRE_PATHS
# ----------------------
# Prevent picking up OS-provided libs on cross-compile-systems
#
AC_DEFUN_ONCE([_AFS_LIB_REQUIRE_PATHS], [
m4_ifdef([AFS_COMPUTENODE], [
AS_IF([test "x${ac_scorep_cross_compiling}" = xyes],
    [afs_lib_require_install_paths=yes],
    [afs_lib_require_install_paths=no])],
[afs_lib_require_install_paths=no])
])# _AFS_LIB_REQUIRE_PATHS


# _AFS_LIB_DOWNLOAD_PREREQ
# ------------------------
# Use either wget or curl via $(AFS_LIB_DOWNLOAD_CMD) for downloading
# packages in Makefile.lib<foo>. In Makefile.lib<foo>, define 'THIS_FILE'
# pointing to the generate file to provide a helpful diagnostic message.
#
# Source a set of variables like url and version needed for
# downloading packages.
#
AC_DEFUN_ONCE([_AFS_LIB_DOWNLOAD_PREREQ], [
# search for either wget or curl
AC_CHECK_PROG([AFS_LIB_DOWNLOAD_CMD], [wget], [$(which wget) \$(BUILD_V_QUIET) --content-disposition --continue], [no])
AS_IF([test "x${AFS_LIB_DOWNLOAD_CMD}" = xno],
    [AS_UNSET([AFS_LIB_DOWNLOAD_CMD])
     AS_UNSET([ac_cv_prog_AFS_LIB_DOWNLOAD_CMD])
     AC_CHECK_PROG([AFS_LIB_DOWNLOAD_CMD], [curl], [$(which curl) \$(BUILD_V_SILENT) --show-error --remote-name --remote-header-name --location], [no])
     AS_IF([test "x${AFS_LIB_DOWNLOAD_CMD}" = xno],
         [AC_MSG_WARN([Neither wget nor curl found.])
          AFS_LIB_DOWNLOAD_CMD="echo \"Neither wget nor curl found. Cannot download package. See \$(THIS_FILE).\" && exit 1 && "])])
AC_SUBST([AFS_LIB_DOWNLOAD_CMD])
AC_CHECK_PROG([AFS_LIB_PATCH_CMD], [patch], [$(which patch)], [no])
AS_IF([test "x${AFS_LIB_PATCH_CMD}" = xno],
      [AC_MSG_WARN([No patch found.])
       AFS_LIB_PATCH_CMD="echo \"No patch found. Cannot patch package. See \$(THIS_FILE).\" && exit 1 && "])
AC_SUBST([AFS_LIB_PATCH_CMD])
# source meta-data for downloadable packages
downloads=$afs_srcdir/build-config/downloads
AS_IF([test -r $downloads],
    [. $downloads],
    [AC_MSG_WARN([File $downloads not readable or does not exist.])])
AS_UNSET([downloads])
AC_SUBST([AFS_LIB_PACKAGE_CACHE], ['$(srcdir)/build-config/packages'])
AC_ARG_WITH([package-cache],
    [AS_HELP_STRING([--with-package-cache=<path>],
         [Path where to provide packages for '--with-lib<foo>=download'
          options to prevent downloads during the build process.
          '<path>' defaults to 'build-config/packages' in the source
          directory. The URLs to the required packages are stated in
          directory. The URLs to the required packages can be listed
          via build-config/packages.sh.])],
    [AS_CASE([$withval],
         [/*], [AFS_LIB_PACKAGE_CACHE=$withval],
         [AFS_LIB_PACKAGE_CACHE=$(cd "$withval" && pwd)])])
])# _AFS_LIB_DOWNLOAD_CMD

# _HEADER_EXISTS_CHECK( HEADERS, DIRECTORY, RESULT-VAR )
# ------------------------------------------------------
#
# Check the existence of HEADERS in DIRECTORY. Honoring boolean operators "and"
# and "or". Defaults to "and" or last operation in list. Short circuit at failed
# "and".
#
# HEADERS:    List of whitespace separated files, can have boolean ops "or" and
#             "and" in it
# DIRECTORY:  User provided dirname, files get appended
# RESULT-VAR: Variable indicating the success of the tests. Will be set to "1"
#             or "0"
m4_define([_HEADER_EXISTS_CHECK], [
_afs_lib_headers_op=
$3=
AS_FOR([_afs_lib_HEADER],
    [_afs_lib_header],
    [$1],
    [AS_CASE([_afs_lib_HEADER],
        [and], [_afs_lib_headers_op='&&'],
        [or],  [_afs_lib_headers_op='||'],
        [AS_IF([test -f $2/_afs_lib_HEADER],
            [_afs_lib_headers_res=1],
            [AC_MSG_WARN([Directory $2 does not contain _afs_lib_HEADER.])
            _afs_lib_headers_res=0])
        AS_IF([test -z "$_afs_lib_headers_op"],
            [$3=$_afs_lib_headers_res
            _afs_lib_headers_op='&&'],
            [AS_VAR_ARITH([$3], ["$3 $_afs_lib_headers_op _afs_lib_headers_res"])
             AS_IF([test x"${_afs_lib_headers_op}" = x'&&' && test ${$3} -eq 0],
                [break])])])])
])# _HEADER_EXISTS_CHECK
