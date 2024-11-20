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

## file build-config/m4/scorep_vendor_externals.m4

# ----
# OTF2
# ----

AC_DEFUN([SCOREP_VENDOR_EXTERNALS_OTF2_FLAGS], [
scorep_otf2_config_backend_arg=
AS_IF([test "x${ac_scorep_cross_compiling}" = xyes],
    [scorep_otf2_config_backend_arg="--target=backend"
     AS_IF([test "x${ac_scorep_platform}" = xmic],
         [scorep_otf2_config_backend_arg="--target=mic"])])
AS_IF([test "x${scorep_otf2_bindir}" != x],
    [otf2_config_cmd="${scorep_otf2_bindir}/otf2-config ${scorep_otf2_config_backend_arg}"
     AC_SUBST([OTF2_LIBS], [$($otf2_config_cmd --libs)])
     AC_SUBST([OTF2_LDFLAGS], [$($otf2_config_cmd --ltldflags)])
     AC_SUBST([OTF2_CPPFLAGS], [$($otf2_config_cmd --cppflags)])
     AC_SUBST([OTF2_BINDIR],   ["${scorep_otf2_bindir}"])],
    [# OTF2_* Makefile variables are evaluated by calling otf2-config,
     # once it has been installed. When used earlier, a compile/link will fail.
     m4_define([OTF2_CONFIG], [\$(DESTDIR)\$(bindir)/otf2-config])dnl
     AC_SUBST([OTF2_LIBS], ["\$\$(if test -x \"OTF2_CONFIG\"; then echo \$\$(OTF2_CONFIG \$\${scorep_otf2_config_backend_arg} --libs); else echo \"-lOTF2_LIBS_not_provided_yet\"; fi)"])
     AC_SUBST([OTF2_LDFLAGS], ["\$\$(if test -x \"OTF2_CONFIG\"; then echo \$\$(OTF2_CONFIG \$\${scorep_otf2_config_backend_arg} --ltldflags | sed  's|-L|-L\$(DESTDIR)|g;s|-R|-R\$(DESTDIR)|g'); else echo \"-LOTF2_LDFLAGS_not_provided_yet -ROTF2_LDFLAGS_not_provided_yet\"; fi)"])
     AC_SUBST([OTF2_CPPFLAGS], ["\$\$(if test -x \"OTF2_CONFIG\"; then echo \$\$(OTF2_CONFIG \$\${scorep_otf2_config_backend_arg} --cppflags | sed  's|-I|-I\$(DESTDIR)|g'); else echo \"-IOTF2_CPPFLAGS_not_provided_yet\"; fi)"])
     m4_undefine([OTF2_CONFIG])dnl
     AC_SUBST([OTF2_BINDIR], ["${BINDIR}"])])
AM_CONDITIONAL([HAVE_SCOREP_EXTERNAL_OTF2], [test "x${scorep_otf2_bindir}" != x])
]) # SCOREP_VENDOR_EXTERNALS_OTF2_FLAGS

