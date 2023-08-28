/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016, 2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 * @ingroup    OpenCL_Wrapper
 *
 * @brief Basic setup for the library wrapping.
 */

#ifndef SCOREP_OPENCL_LIBWRAP_H
#define SCOREP_OPENCL_LIBWRAP_H

#ifdef __APPLE__
    #include "OpenCL/opencl.h"
#else
    #include "CL/cl.h"
#endif

#include <SCOREP_Definitions.h>

/*
 * Do not prefix this symbol with 'scorep_', it needs to survive the unwinding
 * filter.
 */
#define SCOREP_LIBWRAP_ORIGINAL_TYPE( func ) \
    scorep_opencl_original_type__ ## func ## _t

#define SCOREP_LIBWRAP_WRAPPER( func ) \
    __scorep_opencl_wrapper__ ## func

#define SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ) \
    scorep_opencl_original_handle__ ## func

#define SCOREP_LIBWRAP_REGION_HANDLE( func ) \
    scorep_opencl_region__ ## func

#include <scorep/SCOREP_Libwrap_Macros.h>

/*
 * - Declaration of Score-P region handles for wrapped OpenCL functions
 */
#define SCOREP_OPENCL_PROCESS_FUNC( TYPE, return_type, func, func_args ) \
    extern SCOREP_RegionHandle SCOREP_LIBWRAP_REGION_HANDLE( func ); \
    SCOREP_LIBWRAP_DECLARE_ORIGINAL_TYPE( ( return_type ), func, func_args ); \
    SCOREP_LIBWRAP_DECLARE_WRAPPER( func ); \
    SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE( func );
#include "scorep_opencl_function_list.inc.c"

void
scorep_opencl_libwrap_init( void );

#endif /* SCOREP_OPENCL_LIBWRAP_H */
