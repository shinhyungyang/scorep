dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2022-2025,
dnl Forschungszentrum Juelich GmbH, Germany
dnl
dnl This software may be modified and distributed under the terms of
dnl a BSD-style license.  See the COPYING file in the package base
dnl directory for details.
dnl


# SCOREP_OMPT_SUMMARY()
# ---------------------
#
AC_DEFUN_ONCE([SCOREP_OMPT_SUMMARY], [
AC_REQUIRE([SCOREP_OMPT])
AFS_SUMMARY_PUSH
AS_IF([test "x${scorep_have_addr2line}${afs_have_thread_local_storage}${scorep_have_openmp_c_support}" = xyesyesyes],
    [AFS_SUMMARY([OMPT header], [$have_ompt_header${ompt_reason_header:+, $ompt_reason_header}])
     AS_IF([test "x${have_ompt_header}" = xyes],
         [AFS_SUMMARY([OMPT tool], [$have_ompt_tool])
          AS_IF([test "x${have_ompt_tool}" = xyes],
              [AFS_SUMMARY([OMPT C support], [$have_ompt_c_support${ompt_reason_c:+, $ompt_reason_c}])
               AS_IF([test "x${scorep_have_openmp_cxx_support}" = xyes],
                   [AFS_SUMMARY([OMPT C++ support], [$have_ompt_cxx_support${ompt_reason_cxx:+, $ompt_reason_cxx}])])
               AS_IF([test "x${scorep_have_openmp_fc_support}" = xyes],
                   [AFS_SUMMARY([OMPT Fortran support], [$have_ompt_fc_support${ompt_reason_fc:+, $ompt_reason_fc}])])
               AS_IF([test "x${have_ompt_critical_checks_passed}" != xunknown],
                   [AFS_SUMMARY([OMPT critical checks passed], [$have_ompt_critical_checks_passed${ompt_reason_critical_checks_passed:+, $ompt_reason_critical_checks_passed}])])
               AS_IF([test "x${have_ompt_remediable_checks_passed}" != xunknown],
                   [AFS_SUMMARY([OMPT remediable checks passed], [$have_ompt_remediable_checks_passed${ompt_reason_remediable_checks_passed:+, $ompt_reason_remediable_checks_passed}])])
               AS_IF([test "x${have_ompt_target_support}" != xunknown],
                   [AFS_SUMMARY([OMPT target support], [$have_ompt_target_support${ompt_reason_target:+, $ompt_reason_target}])])
               AS_IF([test "x${have_ompt_support}" = xyes && test "x${have_ompt_critical_checks_passed}" = xyes],
                   [scorep_enable_default_ompt=yes
                    AS_IF([test "x$scorep_enable_default_opari2" = xyes],
                        [scorep_enable_default_ompt=no])
                    AFS_SUMMARY([OMPT is default], [$scorep_enable_default_ompt])])])])])
AFS_SUMMARY_POP([OMPT support], [$have_ompt_support${ompt_reason:+, $ompt_reason}])
]) dnl SCOREP_OMPT_SUMMARY

# SCOREP_OMPT()
# -------------
# Check whether OMPT is supported by checking for omp-tools.h,
# building a tool with the C compiler, and using/running the tool with
# the C, C++, and Fortran compiler. Don't call from MPI/SHMEM as they
# need a launcher for running, but rely on build-backend|mpi|shmem
# compatibility.
#
AC_DEFUN_ONCE([SCOREP_OMPT], [
AC_REQUIRE([SCOREP_OPENMP])
AC_REQUIRE([SCOREP_ADDR2LINE])
AC_REQUIRE([AFS_CHECK_THREAD_LOCAL_STORAGE])
AC_REQUIRE([SCOREP_ENABLE_DEFAULT])

have_ompt_header=no
have_ompt_tool=no
have_ompt_support=no
have_ompt_target_support=unknown
have_ompt_critical_checks_passed=unknown
have_ompt_remediable_checks_passed=unknown
have_ompt_c_support=no
have_ompt_cxx_support=no
have_ompt_fc_support=no

AS_IF([test "x${scorep_have_addr2line}" = xyes],
    [AS_IF([test "x${afs_have_thread_local_storage}" = xyes],
         [AS_IF([test "x${scorep_have_openmp_c_support}" = xyes],
              [_CHECK_OMPT_HEADER
               AS_IF([test "x${have_ompt_header}" = xyes],
                   [_BUILD_OMPT_TOOL
                    AS_IF([test "x${have_ompt_tool}" = xyes],
                        [AC_LANG_PUSH([C])
                         _CHECK_OMPT_SUPPORT
                         AC_LANG_POP([C])
                         AS_IF([test "x${scorep_have_openmp_cxx_support}" = xyes],
                             [AC_LANG_PUSH([C++])
                              _CHECK_OMPT_SUPPORT
                              AC_LANG_POP([C++])])
                         AS_IF([test "x${scorep_have_openmp_fc_support}" = xyes],
                             [AC_LANG_PUSH([Fortran])
                              _CHECK_OMPT_SUPPORT
                              AC_LANG_POP([Fortran])])])
                    rm -f tool.$ac_objext])],
              [ompt_reason="OpenMP C support missing"])],
         [ompt_reason="thread local storage support missing"])],
    [ompt_reason="addr2line support missing"])
dnl

AS_IF([test "x$have_ompt_support" = xyes],
    [_CHECK_OMPT_REGISTRATION_STATUS])

AS_IF([test "x$have_ompt_support" = xyes],
    [_CHECK_OMPT_CRITICAL_ISSUES])

AS_IF([test "x$have_ompt_support" = xyes],
    [_CHECK_OMPT_REMEDIABLE_ISSUES
     _ADD_COMPILER_SPECIFIC_COMPILE_FLAGS])

AS_IF([test "x$have_ompt_support" = xyes],
    [_CHECK_OMPT_TARGET_SUPPORT])

AC_SCOREP_COND_HAVE([SCOREP_OMPT_SUPPORT],
    [test "x$have_ompt_support" = xyes],
    [Defined if OMPT is supported for at least one language])

AC_SCOREP_COND_HAVE([SCOREP_OMPT_TARGET_SUPPORT],
    [test "x$have_ompt_target_support" = xyes],
    [Defined if required OMPT target symbols are defined in the OMPT header])

AC_SCOREP_COND_HAVE([SCOREP_OMPT_TARGET_NEEDS_HASHMAP],
    [test "x$ac_cv_sizeof_ompt_id_t" != "x" &&
     test "x$ac_cv_sizeof_voidp" != "x"     &&
     test "$ac_cv_sizeof_voidp" -gt "$ac_cv_sizeof_ompt_id_t"],
    [Defined if size of void pointer is larger than size of ompt_id_t type.])
dnl
])dnl SCOREP_OMPT


