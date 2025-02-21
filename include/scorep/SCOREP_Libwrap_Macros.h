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

#ifndef SCOREP_LIBWRAP_FUNC_NAME

/**
 * @def SCOREP_LIBWRAP_FUNC_NAME
 * Name of the wrapper function for symbol @a func.
 *
 * Do not prefix this symbol with 'scorep_', it needs to survive the unwinding
 * filter.
 *
 * @param func             Function name
 */
#define SCOREP_LIBWRAP_FUNC_NAME( func ) \
    __scorep_libwrap_wrapper__ ## func

#endif

#ifndef SCOREP_LIBWRAP_FUNC_REAL_NAME

/**
 * @def SCOREP_LIBWRAP_FUNC_REAL_NAME
 * The function pointer of the original function.
 *
 * @param func              Function name
 */
#define SCOREP_LIBWRAP_FUNC_REAL_NAME( func ) \
    scorep_libwrap_funcptr__ ## func

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

#ifndef SCOREP_LIBWRAP_API

extern const SCOREP_LibwrapAPI scorep_libwrap_plugin_api;

#define SCOREP_LIBWRAP_API( api ) \
    scorep_libwrap_plugin_api.api

#endif

/** @def SCOREP_LIBWRAP_NULL
 *  NULL handle within Score-P library wrapper
 */
#define SCOREP_LIBWRAP_NULL NULL

#define SCOREP_LIBWRAP_INIT( handle, attributes )                           \
    do                                                                      \
    {                                                                       \
        if ( ( handle ) == SCOREP_LIBWRAP_NULL )                            \
        {                                                                   \
            SCOREP_LIBWRAP_API( create )( &( handle ), &( attributes ) );   \
        }                                                                   \
    }                                                                       \
    while ( 0 )

/**
 * @def SCOREP_LIBWRAP_ENTER_MEASUREMENT
 * Entering the wrapper function
 */
#define SCOREP_LIBWRAP_ENTER_MEASUREMENT() \
    int scorep_libwrap_var_previous; \
    int scorep_libwrap_var_trigger = SCOREP_LIBWRAP_API( enter_measurement )()

/**
 * @def SCOREP_LIBWRAP_EXIT_MEASUREMENT
 * Exiting the wrapper function
 */
#define SCOREP_LIBWRAP_EXIT_MEASUREMENT() \
    SCOREP_LIBWRAP_API( exit_measurement )()

/**
 * @def SCOREP_LIBWRAP_ENTER_WRAPPED_REGION
 *      Transition from wrapper to wrappee
 */
#define SCOREP_LIBWRAP_ENTER_WRAPPED_REGION() \
    scorep_libwrap_var_previous = SCOREP_LIBWRAP_API( enter_wrapped_region )()

/**
 * @def SCOREP_LIBWRAP_EXIT_WRAPPED_REGION
 *      Transition from wrappee to wrapper
 */
#define SCOREP_LIBWRAP_EXIT_WRAPPED_REGION() \
    SCOREP_LIBWRAP_API( exit_wrapped_region )( scorep_libwrap_var_previous )

/**
 * @def SCOREP_LIBWRAP_FUNC_ENTER
 * Write event for entering wrapped function
 *
 */
#define SCOREP_LIBWRAP_FUNC_ENTER( func )                                               \
    do                                                                                  \
    {                                                                                   \
        if ( scorep_libwrap_var_trigger )                                               \
        {                                                                               \
            SCOREP_LIBWRAP_API( enter_region )( SCOREP_LIBWRAP_REGION_HANDLE( func ) ); \
        }                                                                               \
    }                                                                                   \
    while ( 0 )

/**
 * @def SCOREP_LIBWRAP_FUNC_EXIT
 * Write event for leaving wrapped function
 *
 */
#define SCOREP_LIBWRAP_FUNC_EXIT( func )                                                \
    do                                                                                  \
    {                                                                                   \
        if ( scorep_libwrap_var_trigger )                                               \
        {                                                                               \
            SCOREP_LIBWRAP_API( exit_region )( SCOREP_LIBWRAP_REGION_HANDLE( func ) );  \
        }                                                                               \
    }                                                                                   \
    while ( 0 )

/** @internal
 *  @def _SCOREP_LIBWRAP_RETTYPE
 *
 *  Resolves the parentheses of the provided rettype of @a SCOREP_LIBWRAP_FUNC_TYPE
 */
