/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 * @ingroup    POSIX_IO_Wrapper
 *
 * @brief Registration of POSIX I/O regions
 */

#ifdef __PGI
#define restrict
#endif

#include <config.h>

#include "scorep_posix_io.h"

#include <SCOREP_Libwrap_Internal.h>
#include <SCOREP_IoManagement.h>

#include <stdio.h>

#define SCOREP_POSIX_IO_PROCESS_FUNC( PARADIGM, TYPE, return_type, func, func_args ) \
    SCOREP_RegionHandle SCOREP_LIBWRAP_REGION_HANDLE( func ) = SCOREP_INVALID_REGION; \
    SCOREP_LIBWRAP_DEFINE_ORIGINAL_HANDLE( func );

#include "scorep_posix_io_function_list.inc.c"


static SCOREP_LibwrapHandle*          posix_io_libwrap_handle;
static const SCOREP_LibwrapAttributes posix_io_libwrap_attributes =
{
    SCOREP_LIBWRAP_VERSION,
    "posix",    /* name of the library wrapper */
    "POSIX I/O" /* display name of the library wrapper */
};


/**
 * Register POSIX I/O functions and initialize data structures
 */
void
scorep_posix_io_libwrap_init( void )
{
    SCOREP_Libwrap_Create( &posix_io_libwrap_handle,
                           &posix_io_libwrap_attributes );

    const char* paradigm_name;

#define SCOREP_POSIX_IO_PROCESS_FUNC( PARADIGM, TYPE, return_type, func, func_args ) \
    paradigm_name = SCOREP_IoMgmt_GetParadigmName( SCOREP_IO_PARADIGM_ ## PARADIGM ); \
    SCOREP_Libwrap_EnableWrapper( posix_io_libwrap_handle, \
                                  #return_type " " #func #func_args, \
                                  #func, \
                                  paradigm_name, \
                                  SCOREP_INVALID_LINE_NO, \
                                  SCOREP_PARADIGM_IO, \
                                  SCOREP_REGION_ ## TYPE, \
                                  ( void* )SCOREP_LIBWRAP_WRAPPER( func ), \
                                  &SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ), \
                                  &SCOREP_LIBWRAP_REGION_HANDLE( func ) );

#include "scorep_posix_io_function_list.inc.c"
}
