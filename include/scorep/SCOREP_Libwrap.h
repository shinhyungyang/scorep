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
 * @brief Definitions of library wrapping functions
 *        which expands from macros
 *
 * @file
 */

#ifndef SCOREP_LIBWRAP_H
#define SCOREP_LIBWRAP_H

#include <stdint.h>
#include <scorep/SCOREP_PublicTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Version of this library API/ABI */
#define SCOREP_LIBWRAP_VERSION 2

/** @brief Keeps all necessary information of the library wrapping object. */
typedef struct SCOREP_LibwrapHandle SCOREP_LibwrapHandle;

/** Data structure for library wrapper attributes */
typedef struct SCOREP_LibwrapAttributes SCOREP_LibwrapAttributes;
struct SCOREP_LibwrapAttributes
{
    /** @brief The API/ABI version of this library wrapper, Score-P does
     *         currently supports SCOREP_LIBWRAP_VERSION */
    int version;

    /** @brief The short name of this library wrapper. Preferable the name of
     *         the target library and a valid POSIX file name. */
    const char* name;

    /** @brief The display/pretty name of the library wrapper. Could also
     *         include the target library's version. */
    const char* display_name;

    /** @brief Called by Score-P when creating the library wrapper handle via
     *         SCOREP_Libwrap_Create()/SCOREP_LibwrapAPI::create().
     *         Register all symbols in it. */
    void ( * init )( SCOREP_LibwrapHandle* libwrapHandle );
};

/** @brief Return codes when enabling a wrapper. */
typedef enum SCOREP_LibwrapEnableErrorCode
{
    SCOREP_LIBWRAP_ENABLED_ERROR_INVALID_ARGUMENTS = -2,
    SCOREP_LIBWRAP_ENABLED_ERROR_NOT_WRAPPED       = -1,
    SCOREP_LIBWRAP_ENABLED_SUCCESS                 = 0,
    SCOREP_LIBWRAP_ENABLED_FILTERED                = 1
} SCOREP_LibwrapEnableErrorCode;

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
     * Call this function to enable wrapping of one function.
     *
     * @param handle             Score-P library wrapper object
     * @param prettyName         Region display name (i.e., demangled)
     * @param symbolName         Symbol name (i.e., mangled)
     * @param file               Source file name
     * @param line               Line number in source file
     * @param wrapper            Function address of the wrapper
     * @param[out] funcPtr       Pointer to the function address of the actual/wrappee
     * @param[out] regionHandle  Pointer to the region handle
     *
     * @return                   @see SCOREP_LibwrapEnableErrorCode
     */
    SCOREP_LibwrapEnableErrorCode
    ( * enable_wrapper )( SCOREP_LibwrapHandle* handle,
                          const char*           prettyName,
                          const char*           symbolName,
                          const char*           file,
                          int                   line,
                          void*                 wrapper,
                          void**                funcPtr,
                          SCOREP_RegionHandle*  regionHandle );

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
     * Marks the transition between wrapper and wrappee. I.e., just before the wrapper
     * calls the wrappee.
     *
     * @return previous state, must be passed on to @a SCOREP_LibwrapAPI::exit_wrapped_region.
     */
    int
    ( * enter_wrapped_region )( void );

    /**
     * Marks the transition between wrapper and wrappee. I.e., just before the wrapper
     * calls the wrappee.
     *
     * @param previous The return value from @a SCOREP_LibwrapAPI::enter_wrapped_region.
     */
    void
    ( * exit_wrapped_region )( int previous );
};

#ifdef __cplusplus
};
#endif

#endif /* SCOREP_LIBWRAP_H */
