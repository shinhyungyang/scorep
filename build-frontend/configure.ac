dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2009-2013,
dnl RWTH Aachen University, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
dnl
dnl Copyright (c) 2009-2013, 2017-2019, 2021-2022,
dnl Technische Universitaet Dresden, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl University of Oregon, Eugene, USA
dnl
dnl Copyright (c) 2009-2015, 2017-2018, 2021-2023, 2025,
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

AC_PREREQ([2.69])
AC_INIT([Score-P], m4_esyscmd([../build-config/common/generate-package-version.sh ../build-config/VERSION]), [support@score-p.org], [scorep])

AFS_PACKAGE_BUILD_INIT([frontend])
AFS_SUMMARY_INIT

AC_SUBST([LIBRARY_INTERFACE_VERSION], m4_esyscmd([../build-config/common/generate-library-version.sh ../build-config/VERSION]))

AC_CONFIG_AUX_DIR([../build-config])
## See also ACLOCAL_AMFLAGS in Makefile.am.
AC_CONFIG_MACRO_DIRS([../build-config/m4 ../build-config/common/m4])

AC_DEFUN([AC_SCOREP_FRONTEND], [])

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

AM_INIT_AUTOMAKE([foreign color-tests 1.13.4 -Wall tar-pax])
AM_SILENT_RULES([yes])
AM_PROG_AR
AFS_MAINTAINER_MODE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AFS_PROG_CC([99])
AFS_PROG_CC_SUMMARY
AFS_PROG_CXX([11], [noext])
AFS_PROG_CXX_SUMMARY

LT_PREREQ([2.4.6])
# Use libtool defaults such that AFS_LT_RESTRICTIONS succeeds for valid
# configure options. This allows to use the `bin-programs-self-contained`
# Makefile target.
LT_INIT([shared disable-static pic-only])
AC_SUBST([LIBTOOL_DEPS])
AFS_LT_RESTRICTIONS

AC_CONFIG_HEADERS([../src/config-frontend.h])


ac_scorep_frontend="yes"
ac_scorep_backend="no"

m4_include(../build-includes/loginnode.m4)

adl_RECURSIVE_EVAL([${bindir}], [BINDIR])
AC_SUBST([BINDIR])
adl_RECURSIVE_EVAL([${datadir}/${PACKAGE}], [PKGDATADIR])
AC_SUBST([PKGDATADIR])
adl_RECURSIVE_EVAL([${libexecdir}/${PACKAGE}], [PKGLIBEXECDIR])
AC_SUBST([PKGLIBEXECDIR])
adl_RECURSIVE_EVAL([${libexecdir}/${PACKAGE}/backend], [BACKEND_PKGLIBEXECDIR])
AC_SUBST([BACKEND_PKGLIBEXECDIR])

AC_SCOREP_GIT_CONTROLLED

AM_CONDITIONAL([BUILD_TESTS], [true])
AM_CONDITIONAL([CROSS_BUILD], [true])
AC_SCOREP_PLATFORM_SETTINGS
AC_SCOREP_PLATFORM_AND_MACHINE_NAMES
SCOREP_EXPERIMENTAL_PLATFORM

# <AC_SCOREP_COMMON_CHECKS>
SCOREP_COMPILER_COND_AND_SUBST

AC_C_BIGENDIAN

AFS_COMMON_UTILS([build], [<SCOREP_ErrorCodes.h>], [../src/utils/include])
AC_CUTEST_COLOR_TESTS
AC_SCOREP_DEFINE_REVISIONS

AC_SCOREP_RUNTIME_MANAGEMENT_TIMINGS
# </AC_SCOREP_COMMON_CHECKS>

AC_SCOREP_BUILD_MODE
AC_SCOREP_PACKAGE_AND_LIBRARY_VERSION

# assuming that mpi configure already ran
AC_SCOREP_COND_HAVE([MPI_SUPPORT],
                    [test -f ../build-mpi/Makefile],
                    [Defined if MPI support is available.])

# assuming that shmem configure already ran
AC_SCOREP_COND_HAVE([SHMEM_SUPPORT],
                    [test -f ../build-shmem/Makefile],
                    [Defined if SHMEM support is available.])

SCOREP_LIBRARY_WRAPPING_GENERATOR(
    [AC_CONFIG_FILES([../test/tools/libwrap_init/run_libwrap_init_arguments_tests.sh],
                     [chmod +x ../test/tools/libwrap_init/run_libwrap_init_arguments_tests.sh])
     AC_CONFIG_FILES([../test/tools/libwrap_init/run_libwrap_init_simple_tests.sh],
                     [chmod +x ../test/tools/libwrap_init/run_libwrap_init_simple_tests.sh])
     AC_CONFIG_FILES([../test/tools/libwrap_init/run_libwrap_init_filter_tests.sh],
                     [chmod +x ../test/tools/libwrap_init/run_libwrap_init_filter_tests.sh])])

AFS_SET_SILENT_RULE_PREFIX([FE])
AFS_CONFIG_MAKEFILE

AC_OUTPUT
