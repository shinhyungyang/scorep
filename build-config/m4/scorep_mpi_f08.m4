##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2025,
## Forschungszentrum Juelich GmbH, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license. See the COPYING file in the package base
## directory for details.
##

# SCOREP_MPI_F08_FEATURES()
# ----------------
# Check optional features for the Fortran 2008 part of the MPI adapter.
#
AC_DEFUN([SCOREP_MPI_F08_FEATURES], [
AC_FC_SRCEXT([f90])
_FORTRAN_TS_BUFFERS
_FORTRAN_IGNORE_TKR_DECL
SCOREP_MPI_F08_SYMBOLS
])dnl SCOREP_MPI_F08_FEATURES


# _FORTRAN_TS_BUFFERS()
# ---------------------
# Check whether the Fortran-TS29113 method of passing arrays is supported. This
# feature allows to pass polymorphic, assumed-dimension arrays
# (type(*), dimension(..)) between Fortran and C in a standard way.
#
# Outputs:
# - Set the shell variable $scorep_fortran_ts_buffers to 'yes' if the feature is
#   supported, or else to 'no'.
# - AC_DEFINE 'HAVE_FORTRAN_TS_BUFFERS' to '1' if the feature is supported,
#   or else to '0'.
#
m4_define([_FORTRAN_TS_BUFFERS], [
AC_MSG_CHECKING([for Fortran-TS29113 interoperable buffers])
scorep_fortran_ts_buffers=yes

AC_LANG_PUSH([C])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([_INPUT_FORTRAN_TS_BUFFERS_C])],
    [mv conftest.$OBJEXT conftest1.$OBJEXT],
    [scorep_fortran_ts_buffers=no])
AC_LANG_POP([C])

AC_LANG_PUSH([Fortran])
LIBS_save="$LIBS"
LIBS="conftest1.$OBJEXT $LIBS"

AC_LINK_IFELSE([AC_LANG_SOURCE([_INPUT_FORTRAN_TS_BUFFERS_F08])],
    [scorep_fortran_ts_buffers=yes
     AC_DEFINE([HAVE_FORTRAN_TS_BUFFERS], [1], [Fortran-TS29113 interoperable buffers supported])
     AC_MSG_RESULT([yes])],
    [scorep_fortran_ts_buffers=no
     AC_DEFINE([HAVE_FORTRAN_TS_BUFFERS], [0], [Fortran-TS29113 interoperable buffers supported])
     AC_MSG_RESULT([no])])

LIBS="$LIBS_save"
AC_LANG_POP([Fortran])

rm -f conftest1.$OBJEXT
])dnl _FORTRAN_TS_BUFFERS


# _INPUT_FORTRAN_TS_BUFFERS_C
# ---------------------------
# Input source used in _FORTRAN_TS_BUFFERS.
#
m4_define([_INPUT_FORTRAN_TS_BUFFERS_C], [[
#include <ISO_Fortran_binding.h>

int pass_ts_buffers_from_f08(CFI_cdesc_t * a_desc, CFI_cdesc_t * b_desc)
{
    char * a_row = (char*) a_desc->base_addr;
    if (a_desc->type != CFI_type_int) { return 1; }
    if (a_desc->rank != 2) { return 2; }
    if (a_desc->dim[1].extent != b_desc->dim[0].extent) { return 3; }
    return 0;
}
]])


# _INPUT_FORTRAN_TS_BUFFERS_F08
# -----------------------------
# Input source used in _FORTRAN_TS_BUFFERS.
#
m4_define([_INPUT_FORTRAN_TS_BUFFERS_F08], [[
program test_fortran_ts_buffers

    implicit none

    interface
    function pass_ts_buffers_to_c(buf1, buf2) result(err) bind(c, name="pass_ts_buffers_from_f08")
        use, intrinsic :: iso_c_binding, only : c_int
        integer(c_int) :: err
        type(*), dimension(..) :: buf1, buf2
    end function
    end interface

    integer, dimension(4,4) :: A, B
    integer :: err

    err = pass_ts_buffers_to_c(A(1:4:2, :), B(:, 2:4:2))

end program
]])


