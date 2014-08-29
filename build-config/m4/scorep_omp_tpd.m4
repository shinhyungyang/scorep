dnl -*- mode: autoconf -*-

dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2013
dnl Forschungszentrum Juelich GmbH, Germany
dnl
dnl This software may be modified and distributed under the terms of
dnl a BSD-style license.  See the COPYING file in the package base
dnl directory for details.
dnl

dnl file build-config/m4/scorep_omp_tpd.m4


# SCOREP_OPENMP_TPD
# -----------------
# Checks if C compiler supports '__attribute__((aligned (16)))'. This
# is necessary to reliably communicate via an instrumented 'int64_t
# pomp_tpd' variable between Fortran and C. If this communication is
# not possible, disable the OpenMP pomp_tpd implementation using the
# automake conditional 'HAVE_SCOREP_OMP_TPD' and the autoconf
# substitution SCOREP_OMP_TPD.
AC_DEFUN([SCOREP_OPENMP_TPD],[

AC_LANG_PUSH([C])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[]],
    [[int __attribute__((aligned (16))) tpd;]])],
    [scorep_has_alignment_attribute="yes"],
    [scorep_has_alignment_attribute="no"]
)
AC_LANG_POP([C])

AS_IF([test "x${scorep_has_alignment_attribute}" = "xyes"],
    [AC_DEFINE([FORTRAN_ALIGNED],
         [__attribute__((aligned (16)))],
         [Makes C variable alignment consistent with Fortran])
     AC_SUBST([SCOREP_OMP_TPD], [1])],
    [AC_SUBST([SCOREP_OMP_TPD], [0])])
AFS_AM_CONDITIONAL([HAVE_SCOREP_OMP_TPD], [test "x${scorep_has_alignment_attribute}" = "xyes"], [false])

AC_MSG_CHECKING([for alignment attribute])
AC_MSG_RESULT([${scorep_has_alignment_attribute}])
AFS_SUMMARY([OpenMP pomp_tpd], [${scorep_has_alignment_attribute}])
])
