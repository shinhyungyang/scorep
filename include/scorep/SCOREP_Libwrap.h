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

/** @brief Opaque handle to the original function.
 * Must be compatible with gotcha_wrappee_handle_t */
typedef void* SCOREP_Libwrap_OriginalHandle;

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

#ifdef __cplusplus
};
#endif

#endif /* SCOREP_LIBWRAP_H */
