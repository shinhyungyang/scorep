dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2009-2013,
dnl RWTH Aachen University, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
dnl
dnl Copyright (c) 2009-2020, 2022,
dnl Technische Universitaet Dresden, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl University of Oregon, Eugene, USA
dnl
dnl Copyright (c) 2009-2016, 2018-2025,
dnl Forschungszentrum Juelich GmbH, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Technische Universitaet Muenchen, Germany
dnl
dnl Copyright (c) 2016,
dnl Technische Universitaet Darmstadt, Germany
dnl
dnl This software may be modified and distributed under the terms of
dnl a BSD-style license. See the COPYING file in the package base
dnl directory for details.
dnl

AC_PREREQ([2.69])
AC_INIT([Score-P], m4_esyscmd([../build-config/common/generate-package-version.sh ../build-config/VERSION]), [support@score-p.org], [scorep])

AFS_PACKAGE_BUILD_INIT([MPI backend])
AFS_SUMMARY_INIT([../build-backend])

## require $ac_scorep_cross_compiling to be set to 'yes' or 'no'
AS_IF([test "x${ac_scorep_cross_compiling}" != "xyes" && \
       test "x${ac_scorep_cross_compiling}" != "xno"],
    [AC_MSG_ERROR([invalid value '$ac_scorep_cross_compiling' for variable \$ac_scorep_cross_compiling.], [1])])
# Not all MPIs support executing MPI programs without a startup
# command like mpiexec. Therefore prevent executing configure
# generated programs.
cross_compiling=yes
# suppress warnings about 'using cross tools not prefixed with host
# triplet'
ac_tool_warned=yes

AC_SUBST([LIBRARY_INTERFACE_VERSION], m4_esyscmd([../build-config/common/generate-library-version.sh ../build-config/VERSION]))

AC_CONFIG_AUX_DIR([../build-config])
## See also ACLOCAL_AMFLAGS in Makefile.am.
AC_CONFIG_MACRO_DIRS([../build-config/m4 ../build-config/common/m4])

AC_DEFUN([AC_SCOREP_BACKEND_MPI], [])
m4_define([AFS_COMPUTENODE])

AC_SCOREP_PRECIOUS_VARS_MPI

AC_SCOREP_CHECK_COMPILER_VAR_SET([MPICC])
AC_SCOREP_CHECK_COMPILER_VAR_SET([MPICXX])
AC_SCOREP_CHECK_COMPILER_VAR_SET([MPIF77])
AC_SCOREP_CHECK_COMPILER_VAR_SET([MPIFC])

AC_SCOREP_CONVERT_MPI_COMPILER([CC])
AC_SCOREP_CONVERT_MPI_COMPILER([CXX])
AC_SCOREP_CONVERT_MPI_COMPILER([F77])
AC_SCOREP_CONVERT_MPI_COMPILER([FC])

AC_SCOREP_CONVERT_MPI_FLAGS([CPPFLAGS])
AC_SCOREP_CONVERT_MPI_FLAGS([CFLAGS])
AC_SCOREP_CONVERT_MPI_FLAGS([CXXFLAGS])
AC_SCOREP_CONVERT_MPI_FLAGS([FFLAGS])
AC_SCOREP_CONVERT_MPI_FLAGS([FCFLAGS])
AC_SCOREP_CONVERT_MPI_FLAGS([LDFLAGS])
AC_SCOREP_CONVERT_MPI_FLAGS([LIBS])

AM_INIT_AUTOMAKE([foreign color-tests 1.13.4 -Wall tar-pax])
AM_SILENT_RULES([yes])
AM_PROG_AR
AFS_MAINTAINER_MODE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

SCOREP_COMPUTENODE_CC
AFS_PROG_CC_SUMMARY([MPI])
SCOREP_COMPUTENODE_CXX
AFS_PROG_CXX_SUMMARY([MPI])
SCOREP_COMPUTENODE_FC
AFS_PROG_FC_SUMMARY([MPI])
SCOREP_COMPUTENODE_F77
AFS_PROG_F77_SUMMARY([MPI])

AC_FC_MODULE_EXTENSION
dnl The _AC_FC_IMPLICIT_NONE implementation uses $2 and $3 instead
dnl of the documented $1 and $2, thus add a dummy $1 and set
dnl ACTION-IF-FAILURE to a noop instead of the default-abort.
AC_FC_IMPLICIT_NONE([], [], [:])
SCOREP_FORTRAN_ARGUMENT_MISMATCH

LT_PREREQ([2.4.6])
LT_INIT([shared disable-static pic-only])
AC_SUBST([LIBTOOL_DEPS])
AFS_LINKMODE
AFS_LT_RESTRICTIONS
AFS_LT_RPATHS
AFS_SUMMARY([Link mode], [static=$enable_static, shared=$enable_shared])

AC_CONFIG_HEADERS([../src/config-backend-mpi.h])

ac_scorep_backend=yes
AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"],
      [ac_scorep_frontend="no"],
      [ac_scorep_frontend="yes"])

m4_include(../build-includes/computenode-mpi.m4)

adl_RECURSIVE_EVAL([${bindir}], [BINDIR])
AC_SUBST([BINDIR])

AC_SCOREP_GIT_CONTROLLED

AM_CONDITIONAL([BUILD_TESTS], [test "x${ac_scorep_cross_compiling}" = "xno"])
AM_CONDITIONAL([CROSS_BUILD], [test "x${ac_scorep_cross_compiling}" = "xyes"])
AC_SCOREP_PLATFORM_SETTINGS
AC_SCOREP_PLATFORM_AND_MACHINE_NAMES
SCOREP_EXPERIMENTAL_PLATFORM

AC_SCOREP_MPI

