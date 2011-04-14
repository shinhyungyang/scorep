## -*- mode: autoconf -*-

## 
## This file is part of the Score-P software (http://www.score-p.org)
##
## Copyright (c) 2009-2011, 
##    RWTH Aachen University, Germany
##    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
##    Technische Universitaet Dresden, Germany
##    University of Oregon, Eugene, USA
##    Forschungszentrum Juelich GmbH, Germany
##    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
##    Technische Universitaet Muenchen, Germany
##
## See the COPYING file in the package base directory for details.
##


## file       ac_scorep_compiler_and_flags.m4
## maintainer Christian Roessel <c.roessel@fz-juelich.de>

AC_DEFUN([AC_SCOREP_CONVERT_FLAGS],
[
if test "x${ac_cv_env_[$1]_FOR_BUILD_set}" != "xset"; then
   # don't use the backend flags if nothing is specified for the frontend
   unset [$1]
else
   # use the frontend flags 
   [$1]="$ac_cv_env_[$1]_FOR_BUILD_value"
fi
## ac_substituting done automagically 
##AC_SUBST([$1])
])


AC_DEFUN([AC_SCOREP_CHECK_COMPILER_VAR_SET],
[
if test "x${ac_cv_env_[$1]_set}" != "xset"; then
    AC_MSG_ERROR([argument $1 not provided in configure call.], [1])
fi
])


AC_DEFUN([AC_SCOREP_CONVERT_COMPILER],
[
if test "x${ac_cv_env_[$1]_FOR_BUILD_set}" != "xset"; then
    AC_MSG_ERROR([argument $1_FOR_BUILD not provided in configure call.], [1])    
fi
[$1]="$ac_cv_env_[$1]_FOR_BUILD_value"
])


dnl dont' use together with AC_SCOREP_WITH_NOCROSS_COMPILER_SUITE
AC_DEFUN([AC_SCOREP_WITH_COMPILER_SUITE],
[
m4_pattern_allow([AC_SCOREP_WITH_COMPILER_SUITE])
m4_pattern_allow([AC_SCOREP_WITH_NOCROSS_COMPILER_SUITE])
if test "x${ac_scorep_compiler_suite_called}" != "x"; then
    # We need m4 quoting magic here ...
    AC_MSG_ERROR([cannot use [AC_SCOREP_WITH_COMPILER_SUITE] and [AC_SCOREP_WITH_NOCROSS_COMPILER_SUITE] in one configure.ac.])
else
    ac_scorep_compiler_suite_called="yes"
fi

ac_scorep_compiler_suite_files=""
path_to_compiler_files="$srcdir/vendor/common/build-config/platforms/"

AC_ARG_WITH([nocross-compiler-suite],
            [AS_HELP_STRING([--with-nocross-compiler-suite=(gcc|ibm|intel|pathscale|pgi|studio)], 
                            [The compiler suite to build this package in non cross-compiling environments with. Needs to be in $PATH [gcc].])],
            [AS_IF([test "x${ac_scorep_cross_compiling}" = "xno"], 
                   [AS_CASE([$withval],
                            ["gcc"],       [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-gcc"],
                            ["ibm"],       [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-ibm"],
                            ["intel"],     [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-intel"],
                            ["pathscale"], [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-pathscale"],
                            ["pgi"],       [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-pgi"],
                            ["studio"],    [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-studio"],
                            [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-nocross-compiler-suite, ignoring.])])],
                   [AC_MSG_ERROR([Option --with-nocross-compiler-suite not supported in cross-compiling mode. Please use --with-backend-compiler-suite and --with-frontend-compiler-suite instead.])])],
            [])


AC_ARG_WITH([backend-compiler-suite],
            [AS_HELP_STRING([--with-backend-compiler-suite=(ibm|sx)], 
                            [The compiler suite to build the backend parts of this package in cross-compiling environments with. Needs to be in $PATH [gcc].])],
            [AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"], 
                   [AS_CASE([$withval],
                            ["ibm"],       [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-backend-ibm"],
                            ["sx"],        [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-backend-sx"],
                            [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-backend-compiler-suite, ignoring.])])], 
                   [AC_MSG_ERROR([Option --with-backend-compiler-suite not supported in non cross-compiling mode. Please use --with-nocross-compiler-suite instead.])])],
            [])


AC_ARG_WITH([frontend-compiler-suite],
            [AS_HELP_STRING([--with-frontend-compiler-suite=(gcc|ibm|intel|pathscale|pgi|studio)], 
                            [The compiler suite to build the frontend parts of this package in cross-compiling environments with. Needs to be in $PATH [gcc].])],
            [AS_IF([test "x${ac_scorep_cross_compiling}" = "xyes"], 
                   [AS_CASE([$withval],
                            ["gcc"],       [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-frontend-gcc"],
                            ["ibm"],       [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-frontend-ibm"],
                            ["intel"],     [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-frontend-intel"],
                            ["pathscale"], [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-frontend-pathscale"],
                            ["pgi"],       [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-frontend-pgi"],
                            ["studio"],    [ac_scorep_compiler_suite_files="${ac_scorep_compiler_suite_files} ${path_to_compiler_files}compiler-frontend-studio"],
                            [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-frontend-compiler-suite, ignoring.])])],
                   [AC_MSG_ERROR([Option --with-frontend-compiler-suite not supported in non cross-compiling mode. Please use --with-nocross-compiler-suite instead.])])],
            [])

#echo "ac_scorep_compiler_suite_files=${ac_scorep_compiler_suite_files}"
])


dnl dont' use together with AC_SCOREP_WITH_COMPILER_SUITE, inteded to be used by OPARI
AC_DEFUN([AC_SCOREP_WITH_NOCROSS_COMPILER_SUITE],
[
m4_pattern_allow([AC_SCOREP_WITH_COMPILER_SUITE])
m4_pattern_allow([AC_SCOREP_WITH_NOCROSS_COMPILER_SUITE])
if test "x${ac_scorep_compiler_suite_called}" != "x"; then
    # We need m4 quoting magic here ...
    AC_MSG_ERROR([cannot use [AC_SCOREP_WITH_COMPILER_SUITE] and [AC_SCOREP_WITH_NOCROSS_COMPILER_SUITE] in one configure.ac.])
else
    ac_scorep_compiler_suite_called="yes"
fi

ac_scorep_compiler_suite_files=""
path_to_compiler_files="$srcdir/vendor/common/build-config/platforms/"

AC_ARG_WITH([compiler-suite],
            [AS_HELP_STRING([--with-compiler-suite=(gcc|ibm|intel|pathscale|pgi|studio)], 
                            [The compiler suite to build this package with. Needs to be in $PATH [gcc].])],
            [AS_CASE([$withval],
                     ["gcc"],       [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-gcc"],
                     ["ibm"],       [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-ibm"],
                     ["intel"],     [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-intel"],
                     ["pathscale"], [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-pathscale"],
                     ["pgi"],       [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-pgi"],
                     ["studio"],    [ac_scorep_compiler_suite_files="${path_to_compiler_files}compiler-nocross-studio"],
                     [AC_MSG_WARN([Compiler suite "${withval}" not supported by --with-compiler-suite, ignoring.])])],
            [])
])
