dnl -*- mode: autoconf -*-

dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2012-2014,
dnl Technische Universitaet Dresden, Germany
dnl
dnl This software may be modified and distributed under the terms of
dnl a BSD-style license.  See the COPYING file in the package base
dnl directory for details.
dnl

AC_DEFUN([_SCOREP_GCC_PLUGIN_CHECK], [
AC_REQUIRE([LT_OUTPUT])

AC_LANG_PUSH($1)

scorep_gcc_plugin_cppflags=`[$]_AC_CC -print-file-name=plugin`/include

save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS -I${scorep_gcc_plugin_cppflags} -I$srcdir/../src/adapters/compiler/gcc-plugin/fake-gmp"

AC_CHECK_HEADERS([gcc-plugin.h],
    [AC_CHECK_HEADERS([tree.h],
        [scorep_gcc_have_plugin_headers=yes],
        [scorep_gcc_have_plugin_headers=no],
        [[
#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include <gcc-plugin.h>
]])],
    [scorep_gcc_have_plugin_headers=no],
    [[
/* no default includes */
#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
]])

AC_MSG_CHECKING([for GCC $1 plug-in headers])
AC_MSG_RESULT([${scorep_gcc_have_plugin_headers}])

scorep_gcc_have_working_plugin=no
AS_IF([test "x${scorep_gcc_have_plugin_headers}" = "xyes"], [

    # minimalistic GCC plug-in
cat confdefs.h - >conftest.$ac_ext <<\_EOF
/* end confdefs.h.  */
#undef PACKAGE_NAME
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_STRING
#undef PACKAGE_BUGREPORT
#undef PACKAGE_URL
#include "gcc-plugin.h"
#include "tree.h"

int plugin_is_GPL_compatible = 1;

static void
body( void )
{
    ( void )build_fn_decl( "", NULL_TREE );
}

int
plugin_init( struct plugin_name_args*   plugin_info,
             struct plugin_gcc_version* version )
{
    body();
    return 0;
}
_EOF

    # build plug-in with libtool to get an shared object
    # -rpath is needed, else libool will only build an convenient library
    AC_MSG_CHECKING([to build a $1 plug-in])
    plugin_compile='$SHELL ./libtool --mode=compile --tag=_AC_CC [$]_AC_CC $CPPFLAGS [$]_AC_LANG_PREFIX[FLAGS] -c -o conftest.lo conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
    plugin_link='$SHELL ./libtool --mode=link --tag=_AC_CC [$]_AC_CC [$]_AC_LANG_PREFIX[FLAGS] -module $LDFLAGS -rpath $PWD/lib -o confmodule.la conftest.lo >&AS_MESSAGE_LOG_FD'
    plugin_mkdir='$MKDIR_P lib >&AS_MESSAGE_LOG_FD'
    plugin_install='$SHELL ./libtool --mode=install $INSTALL confmodule.la $PWD/lib/confmodule.la >&AS_MESSAGE_LOG_FD'
    plugin_clean='$SHELL ./libtool --mode=clean $RM conftest.lo confmodule.la >&AS_MESSAGE_LOG_FD'
    AS_IF([_AC_DO_VAR([plugin_compile]) &&
        _AC_DO_VAR([plugin_link]) &&
        _AC_DO_VAR([plugin_mkdir]) &&
        _AC_DO_VAR([plugin_install]) &&
        _AC_DO_VAR([plugin_clean])],
    [
        AC_MSG_RESULT([yes])

        # now try to use this plug-in in an compile test
        [save_]_AC_LANG_PREFIX[FLAGS]=[$]_AC_LANG_PREFIX[FLAGS]
        _AC_LANG_PREFIX[FLAGS]="[$save_]_AC_LANG_PREFIX[FLAGS] -fplugin=$PWD/lib/confmodule.so"
        AC_MSG_CHECKING([to load a $1 plug-in])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
            [scorep_gcc_have_working_plugin=yes
            AC_MSG_RESULT([yes])
            ],
            [scorep_gcc_plugin_support_reason="no, failed to load plug-in"
            AC_MSG_RESULT([no])
            ])
        _AC_LANG_PREFIX[FLAGS]=[$save_]_AC_LANG_PREFIX[FLAGS]

        plugin_uninstall='$SHELL ./libtool --mode=uninstall $RM $PWD/lib/confmodule.la >&AS_MESSAGE_LOG_FD'
        _AC_DO_VAR([plugin_uninstall])
        plugin_rmdir='rmdir lib >&AS_MESSAGE_LOG_FD'
        _AC_DO_VAR([plugin_rmdir])
    ], [
        AC_MSG_RESULT([no])
    ])

    AS_UNSET([plugin_compile])
    AS_UNSET([plugin_link])
    AS_UNSET([plugin_mkdir])
    AS_UNSET([plugin_install])
    AS_UNSET([plugin_clean])
    AS_UNSET([plugin_uninstall])
    AS_UNSET([plugin_rmdir])

    $RM conftest.$ac_ext
], [
    scorep_gcc_plugin_support_reason="no, missing plug-in headers, please install"
])

CPPFLAGS=$save_CPPFLAGS

AS_UNSET([save_CPPFLAGS])

AC_LANG_POP($1)

AC_MSG_CHECKING([for working GCC $1 plug-in support])
AS_IF([test "x${scorep_gcc_have_working_plugin}" = "xyes"],
    [AC_MSG_RESULT([yes])
    scorep_gcc_plugin_support_reason="yes, using the $1 compiler and -I${scorep_gcc_plugin_cppflags}"
    $2
    :],
    [AS_UNSET([scorep_gcc_plugin_cppflags])
    AC_MSG_RESULT([no])
    $3
    :])

AS_UNSET([scorep_gcc_have_plugin_headers])
AS_UNSET([scorep_gcc_have_working_plugin])
])