# _FORTRAN_IGNORE_TKR_DECL()
# --------------------------
# Determine how to tell the Fortran compiler to ignore the Type, Kind, and Rank
# of a dummy argument. This is the pre-TS29113 way of passing choice buffers. It
# relies on non-standard compiler extensions which differ from vendor to vendor.
#
# Outputs: For the first method that works, AC_SUBST
# - SCOREP_FORTRAN_IGNORE_TKR_PREDECL: A comment which disables TKR checking for
#   a dummy argument, when inserted before its declaration.
# - SCOREP_FORTRAN_IGNORE_TKR_TYPE: The type in the declaration of the choice
#   buffer dummy argument.
#
# Use these as in the example below:
# subroutine routine_with_buffer(buf)
#   @SCOREP_FORTRAN_IGNORE_TKR_PREDECL@ buf
#   @SCOREP_FORTRAN_IGNORE_TKR_TYPE@ :: buf
# end subroutine
m4_define([_FORTRAN_IGNORE_TKR_DECL], [
SCOREP_FORTRAN_IGNORE_TKR_PREDECL=""
SCOREP_FORTRAN_IGNORE_TKR_TYPE=""

# Generic
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!],[type(*), dimension(*)])])

# GCC
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!GCC\$ ATTRIBUTES NO_ARG_CHECK ::],[type(*), dimension(*)])])
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!GCC\$ ATTRIBUTES NO_ARG_CHECK ::],[real, dimension(*)])])

# Intel
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!DEC\$ ATTRIBUTES NO_ARG_CHECK ::],[real, dimension(*)])])

# Solaris
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!\$PRAGMA IGNORE_TKR],[real, dimension(*)])])

# Cray
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!DIR\$ IGNORE_TKR],[real, dimension(*)])])

# IBM
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [_FORTRAN_IGNORE_TKR_DECL_WITH([!IBM* IGNORE_TKR],[real, dimension(*)])])

# Fallback
AS_IF([test "x$SCOREP_FORTRAN_IGNORE_TKR_TYPE" = x],
    [SCOREP_FORTRAN_IGNORE_TKR_PREDECL="!"
     SCOREP_FORTRAN_IGNORE_TKR_TYPE="real, dimension(*)"])

# '#define MACRO !comment' isn't generally understood by Fortran
# preprocessors, thus AC_DEFINE can't be used to communicate the
# value of 'SCOREP_FORTRAN_IGNORE_TKR_PREDECL'. Use AC_SUBST with
# scorep_mpi_f08.F90.in instead.
AC_SUBST([SCOREP_FORTRAN_IGNORE_TKR_PREDECL])
AC_SUBST([SCOREP_FORTRAN_IGNORE_TKR_TYPE])
])dnl _FORTRAN_IGNORE_TKR_DECL


# _FORTRAN_IGNORE_TKR_DECL_WITH()
# -------------------------------
# Check whether ignoring Type, Kind, and Rank of a dummy argument works with
# the given method.
#
# Parameters:
# - $1: Fortran comment to disable TKR checking
# - $2: Type of the dummy argument
#
# Outputs: If the given method works, set the shell variables
# - SCOREP_FORTRAN_IGNORE_TKR_PREDECL
# - SCOREP_FORTRAN_IGNORE_TKR_TYPE
# accordingly.
#
m4_define([_FORTRAN_IGNORE_TKR_DECL_WITH], [
AC_LANG_PUSH([Fortran])

AC_MSG_CHECKING([whether Fortran compiler supports IGNORE_TKR with $1])
AC_COMPILE_IFELSE(AC_LANG_SOURCE([[
program main

implicit none

    interface
    subroutine routine_with_ignore_tkr(buf)
    implicit none
        $1 buf
        $2 :: buf
    end subroutine
    end interface

    interface assumed_shape
    procedure assumed_shape_double_precision_3
    end interface

    integer :: int_buf_scalar
    integer, dimension(2,3) :: int_buf_2x3
    real, dimension(4,2) :: real_buf_4x2
    double precision, dimension(1,2,3) :: double_buf_1x2x3
    complex, dimension(5), target :: complex_buf_5

    complex, dimension(:), pointer :: complex_buf_5_ptr

    call routine_with_ignore_tkr(int_buf_scalar)
    call routine_with_ignore_tkr(int_buf_2x3)
    call routine_with_ignore_tkr(real_buf_4x2)
    call routine_with_ignore_tkr(complex_buf_5)

    call assumed_shape(double_buf_1x2x3)

    complex_buf_5_ptr => complex_buf_5
    call routine_with_ignore_tkr(complex_buf_5_ptr)

contains
    subroutine assumed_shape_double_precision_3(buf)
        double precision, dimension(:,:,:) :: buf
        call routine_with_ignore_tkr(buf)
    end subroutine
end program
]]),
    [SCOREP_FORTRAN_IGNORE_TKR_PREDECL="$1"
     SCOREP_FORTRAN_IGNORE_TKR_TYPE="$2"
     AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])])

AC_LANG_POP([Fortran])
])dnl _FORTRAN_IGNORE_TKR_DECL_WITH
