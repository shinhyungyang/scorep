## -*- mode: autoconf -*-

##
## This file is part of the Score-P software ecosystem (http://www.score-p.org)
##
## Copyright (c) 2009-2013,
## RWTH Aachen University, Germany
##
## Copyright (c) 2009-2013,
## Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##
## Copyright (c) 2009-2013, 2019, 2021,
## Technische Universitaet Dresden, Germany
##
## Copyright (c) 2009-2013,
## University of Oregon, Eugene, USA
##
## Copyright (c) 2009-2013, 2021, 2024-2025,
## Forschungszentrum Juelich GmbH, Germany
##
## Copyright (c) 2009-2013,
## German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##
## Copyright (c) 2009-2013,
## Technische Universitaet Muenchen, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##


dnl $1: name of the component, e.g., otf2, opari2.
dnl $2: requested interface version to be provided by external $1 to build this package.
dnl $3: requested revision to be provided by external $1 to build this package.
dnl Note that the external's current:revision:age is
dnl accepted if current >= $2 && $2 >= (current - age) && ($2 < current || $3 <= revision)
dnl Macro will check for availability of $1-config. Intended to be
dnl called from a toplevel configure. If with_val=yes, search for
dnl $1-config in PATH, else search in with_val and with_val/bin. Sets
dnl output variables scorep_have_$1_config (yes/no) to be used in
dnl conditionals and scorep_$1_config_arg to pe passed to
dnl sub-configures.
AC_DEFUN([AC_SCOREP_HAVE_CONFIG_TOOL], [

AC_ARG_WITH([$1],
    [AS_HELP_STRING([--with-$1[[=<$1-bindir>]]],
        [Use an already installed $1. Provide path to $1-config. Auto-detected if already in $PATH.])],
    [with_$1="${with_$1%/}"], dnl yes, no, or <path>
    [with_$1="not_given"])

AS_UNSET([scorep_have_$1_config])
AS_UNSET([scorep_$1_config_bin])
AS_UNSET([scorep_$1_config_arg])
AS_IF([test "x${with_$1}" != "xno"],
    [AS_IF([test "x${with_$1}" = "xyes" || test "x${with_$1}" = "xnot_given"],
        [AC_CHECK_PROG([scorep_have_$1_config], [$1-config], ["yes"], ["no"])
         AS_IF([test "x${scorep_have_$1_config}" = "xyes"],
             [scorep_$1_config_bin="`which $1-config`"])],
        [# --with-$1=<path>
         AC_CHECK_PROG([scorep_have_$1_config], [$1-config], ["yes"], ["no"], [${with_$1}])
         AS_IF([test "x${scorep_have_$1_config}" = "xyes"],
             [scorep_$1_config_bin="${with_$1}/$1-config"],
             [AS_UNSET([ac_cv_prog_scorep_have_$1_config])
              AS_UNSET([scorep_have_$1_config])
              AC_CHECK_PROG([scorep_have_$1_config], [$1-config], ["yes"], ["no"], ["${with_$1}/bin"])
              AS_IF([test "x${scorep_have_$1_config}" = "xyes"],
                  [scorep_$1_config_bin="${with_$1}/bin/$1-config"])])
        ])
     AS_IF([test "x${scorep_have_$1_config}" = "xyes"],
         [scorep_$1_config_arg="scorep_$1_bindir=`echo "$(cd "$(dirname "${scorep_$1_config_bin}")" && pwd)"`"
          # version checking, see http://www.gnu.org/software/libtool/manual/libtool.html#Versioning
          interface_version=`${scorep_$1_config_bin} --interface-version 2> /dev/null`
          AS_IF([test $? -eq 0 && test "x${interface_version}" != "x"],
              [# get 'current'
               $1_max_provided_interface_version=`echo ${interface_version} | awk -F ":" '{print $[]1}'`
               # get 'revision'
               $1_provided_revision=`echo ${interface_version} | awk -F ":" '{print $[]2}'`
               # get 'age'
               $1_provided_age=`echo ${interface_version} | awk -F ":" '{print $[]3}'`
              AS_IF([test ${interface_version} = "0:0:0"],
                  [# by convention, non-release is 0:0:0
                   AC_MSG_WARN([using non-release external $1, version checks disabled, might produce compile and link errors.])
                   AFS_SUMMARY([$1 support], [yes, using non-release external via ${scorep_$1_config_bin} (version checks disabled, might produce compile and link errors.)])],
                  [# calc 'current - age'
                   AS_VAR_ARITH([$1_min_provided_interface_version], [${$1_max_provided_interface_version} - ${$1_provided_age}])
                   # this is the version check:
                   AS_IF([test ${$1_max_provided_interface_version} -ge $2 && \
                          test $2 -ge ${$1_min_provided_interface_version} && \
                          ( test $2 -lt ${$1_max_provided_interface_version} || test $3 -le ${$1_provided_revision} ) ],
                       [AFS_SUMMARY([$1 support], [yes, using external via ${scorep_$1_config_bin}])],
                       [AC_MSG_ERROR([provided library interface version '${$1_max_provided_interface_version}:${$1_provided_revision}:${$1_provided_age}' of $1 not sufficient for AC_PACKAGE_NAME, provide '$2:$3' or compatible. If in doubt what to do, contact <AC_PACKAGE_BUGREPORT>.])])
                  ])
              ],
              [AC_MSG_ERROR([required option --interface-version not supported by $1-config.])])
         ],
         [# scorep_have_$1_config = no
          AS_IF([test "x${with_$1}" = "xnot_given"],
              [AFS_SUMMARY([$1 support], [yes, using internal])],
              [test "x${with_$1}" = "xyes"],
              [AC_MSG_ERROR([cannot detect $1-config although it was requested via --with-$1.])],
              [AC_MSG_ERROR([cannot detect $1-config in ${with_$1} and ${with_$1}/bin.])
              ])
         ])
    ],
    [# --without-$1
     AS_IF([test ! -d ${srcdir}/vendor/$1],
         [AC_MSG_ERROR([$1 is required. Opting out an external $1 via --without-$1 is only an option if an internal $1 is available, which isn't the case here. Please provide an external $1.])])
     scorep_have_$1_config="no"
     AFS_SUMMARY([$1 support], [yes, using internal])])
]) # AC_SCOREP_HAVE_CONFIG_TOOL


# AFS_CONFIG_TOOL_HAVE_OPTION([component], [cmd-line-option], [action-if-yes], [action-if-no])
# -------------------------------------------------------------------------------------------
# Checks whether "$1-config $2" succeeds. Execute $3 if result is yes,
# $4 if no.
# Requires AC_SCOREP_HAVE_CONFIG_TOOL([$1]) to be executed earlier.
#
AC_DEFUN([AFS_CONFIG_TOOL_HAVE_OPTION], [
m4_ifblank([$1], [m4_fatal([Component must be given])])dnl
m4_ifblank([$2], [m4_fatal([Option must be given])])dnl
dnl
dnl use the transformation as in AC_ARG_WITH, add '=' -> '_'
m4_pushdef([_afs_config_tool_have], $1[_config_have_]m4_translit($2, [-+.=], [____]))dnl
dnl
dnl Check if $scorep_have_$1_config has been previously set by
dnl AC_SCOREP_HAVE_CONFIG_TOOL. Cannot use AC_REQUIRE here as
dnl AC_SCOREP_HAVE_CONFIG_TOOL takes arguments.
AS_IF([test "x$scorep_have_$1_config" = x],
    [AC_MSG_ERROR([[Macro AC@&t@_SCOREP_HAVE_CONFIG_TOOL([$1], ...) to be called prior to AFS_CONFIG_TOOL_HAVE_OPTION([$1], ...)]])])
AS_IF([test "x$scorep_have_$1_config" = xyes],
    [AS_IF([${scorep_$1_config_bin} $2 >/dev/null 2>&1],
         [_afs_config_tool_have=yes],
         [_afs_config_tool_have=no])],
    [# using internal $1-config, assume $2 is available in vendor/$1
     _afs_config_tool_have=yes])
dnl
m4_ifnblank([$3], [dnl
AS_IF([test x"$_afs_config_tool_have" = xyes],
    [$3])
])dnl
m4_ifnblank([$4], [dnl
AS_IF([test x"$_afs_config_tool_have" = xno],
    [$4])
])dnl
dnl
m4_popdef([_afs_config_tool_have])dnl
]) # AFS_CONFIG_TOOL_HAVE_OPTION
