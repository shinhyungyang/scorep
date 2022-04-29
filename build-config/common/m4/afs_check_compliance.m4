## -*- mode: autoconf -*-

##
## This file is part of the Score-P software ecosystem (http://www.score-p.org)
##
## Copyright (c) 2014, 2025,
## Technische Universitaet Dresden, Germany
##
## This software may be modified and distributed under the terms of
## a BSD-style license.  See the COPYING file in the package base
## directory for details.
##

## _afs_arglist_to_args( ... )
## ---------------------------
## Converts pairs of TYPEs and NAMEs to a function argument list including
## surrounding parentheses.
## Example:
## _afs_arglist_to_args( type1, name1, type2, name2, type3, name3 )
## -> ( type1 name1, type2 name2, type3 name3 )
m4_define([_afs_args_m], [ $1, $2])dnl
m4_define([_afs_args_l], [ $1])dnl
m4_define([_afs_arglist_to_args], [( []$1[]m4_map_args_pair([_afs_args_m], [_afs_args_l], m4_shift($@))[] )])dnl

## _afs_arglist_to_declargs( ... )
## -----------------------------
## Converts pairs of TYPEs and NAMEs to a function argument declaration list
## including surrounding parentheses.
## Example:
## _afs_arglist_to_args( type1, name1, type2, name2, type3, name3 )
## -> ( type1, type2, type3 )
m4_define([_afs_declargs_m], [, $2])dnl
m4_define([_afs_declargs_l], [])dnl
m4_define([_afs_arglist_to_declargs], [( []$1[]m4_map_args_pair([_afs_declargs_m], [_afs_declargs_l], m4_shift($@))[] )])dnl

## AFS_LANG_FUNCTION( $1: PROLOG,
##                    $2: RETURN-TYPE,
##                    $3: RETURN-STMT,
##                    $4: FUNCTION-NAME,
##                    $5: FUNCTION-ARGS )
## ======================================
## Provides a stub for a function in the current language.
AC_DEFUN([AFS_LANG_FUNCTION],
[AC_LANG_SOURCE([_AC_LANG_DISPATCH([$0], _AC_LANG, $@)])])

## AFS_LANG_FUNCTION(C)( $1: PROLOG,
##                       $2: RETURN-TYPE,
##                       $3: RETURN-STMT,
##                       $4: FUNCTION-NAME,
##                       $5: FUNCTION-ARGS )
## =========================================
## Provides the stub for a C function.
m4_define([AFS_LANG_FUNCTION(C)], [dnl
$1
$2 $4$5
{
    $3;
}dnl
])])

## _AFS_CHECK_VARIANTS_REC( $1: PROLOG,
##                          $2: RETURN-TYPE,
##                          $3: RETURN-STMT,
##                          $4: FUNCTION-NAME,
##                          [$5: VARIANT-NAME, $6: VARIANT-ARGS]... )
## ==================================================================
## Recursive helper macro for AFS_CHECK_COMPLIANCE.
m4_define([_AFS_CHECK_VARIANTS_REC], [
m4_if([$#], 0, [], [$#], 1, [], [$#], 2, [], [$#], 3, [], [$#], 4, [], [$#], 5, [], [
AC_CHECK_DECLS([$4], [
    _afs_check_variants_rec_result=no
    AC_MSG_CHECKING([whether $4 is $5 variant])
    afs_save_[]_AC_LANG_ABBREV[]_werror_flag=$ac_[]_AC_LANG_ABBREV[]_werror_flag
    ac_[]_AC_LANG_ABBREV[]_werror_flag=yes
    AC_COMPILE_IFELSE([AFS_LANG_FUNCTION([$1], [$2], [$3], [$4], _afs_arglist_to_args($6))], [
        _afs_check_variants_rec_result=yes
    ], [
        _afs_check_variants_rec_result=no
    ])
    ac_[]_AC_LANG_ABBREV[]_werror_flag=$afs_save_[]_AC_LANG_ABBREV[]_werror_flag
    AC_MSG_RESULT([${_afs_check_variants_rec_result}])
    AS_IF([test "x${_afs_check_variants_rec_result}" = "xyes"], [
        AC_DEFINE(AS_TR_CPP([HAVE_]$4[_]$5[_VARIANT]), [1], [$4 is $5 variant])
        AC_DEFINE(AS_TR_CPP([AFS_PACKAGE_NAME[]_]$4[_PROTO_ARGS]), _afs_arglist_to_args($6),     [Compliant prototype arguments for $4])
        AC_DEFINE(AS_TR_CPP([AFS_PACKAGE_NAME[]_]$4[_DECL_ARGS]),  _afs_arglist_to_declargs($6), [Compliant prototype declaration arguments for $4])
    ], [
        : Iterate over next given variant
        $0([$1], [$2], [$3], [$4], m4_shiftn(6, $@))
    ])
], [], [$1])
])
])

