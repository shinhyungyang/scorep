dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2009-2013,
dnl RWTH Aachen University, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
dnl
dnl Copyright (c) 2009-2022, 2024-2025,
dnl Technische Universitaet Dresden, Germany
dnl
dnl Copyright (c) 2009-2013,
dnl University of Oregon, Eugene, USA
dnl
dnl Copyright (c) 2009-2025,
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

AFS_PACKAGE_BUILD_INIT([backend])
AFS_SUMMARY_INIT

AC_SUBST([LIBRARY_INTERFACE_VERSION], m4_esyscmd([../build-config/common/generate-library-version.sh ../build-config/VERSION]))

AC_CONFIG_AUX_DIR([../build-config])
## See also ACLOCAL_AMFLAGS in Makefile.am.
AC_CONFIG_MACRO_DIRS([../build-config/m4 ../build-config/common/m4])

AC_DEFUN([AC_SCOREP_BACKEND], [])
m4_define([AFS_COMPUTENODE])

## require $ac_scorep_cross_compiling to be set to 'yes' or 'no'
AS_IF([test "x${ac_scorep_cross_compiling}" != "xyes" && \
       test "x${ac_scorep_cross_compiling}" != "xno"],
    [AC_MSG_ERROR([invalid value '$ac_scorep_cross_compiling' for variable \$ac_scorep_cross_compiling.], [1])])
cross_compiling=${ac_scorep_cross_compiling}

AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"], [
    # suppress warnings about 'using cross tools not prefixed with host triplet'
    ac_tool_warned=yes])

AC_SCOREP_CHECK_COMPILER_VAR_SET([CC])
AC_SCOREP_CHECK_COMPILER_VAR_SET([CXX])
AC_SCOREP_CHECK_COMPILER_VAR_SET([F77])
AC_SCOREP_CHECK_COMPILER_VAR_SET([FC])

AM_INIT_AUTOMAKE([foreign color-tests 1.13.4 -Wall tar-pax])
AM_SILENT_RULES([yes])
AM_PROG_AR
AFS_MAINTAINER_MODE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

SCOREP_COMPUTENODE_CC
AFS_PROG_CC_SUMMARY
SCOREP_COMPUTENODE_CXX
AFS_PROG_CXX_SUMMARY
SCOREP_COMPUTENODE_FC
AFS_PROG_FC_SUMMARY
SCOREP_COMPUTENODE_F77
AFS_PROG_F77_SUMMARY
AM_CONDITIONAL([HAVE_FORTRAN_SUPPORT], [test "x${afs_cv_prog_f77_works}" = "xyes" && test "x${afs_cv_prog_fc_works}" = "xyes"])

LT_PREREQ([2.4.6])
LT_INIT([shared disable-static pic-only])
AC_SUBST([LIBTOOL_DEPS])
AFS_LINKMODE
AFS_LT_RESTRICTIONS
AFS_LT_RPATHS
AFS_SUMMARY([Link mode], [static=$enable_static, shared=$enable_shared])

AC_CONFIG_HEADERS([../src/config-backend.h])
AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"],
    [AC_CONFIG_HEADERS([../src/config-backend-for-frontend.h])])

ac_scorep_backend="yes"
AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"],
    [ac_scorep_frontend="no"],
    [ac_scorep_frontend="yes"])

# On crosscompile machines, install backend/mic stuff into
# $(bin|lid)dir/(backend|mic).
backend_suffix=""
AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"],
      [backend_suffix="/backend"
       AS_IF([test "x${ac_scorep_platform}" = "xmic"],
             [backend_suffix="/mic"])
      ])
AC_SUBST([backend_suffix])

m4_include(../build-includes/computenode.m4)
AS_IF([test "x${ac_scorep_cross_compiling}" = "xno"],
    [m4_include(../build-includes/loginnode.m4)])