AC_DEFUN_ONCE([SCOREP_VENDOR_EXTERNALS_OTF2], [
AFS_SUMMARY_SECTION_BEGIN([OTF2 features])
SCOREP_VENDOR_EXTERNALS_OTF2_FLAGS

AS_IF([test "x${scorep_otf2_bindir}" = x],
    [# Sole purpose of this otf2_config_cmd is to query sionlib status, see below.
     otf2_config_cmd="../vendor/otf2/otf2-mini-config.sh ${scorep_otf2_config_backend_arg}"

     # We cannot do compile checks against the internal OTF2, hence we need to
     # assume that it is the expected version
     ac_cv_have_decl_OTF2_REGION_ROLE_KERNEL=yes])

scorep_otf2_have_sion=no
for substrate in $($otf2_config_cmd --features=substrates)
do
    AS_CASE([${substrate}],
        [sion], [scorep_otf2_have_sion=yes])
done
AS_IF([test "x$scorep_otf2_have_sion" = "xyes"], [
    AC_DEFINE([HAVE_OTF2_SUBSTRATE_SION], [1], [Define if the used OTF2 library has SIONlib support.])
])
AFS_SUMMARY([SIONlib support], [$scorep_otf2_have_sion])

AC_LANG_PUSH([C])
save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS $OTF2_CPPFLAGS"
# This should only be executed with an external OTF2, the internal spools the cache
AC_CHECK_DECLS([OTF2_REGION_ROLE_KERNEL], [:], [:], [[#include <otf2/otf2.h>]])
CFLAGS=$save_CFLAGS
AC_LANG_POP([C])

AFS_SUMMARY_SECTION_END # OTF2 features
]) # SCOREP_VENDOR_EXTERNALS_OTF2

# ------
# OPARI2
# ------

AC_DEFUN_ONCE([SCOREP_VENDOR_EXTERNALS_OPARI2], [
opari2_fix_and_free_form_options=1
AS_IF([test -n "${scorep_opari2_bindir}"],
    [# Require major version 2. It is very unlikely that there will a major version > 2.
     AS_IF([${scorep_opari2_bindir}/opari2-config --version | GREP_OPTIONS= grep -v ^2],
         [AC_MSG_ERROR([Provided OPARI2 too old ($(opari2-config --version)), should be at least 2.0])])
     AC_SUBST([OPARI2_BINDIR],            ["${scorep_opari2_bindir}"])
     AC_SUBST([OPARI2_MAKE_CHECK_BINDIR], ["${scorep_opari2_bindir}"])
     AC_SUBST([OPARI2_CPPFLAGS],          ["`${scorep_opari2_bindir}/opari2-config --cflags`"])
     AC_SUBST([HAVE_EXTERNAL_OPARI2],     [1])
     AC_SUBST([OPARI2_REGION_INFO_SRC_DIR],["${scorep_opari2_bindir}/../share/opari2/devel"])
     AC_SUBST([OPARI2_REGION_INFO_INC_DIR],["${scorep_opari2_bindir}/../share/opari2/devel"])
     ${scorep_opari2_bindir}/opari2-config --fix-form > /dev/null 2>&1
     AS_IF([test $? -ne 0], [opari_fix_and_free_form_options=0])
     AM_CONDITIONAL([HAVE_SCOREP_EXTERNAL_OPARI2], [true])],
    [AC_SUBST([OPARI2_BINDIR],            ["${BINDIR}"])
     AC_SUBST([OPARI2_MAKE_CHECK_BINDIR], ["../vendor/opari2/build-frontend"])
     AC_SUBST([OPARI2_CPPFLAGS],          ['-I$(srcdir)/../vendor/opari2/include'])
     AC_SUBST([HAVE_EXTERNAL_OPARI2],     [0])
     AC_SUBST([OPARI2_REGION_INFO_SRC_DIR],["../vendor/opari2/src/opari-lib-dummy"])
     AC_SUBST([OPARI2_REGION_INFO_INC_DIR],['$(srcdir)/../vendor/opari2/src/opari-lib-dummy'])
     AM_CONDITIONAL([HAVE_SCOREP_EXTERNAL_OPARI2], [false])])
AC_SUBST([HAVE_OPARI2_FIX_AND_FREE_FORM_OPTIONS], [${opari2_fix_and_free_form_options}])
]) # SCOREP_VENDOR_EXTERNALS_OPARI2

AC_DEFUN_ONCE([SCOREP_VENDOR_EXTERNALS_OPARI2_CPPFLAGS], [
AS_IF([test -n "${scorep_opari2_bindir}"],
      [AC_SUBST([OPARI2_CPPFLAGS], ["`${scorep_opari2_bindir}/opari2-config --cflags`"])],
      [AC_SUBST([OPARI2_CPPFLAGS], ['-I$(srcdir)/../vendor/opari2/include'])])
]) # SCOREP_VENDOR_EXTERNALS_OPARI2_CPPFLAGS

# -----
# CubeW
# -----

AC_DEFUN_ONCE([SCOREP_VENDOR_EXTERNALS_CUBEW], [
scorep_cubew_config_backend_arg=""
AS_IF([test "x${ac_scorep_cross_compiling}" = xyes],
    [scorep_cubew_config_backend_arg="--target=backend"
     AS_IF([test "x${ac_scorep_platform}" = xmic],
         [scorep_cubew_config_backend_arg="--target=mic"])])
AS_IF([test "x${scorep_cubew_bindir}" != x],
    [cubew_config_cmd="${scorep_cubew_bindir}/cubew-config ${scorep_cubew_config_backend_arg}"
     AC_SUBST([CUBEW_LIBS], [$($cubew_config_cmd --libs)])
     AC_SUBST([CUBEW_LDFLAGS], [$($cubew_config_cmd --ltldflags)])
     AC_SUBST([CUBEW_CPPFLAGS], [$($cubew_config_cmd --cppflags)])],
    [# CUBEW_* Makefile variables are evaluated by calling cubew-config,
     # once it has been installed. When used earlier, a compile/link will fail.
     m4_define([CUBEW_CONFIG], [\$(DESTDIR)\$(bindir)/cubew-config])dnl
     AC_SUBST([CUBEW_LIBS], ["\$\$(if test -x \"CUBEW_CONFIG\"; then echo \$\$(CUBEW_CONFIG \$\${scorep_cubew_config_backend_arg} --libs); else echo \"-lCUBEW_LIBS_not_provided_yet\"; fi)"])
     AC_SUBST([CUBEW_LDFLAGS], ["\$\$(if test -x \"CUBEW_CONFIG\"; then echo \$\$(CUBEW_CONFIG \$\${scorep_cubew_config_backend_arg} --ltldflags | sed 's|-L|-L\$(DESTDIR)|g;s|-R|-R\$(DESTDIR)|g'); else echo \"-LCUBEW_LDFLAGS_not_provided_yet -RCUBEW_LDFLAGS_not_provided_yet\"; fi)"])
     AC_SUBST([CUBEW_CPPFLAGS], ["\$\$(if test -x \"CUBEW_CONFIG\"; then echo \$\$(CUBEW_CONFIG \$\${scorep_cubew_config_backend_arg} --cppflags | sed 's|-I|-I\$(DESTDIR)|g'); else echo \"-ICUBEW_CPPFLAGS_not_provided_yet\"; fi)"])
     m4_undefine([CUBEW_CONFIG])])
AM_CONDITIONAL([HAVE_SCOREP_EXTERNAL_CUBEW], [test "x${scorep_cubew_bindir}" != x])
]) # SCOREP_VENDOR_EXTERNALS_CUBEW

# -------
# CubeLib
# -------

AC_DEFUN_ONCE([SCOREP_VENDOR_EXTERNALS_CUBELIB_GET_COMPILERS], [
# Get CC and CXX (without leading whitespace) that was used to build
# cubelib. Might contain language and architecture options like
# '-std=c++11' or '-q64'.
# Cubelib's CC and CXX need to be known at configure time. Obtaining
# CPPFLAGS, LDFLAGS, and LIBS can be postponed to make time, see
# SCOREP_VENDOR_EXTERNALS_CUBELIB and build-score/Makefile.am
AS_IF([test "x${scorep_cubelib_bindir}" != x],
    [# Get from external cube installation
     cubelib_config_cmd="${scorep_cubelib_bindir}/cubelib-config"
     CC="`$cubelib_config_cmd --cc | ${SED} 's/^[ \t]*//'`"
     CXX="`$cubelib_config_cmd --cxx | ${SED} 's/^[ \t]*//'`"],
    [# Get from internal cubelib
     CC="`GREP_OPTIONS= ${GREP} ^CC= ../vendor/cubelib/build-frontend/config.log | ${AWK} -F\' '{gsub(/^ +/,"",$2); print $2}'`"
     CXX="`GREP_OPTIONS= ${GREP} ^CXX= ../vendor/cubelib/build-frontend/config.log | ${AWK} -F\' '{gsub(/^ +/,"",$2); print $2}'`"])
AM_CONDITIONAL([HAVE_SCOREP_EXTERNAL_CUBELIB], [test "x${scorep_cubelib_bindir}" != x])
]) # SCOREP_VENDOR_EXTERNALS_CUBELIB_GET_COMPILERS

AC_DEFUN([SCOREP_VENDOR_EXTERNALS_CUBELIB], [
m4_ifndef([SCOREP_SCORE],
    [m4_fatal([Invoke SCOREP_VENDOR_EXTERNALS_CUBELIB from build-score only.])])

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
]) # SCOREP_VENDOR_EXTERNALS_CUBELIB
