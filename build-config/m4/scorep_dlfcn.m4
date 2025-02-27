## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2012,
## RWTH Aachen, Germany
##
## Copyright (c) 2009-2012,
## Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##
## Copyright (c) 2009-2014, 2017, 2025,
## Technische Universitaet Dresden, Germany
##
## Copyright (c) 2009-2012,
## University of Oregon, Eugene, USA
##
## Copyright (c) 2009-2012,
## Forschungszentrum Juelich GmbH, Germany
##
## Copyright (c) 2009-2012,
## German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##
## Copyright (c) 2009-2012,
## Technische Universitaet Muenchen, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license. See the COPYING file in the package base
## directory for details.
##

## file build-config/m4/scorep_dlfcn.m4


AC_DEFUN([SCOREP_CHECK_DLFCN], [

dnl Don't check for prerequisite of dlfcn support on the frontend.
AS_IF([test "x$ac_scorep_backend" = xno], [AC_MSG_ERROR([cannot check for dlfcn support on frontend.])])

AC_LANG_PUSH([C])

have_dlfcn_header="yes"
AC_CHECK_HEADERS([dlfcn.h],
                 [],
                 [have_dlfcn_header=no])

# check for mandatory prerequisites
have_dlfcn_decls="yes"
AC_CHECK_DECLS([RTLD_LAZY,RTLD_NOW],
               [],
               [have_dlfcn_decls="no"],
               [[#include <dlfcn.h>]])

libs_save="$LIBS"

have_dlfcn_functions="yes"
AC_CHECK_LIB([dl],[dlopen],[],[have_dlfcn_functions="no"])
AC_CHECK_LIB([dl],[dlerror],[],[have_dlfcn_functions="no"])
AC_CHECK_LIB([dl],[dlsym],[],[have_dlfcn_functions="no"])
AC_CHECK_LIB([dl],[dlclose],[],[have_dlfcn_functions="no"])

AS_IF([test "x${have_dlfcn_functions}" = "xyes"],
      [scorep_with_dlfcn_libs="dl"])
LIBS="$libs_save"

AC_LANG_POP([C])

# generating output
AS_IF([test "x${have_dlfcn_header}" = "xyes" &&
       test "x${have_dlfcn_decls}" = "xyes" &&
       test "x${have_dlfcn_functions}" = "xyes"],
      [scorep_have_dlfcn_support="yes"],
      [scorep_have_dlfcn_support="no"])
AS_UNSET([have_dlfcn_header])
AS_UNSET([have_dlfcn_decls])
AS_UNSET([have_dlfcn_functions])

AC_SCOREP_COND_HAVE([DLFCN_SUPPORT],
                    [test "x${scorep_have_dlfcn_support}" = "xyes"],
                    [Defined if dynamic linking via dlfcn.h is supported.],
                    [AC_SUBST([SCOREP_DLFCN_LIBS], ["-l${scorep_with_dlfcn_libs}"])],
                    [AC_SUBST([SCOREP_DLFCN_LIBS], [""])])
])

AC_DEFUN([SCOREP_DLFCN], [
AC_REQUIRE([SCOREP_CHECK_DLFCN])dnl

AFS_SUMMARY([dlfcn support], [${scorep_have_dlfcn_support}${SCOREP_DLFCN_LIBS:+, using ${SCOREP_DLFCN_LIBS}}])
])
