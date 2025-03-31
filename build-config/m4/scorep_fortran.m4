## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2018-2019,
## Technische Universitaet Dresden, Germany
##
## Copyright (c) 2020-2021, 2025,
## Forschungszentrum Juelich GmbH, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##

## file build-config/m4/scorep_fortran.m4

dnl ------------------------------------------------------------------

# Workaround for GCC 10 argument mismatch errors:
# GCC 10 now rejects argument mismatches occurring in the same source
# file, see #9. This affects jacobi MPI tests that use MPI_GET_ADDRESS on
# Cray EX (does it affect all MPICH-based MPIs? Open MPI seems to work
# OK). Porting the jacobi code would be a solution. Replacing it with
# a conforming code would be another. As both can't be implemented in
# time for 7.0, downgrade the error to a warning via
# -fallow-argument-mismatch.
AC_DEFUN([SCOREP_FORTRAN_ARGUMENT_MISMATCH], [
AS_IF([test "x${afs_cv_prog_fc_works}" = xyes],
    [for fcflag in "" "-fallow-argument-mismatch"; do
         FCFLAGS_save="$FCFLAGS"
         FCFLAGS="$fcflag $FCFLAGS"
         _FORTRAN_HAVE_ARGUMENT_MISMATCH
         FCFLAGS="$FCFLAGS_save"
         AS_IF([test "x${have_argument_mismatch_error}" = xno],
             [AC_SUBST([SCOREP_FORTRAN_ALLOW_ARGUMENT_MISMATCH], ["$fcflag"])
              break])
     done
    ])
]) # SCOREP_FORTRAN_ARGUMENT_MISMATCH

AC_DEFUN([_FORTRAN_HAVE_ARGUMENT_MISMATCH], [
# see https://gcc.gnu.org/gcc-10/porting_to.html#argument-mismatch
AC_LANG_PUSH([Fortran])
AC_COMPILE_IFELSE([
    AC_LANG_SOURCE(
[[
      subroutine sub_assumed(array, n)
        real array(*)
        integer n
      end

      program arg_mismatch
        real var
        call sub_assumed(var, 1)
      end
]])],
    [have_argument_mismatch_error=no],
    [have_argument_mismatch_error=yes])
AC_LANG_POP([Fortran])
]) # _FORTRAN_HAVE_ARGUMENT_MISMATCH


# SCOREP_FORTRAN_CHARLEN_TYPE()
# -----------------------------
# Determine the type for Fortran character lengths
AC_DEFUN([SCOREP_FORTRAN_CHARLEN_TYPE],
[dnl FIX REQUIRE: Needs AFS_PROG_FC
AS_IF([test "x${afs_cv_prog_fc_works}" = xyes],
    [AC_LANG_PUSH([Fortran])
     # $ac_ext is the current value of $ac_fc_srcext, or his default
     # $ac_fc_srcext is used with parameter expansion for a default value:
     # ${ac_fc_srcext-f}, but that also honors '' as a value, and would not select
     # the default value, thus the usual 'save=$var; : use/modify $var; var=$save'
     # does not work, as it changes the value to '', even though it was 'f' before
     # reported upstream: http://lists.gnu.org/archive/html/bug-autoconf/2018-12/msg00000.html
     scorep_ac_ext_save=$ac_ext
     scorep_ac_fcflags_srcext_save=$ac_fcflags_srcext
     AC_FC_PP_SRCEXT([F])
     AC_MSG_CHECKING([for Fortran character length type])
     scorep_FCFLAGS_save=$FCFLAGS
     FCFLAGS="${FCFLAGS_F:+$FCFLAGS_F }$FCFLAGS"
     AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[
#if defined( __GFORTRAN__ ) && __GNUC__ >= 8
       choke me
#endif
     ]])], [
         scorep_fortran_charlen_type=int
     ], [
         scorep_fortran_charlen_type=size_t
     ])
     FCFLAGS=$scorep_FCFLAGS_save
     AS_UNSET([scorep_FCFLAGS_save])
     AC_DEFINE_UNQUOTED([scorep_fortran_charlen_t], [$scorep_fortran_charlen_type], [Type to use for implicit character length arguments.])
     AC_MSG_RESULT([$scorep_fortran_charlen_type])
     ac_fc_srcext=$scorep_ac_ext_save
     ac_fcflags_srcext_save=$scorep_ac_fcflags_srcext
     AS_UNSET([scorep_ac_ext_save])
     AC_LANG_POP([Fortran])
    ])
])# SCOREP_FORTRAN_CHARLEN_TYPE


# SCOREP_FORTRAN_FC_NEEDS_FLAGS()
# -------------------------------
#
AC_DEFUN([SCOREP_FORTRAN_FC_NEEDS_FLAGS], [
# ifx complains:
#
#   error #5472: When passing logicals to C, specify '-fpscomp logicals' to
#   get the zero/non-zero behavior of FALSE/TRUE.
#
# Thus, check if compiling  _LOGICALS_TO_C needs flags. If so, add it to
# FCFLAGS to be used for every subsequent compilation unit.
#
AS_IF([test "x${afs_cv_prog_fc_works}" = xyes],
    [AC_LANG_PUSH([Fortran])
     ac_ext_save="${ac_ext}"
     ac_ext=f90
     FCFLAGS_save="$FCFLAGS"
     success=no
     AC_MSG_CHECKING([whether Fortran needs logical-to-C flag])
     for flag in "" "-fpscomp logicals"; do
         FCFLAGS="$FCFLAGS $flag"
         AC_COMPILE_IFELSE([_LOGICALS_TO_C],
             [success=yes
              break])
         FCFLAGS="$FCFLAGS_save"
     done
     ac_ext="${ac_ext_save}"
     AC_LANG_POP([Fortran])

     AS_IF([test "x$success" = xno],
         [AC_MSG_RESULT([unable])
          AC_MSG_ERROR([Fortran compiler cannot pass logicals to C.])],
         [AS_IF([test "x$flag" = x],
            [AC_MSG_RESULT([none])],
            [AC_MSG_RESULT(["$flag"])])])
     AS_UNSET([success])
     AS_UNSET([flag])
    ])
])dnl SCOREP_FORTRAN_FC_NEEDS_FLAGS


# _LOGICALS_TO_C()
# ----------------
#
m4_define([_LOGICALS_TO_C], [
AC_LANG_SOURCE([[
module conftest_bindc
    use, intrinsic :: iso_c_binding, only: c_bool

    implicit none

    private :: c_bool

    interface conftest_toC
        subroutine conftest_toC_default(flag) &
            bind(c, name="conftest_fromF08")
            import
            implicit none
            logical(c_bool) :: flag
        end subroutine
    end interface
end module

module conftest_default
    use, intrinsic :: iso_c_binding, only: c_bool

    use conftest_bindc

    implicit none

    private

    public :: conftest

    interface conftest
        module procedure conftest_impl
    end interface

    contains

    subroutine conftest_impl(flag)
        logical(c_bool), intent(in) :: flag

        call conftest_toC(flag)
    end subroutine
end module
]])])# _LOGICALS_TO_C
