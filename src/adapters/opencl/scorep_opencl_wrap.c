/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014-2015, 2017, 2025,
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
 * @brief C interface wrappers for OpenCL routines
 */

#include <config.h>

#include "scorep_opencl_libwrap.h"

#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Events.h>
#include <SCOREP_Libwrap_Internal.h>

#include "scorep_opencl.h"
#include "scorep_opencl_config.h"
#include "scorep_opencl_libwrap.h"

#include <stdio.h>


#define SCOREP_OPENCL_FUNC_ENTER( func ) \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        if ( scorep_opencl_record_api ) \
        { \
            SCOREP_EnterWrappedRegion( SCOREP_LIBWRAP_REGION_HANDLE( func ) ); \
        } \
        else if ( SCOREP_IsUnwindingEnabled() ) \
        { \
            SCOREP_EnterWrapper( SCOREP_LIBWRAP_REGION_HANDLE( func ) ); \
        } \
    }
/* *INDENT-ON* */

#define SCOREP_OPENCL_FUNC_EXIT( func ) \
    if ( trigger && SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        if ( scorep_opencl_record_api ) \
        { \
            SCOREP_ExitRegion( SCOREP_LIBWRAP_REGION_HANDLE( func ) ); \
        } \
        else if ( SCOREP_IsUnwindingEnabled() ) \
        { \
            SCOREP_ExitWrapper( SCOREP_LIBWRAP_REGION_HANDLE( func ) ); \
        } \
    } \
    SCOREP_IN_MEASUREMENT_DECREMENT();

#define SCOREP_OPENCL_WRAP_ENTER() SCOREP_ENTER_WRAPPED_REGION()

#define SCOREP_OPENCL_WRAP_EXIT() SCOREP_EXIT_WRAPPED_REGION()

/**
 * Handles wrapped OpenCL data transfer functions. This macro returns the
 * function that uses it! It evaluates the return value of the given function
 * 'call' and uses it in the return statement.
 *
 * @param func                  function name to wrap
 * @param direction             data transfer direction
 * @param bytes                 number of bytes to be transfered
 * @param clQueue               OpenCL command queue
 * @param clEvt                 address of the OpenCL profiling event
 * @param isBlocking            true, if function is blocking, otherwise false
 * @param args                  args for the 'real' function call
 */
#define SCOREP_OPENCL_ENQUEUE_BUFFER( func, direction, bytes, clQueue,         \
                                      clEvt, isBlocking, args )                \
    {                                                                          \
        SCOREP_OPENCL_FUNC_ENTER( func );                                      \
                                                                               \
        scorep_opencl_queue*        queue;                                     \
        scorep_opencl_buffer_entry* mcpy;                                      \
        if ( trigger                                                           \
             && SCOREP_IS_MEASUREMENT_PHASE( WITHIN )                          \
             && scorep_opencl_record_memcpy )                                  \
        {                                                                      \
            queue = scorep_opencl_queue_get( clQueue );                        \
            mcpy  = scorep_opencl_get_buffer_entry( queue );                   \
                                                                               \
            if ( mcpy && clEvt == NULL )                                       \
            {                                                                  \
                clEvt = &( mcpy->event );                                      \
            }                                                                  \
        }                                                                      \
                                                                               \
        SCOREP_OPENCL_WRAP_ENTER();                                            \
        cl_int ret = SCOREP_LIBWRAP_ORIGINAL( func )args;                      \
        SCOREP_OPENCL_WRAP_EXIT();                                             \
                                                                               \
        if ( trigger                                                           \
             && SCOREP_IS_MEASUREMENT_PHASE( WITHIN )                          \
             && scorep_opencl_record_memcpy )                                  \
        {                                                                      \
            if ( queue && mcpy && CL_SUCCESS == ret )                          \
            {                                                                  \
                scorep_opencl_retain_buffer( queue, mcpy, direction, bytes );  \
            }                                                                  \
        }                                                                      \
                                                                               \
        SCOREP_OPENCL_FUNC_EXIT( func );                                       \
                                                                               \
        return ret;                                                            \
    }


#if HAVE( OPENCL_VERSION_1_0_SUPPORT )
#include "scorep_opencl_wrap_version_1_0.inc.c"
#endif
#if HAVE( OPENCL_VERSION_1_1_SUPPORT )
#include "scorep_opencl_wrap_version_1_1.inc.c"
#endif
#if HAVE( OPENCL_VERSION_1_2_SUPPORT )
#include "scorep_opencl_wrap_version_1_2.inc.c"
#endif
#if HAVE( OPENCL_VERSION_2_0_SUPPORT )
#include "scorep_opencl_wrap_version_2_0.inc.c"
#endif
#if HAVE( OPENCL_VERSION_2_1_SUPPORT )
#include "scorep_opencl_wrap_version_2_1.inc.c"
#endif
#if HAVE( OPENCL_VERSION_2_2_SUPPORT )
#include "scorep_opencl_wrap_version_2_2.inc.c"
#endif