# <AC_SCOREP_COMMON_CHECKS>
SCOREP_COMPILER_COND_AND_SUBST
SCOREP_FORTRAN_CHARLEN_TYPE

AC_C_BIGENDIAN

AFS_COMMON_UTILS([build], [<SCOREP_ErrorCodes.h>], [../src/utils/include])
AC_CUTEST_COLOR_TESTS
AC_SCOREP_DEFINE_REVISIONS
SCOREP_ALLOCATOR_ALIGNMENT

AC_SCOREP_RUNTIME_MANAGEMENT_TIMINGS
# </AC_SCOREP_COMMON_CHECKS>

AC_SCOREP_BACKEND_TEST_RUNS
AC_SCOREP_FORTRAN_SUPPORT_ALLOCATABLE
SCOREP_ADDR2LINE_SUMMARY
SCOREP_SAMPLING
SCOREP_METRICS_CHECK
SCOREP_DLFCN
SCOREP_OPENCL
SCOREP_IO_RECORDING

## Configure internal/external libraries
SCOREP_VENDOR_EXTERNALS_OTF2_FLAGS
SCOREP_VENDOR_EXTERNALS_OPARI2_CPPFLAGS

AC_SCOREP_BUILD_MODE
SCOREP_CUDA
SCOREP_HIP
SCOREP_KOKKOS
SCOREP_CRAY_PMI
SCOREP_MNTENT_CHECK
SCOREP_LUSTREAPI_CHECK

# On crosscompile machines, install backend/mic stuff into
# $(bin|lid)dir/(backend|mic).
backend_suffix=""
AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"],
      [backend_suffix="/backend"
       AS_IF([test "x${ac_scorep_platform}" = "xmic"],
             [backend_suffix="/mic"])
      ])
AC_SUBST([backend_suffix])

SCOREP_OPENMP_SUMMARY
AFS_SUMMARY_SECTION_BEGIN
SCOREP_OPENMP_TPD
SCOREP_OPENMP_ANCESTRY
AFS_SUMMARY_SECTION_END

# Need timer defines and substitution for mpi_unification_test
SCOREP_TIMER

SCOREP_MEMORY_SUPPORT
SCOREP_ALIGN

AC_SCOREP_PACKAGE_AND_LIBRARY_VERSION

AFS_SET_SILENT_RULE_PREFIX([MPI])
AFS_CONFIG_MAKEFILE

AC_CONFIG_FILES([../src/adapters/mpi/f08/scorep_mpi_f08.F90])
AC_CONFIG_FILES([run_otf2_speed_test.sh \
                 ../installcheck/instrumenter_checks/mpi/Makefile:../test/instrumenter_checks/mpi/Makefile.in
                 ../installcheck/instrumenter_checks/mpi_omp/Makefile:../test/instrumenter_checks/mpi_omp/Makefile.in
                 ../src/scorep_config_tool_mpi.h:../src/tools/config/scorep_config_tool_mpi.h.in])
AC_CONFIG_FILES([../test/mpi_omp/run_metric_collection_test.sh], \
                [chmod +x ../test/mpi_omp/run_metric_collection_test.sh])
AC_CONFIG_FILES([../test/mpi_omp/run_mpi_omp_sequence_definition_test.sh], \
                [chmod +x ../test/mpi_omp/run_mpi_omp_sequence_definition_test.sh])
AC_CONFIG_FILES([run_io_mpi_test.sh:../test/io/mpi/run_io_mpi_test.sh.in],
                [chmod +x run_io_mpi_test.sh])

SCOREP_LIBRARY_WRAPPING_GENERATOR(
    [AC_CONFIG_FILES([../test/tools/libwrap_init/mpi.sh])])

# On Cray, libunwind.la's dependecy_libs contain -lc which leads to a
# "multiple definition of `__lll_unlock_wake_private'" error. The patch
# below works around this issue. A better solution would be to fix the
# .la file or to provide a libunwind together with scorep as the
# side effects of this patch aren't investigated.
AS_CASE([${ac_scorep_platform}],
    [crayx*],
         [AC_CONFIG_COMMANDS([libtool-fix-cray-libunwind],
              [sed -e 's/for search_ext in \.la /for search_ext in /' \
               libtool > libtool.fix && mv libtool.fix libtool])
         ])

# Libtool determines some variables at configure time by parsing verbose
# compiler output. For FC, these variables are not needed and sometimes contain
# garbage (e.g. postdeps contains '-l -l' with GNU, or '-L -L' with Cray).
# Therefore, we set the contents of these variables to the empty string.
# See https://lists.gnu.org/archive/html/libtool/2025-02/msg00003.html and
# https://lists.gnu.org/archive/html/bug-libtool/2025-02/msg00000.html.
#
# Configurations for different tags are included in the libtool script itself
# in sections delimited with special comments.
# Find the section for the FC tag, and replace the RHS of the assignment with ""
# for some variables.
AC_CONFIG_COMMANDS([libtool-fix-fc-postdeps], [[sed -e '/^# ### BEGIN LIBTOOL TAG CONFIG: FC$/bfc_section
{ p; d }
:fc_section
n
s/^predep_objects=.*/predep_objects=""/
s/^postdep_objects=.*/postdep_objects=""/
s/^predeps=.*/predeps=""/
s/^postdeps=.*/postdeps=""/
s/^compiler_lib_search_path=.*/compiler_lib_search_path=""/
/^# ### END LIBTOOL TAG CONFIG: FC$/ { p; d }
bfc_section' libtool > libtool.fix && mv libtool.fix libtool]])

dnl Linux
SCOREP_CHECK_SYSCALL([SYS_gettid],
                     [],
                     [])

dnl macOS +10.5
SCOREP_CHECK_SYSCALL([SYS_thread_selfid],
                     [],
                     [])

AC_OUTPUT
