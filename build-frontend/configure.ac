dnl 
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2009-2013,
dnl RWTH Aachen University, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Technische Universitaet Dresden, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl University of Oregon, Eugene, USA
dnl
dnl Copyright (c) 2009-2013,
dnl Forschungszentrum Juelich GmbH, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Technische Universitaet Muenchen, Germany
dnl
dnl This software may be modified and distributed under the terms of
dnl a BSD-style license.  See the COPYING file in the package base
dnl directory for details.
dnl

AC_PREREQ([2.68])
AC_INIT([Score-P], m4_esyscmd([../vendor/common/build-config/generate-package-version.sh ../build-config/VERSION]), [support@score-p.org], [scorep])

AFS_PACKAGE_INIT([frontend])
AFS_SUMMARY_INIT

AC_SUBST([LIBRARY_INTERFACE_VERSION], m4_esyscmd([../vendor/common/build-config/generate-library-version.sh ../build-config/VERSION]))

AC_CONFIG_AUX_DIR([../build-config])
AC_CONFIG_MACRO_DIR([../vendor/common/build-config/m4
                     ../build-config/m4])

AC_CONFIG_HEADERS([../src/config-frontend.h])

AC_DEFUN([AC_SCOREP_FRONTEND], [])

ac_scorep_frontend="yes"
ac_scorep_backend="no"

AC_SCOREP_PRECIOUS_VARS_FOR_BUILD

AC_SCOREP_CHECK_COMPILER_VAR_SET([CC_FOR_BUILD])
AC_SCOREP_CHECK_COMPILER_VAR_SET([CXX_FOR_BUILD])
#AC_SCOREP_CHECK_COMPILER_VAR_SET([F77_FOR_BUILD])
#AC_SCOREP_CHECK_COMPILER_VAR_SET([FC_FOR_BUILD])

AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([CPPFLAGS])
AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([CFLAGS])
AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([CXXFLAGS])
AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([FFLAGS])
AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([FCFLAGS])
AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([LDFLAGS])
AC_SCOREP_CONVERT_FOR_BUILD_FLAGS([LIBS])

AC_SCOREP_CONVERT_FOR_BUILD_COMPILER([CC])
AC_SCOREP_CONVERT_FOR_BUILD_COMPILER([CXX])
AC_SCOREP_CONVERT_FOR_BUILD_COMPILER([F77])
AC_SCOREP_CONVERT_FOR_BUILD_COMPILER([FC])

AC_SCOREP_SVN_CONTROLLED

AM_INIT_AUTOMAKE([foreign color-tests 1.11.1 -Wall tar-pax])
AM_SILENT_RULES([yes])

AM_CONDITIONAL([BUILD_TESTS], [true])
AM_CONDITIONAL([CROSS_BUILD], [true])
AC_SCOREP_PLATFORM_SETTINGS
AC_SCOREP_PLATFORM_AND_MACHINE_NAMES

AC_SCOREP_COMMON_CHECKS
AC_SCOREP_ONLINE_ACCESS
AC_SCOREP_PDT
AC_SCOREP_COBI

AS_IF([test "x${scorep_have_score_build}" = "xno"],
      [AC_SCOREP_CUBE_READER],
      [AM_CONDITIONAL([HAVE_SCORE], [test 1 -eq 0])])

AC_CHECK_FUNCS_ONCE([readlink])

LT_PREREQ([2.4])
LT_INIT([disable-shared])
AC_SUBST([LIBTOOL_DEPS])

AC_SCOREP_BUILD_MODE
AC_SCOREP_PACKAGE_AND_LIBRARY_VERSION

AC_CONFIG_FILES([Makefile], [[
sed -e 's/_0 = @echo "  \(YACC\|GEN\|LEX\)/&  /; t; s/_0 = @echo "  /&FE/' \
Makefile >Makefile.fe && mv -f Makefile.fe Makefile || rm -f Makefile.fe
]])

AC_OUTPUT