## AFS_CHECK_COMPLIANCE( $1: PROLOG,
##                       $2: RETURN-TYPE,
##                       $3: RETURN-STMT,
##                       $4: FUNCTION-NAME,
##                       $5: COMPLIANT-ARGS,
##                       [$6: VARIANT-NAME, $7: VARIANT-ARGS]... )
## ===============================================================
## Checks if the given function comply to the definition in the system.
## If not, an arbitrary number of variants are checked too. AC_DEFINEs
## HAVE_<FUNCTION-NAME>_COMPLIANT to 1, if this function is compliant,
## else it may define the first HAVE_<FUNCTION-NAME>_<VARIANT-NAME>_VARIANT
## to 1 which matches the prototype.
## If any of the prototypes matches, it also AC_DEFINEs
## AFS_PACKAGE_NAME_<FUNCTION-NAME>_PROTO_ARGS to the matching argument list
## (types and names) and AFS_PACKAGE_NAME_<FUNCTION-NAME>_DECL_ARGS to the
## matching argument declaration list (types only).
##
## Example:
##
## AFS_CHECK_COMPLIANCE([AC_INCLUDES_DEFAULT],
##                      [int], [return 0],
##                      [mkdir], [const char*, path, mode_t, mode],
##                      [MINGW], [const char*, path])
##
## This may result in:
##
## #define HAVE_MKDIR_COMPLIANT 1
## #define AFS_PACKAGE_NAME_MKDIR_PROTO_ARGS ( const char* path, mode_t mode )
## #define AFS_PACKAGE_NAME_MKDIR_DECL_ARGS  ( const char*, mode_t )
##
## or in:
##
## #define HAVE_MKDIR_MINGW_VARIANT 1
## #define AFS_PACKAGE_NAME_MKDIR_PROTO_ARGS ( const char* path )
## #define AFS_PACKAGE_NAME_MKDIR_DECL_ARGS  ( const char* )
##
AC_DEFUN([AFS_CHECK_COMPLIANCE], [
AC_CHECK_DECLS([$4], [
    afs_check_compliance_result=no
    AC_MSG_CHECKING([whether $4 is standard compliant])
    afs_save_[]_AC_LANG_ABBREV[]_werror_flag=$ac_[]_AC_LANG_ABBREV[]_werror_flag
    ac_[]_AC_LANG_ABBREV[]_werror_flag=yes
    AC_COMPILE_IFELSE([AFS_LANG_FUNCTION([$1], [$2], [$3], [$4], _afs_arglist_to_args($5))], [
        afs_check_compliance_result=yes
    ], [
        afs_check_compliance_result=no
    ])
    ac_[]_AC_LANG_ABBREV[]_werror_flag=$afs_save_[]_AC_LANG_ABBREV[]_werror_flag
    AC_MSG_RESULT([${afs_check_compliance_result}])
    AS_IF([test "x${afs_check_compliance_result}" = "xyes"], [
        AC_DEFINE(AS_TR_CPP([HAVE_]$4[_COMPLIANT]), [1], [$4 is standard compliant])
        AC_DEFINE(AS_TR_CPP([AFS_PACKAGE_NAME[]_]$4[_PROTO_ARGS]), _afs_arglist_to_args($5),     [Compliant prototype arguments for $4])
        AC_DEFINE(AS_TR_CPP([AFS_PACKAGE_NAME[]_]$4[_DECL_ARGS]),  _afs_arglist_to_declargs($5), [Compliant prototype declaration arguments for $4])
    ], [
        : Iterate over next given variant
        _AFS_CHECK_VARIANTS_REC([$1], [$2], [$3], [$4], m4_shiftn(5, $@))
    ])
], [], [$1])
])
