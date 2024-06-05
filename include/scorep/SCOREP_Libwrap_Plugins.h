/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2015, 2017, 2020, 2025,
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

#include <scorep/SCOREP_Libwrap.h>

#include <scorep/SCOREP_PublicTypes.h>

#ifndef SCOREP_LIBWRAP_PLUGINS_H
#define SCOREP_LIBWRAP_PLUGINS_H

#ifdef __cplusplus
extern "C"
{
#endif

/** Data structure for the library wrapper plug-in API */
typedef struct SCOREP_LibwrapAPI SCOREP_LibwrapAPI;
struct SCOREP_LibwrapAPI
{
    /**
     * Create a library wrapper object based on the attributes structure.
     *
     * @param[out] handle       Score-P library wrapper object
     * @param      attributes   Attributes of the Score-P library wrapper object
     */
    void
    ( * create )( SCOREP_LibwrapHandle**          handle,
                  const SCOREP_LibwrapAttributes* attributes );

    /**
     * Call this function to register a wrapper for one function.
     *
     * @param handle                  Score-P library wrapper object
     * @param prettyName              Region display name (i.e., demangled)
     * @param symbolName              Symbol name (i.e., mangled)
     * @param file                    Source file name
     * @param line                    Line number in source file
     * @param wrapper                 Function address of the wrapper
     * @param[out] originalHandleOut  Pointer to the orignal handle
     * @param[out] regionHandleOut    Pointer to the region handle
     */
    void
    ( * register_wrapper )( SCOREP_LibwrapHandle*          handle,
                            const char*                    prettyName,
                            const char*                    symbolName,
                            const char*                    file,
                            int                            line,
                            void*                          wrapper,
                            SCOREP_Libwrap_OriginalHandle* originalHandleOut,
                            SCOREP_RegionHandle*           regionHandleOut );

    void*
    ( * get_original )( SCOREP_Libwrap_OriginalHandle );

    /**
     * Enter the measurement. First action a wrapper must do.
     *
     * @return Returns true if the wrapper is allowed to call any other measurement
     *         functions (except @a SCOREP_Libwrap_ExitMeasurement).
     */
    int
    ( * enter_measurement )( void );

    /**
     * Exit the measurement. Last action (before any return statement) a wrapper must do.
     */
    void
    ( * exit_measurement )( void );

    /**
     * Wrapper to write an event for entering wrapped function.
     *
     * @param region Handle of entered region
     */
    void
    ( * enter_region )( SCOREP_RegionHandle region );

    /**
     * Wrapper to write an event for leaving wrapped function.
     *
     * @param region Handle of entered region
     */
    void
    ( * exit_region )( SCOREP_RegionHandle region );

    /**
     * Marks the transition between wrapper and original. I.e., just before the wrapper
     * calls the original.
     *
     * @return previous state, must be passed on to @a SCOREP_LibwrapAPI::exit_wrapped_region.
     */
    int
    ( * enter_wrapped_region )( void );

    /**
     * Marks the transition between wrapper and original. I.e., just before the wrapper
     * calls the original.
     *
     * @param previous The return value from @a SCOREP_LibwrapAPI::enter_wrapped_region.
     */
    void
    ( * exit_wrapped_region )( int previous );
};

#ifdef SCOREP_LIBWRAP_API

/** @def SCOREP_LIBWRAP_API( api )
 *  Define to access the libwrap plugin API passed to @a scorep_libwrap_plugin.
 *
 *  Example:
 *  @code
 *  #define SCOREP_LIBWRAP_API( api ) libwrap_plugin_api->api
 *
 *  #include <scorep/SCOREP_Libwrap_Plugins.h>
 *
 *  static const SCOREP_LibwrapAPI* libwrap_plugin_api;
 *
 *  void
 *  scorep_libwrap_plugin( const SCOREP_LibwrapAPI* const libwrapAPI,
 *                         size_t                         libwrapAPISize )
 *  {
 *      if ( libwrapAPISize < sizeof( *libwrap_plugin_api ) )
 *      {
 *          return;
 *      }
 *      libwrap_plugin_api = libwrapAPI;
 *      ...
 *  }
 *  @endcode
 */

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
 *      Transition from wrapper to original
 */
#define SCOREP_LIBWRAP_ENTER_WRAPPED_REGION() \
    scorep_libwrap_var_previous = SCOREP_LIBWRAP_API( enter_wrapped_region )()

/**
 * @def SCOREP_LIBWRAP_EXIT_WRAPPED_REGION
 *      Transition from original to wrapper
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

/**
 * @def SCOREP_LIBWRAP_FUNC_REGISTER
 * Register wrapper for requested function.
 *
 * @param handle            Library wrapper handle
 * @param func              Function symbol
 * @param prettyname        The pretty name of the function (i.e., demangled; as `const char*`)
 * @param file              Source code location (file as `const char*`)
 * @param line              Source code location (line as `int`)
 */
#define SCOREP_LIBWRAP_FUNC_REGISTER( handle, func, prettyname, file, line )            \
    SCOREP_LIBWRAP_API( register_wrapper )( handle,                                     \
                                            prettyname, #func, file, line,              \
                                            ( void* )SCOREP_LIBWRAP_WRAPPER( func ),    \
                                            &SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ),    \
                                            &SCOREP_LIBWRAP_REGION_HANDLE( func ) )     \

/**
 * @def SCOREP_LIBWRAP_ORIGINAL
 * Get the function pointer to the original function for @a func
 *
 * @param func              Function symbol
 */
#define SCOREP_LIBWRAP_ORIGINAL( func ) \
    ( ( SCOREP_LIBWRAP_ORIGINAL_TYPE( func )* )SCOREP_LIBWRAP_API( get_original )( SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ) ) )

#endif /* !SCOREP_LIBWRAP_API */

#ifdef __cplusplus
};
#endif

#endif /* SCOREP_LIBWRAP_PLUGINS_H */