#define _SCOREP_LIBWRAP_RETTYPE( ... ) __VA_ARGS__

/**
 * @def SCOREP_LIBWRAP_FUNC_TYPE
 * Produces a function declaration.
 *
 * @param rettype           Function return type in parentheses.
 * @param name              A name
 * @param argtypes          Function arguments in parentheses
 */
#define SCOREP_LIBWRAP_FUNC_TYPE( rettype, name, argtypes ) \
    _SCOREP_LIBWRAP_RETTYPE rettype name argtypes

/**
 * @def SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC_SPECIFIER
 * The default storage specifier for the wrapper-func declaration is empty.
 * Overwrite this macro if you want a different, like `static`.
 */
#ifndef SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC_SPECIFIER

#define SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC_SPECIFIER

#endif

/**
 * @def SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC
 * Declares the wrapper function.
 *
 * @param rettype           Function return type in parentheses.
 * @param func              Function name
 * @param argtypes          Function arguments in parentheses
 */
#define SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC( rettype, func, argtypes ) \
    SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC_SPECIFIER SCOREP_LIBWRAP_FUNC_TYPE( rettype, SCOREP_LIBWRAP_FUNC_NAME( func ), argtypes )

/**
 * @def SCOREP_LIBWRAP_DECLARE_REAL_FUNC_SPECIFIER
 * The default storage specifier for the real-func declaration is 'extern'.
 * Overwrite this macro if you want a different.
 */
#ifndef SCOREP_LIBWRAP_DECLARE_REAL_FUNC_SPECIFIER

#define SCOREP_LIBWRAP_DECLARE_REAL_FUNC_SPECIFIER extern

#endif

/**
 * @def SCOREP_LIBWRAP_DECLARE_REAL_FUNC
 * Declares the function pointer for the original function.
 *
 * @param rettype           Function return type in parentheses.
 * @param func              Function name
 * @param argtypes          Function arguments in parentheses
 */
#define SCOREP_LIBWRAP_DECLARE_REAL_FUNC( rettype, func, argtypes ) \
    SCOREP_LIBWRAP_DECLARE_REAL_FUNC_SPECIFIER SCOREP_LIBWRAP_FUNC_TYPE( rettype, ( *SCOREP_LIBWRAP_FUNC_REAL_NAME( func )), argtypes )

/**
 * @def SCOREP_LIBWRAP_DEFINE_REAL_FUNC
 * Defines the function pointer of the original function, initialized with NULL.
 *
 * @param rettype           Function return type in parentheses.
 * @param func              Function name
 * @param argtypes          Function arguments in parentheses
 */
#define SCOREP_LIBWRAP_DEFINE_REAL_FUNC( rettype, func, argtypes ) \
    SCOREP_LIBWRAP_FUNC_TYPE( rettype, ( *SCOREP_LIBWRAP_FUNC_REAL_NAME( func )), argtypes ) = NULL

/**
 * @def SCOREP_LIBWRAP_FUNC_CALL
 * Call real function symbol
 *
 * @param func              Function name
 * @param args              Function arguments
 */
#define SCOREP_LIBWRAP_FUNC_CALL( func, args ) \
    SCOREP_LIBWRAP_FUNC_REAL_NAME( func )args

/**
 * @def SCOREP_LIBWRAP_FUNC_INIT
 * Enable wrapping of requested function. Ignoring return code.
 *
 * @param handle            Library wrapper handle
 * @param func              Function symbol
 * @param prettyname        The pretty name of the function (i.e., demangled; as `const char*`)
 * @param file              Source code location (file as `const char*`)
 * @param line              Source code location (line as `int`)
 */
#define SCOREP_LIBWRAP_FUNC_INIT( handle, func, prettyname, file, line )                        \
    do                                                                                          \
    {                                                                                           \
        SCOREP_LIBWRAP_API( enable_wrapper )( handle,                                           \
                                              prettyname, #func, file, line,                    \
                                              ( void* )SCOREP_LIBWRAP_FUNC_NAME( func ),        \
                                              ( void** )&SCOREP_LIBWRAP_FUNC_REAL_NAME( func ), \
                                              &SCOREP_LIBWRAP_REGION_HANDLE( func ) );          \
    }                                                                                           \
    while ( 0 )

#endif /* SCOREP_LIBWRAP_MACROS_H */
