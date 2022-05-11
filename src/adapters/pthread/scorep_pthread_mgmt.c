/**
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014-2016, 2018,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2014-2015, 2017, 2020, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */


/**
 * @file
 *
 * @brief Pthread adapter interface support to the measurement system.
 *
 * This file contains pthread adapter initialization and finalization functions.
 */

#include <config.h>

#include "scorep_pthread.h"
#include "scorep_pthread_mutex.h"

#include <SCOREP_Subsystem.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Paradigms.h>
#include <SCOREP_Memory.h>
#include <SCOREP_Libwrap_Internal.h>

#define SCOREP_DEBUG_MODULE_NAME PTHREAD
#include <UTILS_Debug.h>

#include <stddef.h>
#include <stdbool.h>


// function pointers

#define SCOREP_PTHREAD_REGION( rettype, name, NAME, TYPE, ARGS ) \
    SCOREP_LIBWRAP_DEFINE_ORIGINAL( ( rettype ), name, ARGS );

SCOREP_PTHREAD_REGIONS

#undef SCOREP_PTHREAD_REGION

/* *INDENT-OFF* */
static void enable_pthread_wrapper( void );
/* *INDENT-ON* */


struct SCOREP_Location;


size_t    scorep_pthread_subsystem_id;
pthread_t scorep_pthread_main_thread;


static SCOREP_LibwrapHandle*          pthread_libwrap_handle;
static const SCOREP_LibwrapAttributes pthread_libwrap_attributes =
{
    SCOREP_LIBWRAP_VERSION,
    "pthread", /* name of the library wrapper */
    "POSIX Threads"
};

static SCOREP_ErrorCode
pthread_subsystem_register( size_t subsystemId )
{
    UTILS_DEBUG_ENTRY();

    scorep_pthread_subsystem_id = subsystemId;

    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
pthread_subsystem_init( void )
{
    UTILS_DEBUG_ENTRY();

    SCOREP_Paradigms_RegisterParallelParadigm(
        SCOREP_PARADIGM_PTHREAD,
        SCOREP_PARADIGM_CLASS_THREAD_CREATE_WAIT,
        "Pthread",
        SCOREP_PARADIGM_FLAG_NONE );

    enable_pthread_wrapper();

    scorep_pthread_main_thread = pthread_self();

    UTILS_DEBUG_EXIT();
    return SCOREP_SUCCESS;
}


SCOREP_RegionHandle scorep_pthread_regions[ SCOREP_PTHREAD_REGION_SENTINEL ];


static void
enable_pthread_wrapper( void )
{
    SCOREP_Libwrap_Create( &pthread_libwrap_handle,
                           &pthread_libwrap_attributes );

#define SCOREP_PTHREAD_REGION( rettype, name, NAME, TYPE, ARGS ) \
    if ( 0 != SCOREP_Libwrap_EnableWrapper( pthread_libwrap_handle, \
                                            #rettype " " #name #ARGS, \
                                            #name, \
                                            "PTHREAD", \
                                            SCOREP_INVALID_LINE_NO, \
                                            SCOREP_PARADIGM_PTHREAD, \
                                            SCOREP_REGION_ ## TYPE, \
                                            ( void* )SCOREP_LIBWRAP_WRAPPER( name ), \
                                            ( void** )&SCOREP_LIBWRAP_ORIGINAL( name ), \
                                            &scorep_pthread_regions[ SCOREP_PTHREAD_ ## NAME ] ) ) \
    { \
        UTILS_FATAL( "Could not enable wrapping for function '" #name "'" ); \
    }

    SCOREP_PTHREAD_REGIONS

#undef SCOREP_PTHREAD_REGION
}


static void
pthread_subsystem_finalize( void )
{
    UTILS_DEBUG_ENTRY();

    UTILS_DEBUG_EXIT();
}


static SCOREP_ErrorCode
pthread_subsystem_init_location( struct SCOREP_Location* locationData,
                                 struct SCOREP_Location* parent )
{
    UTILS_DEBUG_ENTRY();

    struct scorep_pthread_location_data* data =
        SCOREP_Location_AllocForMisc( locationData, sizeof( *data ) );
    data->wrapped_arg = NULL;
    data->free_list   = NULL;

    SCOREP_Location_SetSubsystemData( locationData,
                                      scorep_pthread_subsystem_id,
                                      data );

    return SCOREP_SUCCESS;
}


/* Implementation of the pthread adapter initialization/finalization struct */
const SCOREP_Subsystem SCOREP_Subsystem_PthreadAdapter =
{
    .subsystem_name          = "PTHREAD",
    .subsystem_register      = &pthread_subsystem_register,
    .subsystem_init          = &pthread_subsystem_init,
    .subsystem_init_location = &pthread_subsystem_init_location,
    .subsystem_finalize      = &pthread_subsystem_finalize,
};
