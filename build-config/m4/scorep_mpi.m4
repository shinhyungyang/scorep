## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2013,
##    RWTH Aachen University, Germany
##
## Copyright (c) 2009-2013,
##    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##
## Copyright (c) 2009-2014, 2019, 2024-2025,
##    Technische Universitaet Dresden, Germany
##
## Copyright (c) 2009-2013,
##    University of Oregon, Eugene, USA
##
## Copyright (c) 2009-2015, 2023, 2025,
##    Forschungszentrum Juelich GmbH, Germany
##
## Copyright (c) 2009-2013,
##    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##
## Copyright (c) 2009-2013,
##    Technische Universitaet Muenchen, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##


## file build-config/m4/scorep_mpi.m4

dnl ----------------------------------------------------------------------------


AC_DEFUN([AC_SCOREP_MPI_C_DATATYPES], [
AC_LANG_PUSH([C])

AFS_MPI_DATATYPE_INT32_T
AFS_MPI_DATATYPE_UINT32_T
AFS_MPI_DATATYPE_INT64_T
AFS_MPI_DATATYPE_UINT64_T

AC_LANG_POP([C])
]) # AC_DEFUN([AC_SCOREP_MPI_C_CONSTANTS])


dnl ----------------------------------------------------------------------------

AC_DEFUN([_SCOREP_MPI_CHECK_COMPLIANCE],
         [AFS_CHECK_COMPLIANCE([@%:@include <mpi.h>], $@)])

dnl ----------------------------------------------------------------------------

