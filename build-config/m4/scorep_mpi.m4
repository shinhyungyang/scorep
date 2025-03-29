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

_MPI_CC_WORKING(
    [AFS_AM_CONDITIONAL([HAVE_MPI_SUPPORT], [test 1 -eq 1], [true])],
    [AC_MSG_FAILURE([MPI compiler $CC does not work. To build Score-P without MPI support, configure --without-mpi.])])

AC_DEFINE([OMPI_WANT_MPI_INTERFACE_WARNING], [0], [Disable deprecation warnings in Open MPI])
AC_DEFINE([OMPI_OMIT_MPI1_COMPAT_DECLS],     [0], [Possibly expose deprecated MPI-1 bindings in Open MPI 4.0+])
AC_LANG_PUSH([C])
AC_COMPUTE_INT([scorep_mpi_version],
    [MPI_VERSION * 100 + MPI_SUBVERSION],
    [#include <mpi.h>],
    [scorep_mpi_version=0])
AC_LANG_POP([C])
AS_IF([test ${scorep_mpi_version} -lt 202],
    [AC_MSG_ERROR([MPI version is incompatible (< 2.2)])],
    [AC_MSG_NOTICE([MPI version is ${scorep_mpi_version}.])])
AC_LANG_PUSH([C])
SCOREP_MPI_C_DECLS
AC_LANG_POP([C])
AC_SCOREP_MPI_C_DATATYPES
AC_SCOREP_MPI_COMPLIANCE
AC_SUBST([MPILIB_LDFLAGS], ["${MPILIB_LDFLAGS}"])

_MPI_CXX_WORKING([], [])

_MPI_F77_WORKING([mpif77_supported=yes], [mpif77_supported=no])
_MPI_FC_WORKING([mpifc_supported=yes], [mpifc_supported=no])
usempi_supported=no
AS_IF([test "x${mpif77_supported}${mpifc_supported}" = xyesyes],
    [usempi_supported=yes
     AFS_AM_CONDITIONAL([HAVE_MPI_USEMPI_SUPPORT], [test 1 -eq 1], [false])])

_MPI_F08_WORKING
AS_IF([test "x${scorep_mpi_usempif08_supported}" = xyes],
    [SCOREP_MPI_F08_FEATURES])
AC_SCOREP_COND_HAVE([MPI_USEMPIF08_SUPPORT],
    [test "x${scorep_mpi_usempif08_supported}" = xyes],
    [Defined if Score-P is built with support for the F08 interface of MPI])

AM_CONDITIONAL([HAVE_MPI_FORTRAN_SUPPORT],
    [test "x${usempi_supported}" = xyes || test "x${scorep_mpi_usempif08_supported}" = xyes])

AFS_SUMMARY_PUSH
AFS_SUMMARY([C bindings], [yes])
AFS_SUMMARY([Fortran bindings], [$usempi_supported])
AFS_SUMMARY([Fortran 2008 bindings], [$scorep_mpi_usempif08_supported])
AS_IF([test "x${MPI_LIBS}" != x],
    [AFS_SUMMARY([Library used], [$MPI_LIBS])])
AFS_SUMMARY_POP([MPI support], [yes])
])# AC_SCOREP_MPI


# _MPI_CC_WORKING([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ----------------------------------------------------
#
m4_define([_MPI_CC_WORKING],[
AC_LANG_PUSH([C])
AC_MSG_CHECKING([whether C compiler can build an MPI program])
AC_LINK_IFELSE([_MPI_HELLO_WORLD_CC_CXX],
    [AC_MSG_RESULT([yes])
     $1],
    [AC_MSG_RESULT([no])
     $2])
AC_LANG_POP([C])
])# _MPI_CC_WORKING

# _MPI_HELLO_WORLD_CC_CXX()
# -------------------------
#
m4_define([_MPI_HELLO_WORLD_CC_CXX], [
AC_LANG_SOURCE([[
#include <mpi.h>
#include <stdio.h>
int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("hello world rank %d/%d, %s\n",
           world_rank, world_size, processor_name);
    MPI_Finalize();
}]])
])# _MPI_HELLO_WORLD_CC_CXX

# _MPI_CXX_WORKING([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# -----------------------------------------------------
#
m4_define([_MPI_CXX_WORKING],[
AC_LANG_PUSH([C++])
AC_MSG_CHECKING([whether C++ compiler can build an MPI program])
AC_LINK_IFELSE([_MPI_HELLO_WORLD_CC_CXX],
    [AC_MSG_RESULT([yes])
     $1],
    [AC_MSG_RESULT([no])
     $2])
AC_LANG_POP([C++])
])# _MPI_CXX_WORKING

# _MPI_F77_WORKING([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# -----------------------------------------------------
#
m4_define([_MPI_F77_WORKING],[
AC_LANG_PUSH([Fortran 77])
AC_MSG_CHECKING([whether Fortran 77 compiler can build a use mpi program])
AC_LINK_IFELSE([AC_LANG_SOURCE([[
       program hello
       use mpi
       integer rank, size, ierror, tag, status(MPI_STATUS_SIZE)

       call MPI_INIT(ierror)
       call MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)
       call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierror)

       print *, "Fortran mpi: hello world from rank ", rank,
     & "/", size

       call MPI_FINALIZE(ierror)
       end program hello
]])],
    [AC_MSG_RESULT([yes])
     $1],
    [AC_MSG_RESULT([no])
     $2])
AC_LANG_POP([Fortran 77])
])# _MPI_F77_WORKING

# _MPI_FC_WORKING([ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ----------------------------------------------------
#
m4_define([_MPI_FC_WORKING],[
AC_LANG_PUSH([Fortran])
ac_ext_save="${ac_ext}"
ac_ext=f90
AC_MSG_CHECKING([whether Fortran compiler can build a use mpi program])
AC_LINK_IFELSE([AC_LANG_SOURCE([[
program helloworld
  use mpi
  implicit none
  integer :: rank, comsize, ierr

  call MPI_Init(ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, comsize, ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr)

  print *,'Hello World, from task ', rank, 'of', comsize

  call MPI_Finalize(ierr)
end program helloworld
]])],
    [AC_MSG_RESULT([yes])
     $1],
    [AC_MSG_RESULT([no])
     $2])
ac_ext="${ac_ext_save}"
AC_LANG_POP([Fortran])
])# _MPI_FC_WORKING

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
AC_MSG_CHECKING([whether Fortran compiler supports use mpi_f08])

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
