/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014, 2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @brief These macros represent the interface for library wrapping
 *        to the Score-P measurement core.
 *
 * @file
 */

#ifndef SCOREP_LIBWRAP_MACROS_H
#define SCOREP_LIBWRAP_MACROS_H

#include <stdlib.h>
#include <scorep/SCOREP_Libwrap.h>

#ifndef SCOREP_LIBWRAP_WRAPPER

/**
 * @def SCOREP_LIBWRAP_WRAPPER
 * Name of the wrapper function for symbol @a func.
 *
 * Do not prefix this symbol with 'scorep_', it needs to survive the unwinding
 * filter.
 *
 * @param func             Function name
 */
#define SCOREP_LIBWRAP_WRAPPER( func ) \
    __scorep_libwrap_wrapper__ ## func

#endif

#ifndef SCOREP_LIBWRAP_ORIGINAL_HANDLE

/**
 * @def SCOREP_LIBWRAP_ORIGINAL_HANDLE
 * The function pointer of the original function.
 *
 * @param func              Function name
 */
#define SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ) \
    scorep_libwrap_original_handle__ ## func

#endif

#ifndef SCOREP_LIBWRAP_ORIGINAL_TYPE

/**
 * @def SCOREP_LIBWRAP_ORIGINAL_TYPE
 * The function pointer typename of the original function.
 *
 * @param func              Function name
 */
#define SCOREP_LIBWRAP_ORIGINAL_TYPE( func ) \
    scorep_libwrap_original_type__ ## func ## _t

#endif

#ifndef SCOREP_LIBWRAP_REGION_HANDLE

/**
 * @def SCOREP_LIBWRAP_REGION_HANDLE
 * The name of the SCOREP_RegionHandle for the function @p func.
 *
 * @param func Function name
 */
#define SCOREP_LIBWRAP_REGION_HANDLE( func ) \
    scorep_libwrap_region__ ## func

#endif

/**
 * @internal
 * @def _SCOREP_LIBWRAP_RETTYPE
 *
 * Resolves the parentheses of the provided rettype of @a _SCOREP_LIBWRAP_FUNC_TYPE
 */
#define _SCOREP_LIBWRAP_RETTYPE( ... ) __VA_ARGS__

/**
 * @internal
 * @def _SCOREP_LIBWRAP_FUNC_TYPE
 * Produces a function declaration.
 *
 * @param rettype           Function return type in parentheses.
 * @param name              A name
 * @param argtypes          Function arguments in parentheses
 */
#define _SCOREP_LIBWRAP_FUNC_TYPE( rettype, name, argtypes ) \
    _SCOREP_LIBWRAP_RETTYPE rettype name argtypes

/**
 * @def SCOREP_LIBWRAP_DECLARE_ORIGINAL_TYPE
 * Declares the wrapper function.
 *
 * @param rettype           Function return type in parentheses.
 * @param func              Function name
 * @param argtypes          Function arguments in parentheses
 */
#define SCOREP_LIBWRAP_DECLARE_ORIGINAL_TYPE( rettype, func, argtypes ) \
    typedef _SCOREP_LIBWRAP_FUNC_TYPE ( rettype, ( SCOREP_LIBWRAP_ORIGINAL_TYPE( func ) ), argtypes )

/**
 * @def SCOREP_LIBWRAP_DECLARE_WRAPPER_SPECIFIER
 * The default storage specifier for the wrapper-func declaration is empty.
 * Overwrite this macro if you want a different, like `static`.
 */
#ifndef SCOREP_LIBWRAP_DECLARE_WRAPPER_SPECIFIER

#define SCOREP_LIBWRAP_DECLARE_WRAPPER_SPECIFIER

#endif

/**
 * @def SCOREP_LIBWRAP_DECLARE_WRAPPER
 * Declares the wrapper function.
 *
 * @param rettype           Function return type in parentheses.
 * @param func              Function name
 * @param argtypes          Function arguments in parentheses
 */
#define SCOREP_LIBWRAP_DECLARE_WRAPPER( func ) \
    SCOREP_LIBWRAP_DECLARE_WRAPPER_SPECIFIER SCOREP_LIBWRAP_ORIGINAL_TYPE( func ) SCOREP_LIBWRAP_WRAPPER( func )

/**
 * @def SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE_SPECIFIER
 * The default storage specifier for the function pointer declaration is 'extern'.
 * Overwrite this macro if you want a different.
 */
#ifndef SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE_SPECIFIER

#define SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE_SPECIFIER extern

#endif

/**
 * @def SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE
 * Declares the function pointer for the original function.
 *
 * @param func              Function name
 */
#define SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE( func ) \
    SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE_SPECIFIER SCOREP_Libwrap_OriginalHandle SCOREP_LIBWRAP_ORIGINAL_HANDLE( func )

/**
 * @def SCOREP_LIBWRAP_DEFINE_ORIGINAL_HANDLE
 * Defines the handle to the original function, initialized with NULL.
 *
 * @param func              Function name
 */
#define SCOREP_LIBWRAP_DEFINE_ORIGINAL_HANDLE( func ) \
    SCOREP_Libwrap_OriginalHandle SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ) = NULL

/** @def SCOREP_LIBWRAP_NULL
 *  NULL handle within Score-P library wrapper
 */
#define SCOREP_LIBWRAP_NULL NULL

#endif /* SCOREP_LIBWRAP_MACROS_H */
