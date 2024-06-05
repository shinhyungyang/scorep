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
 * @file
 *
 *
 */


#ifndef SCOREP_LIBWRAP_INTERNAL_H
#define SCOREP_LIBWRAP_INTERNAL_H


#include <scorep/SCOREP_Libwrap_Macros.h>
#include <scorep/SCOREP_Libwrap.h>


/**
 * Create a library wrapper object based on the attributes structure.
 *
 * @param[out] handle       Score-P library wrapper object
 * @param      attributes   Attributes of the Score-P library wrapper object
 */
void
SCOREP_Libwrap_Create( SCOREP_LibwrapHandle**          handle,
                       const SCOREP_LibwrapAttributes* attributes );


/**
 * Call this after all wrappers were registered (@see SCOREP_Libwrap_RegisterWrapper)
 *
 * @param handle             Score-P library wrapper object
 */
void
SCOREP_Libwrap_Enable( SCOREP_LibwrapHandle* handle );

/**
 * Call this function to register a wrapper for one function.
 *
 * @param handle               Score-P library wrapper object
 * @param prettyName           Region display name (i.e., demangled)
 * @param symbolName           Symbol name (i.e., mangled)
 * @param file                 Source file name
 * @param line                 Line number in source file
 * @param wrapper              Function address of the wrapper
 * @param[out] originalHandle  Pointer to the orignal handle
 * @param[out] regionHandle    Pointer to the region handle
 */
void
SCOREP_Libwrap_RegisterWrapper( SCOREP_LibwrapHandle*          handle,
                                const char*                    prettyName,
                                const char*                    symbolName,
                                const char*                    file,
                                int                            line,
                                SCOREP_ParadigmType            paradigm,
                                SCOREP_RegionType              regionType,
                                void*                          wrapper,
                                SCOREP_Libwrap_OriginalHandle* originalHandle,
                                SCOREP_RegionHandle*           regionHandle );

#ifndef SCOREP_LIBWRAP_ORIGINAL

/**
 * @def SCOREP_LIBWRAP_ORIGINAL
 * The function pointer of the original function for @a func
 *
 * @param func              Function name
 */
#define SCOREP_LIBWRAP_ORIGINAL( func ) \
    ( ( SCOREP_LIBWRAP_ORIGINAL_TYPE( func )* )SCOREP_Libwrap_GetOriginal( SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ) ) )

static inline void*
SCOREP_Libwrap_GetOriginal( SCOREP_Libwrap_OriginalHandle originalHandle )
{
    void*
    gotcha_get_wrappee( void* );

    return gotcha_get_wrappee( originalHandle );
}

#endif /* SCOREP_LIBWRAP_ORIGINAL */

#endif /* SCOREP_LIBWRAP_INTERNAL_H */