# _CHECK_OMPT_HEADER()
# --------------------
#
m4_define([_CHECK_OMPT_HEADER], [
AS_IF([test "x${ac_cv_prog_cc_openmp}" != xunsupported],
    [AC_LANG_PUSH([C])
     CFLAGS_save="${CFLAGS}"
     CFLAGS="${OPENMP_CFLAGS} ${CFLAGS}"
     # ompt-tools.h
     AC_CHECK_HEADER([omp-tools.h],
         [have_ompt_header=yes])
     AS_IF([test "x${have_ompt_header}" = xyes],
         [# check for required enum values
          AC_COMPILE_IFELSE(
              [AC_LANG_PROGRAM([#include <omp-tools.h>], [
/* require version 5.1+ for sync_region */
ompt_sync_region_t foo1 = ompt_sync_region_barrier_implicit_workshare;
ompt_sync_region_t foo2 = ompt_sync_region_barrier_implicit_parallel;
ompt_sync_region_t foo3 = ompt_sync_region_barrier_teams;
          ])],
              [# success
               # check for optional enum values, if any
               #
               # ompt_scope_beginend: used in target-global-data-op,
               # target-data-associate, target-data-disassociate
               # events. Omission produces warnings.
               #
               # ompt_task_taskwait: to detect taskwait-init event
               # (since 5.1)
               #
               AC_CHECK_DECLS([ompt_scope_beginend,
                               ompt_task_taskwait,
                               ompt_taskwait_complete,
                               ompt_sync_region_barrier_implicit_workshare,
                               ompt_sync_region_barrier_implicit_parallel,
                               ompt_sync_region_barrier_teams,
                               ompt_work_loop_static,
                               ompt_work_loop_dynamic,
                               ompt_work_loop_guided,
                               ompt_work_loop_other,
                               ompt_work_scope,
                               ompt_dispatch_ws_loop_chunk,
                               ompt_dispatch_taskloop_chunk,
                               ompt_dispatch_distribute_chunk], [], [], [[#include <omp-tools.h>]])],
              [# failure
               AC_MSG_WARN([OMPT header lacks version 5.1 enum values])
               ompt_reason_header="lacks version 5.1 enum values"
               have_ompt_header=no])])
     CFLAGS="$CFLAGS_save"
     AC_LANG_POP([C])])
])dnl _CHECK_OMPT_HEADER


# _BUILD_OMPT_TOOL()
# ------------------
# Build tool.$ac_objext
#
m4_define([_BUILD_OMPT_TOOL], [
AC_LANG_PUSH([C])
CFLAGS_save="${CFLAGS}"
CFLAGS="${OPENMP_CFLAGS} ${CFLAGS}"
AC_COMPILE_IFELSE([AC_LANG_SOURCE(_INPUT_OMPT_TOOL)],
    [have_ompt_tool=yes
     tool_o="tool.$ac_objext"
     # preserve object file for use at a later time
     cp conftest.$ac_objext $tool_o])
CFLAGS="$CFLAGS_save"
AC_LANG_POP([C])
AC_MSG_CHECKING([whether an OMPT tool can be compiled])
AC_MSG_RESULT([$have_ompt_tool])
])dnl _BUILD_OMPT_TOOL


# _CHECK_TOOL_ACTIVATION()
# ------------------------
# Common part of _CHECK_OMPT_SUPPORT (see below).
#
m4_define([_CHECK_TOOL_ACTIVATION], [
AC_LINK_IFELSE([_INPUT_IGNORED],
    [# Assumption: Cray systems use same architecture for login- and
     # compute-nodes. Thus, running conftest programs is ok. To do
     # so, set cross_compiling temporarily to "no".
     cross_compiling_save="${cross_compiling}"
     AS_IF([test "x${afs_platform_cray}" = xyes],
         [cross_compiling=no])
     AFS_RUN_IFELSE(
         [# do not try to run MPI or SHMEM programs (no launcher)
          have_ompt_support=yes
          have_ompt_[]_AC_LANG_ABBREV[]_support=yes
          # Source omp_version, runtime_version, and
          # initial_device_num as reported from conftest run.
          # Could be used in summary output.
          . ./ompt
          rm -f ./ompt
          AS_UNSET([ompt_reason_]_AC_LANG_ABBREV)],
         [tool_got_activated=yes
          AS_CASE(["x${ac_retval}"],
              [x1], [ompt_reason_[]_AC_LANG_ABBREV="tool not activated"
                     tool_got_activated=no],
              [x2], [ompt_reason_[]_AC_LANG_ABBREV="overdue events not dispatched"],
              [x3], [ompt_reason_[]_AC_LANG_ABBREV="ompt_set_callback lookup failed"],
              [x4], [ompt_reason_[]_AC_LANG_ABBREV="ompt_finalize_tool lookup failed"],
              [x5], [ompt_reason_[]_AC_LANG_ABBREV="thread-begin cb registration failed"],
              [x6], [ompt_reason_[]_AC_LANG_ABBREV="thread-end cb registration failed"],
              [tool_got_activated=no])
         ],
         [AC_MSG_FAILURE([TODO: handle real cross-compile systems])])
     cross_compiling="${cross_compiling_save}"],
    [# Do not override a known reason if linkage fails in next iteration
     AS_IF([test "x${ompt_reason_[]_AC_LANG_ABBREV}" = x],
         [ompt_reason_[]_AC_LANG_ABBREV="tool cannot be linked"])])
])dnl _CHECK_TOOL_ACTIVATION


# _CHECK_OMPT_SUPPORT()
# ---------------------
# Compile OpenMP application and link with tool.$ac_objext (see
# _BUILD_OMPT_TOOL). Run the binary; the return value indicates
# whether OMPT got activated. Call in C context first.
#
m4_define([_CHECK_OMPT_SUPPORT], [
_AC_LANG_PREFIX[]FLAGS_save="$_AC_LANG_PREFIX[]FLAGS"
_AC_LANG_PREFIX[]FLAGS="$OPENMP_[]_AC_LANG_PREFIX[]FLAGS $_AC_LANG_PREFIX[]FLAGS"
AS_UNSET([tool_got_activated])
AS_UNSET([ompt_reason_]_AC_LANG_ABBREV)
AC_COMPILE_IFELSE([AC_LANG_SOURCE(_INPUT_OPENMP_[]_AC_LANG_PREFIX)],
    [cp conftest.$ac_objext main.$ac_objext
     # use customized link command
     ac_link_save="$ac_link"
     m4_case(_AC_CC,
         [CC], [ac_link='$CC -o conftest$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS $tool_o main.$ac_objext $LIBS >&5'],
         [CXX], [ac_link='$CXX -o conftest$ac_exeext $CXXFLAGS $CPPFLAGS $LDFLAGS $tool_o main.$ac_objext $LIBS >&5'],
         [FC], [ac_link='$FC -o conftest$ac_exeext $FCFLAGS $LDFLAGS $tool_o main.$ac_objext $LIBS >&5'],
         [m4_fatal([Unsupported language ]_AC_CC)])
     #
     # Compilers might need extra_ldflags to activate OMPT. Require the CC
     # setup to work with CXX and FC as well (as we don't distinguish
     # language at link time).
     #
     # NVHPC 22.5, 22.7: -mt=ompt
     #
     LDFLAGS_save="$LDFLAGS"
     m4_case(_AC_CC,
         [CC],
             [for extra_ldflags in "" "-mp=ompt"; do
                  LDFLAGS="$extra_ldflags $LDFLAGS_save"
                  _CHECK_TOOL_ACTIVATION
                  AS_IF([test "x${have_ompt_[]_AC_LANG_ABBREV[]_support}" = xyes ||
                         test "x${tool_got_activated}" = xyes],
                      [break])
              done
              AS_IF([test "x${have_ompt_[]_AC_LANG_ABBREV[]_support}" = xyes],
                  [ompt_[]_AC_LANG_ABBREV[]_ldflags="${extra_ldflags}"])
              AC_SUBST([SCOREP_OMPT_LDFLAGS], [${ompt_[]_AC_LANG_ABBREV[]_ldflags}])],
         [LDFLAGS="${ompt_c_ldflags} ${LDFLAGS}"
          _CHECK_TOOL_ACTIVATION])
     LDFLAGS="$LDFLAGS_save"
     rm -f main.$ac_objext
     ac_link="$ac_link_save"],
    [ompt_reason_[]_AC_LANG_ABBREV="OpenMP program does not compile"])
_AC_LANG_PREFIX[]FLAGS="$_AC_LANG_PREFIX[]FLAGS_save"
AC_MSG_CHECKING([whether OMPT is supported for language ]_AC_LANG)
AC_MSG_RESULT([$have_ompt_[]_AC_LANG_ABBREV[]_support${ompt_reason_[]_AC_LANG_ABBREV:+, $ompt_reason_[]_AC_LANG_ABBREV}])
])dnl _CHECK_OMPT_SUPPORT


# _ADD_COMPILER_SPECIFIC_COMPILE_FLAGS()
# ------------------------------
# This function adds the required compile flags to a macro used in `scorep-config`.
# Starting with NVHPC 23.7, each compilation unit requires that we specify the OMPT runtime on the command line.
# See: https://forums.developer.nvidia.com/t/nvhpc-23-7-differences-to-nvhpc-23-5-regarding-ompt-in-compiler-and-linker-flags/261390/2
#
m4_define([_ADD_COMPILER_SPECIFIC_COMPILE_FLAGS], [
AS_CASE([${ax_cv_c_compiler_vendor}],
    [nvhpc], [ompt_cflags="-mp=ompt"],
    [ompt_cflags=""])

AC_SUBST([SCOREP_OMPT_CFLAGS], [${ompt_cflags}])
])dnl _ADD_COMPILER_SPECIFIC_COMPILE_FLAGS

# _CHECK_OMPT_TARGET_SUPPORT()
# ----------------------------
# Checks if symbols required for OMPT target support are defined in the OMPT
# header. If any is missing, target support will be disabled. This check
# deliberately does not check if the callbacks can be enabled, as this might
# rely on the machine where the application is ran, and might differ between
# login and compute nodes. Since these callbacks are not essential, we can
# still enable OMPT host support even if the target ones are not available.
# In that case, the adapter will issue a warning and disable the related
# callbacks.
m4_define([_CHECK_OMPT_TARGET_SUPPORT], [
# Check for symbols added in OpenMP 5.1, which may not be defined in the OMPT header.
# If any is missing, target support will be disabled.
AC_LANG_PUSH([C])
CFLAGS_save="${CFLAGS}"
CFLAGS="${OPENMP_CFLAGS} ${CFLAGS}"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <omp-tools.h>], [
/* Essential types */
ompt_device_t*                     device;
ompt_target_t                      target_kind;
ompt_target_data_op_t              target_data_op;

/* Device tracing */
ompt_buffer_t*                     buffer;
ompt_record_ompt_t*                record;
ompt_buffer_cursor_t               cursor;
ompt_function_lookup_t             lookup;
ompt_get_device_time_t             get_device_time;
ompt_set_trace_ompt_t              set_trace_ompt;
ompt_start_trace_t                 start_trace;
ompt_flush_trace_t                 flush_trace;
ompt_advance_buffer_cursor_t       advance_buffer_cursor;
ompt_get_record_ompt_t             get_record_ompt;
ompt_stop_trace_t                  stop_trace;
ompt_pause_trace_t                 pause_trace;

/* Enum values */
ompt_target_data_op_t optype1   = ompt_target_data_transfer_to_device;
ompt_target_data_op_t optype2   = ompt_target_data_transfer_from_device;
ompt_target_data_op_t optype3   = ompt_target_data_alloc;
ompt_target_data_op_t optype4   = ompt_target_data_delete;
ompt_target_t         kind1     = ompt_target;
ompt_target_t         kind2     = ompt_target_enter_data;
ompt_target_t         kind3     = ompt_target_exit_data;
ompt_target_t         kind4     = ompt_target_update;
ompt_callbacks_t      callback1 = ompt_callback_device_initialize;
ompt_callbacks_t      callback2 = ompt_callback_device_finalize;
ompt_callbacks_t      callback3 = ompt_callback_target_emi;
ompt_callbacks_t      callback4 = ompt_callback_target_data_op_emi;
ompt_callbacks_t      callback5 = ompt_callback_target_submit_emi;
    ])],
    [# Check for optional symbols
     have_ompt_target_support=yes
     AC_CHECK_DECLS([ompt_callback_target,
                     ompt_callback_target_data_op,
                     ompt_callback_target_submit,
                     ompt_target_nowait,
                     ompt_target_enter_data_nowait,
                     ompt_target_exit_data_nowait,
                     ompt_target_update_nowait,
                     ompt_target_data_alloc_async,
                     ompt_target_data_delete_async,
                     ompt_target_data_transfer_from_device_async,
                     ompt_target_data_transfer_to_device_async,
                     ompt_target_data_associate,
                     ompt_target_data_disassociate], [], [], [[#include <omp-tools.h>]])],
    [have_ompt_target_support=no
     ompt_reason_target="missing symbols in omp-tools.h header"])
CFLAGS="${CFLAGS_save}"
AC_LANG_POP([C])

AC_SCOREP_CHECK_SIZEOF([ompt_id_t], [], [[#include <omp-tools.h>]])
AC_SCOREP_CHECK_SIZEOF([void*], [], [[#include <omp-tools.h>]])
])dnl _CHECK_OMPT_TARGET_SUPPORT

# _INPUT_IGNORED()
# ----------------
#
m4_define([_INPUT_IGNORED], [AC_LANG_PROGRAM([], [])])


# _INPUT_OMPT_TOOL()
# ------------------
#
m4_define([_INPUT_OMPT_TOOL], [[
#include <omp-tools.h>
#include <stdlib.h>
#include <stdio.h>
static int initialized;
static int overdue_dispatched;
static ompt_finalize_tool_t finalize_tool;
FILE* ompt;

void cb_thread_begin( ompt_thread_t thread_type,
                      ompt_data_t*  thread_data )
{
    static uint64_t thread_cnt;
    /* no need to lock increment, just a single worker thread, see main() below. */
    thread_data->value = thread_cnt++;
}

void cb_thread_end( ompt_data_t* thread_data )
{
    if ( thread_data->value ==1 )
    {
        /* overdue event on worker thread got dispatched. */
        overdue_dispatched = 1;
    }
}

static int ompt_initialize( ompt_function_lookup_t lookup,
                            int initial_device_num,
                            ompt_data_t *tool_data )
{
    fprintf( ompt, "initial_device_num=\"%d\"\n", initial_device_num );
    fclose( ompt );

    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }

    ompt_set_result_t result;
    result = set_cb( ompt_callback_thread_begin, ( ompt_callback_t )&cb_thread_begin );
    if ( result != ompt_set_always )
    {
        _Exit( 5 ); /* Tool got initialized but thread_begin cb couldn't be registered. */
    }
    result = set_cb( ompt_callback_thread_end, ( ompt_callback_t )&cb_thread_end );
    if ( result != ompt_set_always )
    {
        _Exit( 6 ); /* Tool got initialized but thread_end cb couldn't be registered. */
    }

    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void ompt_finalize( ompt_data_t *tool_data )
{

    if ( initialized == 1 )
    {
        if ( overdue_dispatched == 1 )
        {
            _Exit( 0 ); /* Tool got initialized and finalized as well as overdue event on worker got dispatched. */
        }
        _Exit( 2 ); /* Tool got initialized and finalized but overdue event on worker wasn't dispatched. */
    }
}

ompt_start_tool_result_t* ompt_start_tool( unsigned int omp_version, /* == _OPENMP */
                                           const char*  runtime_version )
{
    ompt = fopen( "./ompt", "w");
    fprintf( ompt, "omp_version=\"%d\"\nruntime_version=\"%s\"\n", omp_version, runtime_version );
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

void foo( int tid )
{
    /* called from main.o */
}

void shut_down_tool( int dummy )
{
    if ( initialized == 1 )
    {
        finalize_tool(); /* supposed to trigger overdue events */
    }
}
]])dnl _INPUT_OMPT_TOOL


# _INPUT_OPENMP_C()
# -----------------
#
m4_define([_INPUT_OPENMP_C], [[
#include <omp.h>
#ifdef __cplusplus
extern "C" {
#endif
void foo( int tid ); /* defined in tool.o, thus the parallel region can't be optimized out */
void shut_down_tool( int dummy );
#ifdef __cplusplus
}
#endif

int main()
{
    #pragma omp parallel num_threads( 2 )
    foo( omp_get_thread_num() );

    shut_down_tool(42);
    return 1; /* Indicates that the tool was not initialized and finalized. */
}
]])dnl _INPUT_OPENMP_C


# _INPUT_OPENMP_CXX()
# -------------------
#
m4_copy([_INPUT_OPENMP_C], [_INPUT_OPENMP_CXX])


# _INPUT_OPENMP_FC()
# ------------------
#
m4_define([_INPUT_OPENMP_FC], [[
      PROGRAM main
      USE OMP_LIB
      INTERFACE
      SUBROUTINE foo(tid) BIND(C)
      USE, INTRINSIC :: ISO_C_BINDING, ONLY: C_INT
      IMPLICIT NONE
      INTEGER(C_INT) :: tid
      END SUBROUTINE foo
      SUBROUTINE shut_down_tool(dummy) BIND(C)
      USE, INTRINSIC :: ISO_C_BINDING, ONLY: C_INT
      IMPLICIT NONE
      INTEGER(C_INT) :: dummy
      END SUBROUTINE shut_down_tool
      END INTERFACE
!\$OMP PARALLEL DEFAULT(NONE) NUM_THREADS(2)
      call foo(OMP_GET_THREAD_NUM())
!\$OMP END PARALLEL
      call shut_down_tool(42)
      CALL EXIT(1)
      END
]])dnl _INPUT_OPENMP_FC

# _TRY_RUN_OMPT_TEST
# ------------------
# Run a runtime test defined by _TRY_RUN_OMPT_TEST_INPUT
# The test is compiled, linked and run with the flags
# defined during the previous tests in SCOREP_OMPT.
# Variables:
# - _TRY_RUN_OMPT_TEST_INPUT: Define for source code
# - _OMPT_TEST: Test name for the ran test
# - have[]_OMPT_TEST[]: Result of executed test
#   - yes: Exit code 0
#   - no: Exit code non 0
#
m4_define([_TRY_RUN_OMPT_TEST], [
cross_compiling_save="${cross_compiling}"
AS_IF([test "x${afs_platform_cray}" = xyes],
    [cross_compiling=no])
AC_LANG_PUSH([C])
CFLAGS_save="${CFLAGS}"
LDFLAGS_save="${LDFLAGS}"
CFLAGS="${OPENMP_CFLAGS} ${CFLAGS}"
LDFLAGS="${ompt_c_ldflags} ${LDFLAGS}"
AC_RUN_IFELSE([AC_LANG_SOURCE(_TRY_RUN_OMPT_TEST_INPUT)],
    [have_[]_OMPT_TEST[]=yes],
    [have_[]_OMPT_TEST[]=no],
    [AC_MSG_FAILURE([TODO: handle real cross-compile systems])])
CFLAGS="${CFLAGS_save}"
LDFLAGS="${LDFLAGS_save}"
AC_LANG_POP([C])
cross_compiling="${cross_compiling_save}"
AC_MSG_CHECKING([whether the OMPT runtime passes the []_OMPT_TEST[] test])
AC_MSG_RESULT([$have_[]_OMPT_TEST[]])
]) dnl _TEST_OMPT_CALLBACK

# ----------------------------------------
# Section for callback registration checks
# ----------------------------------------

# _CHECK_OMPT_REGISTRATION_STATUS
# -------------------------------
# Run configure check to ensure that required callbacks
# are registered with ompt_set_always or ompt_set_sometimes_paired,
# if allowed by the specifications. Set have_ompt_support to no if
# minimum requirements are not met.
#
m4_define([_CHECK_OMPT_REGISTRATION_STATUS], [
# First, test the registration status of the callbacks which are required to report ompt_set_always.
# Variable is named _OMPT_TEST as it is used in _TRY_RUN_OMPT_TEST.
m4_foreach_w([_OMPT_TEST],
    [thread_begin thread_end parallel_begin parallel_end task_create task_schedule implicit_task],
    [m4_define([_TRY_RUN_OMPT_TEST_INPUT], [_INPUT_OMPT_TEST_CALLBACK_ALWAYS])
     _TRY_RUN_OMPT_TEST
     AS_IF([test "x${have_[]_OMPT_TEST}" != "xyes"],
        [have_ompt_support=no
         ompt_reason="[]_OMPT_TEST[] failed to register"])])
# The sync_region callback may report any non-error code. For our
# adapter to work, we require at least ompt_set_sometimes_paired.
# Check this separately.
m4_foreach_w([_OMPT_TEST],
    [sync_region],
    [m4_define([_TRY_RUN_OMPT_TEST_INPUT], [_INPUT_OMPT_TEST_CALLBACK_SOMETIMES_PAIRED])
     _TRY_RUN_OMPT_TEST
     AS_IF([test "x${have_[]_OMPT_TEST}" != "xyes"],
        [have_ompt_support=no
         ompt_reason="[]_OMPT_TEST[] failed to register"])])
m4_undefine([_TRY_RUN_OMPT_TEST_INPUT])
]) dnl _CHECK_OMPT_REGISTRATION_STATUS

m4_define([_INPUT_OMPT_TEST_CALLBACK_ALWAYS], [[
#include <omp.h>
#include <omp-tools.h>
#include <stdlib.h>

#define CALLBACK_NAME( cb ) ompt_callback_ ## cb

static int initialized = 0;

void
callback()
{
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    ompt_set_result_t result = set_cb( CALLBACK_NAME( _OMPT_TEST ), ( ompt_callback_t )&callback );
    if ( result != ompt_set_always )
    {
        _Exit( 5 ); /* Tool got initialized but cb couldn't be registered. */
    }
    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        _Exit( 0 ); /* Tool got initialized and finalized. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

void
foo( int num )
{
}

int
main( void )
{
#pragma omp parallel num_threads( 2 )
    {
        foo( omp_get_thread_num() );
    }
    return 1;
}
]]) dnl _INPUT_OMPT_TEST_CALLBACK_ALWAYS

m4_define([_INPUT_OMPT_TEST_CALLBACK_SOMETIMES_PAIRED], [[
#include <omp.h>
#include <omp-tools.h>
#include <stdlib.h>

#define CALLBACK_NAME( cb ) ompt_callback_ ## cb

static int initialized = 0;

void
callback(){}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    ompt_set_result_t result = set_cb( CALLBACK_NAME( _OMPT_TEST ), ( ompt_callback_t )&callback );
    if ( result < ompt_set_sometimes_paired )
    {
        _Exit( 5 ); /* Tool got initialized but cb couldn't be registered. */
    }
    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        _Exit( 0 ); /* Tool got initialized and finalized. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

void foo( int num )
{}

int main( void )
{
    #pragma omp parallel num_threads( 2 )
    {
        foo( omp_get_thread_num() );
    }
    return 1;
}
]]) dnl _INPUT_OMPT_TEST_CALLBACK_SOMETIMES_PAIRED

# -----------------------------------
# Section for specific problem checks
# -----------------------------------

# _CHECK_OMPT_CRITICAL_ISSUES
# ---------------------------
# Run configure checks to make sure that critical issues
# aren't present in the runtime. Each test is compiled
# and executed using _TRY_RUN_OMPT_TEST. If at least
# one test fails, have_ompt_support is set no no.
#
m4_define([_CHECK_OMPT_CRITICAL_ISSUES], [
have_ompt_critical_checks_passed=yes
AS_UNSET([issues_detected])
m4_foreach_w([_OMPT_TEST],
    [missing_ws_loop_end using_sync_region_barrier incorrect_implicit_barrier_end],
    [m4_define([_TRY_RUN_OMPT_TEST_INPUT], [_INPUT_OMPT_KNOWN_ISSUE_[]m4_toupper(_OMPT_TEST)[]])
     _TRY_RUN_OMPT_TEST
     AS_IF([test "x${have_[]_OMPT_TEST[]}" != xyes],
        [have_ompt_support=no
         have_ompt_critical_checks_passed=no
         issues_detected=${issues_detected:+$issues_detected, }_OMPT_TEST])
     m4_undefine([_TRY_RUN_OMPT_TEST_INPUT])])
AS_IF([test "x${have_ompt_critical_checks_passed}" = xno],
    [ompt_reason_critical_checks_passed="${issues_detected} detected"])
]) dnl _CHECK_OMPT_CRITICAL_ISSUES

# _INPUT_OMPT_KNOWN_ISSUE_MISSING_WS_LOOP_END
# -------------------------------------------
# Test code to check if ws-loop-end is called for
# work callbacks.
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_MISSING_WS_LOOP_END], [[
#include <omp-tools.h>
#include <stdlib.h>

static int                  ws_loop_end_reached = 0;
static int                  initialized         = 0;
static ompt_finalize_tool_t finalize_tool;

void
callback_ompt_work( ompt_work_t           work_type,
                    ompt_scope_endpoint_t endpoint,
                    ompt_data_t*          parallel_data,
                    ompt_data_t*          task_data,
                    uint64_t              count,
                    const void*           codeptr_ra )
{
    if ( endpoint == ompt_scope_end )
    {
        ws_loop_end_reached = 1;
    }
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }
    ompt_set_result_t result;

    /* This test checks if ompt_callback_work is dispatched correctly. The test is only important if the callback
     * itself will ever be dispatched. As this test is marked as critical, it causes the OMPT adapter to not be
     * available if it fails. However, ompt_callback_work is not a mandatory callback for the OMPT interface to work.
     * Therefore, continue the test if at least ompt_set_sometimes_paired is returned. If not, then skip this test,
     * as the callback will be disabled anyway. */
    result = set_cb( ompt_callback_work, ( ompt_callback_t )&callback_ompt_work );
    /* In the case where the runtimes fails to register the callback, exit the test.
     * The activation failure is handled during OMPT adapter initialization.
     * This is important for Cray CCE 17.0.0, which returns ompt_set_sometimes for the
     * ompt_callback_work registration. */
    if ( result < ompt_set_sometimes_paired )
    {
        _Exit( 0 );
    }

    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        if ( ws_loop_end_reached == 1 )
        {
            _Exit( 0 ); /* Tool got initialized and finalized. */
        }
        _Exit( 2 );     /* Tool got initialized and finalized but ws_loop_end was not reached. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,                           /* == _OPENMP */
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

int
foo( int i )
{
    return i;
}

int
main( void )
{
#pragma omp parallel
    {
#pragma omp for
        for ( int i = 1; i < 5; i++ )
        {
            foo( i );
        }
    }
    finalize_tool();
    return 1;
}
]]) dnl _INPUT_OMPT_KNOWN_ISSUE_MISSING_WS_LOOP_END

# _INPUT_OMPT_KNOWN_ISSUE_USING_SYNC_REGION_BARRIER
# -------------------------------------------
# Test code to check if deprecated (since 5.1) ompt_sync_region_t kind
# `ompt_sync_region_barrier` is used in sync_region callbacks. Score-P
# cannot handle this generic barrier as it needs to be able to
# distinguish barrier types.
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_USING_SYNC_REGION_BARRIER], [[
#include <omp-tools.h>
#include <omp.h>
#include <stdlib.h>

static int                  initialized = 0;
static ompt_finalize_tool_t finalize_tool;

void
callback_sync_region( ompt_sync_region_t    kind,
                      ompt_scope_endpoint_t endpoint,
                      ompt_data_t*          parallel_data,
                      ompt_data_t*          task_data,
                      const void*           codeptr_ra )
{
    if ( kind == ompt_sync_region_barrier )
    {
        _Exit( 2 ); /* sync_region callback used ompt_sync_region_barrier */
    }
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }
    ompt_set_result_t result;

    /* This test checks if ompt_callback_sync_region is dispatched correctly. As this test is marked as critical,
     * it causes the OMPT adapter to not be available if the registration fails. This should not happen though,
     * as we check the registration result for this callback earlier in the OMPT configure. */
    result = set_cb( ompt_callback_sync_region, ( ompt_callback_t )&callback_sync_region );
    if ( result < ompt_set_sometimes_paired )
    {
        _Exit( 5 ); /* Tool got initialized but callback failed to register with at least ompt_set_sometimes_paired. */
    }

    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        _Exit( 0 ); /* Tool got initialized and finalized. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,                           /* == _OPENMP */
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

int
foo( int i )
{
    return i;
}

int
main( void )
{
    /* CCE 16.0.1 and 17.0.0 dispatch ompt_sync_region_barrier on ompt_callback_sync_region callbacks
     * for parallel implicit-barrier events. This ompt_sync_region_t field is too generic for Score-P.
     * Therefore, we abort the measurement when we encounter this enum value. Check if a generic parallel
     * region dispatches ompt_sync_region_barrier. If this is the case, we consider the OMPT runtime to
     * be unusable for proper measurements (LUMI #4110). */
#pragma omp parallel for collapse( 2 )
    for ( int i = 0; i < 10; ++i )
    {
        for ( int j = 0; j < 10; ++j )
        {
            foo( omp_get_thread_num() );
        }
    }

    /* Second test case which caused ompt_sync_region_barrier to appear while testing CCE 16.0.1 / 17.0.0 */
#pragma omp parallel
    {
#pragma omp single
        foo( omp_get_thread_num() );

#pragma omp single nowait
        foo( omp_get_thread_num() );
    }
    finalize_tool();
    return 1;
}
]]) dnl _INPUT_OMPT_KNOWN_ISSUE_USING_SYNC_REGION_BARRIER

# _INPUT_OMPT_KNOWN_ISSUE_INCORRECT_IMPLICIT_BARRIER_END
# -------------------------------------------
# Test code to check if implicit-barrier-end complies to the
# specification and provides `parallel_data == NULL` for
# the end of a parallel region.
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_INCORRECT_IMPLICIT_BARRIER_END], [[
#include <omp-tools.h>
#include <omp.h>
#include <stdlib.h>

static int                  initialized = 0;
static ompt_finalize_tool_t finalize_tool;

void
callback_sync_region( ompt_sync_region_t    kind,
                      ompt_scope_endpoint_t endpoint,
                      ompt_data_t*          parallel_data,
                      ompt_data_t*          task_data,
                      const void*           codeptr_ra )
{
    if ( endpoint == ompt_scope_end )
    {
        if ( parallel_data != NULL )
        {
            _Exit( 2 ); /* parallel_data reqired to be NULL for implicit-barrier-end */
        }
    }
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }
    ompt_set_result_t result;

    /* This test checks if ompt_callback_sync_region is dispatched correctly. As this test is marked as critical,
     * it causes the OMPT adapter to not be available if the registration fails. This should not happen though,
     * as we check the registration result for this callback earlier in the OMPT configure. */
    result = set_cb( ompt_callback_sync_region, ( ompt_callback_t )&callback_sync_region );
    if ( result < ompt_set_sometimes_paired )
    {
        _Exit( 5 ); /* Tool got initialized but callback failed to register with at least ompt_set_sometimes_paired. */
    }

    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        _Exit( 0 ); /* Tool got initialized and finalized. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,                           /* == _OPENMP */
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

int
foo( int i )
{
    return i;
}

int
main( void )
{
#pragma omp parallel
    {
        foo( omp_get_thread_num() );
    }
    finalize_tool();
    return 1;
}
]]) dnl _INPUT_OMPT_KNOWN_ISSUE_INCORRECT_PARALLEL_BARRIER

# _CHECK_OMPT_REMEDIABLE_ISSUES
# -----------------------------
# Run configure checks to make sure if remediable issues
# are present in the runtime. Each test is compiled and
# executed using _TRY_RUN_OMPT_TEST. For each test that
# fails, AC_DEFINE HAVE_SCOREP_OMPT_<TEST> to 1.
#
m4_define([_CHECK_OMPT_REMEDIABLE_ISSUES], [
have_ompt_remediable_checks_passed=yes
AS_UNSET([issues_detected])
m4_foreach_w([_OMPT_TEST],
    [wrong_test_lock_mutex missing_work_loop_schedule missing_work_sections_end teams_dispatch_implicit_task],
    [m4_define([_TRY_RUN_OMPT_TEST_INPUT], [_INPUT_OMPT_KNOWN_ISSUE_[]m4_toupper(_OMPT_TEST)[]])
     _TRY_RUN_OMPT_TEST
     AS_IF([test "x${have_[]_OMPT_TEST[]}" != xyes],
        [AC_DEFINE([HAVE_SCOREP_OMPT_]m4_toupper(_OMPT_TEST), [1], [OMPT: Defined to 1 on detection of ]_OMPT_TEST)
         have_ompt_remediable_checks_passed=no
         issues_detected=${issues_detected:+$issues_detected, }_OMPT_TEST])
     m4_undefine([_TRY_RUN_OMPT_TEST_INPUT])])
AS_IF([test "x${have_ompt_remediable_checks_passed}" = xno],
    [ompt_reason_remediable_checks_passed="${issues_detected} detected"])
]) dnl _CHECK_OMPT_REMEDIABLE_ISSUES

# _INPUT_OMPT_KNOWN_ISSUE_WRONG_TEST_LOCK_MUTEX
# ---------------------------------------------
# Test code to check if ompt_mutex_t provided to acquire and acquired
# callbacks for test-locks or test-nest-locks is as specified. Wrong
# kinds are provided for all but NVHPC (2023-06).
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_WRONG_TEST_LOCK_MUTEX], [[
#include <omp.h>
#include <omp-tools.h>
#include <stdlib.h>

static int                  wrong_mutex = 0;
static int                  initialized = 0;
static ompt_finalize_tool_t finalize_tool;

void
callback_ompt_mutex_acquire( ompt_mutex_t   kind,
                             unsigned int   hint,
                             unsigned int   impl,
                             ompt_wait_id_t wait_id,
                             const void*    codeptr_ra )
{
    if ( kind != ompt_mutex_test_lock &&
         kind != ompt_mutex_test_nest_lock )
    {
        wrong_mutex = 1;
    }
}

void
callback_ompt_mutex_acquired( ompt_mutex_t   kind,
                              ompt_wait_id_t wait_id,
                              const void*    codeptr_ra )
{
    if ( kind != ompt_mutex_test_lock &&
         kind != ompt_mutex_test_nest_lock )
    {
        wrong_mutex = 1;
    }
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }

    /* In the case where the runtimes fails to register the callback, exit the test.
     * The activation failure is handled during OMPT adapter initialization.
     * This is important for Cray CCE 17.0.0, which returns ompt_set_sometimes for both
     * registrations. */
    ompt_set_result_t result;
    result = set_cb( ompt_callback_mutex_acquire, ( ompt_callback_t )&callback_ompt_mutex_acquire );
    if( result < ompt_set_sometimes_paired )
    {
        _Exit( 0 );
    }
    result = set_cb( ompt_callback_mutex_acquired, ( ompt_callback_t )&callback_ompt_mutex_acquired );
    if( result < ompt_set_sometimes_paired )
    {
        _Exit( 0 );
    }

    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        if ( wrong_mutex == 0 )
        {
            _Exit( 0 ); /* Tool got initialized and finalized. */
        }
        _Exit( 2 );     /* Tool got initialized and finalized but wrong
                           mutex was provided for test-[nest-]lock events. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

int
main( void )
{
    /* test-lock's acquire and acquired must provide ompt_mutex_test_lock */
    omp_lock_t lck;
    omp_init_lock( &lck );
    while ( !omp_test_lock( &lck ) ) {}
    omp_unset_lock( &lck) ;
    omp_destroy_lock( &lck );

    /* test-nest-lock's acquire and acquired must provide ompt_mutex_test_nest_lock */
    omp_nest_lock_t nlck;
    omp_init_nest_lock( &nlck );
    while ( !omp_test_nest_lock( &nlck ) ) {}
    omp_unset_nest_lock( &nlck );
    omp_destroy_nest_lock( &nlck );

    return 1;
}
]]) dnl _INPUT_OMPT_KNOWN_ISSUE_WRONG_TEST_LOCK_MUTEX

# _INPUT_OMPT_KNOWN_ISSUE_MISSING_WORK_LOOP_SCHEDULE
# ---------------------------------------------
# Test code to check if work callback reports schedule type
# for parallel for loops
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_MISSING_WORK_LOOP_SCHEDULE], [[
#include <omp.h>
#include <omp-tools.h>
#include <stdlib.h>

static int                  has_schedule = 0;
static int                  initialized = 0;
static ompt_finalize_tool_t finalize_tool;

void
callback_ompt_work( ompt_work_t           work_type,
                    ompt_scope_endpoint_t endpoint,
                    ompt_data_t*          parallel_data,
                    ompt_data_t*          task_data,
                    uint64_t              count,
                    const void*           codeptr_ra )
{
    switch ( work_type )
    {
        case ompt_work_loop_static:
        case ompt_work_loop_dynamic:
        case ompt_work_loop_guided:
        case ompt_work_loop_other:
            has_schedule = 1;
            break;
        default:
            break;
    }
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }

    /* In the case where the runtimes fails to register the callback, exit the test.
     * The activation failure is handled during OMPT adapter initialization.
     * This is important for Cray CCE 17.0.0, which returns ompt_set_sometimes for the
     * ompt_callback_work registration. */
    ompt_set_result_t result;
    result = set_cb( ompt_callback_work, ( ompt_callback_t )&callback_ompt_work );
    if( result < ompt_set_sometimes_paired )
    {
        _Exit( 0 );
    }
    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        if ( has_schedule == 1 )
        {
            _Exit( 0 ); /* Tool got initialized and finalized. */
        }
        _Exit( 2 );     /* Tool got initialized and finalized but no loop schedule
                           was provided. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

int
main( void )
{
#pragma omp parallel for schedule(static)
    for ( unsigned i = 0; i < 100; ++i )
    {
    }

#pragma omp parallel for schedule(dynamic)
    for ( unsigned i = 0; i < 100; ++i )
    {
    }

#pragma omp parallel for schedule(guided)
    for ( unsigned i = 0; i < 100; ++i )
    {
    }

    return 1;
}
]]) dnl _INPUT_OMPT_KNOWN_ISSUE_MISSING_WORK_LOOP_SCHEDULE

# _INPUT_OMPT_KNOWN_ISSUE_MISSING_WORK_SECTIONS_END
# ---------------------------------------------
# Test code to check if work callback reports endpoint end
# for sections directive
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_MISSING_WORK_SECTIONS_END], [[
#include <omp-tools.h>
#include <stdlib.h>

static int                  ws_sections_end_reached = 0;
static int                  initialized         = 0;
static ompt_finalize_tool_t finalize_tool;

void
callback_ompt_work( ompt_work_t           work_type,
                    ompt_scope_endpoint_t endpoint,
                    ompt_data_t*          parallel_data,
                    ompt_data_t*          task_data,
                    uint64_t              count,
                    const void*           codeptr_ra )
{
    if ( endpoint == ompt_scope_end )
    {
        ws_sections_end_reached = 1;
    }
}

static int
ompt_initialize( ompt_function_lookup_t lookup,
                 int                    initial_device_num,
                 ompt_data_t*           tool_data )
{
    ompt_set_callback_t set_cb = ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    if ( !set_cb )
    {
        _Exit( 3 ); /* Tool got initialized but lookup of runtime-entry-point ompt_set_callback failed. */
    }
    finalize_tool = ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    if ( !finalize_tool )
    {
        _Exit( 4 ); /* Tool got initialized but lookup of runtime-entry-point ompt_finalize_tool failed. */
    }

    /* In the case where the runtimes fails to register the callback, exit the test.
     * The activation failure is handled during OMPT adapter initialization.
     * This is important for Cray CCE 17.0.0, which returns ompt_set_sometimes for the
     * ompt_callback_work registration. */
    ompt_set_result_t result;
    result = set_cb( ompt_callback_work, ( ompt_callback_t )&callback_ompt_work );
    if( result < ompt_set_sometimes_paired )
    {
        _Exit( 0 );
    }

    initialized = 1;
    return 1; /* non-zero indicates success for OMPT runtime. */
}

static void
ompt_finalize( ompt_data_t* tool_data )
{
    if ( initialized == 1 )
    {
        if ( ws_sections_end_reached == 1 )
        {
            _Exit( 0 ); /* Tool got initialized and finalized. */
        }
        _Exit( 2 );     /* Tool got initialized and finalized but ws_loop_end was not reached. */
    }
}

ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version,                           /* == _OPENMP */
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t ompt_start_tool_result = { &ompt_initialize,
                                                               &ompt_finalize,
                                                               ompt_data_none };
    return &ompt_start_tool_result;
}

int
foo( int i )
{
    return i;
}

int
main( void )
{
    #pragma omp sections
    {
        #pragma omp section
        {
            foo( 42 );
        }
    }
    return 1;
}
]]) dnl _INPUT_OMPT_KNOWN_ISSUE_MISSING_WORK_SECTIONS_END