adl_RECURSIVE_EVAL([${prefix}], [PREFIX])
AC_SUBST([PREFIX])
adl_RECURSIVE_EVAL([${includedir}], [INCLUDEDIR])
AC_SUBST([INCLUDEDIR])
adl_RECURSIVE_EVAL([${bindir}], [BINDIR])
AC_SUBST([BINDIR])
adl_RECURSIVE_EVAL([${libdir}${backend_suffix}], [BACKEND_LIBDIR])
AC_SUBST([BACKEND_LIBDIR])
AC_DEFINE_UNQUOTED([SCOREP_BACKEND_LIBDIR], ["$BACKEND_LIBDIR"], [build-backend's $libdir])
adl_RECURSIVE_EVAL([${libdir}/${PACKAGE}], [PKGLIBDIR])
AC_SUBST([PKGLIBDIR])
adl_RECURSIVE_EVAL([${libdir}${backend_suffix}/${PACKAGE}], [BACKEND_PKGLIBDIR])
AC_SUBST([BACKEND_PKGLIBDIR])
adl_RECURSIVE_EVAL([${datadir}/${PACKAGE}], [PKGDATADIR])
AC_SUBST([PKGDATADIR])
adl_RECURSIVE_EVAL([${libexecdir}/${PACKAGE}], [PKGLIBEXECDIR])
AC_SUBST([PKGLIBEXECDIR])
adl_RECURSIVE_EVAL([${libexecdir}/${PACKAGE}${backend_suffix}], [BACKEND_PKGLIBEXECDIR])
AC_SUBST([BACKEND_PKGLIBEXECDIR])
adl_RECURSIVE_EVAL([${docdir}], [DOCDIR])
AC_SUBST([DOCDIR])

AC_SCOREP_GIT_CONTROLLED

AM_CONDITIONAL([BUILD_TESTS], [test "x${ac_scorep_cross_compiling}" = "xno"])
AM_CONDITIONAL([CROSS_BUILD], [test "x${ac_scorep_cross_compiling}" = "xyes"])
AC_SCOREP_PLATFORM_SETTINGS
AC_SCOREP_PLATFORM_AND_MACHINE_NAMES
SCOREP_EXPERIMENTAL_PLATFORM

# <AC_SCOREP_COMMON_CHECKS>
SCOREP_COMPILER_COND_AND_SUBST
SCOREP_FORTRAN_CHARLEN_TYPE

AC_C_BIGENDIAN

AFS_COMMON_UTILS([build], [<SCOREP_ErrorCodes.h>], [../src/utils/include])
AC_CUTEST_COLOR_TESTS
AC_SCOREP_DEFINE_REVISIONS
SCOREP_ALLOCATOR_ALIGNMENT

# </AC_SCOREP_COMMON_CHECKS>

AS_IF([test "x${ac_scorep_platform}" = "xbgq"],
    [SCOREP_PAMI])
AC_SCOREP_RUNTIME_MANAGEMENT_TIMINGS
AC_SCOREP_BACKEND_TEST_RUNS
AC_SCOREP_FORTRAN_SUPPORT_ALLOCATABLE
SCOREP_ADDR2LINE_SUMMARY
SCOREP_LIBRARY_WRAPPING_SUMMARY
SCOREP_SAMPLING
SCOREP_METRICS_CHECK
SCOREP_DLFCN
SCOREP_OPENCL
SCOREP_IO_RECORDING
AC_SCOREP_LINK_FLAGS_CHECK

SCOREP_VENDOR_EXTERNALS_OTF2
SCOREP_VENDOR_EXTERNALS_OPARI2
SCOREP_VENDOR_EXTERNALS_CUBEW
# For make check only.
AS_IF([test -n "${scorep_cubelib_bindir}"],
    [AC_SUBST([CUBELIB_BINDIR], ["${scorep_cubelib_bindir}"])],
    [AC_SUBST([CUBELIB_BINDIR], ["${BINDIR}"])])

AFS_POSIX_ACCESS
AFS_POSIX_CLOSE
AFS_POSIX_FILENO
AFS_POSIX_GETHOSTID
AM_CONDITIONAL([HAVE_POSIX_GETHOSTID], [test "x$afs_cv_have_posix_gethostid" = "xyes"])
AFS_POSIX_GETRLIMIT
AFS_POSIX_READ
AFS_POSIX_READLINK
AFS_POSIX_REALPATH

AC_SCOREP_BUILD_MODE
SCOREP_CUDA
SCOREP_HIP
SCOREP_OPENACC_SUMMARY
SCOREP_KOKKOS
SCOREP_CRAY_PMI
SCOREP_MNTENT_CHECK
SCOREP_LUSTREAPI_CHECK


AC_SCOREP_CHECK_SIZEOF([int64_t])
if (( ac_cv_sizeof_void_p > ac_cv_sizeof_int64_t )); then
    AC_MSG_ERROR([sizeof(void*) > sizeof(int64_t): internal thread handling will not work.])
fi

SCOREP_OPENMP_SUMMARY
AFS_SUMMARY_SECTION_BEGIN
SCOREP_OPENMP_TPD
SCOREP_OPENMP_ANCESTRY
AFS_SUMMARY_SECTION_END
SCOREP_OMPT_SUMMARY

SCOREP_TIMER
SCOREP_CHECK_PREINCLUDES

SCOREP_MEMORY_SUPPORT
SCOREP_IQUOTE_SUPPORT
SCOREP_ALIGN

## trunc and ceil needed by the cube4 writer
AC_SEARCH_LIBS([trunc], [m])
AC_SEARCH_LIBS([ceil], [m])

AS_IF([test "x${ac_scorep_platform}" = "xmac"],
      [SCOREP_MACOS_GETEXEC
       SCOREP_MACOS_LIBPROC])

# assuming that mpi configure already ran
AC_SCOREP_COND_HAVE([MPI_SUPPORT],
                    [test -f ../build-mpi/Makefile],
                    [Defined if MPI support is available.])

# assuming that shmem configure already ran
AC_SCOREP_COND_HAVE([SHMEM_SUPPORT],
                    [test -f ../build-shmem/Makefile],
                    [Defined if SHMEM support is available.])

AC_SCOREP_PACKAGE_AND_LIBRARY_VERSION

dnl Predicate to use HARDCODE_LIBDIR_FLAG_* only if linker needs it.
AS_IF([test "x${hardcode_into_libs}" = "xyes"],
      [ac_scorep_hardcode_into_libs=1],
      [ac_scorep_hardcode_into_libs=0])
AC_SUBST([HARDCODE_INTO_LIBS], [${ac_scorep_hardcode_into_libs}])
dnl Flag to hardcode libdir into a binary during linking, C version.
AC_SUBST([HARDCODE_LIBDIR_FLAG_CC], [${hardcode_libdir_flag_spec}])
dnl Flag to hardcode libdir into a binary during linking, C++ version.
AC_SUBST([HARDCODE_LIBDIR_FLAG_CXX], [${hardcode_libdir_flag_spec_CXX}])
dnl Flag to hardcode libdir into a binary during linking, F77 version.
AC_SUBST([HARDCODE_LIBDIR_FLAG_F77], [${hardcode_libdir_flag_spec_F77}])
dnl Flag to hardcode libdir into a binary during linking, Fortran version.
AC_SUBST([HARDCODE_LIBDIR_FLAG_FC], [${hardcode_libdir_flag_spec_FC}])
dnl Use wl of the C compiler and assume it is the same for C++ and
dnl Fortran, instead of using $wl (wl is set by CC, CXX, F77, and FC in
dnl that order. If a compiler isn't recognized by libtool (e.g. flang in
dnl 2020-01), wl's previous value will be set to "", although
dnl lt_prog_compiler_wl usually works).
AC_SUBST([HARDCODE_LIBDIR_WL], [${lt_prog_compiler_wl}])
dnl The ${aix_libpath} part of the HARDCODE_LIBDIR_FLAG_* defines. Only available on AIX.
AC_SUBST([HARDCODE_LIBDIR_AIX_LIBPATH], [${aix_libpath}])

AC_SUBST([SYS_LIB_DLSEARCH_PATH_SPEC], [${sys_lib_dlsearch_path_spec}])
AC_SUBST([SYS_LIB_SEARCH_PATH_VAR],    [${shlibpath_var}])
AC_SUBST([LT_OBJDIR],                  ["$lt_cv_objdir/"])

scorep_list_of_wrappers=

additional_compiler_list=""
AS_IF([test "x${afs_cv_prog_f77_works}" = xyes], [
    AS_VAR_APPEND([additional_compiler_list], [" F77"])
])
AS_IF([test "x${afs_cv_prog_fc_works}" = xyes], [
    AS_VAR_APPEND([additional_compiler_list], [" FC"])
])

additional_compiler_prefix_list=
AM_COND_IF([HAVE_MPI_SUPPORT], [
    AS_VAR_APPEND([additional_compiler_prefix_list], [" MPI"])
])
AM_COND_IF([HAVE_SHMEM_SUPPORT], [
    AS_VAR_APPEND([additional_compiler_prefix_list], [" SHMEM"])
])

for compiler_prefix in '' ${additional_compiler_prefix_list}; do
    for compiler_var in CC CXX ${additional_compiler_list}; do
        full_compiler_var=${compiler_prefix}${compiler_var}
        if eval test x\${${full_compiler_var}:+set} != xset; then
            continue
        fi
        eval value=\$${full_compiler_var}

        # strip of compiler flags
        value="$( (set -- $value; echo $1) )"

        # strip of path elements
        value=${value##*/}

        # avoid duplicates
        case " ${scorep_list_of_wrappers} " in
        (*" scorep-$value "*)
            continue
        ;;
        esac

        AS_VAR_APPEND([scorep_list_of_wrappers], ["scorep-$value "])
    done
done

# add nvcc wrapper
AM_COND_IF([HAVE_CUDA_SUPPORT], [
    case " ${scorep_list_of_wrappers} " in
    (*" scorep-nvcc "*) : ignore duplicate ;;
    (*)
        AS_VAR_APPEND([scorep_list_of_wrappers], ["scorep-nvcc "])
    ;;
    esac
])

# add hipcc wrapper
AM_COND_IF([HAVE_HIP_SUPPORT], [
    case " ${scorep_list_of_wrappers} " in
    (*" scorep-hipcc "*) : ignore duplicate ;;
    (*)
        AS_VAR_APPEND([scorep_list_of_wrappers], ["scorep-hipcc "])
    ;;
    esac
])

AC_CONFIG_FILES([scorep-wrapper:../src/tools/wrapper/scorep-wrapper.in],
                [chmod +x scorep-wrapper])
AC_SUBST([SCOREP_WRAPPER_NAMES], [${scorep_list_of_wrappers}])
AFS_SUMMARY([Compiler wrappers], [${scorep_list_of_wrappers}], 0)

AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"],
    [AFS_SET_SILENT_RULE_PREFIX([BE])])
AFS_CONFIG_MAKEFILE

AC_CONFIG_FILES([../installcheck/instrumenter_checks/Makefile.instrument_configurations:../test/instrumenter_checks/Makefile.instrument_configurations.in
                 ../installcheck/instrumenter_checks/serial/Makefile:../test/instrumenter_checks/serial/Makefile.in
                 ../installcheck/instrumenter_checks/omp/Makefile:../test/instrumenter_checks/omp/Makefile.in
                 ../installcheck/instrumenter_checks/cmdline/Makefile:../test/instrumenter_checks/cmdline/Makefile.in
                 ../src/scorep_config_tool_backend.h:../src/tools/config/scorep_config_tool_backend.h.in])

AM_COND_IF([HAVE_PTHREAD_SUPPORT],
    [AC_CONFIG_FILES([../installcheck/instrumenter_checks/create_wait/pthread/Makefile:../test/instrumenter_checks/create_wait/pthread/Makefile.in])])
AM_COND_IF([HAVE_MEMORY_SUPPORT],
    [AC_CONFIG_FILES([../installcheck/instrumenter_checks/memory/Makefile:../test/instrumenter_checks/memory/Makefile.in])])
AM_COND_IF([HAVE_CUDA_SUPPORT],
    [AC_CONFIG_FILES([../installcheck/instrumenter_checks/cuda/Makefile:../test/instrumenter_checks/cuda/Makefile.in])])
AM_COND_IF([HAVE_POSIX_IO_SUPPORT],
    [AC_CONFIG_FILES([../installcheck/instrumenter_checks/io/Makefile:../test/instrumenter_checks/io/Makefile.in])])
AM_COND_IF([HAVE_HIP_CHECK_SUPPORT],
    [AC_CONFIG_FILES([../installcheck/instrumenter_checks/hip/Makefile:../test/instrumenter_checks/hip/Makefile.in])])

AC_CONFIG_FILES([../test/adapters/opencl/run_opencl_test.sh], \
                [chmod +x ../test/adapters/opencl/run_opencl_test.sh])
AC_CONFIG_FILES([../test/adapters/cuda/run_cuda_test.sh], \
                [chmod +x ../test/adapters/cuda/run_cuda_test.sh])
AC_CONFIG_FILES([../test/adapters/openacc/run_openacc_test.sh], \
                [chmod +x ../test/adapters/openacc/run_openacc_test.sh])
AC_CONFIG_FILES([../test/adapters/user/C/run_phase_test.sh], \
                [chmod +x ../test/adapters/user/C/run_phase_test.sh])
AC_CONFIG_FILES([../test/adapters/user/C/run_selective_test.sh], \
                [chmod +x ../test/adapters/user/C/run_selective_test.sh])
AC_CONFIG_FILES([../test/adapters/user/C++/run_user_cxx_test.sh], \
                [chmod +x ../test/adapters/user/C++/run_user_cxx_test.sh])
AC_CONFIG_FILES([../test/adapters/user/Fortran/run_selective_test.sh], \
                [chmod +x ../test/adapters/user/Fortran/run_selective_test.sh])
AC_CONFIG_FILES([../test/omp_tasks/run_fibonacci_test.sh], \
                [chmod +x ../test/omp_tasks/run_fibonacci_test.sh])
AC_CONFIG_FILES([../test/omp_tasks/run_single_loop_test.sh], \
                [chmod +x ../test/omp_tasks/run_single_loop_test.sh])
AC_CONFIG_FILES([../installcheck/instrumenter_checks/instrumenter-configurations.sh:../test/instrumenter_checks/instrumenter-configurations.sh.in],
                [chmod +x ../installcheck/instrumenter_checks/instrumenter-configurations.sh])
AC_CONFIG_FILES([../installcheck/instrumenter_checks/check-instrumentation.sh:../test/instrumenter_checks/check-instrumentation.sh.in],
                [chmod +x ../installcheck/instrumenter_checks/check-instrumentation.sh])
AC_CONFIG_FILES([../test/unification/run_serial_sequence_definitions_test.sh], \
                [chmod +x ../test/unification/run_serial_sequence_definitions_test.sh])
AC_CONFIG_FILES([../test/filtering/run_filter_test.sh], \
                [chmod +x ../test/filtering/run_filter_test.sh])
AC_CONFIG_FILES([../test/filtering/run_filter_test_crlf.sh], \
                [chmod +x ../test/filtering/run_filter_test_crlf.sh])
AC_CONFIG_FILES([../test/filtering/run_filter_f_test.sh], \
                [chmod +x ../test/filtering/run_filter_f_test.sh])
AC_CONFIG_FILES([../test/filtering/run_compiler_filter_test.sh], \
                [chmod +x ../test/filtering/run_compiler_filter_test.sh])
AC_CONFIG_FILES([../test/services/metric/run_rusage_serial_metric_test.sh], \
                [chmod +x ../test/services/metric/run_rusage_serial_metric_test.sh])
AC_CONFIG_FILES([../test/services/metric/run_rusage_openmp_metric_test.sh], \
                [chmod +x ../test/services/metric/run_rusage_openmp_metric_test.sh])
AC_CONFIG_FILES([../test/services/metric/run_rusage_openmp_per_process_metric_test.sh], \
                [chmod +x ../test/services/metric/run_rusage_openmp_per_process_metric_test.sh])
AC_CONFIG_FILES([../test/services/metric/run_papi_serial_metric_test.sh], \
                [chmod +x ../test/services/metric/run_papi_serial_metric_test.sh])
AC_CONFIG_FILES([../test/services/metric/run_papi_openmp_metric_test.sh], \
                [chmod +x ../test/services/metric/run_papi_openmp_metric_test.sh])
AC_CONFIG_FILES([../test/services/metric/run_papi_openmp_per_process_metric_test.sh], \
                [chmod +x ../test/services/metric/run_papi_openmp_per_process_metric_test.sh])
AC_CONFIG_FILES([../test/rewind/run_rewind_test.sh], \
                [chmod +x ../test/rewind/run_rewind_test.sh])
AC_CONFIG_FILES([../installcheck/constructor_checks/bin/run_constructor_checks.sh:../test/constructor_checks/run_constructor_checks.sh.in],
                [chmod +x ../installcheck/constructor_checks/bin/run_constructor_checks.sh])
AC_CONFIG_FILES([../installcheck/constructor_checks/generate_makefile.sh:../test/constructor_checks/generate_makefile.sh.in],
                [chmod +x ../installcheck/constructor_checks/generate_makefile.sh])

AC_CONFIG_FILES([run_cct_tests.sh:../test/services/unwinding/run_cct_tests.sh.in],
                [chmod +x run_cct_tests.sh])

AC_CONFIG_FILES([../test/tools/wrapper/run_wrapper_checks.sh],
                [chmod +x ../test/tools/wrapper/run_wrapper_checks.sh])

AM_COND_IF([BUILD_SHARED_LIBRARIES],
           [AC_CONFIG_FILES([scorep-preload-init:../src/tools/preload_init/scorep-preload-init.in],
               [chmod +x scorep-preload-init])])

SCOREP_LIBRARY_WRAPPING_GENERATOR(
    [AC_CONFIG_FILES([scorep-libwrap-init:../src/tools/libwrap_init/scorep-libwrap-init.in],
        [chmod +x scorep-libwrap-init])
     AC_CONFIG_FILES([../test/tools/libwrap_init/common.sh])
     AS_IF([test "x${ac_scorep_cross_compiling}" = "xno"],
        [AC_CONFIG_FILES([../test/tools/libwrap_init/run_libwrap_init_arguments_tests.sh],
            [chmod +x ../test/tools/libwrap_init/run_libwrap_init_arguments_tests.sh])
         AC_CONFIG_FILES([../test/tools/libwrap_init/run_libwrap_init_simple_tests.sh],
            [chmod +x ../test/tools/libwrap_init/run_libwrap_init_simple_tests.sh])
         AC_CONFIG_FILES([../test/tools/libwrap_init/run_libwrap_init_filter_tests.sh],
            [chmod +x ../test/tools/libwrap_init/run_libwrap_init_filter_tests.sh])])])

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

dnl Linux
SCOREP_CHECK_SYSCALL([SYS_gettid],
                     [],
                     [])

dnl macOS +10.5
SCOREP_CHECK_SYSCALL([SYS_thread_selfid],
                     [],
                     [])

AC_OUTPUT
