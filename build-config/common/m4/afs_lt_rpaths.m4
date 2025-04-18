## -*- mode: autoconf -*-

##
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2023-2024,
## Forschungszentrum Juelich GmbH, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##

# AFS_LT_RPATHS()
# ---------------
#
# Define filenames for a Makefile snippet and a config-header, both
# containing library-linking information. The Makefile snippet gets generated
# by config.status via AC_CONFIG_COMMANDS.
#
# The config-header contains preprocessor defines and a single
# function providing information about libdirs and libs needed to link
# our installed libraries (in a non-libtool way). Intended to be used
# by the config-tools.
#
# The Makefile snippet gets generated by config.status via
# AC_CONFIG_COMMANDS. It is used to export noinst LDFLAGS and LIBS
# from build-<foo> to be used in build-<bar>. Include in build<bar>'s
# Makefile via '-include ../build-<foo>/Makefile.export' (the leading
# hyphen guarantees progress even if the file doesn't exist).
#
# Intended to be used in conjunction with Makefile-rpaths.inc.am.
#
AC_DEFUN_ONCE([AFS_LT_RPATHS], [
# Needed to perform shared-objects-self-contained check only if we can use ldd.
AC_REQUIRE([AFS_CPU_INSTRUCTION_SETS])

AC_SUBST([CONFIG_EXTERNAL_LIBS_HPP], AFS_PACKAGE_TO_TOP[src/config-external-libs-]AFS_PACKAGE_BUILD_name[.hpp])
AC_SUBST([CONFIG_EXTERNAL_LIBS_HPP_INCLUDE_GUARD], [CONFIG_EXTERNAL_LIBS_]AFS_PACKAGE_BUILD_NAME[_HPP])

# Compile-time system search path for libraries: use e.g., to filter
# -L paths in config-tools.
AC_SUBST([SYS_LIB_SEARCH_PATH_SPEC], [${sys_lib_search_path_spec}])
# Detected run-time system search path for libraries: use e.g., to
# filter -R/-Wl,-rpath paths in config-tools.
AC_SUBST([SYS_LIB_DLSEARCH_PATH_SPEC], [${sys_lib_dlsearch_path_spec}])

AC_CONFIG_COMMANDS([Makefile.export], [make Makefile.export || exit 1])
]) # AFS_LT_RPATHS