# _INPUT_OMPT_KNOWN_ISSUE_TEAMS_DISPATCH_IMPLICIT_TASK
# ---------------------------------------------
# Test code to check if teams callback dispatches additional
# implicit task callback with flag initial
#
m4_define([_INPUT_OMPT_KNOWN_ISSUE_TEAMS_DISPATCH_IMPLICIT_TASK], [[
#include <assert.h>
#include <omp.h>
#include <omp-tools.h>
#include <stdatomic.h>
#include <stdio.h>

static ompt_finalize_tool_t ompt_finalize_tool;
atomic_int_least32_t        num_implicit_tasks = 0;
atomic_int_least32_t        num_initial_tasks = 0;
atomic_int_least32_t        teams_created = 0;

void
on_ompt_implicit_task( ompt_scope_endpoint_t endpoint,
                       ompt_data_t*          parallel_data,
                       ompt_data_t*          task_data,
                       unsigned int          actual_parallelism,
                       unsigned int          index, /* thread or team num */
                       int                   flags )
{
    if( flags & ompt_task_initial )
    {
        atomic_fetch_add( &num_initial_tasks, 1 );
    }
    else if (flags & ompt_task_implicit )
    {
        atomic_fetch_add( &num_implicit_tasks, 1 );
    }
}

static int
initialize_tool( ompt_function_lookup_t lookup,
                 int                    initialDeviceNum,
                 ompt_data_t*           toolData )
{
    ompt_set_callback_t set_callback =
        ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    assert( set_callback != 0 );
    ompt_finalize_tool =
        ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    assert( ompt_finalize_tool != 0 );

    set_callback( ompt_callback_implicit_task, (ompt_callback_t) &on_ompt_implicit_task );
    return 1; /* non-zero indicates success */
}

static void
finalize_tool( ompt_data_t* toolData )
{

}

/* Called by the OpenMP runtime. Everything starts from here. */
ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version, /* == _OPENMP */
                 const char*  runtime_version )
{
    static ompt_start_tool_result_t tool = { &initialize_tool,
                                             &finalize_tool,
                                             ompt_data_none };
    return &tool;
}

void
summary()
{
    const int fetched_initial_tasks   = atomic_load( &num_initial_tasks );
    const int fetched_implicit_tasks  = atomic_load( &num_implicit_tasks );
    const int fetched_created_teams   = atomic_load( &teams_created );
    const int expected_initial_tasks  = ( fetched_created_teams + 1 ) * 2;
    const int expected_implicit_tasks = 0;
    fprintf( stderr, "------------------------------\n");
    fprintf( stderr, "Expected: initial_tasks encountered = %u | implicit_tasks encountered = %u\n", expected_initial_tasks, expected_implicit_tasks );
    fprintf( stderr, "Actual:   initial_tasks encountered = %u | implicit_tasks encountered = %u\n", fetched_initial_tasks, fetched_implicit_tasks );
    fprintf( stderr, "------------------------------\n");
    assert( fetched_initial_tasks == expected_initial_tasks );
    assert( fetched_implicit_tasks == expected_implicit_tasks );
}

int main( void )
{
// NVHPC (until at least 24.11) does not support the teams directive without
// a surrounding target construct, resulting in:
// 'NVC++-S-0155-TEAMS construct must be contained within TARGET construct'
// However, a 'target teams' construct still dispatches host callbacks if
// offloaded to the host. Therefore, execute this test with an additional
// 'target' construct on the initial device (i.e. host).
#ifdef __NVCOMPILER
    #pragma omp target teams num_teams( 2 ) device( omp_get_initial_device() )
#else
    #pragma omp teams num_teams( 2 )
#endif
    {
        atomic_fetch_add( &teams_created, 1 );
    }
    ompt_finalize_tool();
    summary();
    return 0;
}
]])