AC_DEFUN([AC_SCOREP_MPI_COMPLIANCE], [
    AC_LANG_PUSH(C)

    AC_MSG_CHECKING([whether MPI_Info_delete is standard compliant])
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            #if MPI_VERSION >= 3
            #define SCOREP_MPI_CONST_DECL const
            #else
            #define SCOREP_MPI_CONST_DECL
            #endif
            int MPI_Info_delete(MPI_Info info, SCOREP_MPI_CONST_DECL char *c)
            {
                return 0;
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_INFO_DELETE_COMPLIANT, 1, [MPI_Info_delete is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_COMPILE_IF_ELSE

    AC_MSG_CHECKING([whether MPI_Info_get is standard compliant])
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            #if MPI_VERSION >= 3
            #define SCOREP_MPI_CONST_DECL const
            #else
            #define SCOREP_MPI_CONST_DECL
            #endif
            int MPI_Info_get(MPI_Info info, SCOREP_MPI_CONST_DECL char *c1, int i1, char *c2, int *i2)
            {
                return 0;
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_INFO_GET_COMPLIANT, 1, [MPI_Info_get is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_COMPILE_IF_ELSE

    AC_MSG_CHECKING([whether MPI_Info_get_valuelen is standard compliant])
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            #if MPI_VERSION >= 3
            #define SCOREP_MPI_CONST_DECL const
            #else
            #define SCOREP_MPI_CONST_DECL
            #endif
            int MPI_Info_get_valuelen(MPI_Info info, SCOREP_MPI_CONST_DECL char *c, int *i1, int *i2)
            {
                return 0;
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_INFO_GET_VALUELEN_COMPLIANT, 1, [MPI_Info_get_valuelen is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_COMPILE_IF_ELSE

    AC_MSG_CHECKING([whether MPI_Info_set is standard compliant])
    AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            #if MPI_VERSION >= 3
            #define SCOREP_MPI_CONST_DECL const
            #else
            #define SCOREP_MPI_CONST_DECL
            #endif
            int MPI_Info_set(MPI_Info info, SCOREP_MPI_CONST_DECL char *c1, SCOREP_MPI_CONST_DECL char *c2)
            {
                return 0;
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_INFO_SET_COMPLIANT, 1, [MPI_Info_set is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_COMPILE_IF_ELSE

    AC_MSG_CHECKING([whether MPI_Grequest_complete is standard compliant])
    AC_LINK_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            int MPI_Grequest_complete(MPI_Request request)
            {
                return 0;
            }

            int main()
            {
                MPI_Request r;
                return  MPI_Grequest_complete(r);
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_GREQUEST_COMPLETE_COMPLIANT, 1, [MPI_Grequest_complete is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_LINK_IF_ELSE

    AC_MSG_CHECKING([whether PMPI_Type_create_f90_complex is standard compliant])
    AC_LINK_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            int MPI_Type_create_f90_complex(int p, int r, MPI_Datatype *newtype)
            {
                return PMPI_Type_create_f90_complex(p, r, newtype);
            }

            int main()
            {
                return  MPI_Type_create_f90_complex(3,3,0);
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_TYPE_CREATE_F90_COMPLEX_COMPLIANT, 1, [MPI_Type_create_f90_complex is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_LINK_IF_ELSE

    AC_MSG_CHECKING([whether PMPI_Type_create_f90_integer is standard compliant])
    AC_LINK_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            int MPI_Type_create_f90_integer(int r, MPI_Datatype *newtype)
            {
                return PMPI_Type_create_f90_integer(r, newtype);
            }

            int main()
            {
                return  MPI_Type_create_f90_integer(3,0);
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_TYPE_CREATE_F90_INTEGER_COMPLIANT, 1, [MPI_Type_create_f90_integer is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_LINK_IF_ELSE

    AC_MSG_CHECKING([whether PMPI_Type_create_f90_real is standard compliant])
    AC_LINK_IFELSE([
        AC_LANG_SOURCE([
            #include<mpi.h>
            int MPI_Type_create_f90_real(int p, int r, MPI_Datatype *newtype)
            {
                return PMPI_Type_create_f90_real(p, r, newtype);
            }

            int main()
            {
                return  MPI_Type_create_f90_real(3,3,0);
            }
            ])],
        [AC_MSG_RESULT(yes);
         AC_DEFINE(HAVE_MPI_TYPE_CREATE_F90_REAL_COMPLIANT, 1, [MPI_Type_create_f90_integer is standard compliant])],
        [AC_MSG_RESULT(no)]
    ) # AC_LINK_IF_ELSE

    _SCOREP_MPI_COMPLIANCE_CHECKS

    AC_LANG_POP(C)
]) # AC_DEFUN(AC_SCOREP_MPI_COMPLIANCE)


dnl ----------------------------------------------------------------------------


AC_DEFUN([AC_SCOREP_MPI], [

AC_DEFINE([OMPI_WANT_MPI_INTERFACE_WARNING], [0], [Disable deprecation warnings in Open MPI])
AC_DEFINE([OMPI_OMIT_MPI1_COMPAT_DECLS],     [0], [Possibly expose deprecated MPI-1 bindings in Open MPI 4.0+])

scorep_mpi_usempif08_supported="no"
if test "x${scorep_mpi_c_supported}" = "xyes"; then
  scorep_mpi_supported="yes"
  if test "x${scorep_mpi_f77_supported}" = "xyes" && test "x${scorep_mpi_f90_supported}" = "xyes"; then
    scorep_mpi_usempi_supported="yes"
  else
    scorep_mpi_usempi_supported="no"
  fi
  _MPI_F08_WORKING
else
  scorep_mpi_supported="no"
  scorep_mpi_usempi_supported="no"
fi

if test "x${scorep_mpi_supported}" = "xno"; then
  AC_MSG_WARN([No suitable MPI compilers found. SCOREP MPI and hybrid libraries will not be build.])
fi
AM_CONDITIONAL([HAVE_MPI_SUPPORT], [test "x${scorep_mpi_supported}" = "xyes"])
AM_CONDITIONAL([HAVE_MPI_USEMPI_SUPPORT], [test "x${scorep_mpi_usempi_supported}" = "xyes"])
AC_SCOREP_COND_HAVE([MPI_USEMPIF08_SUPPORT],
    [test "x${scorep_mpi_usempif08_supported}" = "xyes"],
    [Defined if Score-P is built with support for the F08 interface of MPI])
AM_CONDITIONAL([HAVE_MPI_FORTRAN_SUPPORT], [test "x${scorep_mpi_usempi_supported}" = "xyes" || test "x${scorep_mpi_usempif08_supported}" = "xyes"])

if test "x${scorep_mpi_supported}" = "xyes"; then

  AC_COMPUTE_INT([scorep_mpi_version],
                 [MPI_VERSION * 100 + MPI_SUBVERSION],
                 [#include <mpi.h>],
                 [scorep_mpi_version=0])
  AS_IF([test ${scorep_mpi_version} -lt 202],
        [AC_MSG_ERROR([MPI version is incompatible (< 2.2)])],
        [AC_MSG_NOTICE([MPI version is ${scorep_mpi_version}.])])


  SCOREP_MPI_C_DECLS
  AC_SCOREP_MPI_C_DATATYPES
  AC_SCOREP_MPI_COMPLIANCE
  AS_IF([test "x${scorep_mpi_usempif08_supported}" = xyes],
  [
    SCOREP_MPI_F08_FEATURES
  ])

  AFS_SUMMARY_PUSH
  AFS_SUMMARY([C bindings], [$scorep_mpi_c_supported])
  AFS_SUMMARY([Fortran bindings], [$scorep_mpi_usempi_supported])
  AFS_SUMMARY([Fortran 2008 bindings], [$scorep_mpi_usempif08_supported])
  AFS_SUMMARY_POP([MPI support], [$scorep_mpi_supported])

fi # if test "x${scorep_mpi_supported}" = "xyes"
])


# _MPI_F08_WORKING()
# ------------------
# Checks the minimum required compiler and MPI features to enable wrapping
# procedures from the MPI Fortran 2008 bindings.
#
# Outputs:
# - Set the shell variable scorep_mpi_usempif08_supported to 'yes' on success, or
#   'no, <reason>' on failure.
#
m4_define([_MPI_F08_WORKING],[
AC_LANG_PUSH([Fortran])
ac_ext_save="${ac_ext}"
ac_ext=f90
AC_MSG_CHECKING([for useable mpi_f08])

scorep_mpi_usempif08_supported="yes"

if test x"$scorep_mpi_usempif08_supported" = xyes; then
AC_COMPILE_IFELSE(AC_LANG_SOURCE([[program f08
    use :: mpi_f08
end program]]), [], [scorep_mpi_usempif08_supported="no, cannot use mpi_f08 module"])
fi

if test x"$scorep_mpi_usempif08_supported" = xyes; then
AC_COMPILE_IFELSE(AC_LANG_SOURCE([[program assumed_type
    implicit none
    interface
    subroutine sub_with_assumed_type(buf)
        type(*) :: buf
    end subroutine
    end interface
end program]]), [], [scorep_mpi_usempif08_supported="no, Fortran compiler does not support assumed type"])
fi

if test x"$scorep_mpi_usempif08_supported" = xyes; then
AC_LINK_IFELSE(AC_LANG_SOURCE([[program pmpi_funcs
    implicit none

    call check_pmpi_comm_size()
    call check_pmpi_type_size()
    call check_pmpi_get_count()
    call check_pmpi_test_cancelled()

    contains

    subroutine check_pmpi_comm_size()
        use :: mpi_f08, only: MPI_Comm, PMPI_Comm_size

        type(MPI_Comm) :: comm
        integer :: size, ierror
        call PMPI_Comm_size(comm, size, ierror)
    end subroutine

    subroutine check_pmpi_type_size()
        use :: mpi_f08, only: MPI_Datatype, PMPI_Type_size

        integer :: size, ierror
        type(MPI_Datatype) :: type
        call PMPI_Type_size(type, size, ierror)
    end subroutine

    subroutine check_pmpi_get_count()
        use :: mpi_f08, only: MPI_Status, MPI_Datatype, PMPI_Get_count
        type(MPI_Status) :: status
        type(MPI_Datatype) :: datatype
        integer :: count, ierror
        call PMPI_Get_count(status, datatype, count, ierror)
    end subroutine

    subroutine check_pmpi_test_cancelled()
        use :: mpi_f08, only: MPI_Status, PMPI_Test_cancelled

        type(MPI_Status) :: status
        logical :: flag
        integer :: ierror
        call PMPI_Test_cancelled(status, flag, ierror)
    end subroutine
end program]]), [], [scorep_mpi_usempif08_supported="no, MPI library does not provide all required PMPI procedures"])
fi

AC_MSG_RESULT([$scorep_mpi_usempif08_supported])
ac_ext="${ac_ext_save}"
AC_LANG_POP([Fortran])
])dnl _MPI_F08_WORKING



dnl ----------------------------------------------------------------------------


AC_DEFUN([_AC_SCOREP_MPI_INCLUDE], [
AC_REQUIRE([AC_PROG_GREP])
ac_scorep_have_mpi_include="no"
AC_LANG_PUSH([C])
AC_PREPROC_IFELSE([AC_LANG_PROGRAM([[#include <mpi.h>]], [])],
                  [GREP_OPTIONS= ${GREP} '/mpi.h"' conftest.i > conftest_mpi_includes
                   AS_IF([test $? -eq 0],
                         [scorep_mpi_include=`cat conftest_mpi_includes | GREP_OPTIONS= ${GREP} '/mpi.h"' | \
                          head -1 | sed -e 's#^.* "##' -e 's#/mpi.h".*##'`
                          ac_scorep_have_mpi_include="yes"],
                         [])
                   rm -f conftest_mpi_includes],
                  [])
AC_LANG_POP([C])

AS_IF([test "x${ac_scorep_have_mpi_include}" = "xyes"],
      [AC_SUBST([SCOREP_MPI_INCLUDE], [${scorep_mpi_include}])],
      [])
])
