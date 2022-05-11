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

#include <SCOREP_Libwrap_Internal.h>

#include <stdio.h>


#define SCOREP_OPENCL_PROCESS_FUNC( TYPE, return_type, func, func_args ) \
    SCOREP_RegionHandle SCOREP_LIBWRAP_REGION_HANDLE( func ) = SCOREP_INVALID_REGION; \
    SCOREP_LIBWRAP_DEFINE_ORIGINAL( ( return_type ), func, func_args );
#include "scorep_opencl_function_list.inc.c"


static SCOREP_LibwrapHandle*          opencl_libwrap_handle;
static const SCOREP_LibwrapAttributes opencl_libwrap_attributes =
{
    SCOREP_LIBWRAP_VERSION,
    "opencl",                      /* name of the library wrapper */
    "OpenCL"
};


/**
 * Register OpenCL functions and initialize data structures
 */
void
scorep_opencl_libwrap_init( void )
{
    SCOREP_Libwrap_Create( &opencl_libwrap_handle,
                           &opencl_libwrap_attributes );

#define SCOREP_OPENCL_PROCESS_FUNC( TYPE, return_type, func, func_args ) \
    SCOREP_Libwrap_EnableWrapper( opencl_libwrap_handle, \
                                  #return_type " " #func #func_args, \
                                  #func, \
                                  "OpenCL", \
                                  SCOREP_INVALID_LINE_NO, \
                                  SCOREP_PARADIGM_OPENCL, \
                                  SCOREP_REGION_ ## TYPE, \
                                  ( void* )SCOREP_LIBWRAP_WRAPPER( func ), \
                                  ( void** )&SCOREP_LIBWRAP_ORIGINAL( func ), \
                                  &SCOREP_LIBWRAP_REGION_HANDLE( func ) );
#include "scorep_opencl_function_list.inc.c"
}
