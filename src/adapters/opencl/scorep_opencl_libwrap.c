/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014, 2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 * @ingroup    OpenCL_Wrapper
 *
 * @brief Registration of OpenCL function pointers
 */

#include <config.h>

#include "scorep_opencl_libwrap.h"

#include <scorep/SCOREP_Libwrap.h>

#include <stdio.h>


#define SCOREP_OPENCL_PROCESS_FUNC( TYPE, return_type, func, func_args ) \
    SCOREP_RegionHandle scorep_opencl_region__ ## func = SCOREP_INVALID_REGION; \
    SCOREP_LIBWRAP_DEFINE_REAL_FUNC( ( return_type ), func, func_args );
#include "scorep_opencl_function_list.inc.c"


static const char*                    wrapped_lib_name = "libOpenCL.so.1";
static SCOREP_LibwrapHandle*          opencl_libwrap_handle;
static const SCOREP_LibwrapAttributes opencl_libwrap_attributes =
{
    SCOREP_LIBWRAP_VERSION,
    "opencl",                      /* name of the library wrapper */
    "OpenCL",                      /* display name of the library wrapper */
    SCOREP_LIBWRAP_MODE_SHARED,    /* library wrapper mode */
    NULL,
    1,                             /* number of wrapped libraries */
    &wrapped_lib_name              /* name of wrapped library */
};


/**
 * Register OpenCL functions and initialize data structures
 */
void
scorep_opencl_libwrap_init( void )
{
    SCOREP_Libwrap_Create( &opencl_libwrap_handle,
                           &opencl_libwrap_attributes );

    SCOREP_SourceFileHandle source_file = SCOREP_Definitions_NewSourceFile( "OpenCL" );

#define SCOREP_OPENCL_PROCESS_FUNC( TYPE, return_type, func, func_args ) \
    scorep_opencl_region__ ## func = SCOREP_Definitions_NewRegion( #return_type " " #func #func_args, \
                                                                   #func, \
                                                                   source_file, \
                                                                   SCOREP_INVALID_LINE_NO, \
                                                                   SCOREP_INVALID_LINE_NO, \
                                                                   SCOREP_PARADIGM_OPENCL, \
                                                                   SCOREP_REGION_ ## TYPE ); \
    SCOREP_Libwrap_SharedPtrInit( opencl_libwrap_handle, #func, \
                                  ( void** )( &SCOREP_LIBWRAP_FUNC_REAL_NAME( func ) ) );
#include "scorep_opencl_function_list.inc.c"
}