# SCOREP_GCC_VERSION
# ------------------
# Provides the GCC version as number via AC_SUBST([SCOREP_GCC_VERSION_NUMBER]).
# version = major * 1000 + minor
# equals 0 for non-gcc compilers
AC_DEFUN([SCOREP_GCC_VERSION], [
AC_REQUIRE([AX_GCC_VERSION])

scorep_gcc_version=0
AS_IF([test "x${GCC_VERSION}" != "x"],
      [scorep_gcc_version_major=${GCC_VERSION%%.*}
       scorep_gcc_version_minor=${GCC_VERSION#*.}
       scorep_gcc_version_minor=${scorep_gcc_version_minor%%.*}
       scorep_gcc_version=`expr ${scorep_gcc_version_major} "*" 1000 + ${scorep_gcc_version_minor}`])

AC_SUBST([SCOREP_GCC_VERSION_NUMBER], [${scorep_gcc_version}])
])

# SCOREP_GCC_PLUGIN
# -----------------
# Performs checks whether the GCC compiler has plug-in support, and with which
# compiler it was built.
AC_DEFUN([SCOREP_GCC_PLUGIN], [
AC_REQUIRE([SCOREP_GCC_VERSION])

rm -f gcc_plugin_supported

AS_IF([test ${scorep_gcc_version} -lt 4005],
    [scorep_gcc_plugin_support_reason="no, GCC ${GCC_VERSION} is too old, no plug-in support"],
    [_SCOREP_GCC_PLUGIN_CHECK([C],
        [AC_SUBST([GCC_PLUGIN_CPPFLAGS], ["-I${scorep_gcc_plugin_cppflags} -I$srcdir/../src/adapters/compiler/gcc-plugin/fake-gmp"])
         AFS_AM_CONDITIONAL([GCC_COMPILED_WITH_CXX], [false], [false])
         touch gcc_plugin_supported],
        [AS_UNSET([ac_cv_header_gcc_plugin_h])
         AS_UNSET([ac_cv_header_tree_h])
         _SCOREP_GCC_PLUGIN_CHECK([C++],
            [AC_SUBST([GCC_PLUGIN_CPPFLAGS], ["-I${scorep_gcc_plugin_cppflags} -I$srcdir/../src/adapters/compiler/gcc-plugin/fake-gmp"])
             AFS_AM_CONDITIONAL([GCC_COMPILED_WITH_CXX], [true], [false])
             touch gcc_plugin_supported])])])

AFS_AM_CONDITIONAL([GCC_VERSION_GE_49], [test ${scorep_gcc_version} -ge 4009], [false])
AM_COND_IF([GCC_VERSION_GE_49],
    [AC_SUBST([GCC_PLUGIN_CXXFLAGS], ["-fno-rtti"])])

AFS_SUMMARY([GCC plug-in support], [${scorep_gcc_plugin_support_reason}])
AFS_AM_CONDITIONAL([HAVE_GCC_PLUGIN_SUPPORT], [test -f gcc_plugin_supported], [false])

AS_UNSET([scorep_gcc_plugin_cppflags])
AS_UNSET([scorep_gcc_plugin_support_reason])
])
