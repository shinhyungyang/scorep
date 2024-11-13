## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2011,
## RWTH Aachen University, Germany
##
## Copyright (c) 2009-2011,
## Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##
## Copyright (c) 2009-2011,
## Technische Universitaet Dresden, Germany
##
## Copyright (c) 2009-2011,
## University of Oregon, Eugene, USA
##
## Copyright (c) 2009-2011, 2015-2017, 2024,
## Forschungszentrum Juelich GmbH, Germany
##
## Copyright (c) 2009-2011,
## German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##
## Copyright (c) 2009-2011,
## Technische Universitaet Muenchen, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##

## file build-config/m4/scorep_cube4.m4


AC_DEFUN([AC_SCOREP_CUBEW], [
# provide a link check here
])

dnl ----------------------------------------------------------------------------

AC_DEFUN([SCOREP_CUBELIB], [
m4_ifndef([SCOREP_SCORE],
    [m4_fatal([Invoke SCOREP_CUBELIB from build-score only.])])

AS_IF([test "x${scorep_cubelib_bindir}" != x],
    [AC_SUBST([CUBELIB_CPPFLAGS], ["$($cubelib_config_cmd --cppflags)"])
     AC_SUBST([CUBELIB_LIBS],     ["$($cubelib_config_cmd --libs)"])
     AC_SUBST([CUBELIB_LDFLAGS],  ["$($cubelib_config_cmd --ltldflags)"])
     AC_SUBST([CUBELIB_BINDIR],   [${scorep_cubelib_bindir}])],
    [# CUBELIB_* Makefile variables are evaluated by calling cubelib-config,
     # once it has been installed. When used earlier, a compile/link will fail.
     m4_define([CUBELIB_CONFIG], [\$(DESTDIR)\$(bindir)/cubelib-config])dnl
     AC_SUBST([CUBELIB_CPPFLAGS], ["\$\$(if test -x \"CUBELIB_CONFIG\"; then echo \$\$(CUBELIB_CONFIG --cppflags | sed 's|-I|-I\$(DESTDIR)|g'); else echo \"-ICUBELIB_CPPFLAGS_not_provided_yet\"; fi)"])
     AC_SUBST([CUBELIB_LIBS],     ["\$\$(if test -x \"CUBELIB_CONFIG\"; then echo \$\$(CUBELIB_CONFIG --libs); else echo \"-lCUBELIB_LIBS_not_provided_yet\"; fi)"])
     AC_SUBST([CUBELIB_LDFLAGS],  ["\$\$(if test -x \"CUBELIB_CONFIG\"; then echo \$\$(CUBELIB_CONFIG --ltldflags | sed 's|-L|-L\$(DESTDIR)|g;s|-R|-R\$(DESTDIR)|g'); else echo \"-LCUBELIB_LDFLAGS_not_provided_yet -RCUBELIB_LDFLAGS_not_provided_yet\"; fi)"])
     m4_undefine([CUBELIB_CONFIG])dnl
     AC_SUBST([CUBELIB_BINDIR],   ["${BINDIR}"])])

## Check for cube reader header and library only if we are using an
## external cube.
AS_IF([test -n "${scorep_cubelib_bindir}"],
    [AC_LANG_PUSH([C++])
     scorep_save_cppflags="${CPPFLAGS}"
     CPPFLAGS="${CUBELIB_CPPFLAGS} ${CPPFLAGS}"
     AC_CHECK_HEADER([Cube.h],
         [has_cubelib_header="yes"],
         [has_cubelib_header="no"])

     scorep_save_libs="${LIBS}"
     LIBS="${LIBS} ${CUBELIB_LIBS}"
     scorep_save_ldflags="${LDFLAGS}"
     # For the non-libtool LINK_IFELSE here, use --ldflags instead of
     # --ltldflags (=CUBELIB_LDFLAGS). Later on, CUBELIB_LDFLAGS are
     # used in a libtool context.
     LDFLAGS="$($cubelib_config_cmd --ldflags) ${LDFLAGS}"
     AC_LINK_IFELSE([AC_LANG_PROGRAM(
                         [[#include <Cube.h>]],
                         [[cube::Cube mycube;]])],
                         [has_cubelib_reader_lib="yes"],
                         [has_cubelib_reader_lib="no"])

     AC_MSG_CHECKING([for cube reader library])
     AS_IF([test "x${has_cubelib_header}" = "xyes" && test "x${has_cubelib_reader_lib}" = "xyes"],
         [AC_MSG_RESULT([yes])
          AFS_SUMMARY([cube c++ library support], [yes, using ${CUBELIB_CPPFLAGS} ${CUBELIB_LDFLAGS} ${CUBELIB_LIBS}])],
         [AC_MSG_ERROR([A compatible Cube reader is required.])])

     ## Clean up
     LIBS="${scorep_save_libs}"
     LDFLAGS="${scorep_save_ldflags}"
     CPPFLAGS="${scorep_save_cppflags}"
     AC_LANG_POP([C++])
    ],
    [# using internal cube c++ library, header and lib assumend to be in place
     AFS_SUMMARY([cube c++ library support], [yes, using internal])
    ])
])
