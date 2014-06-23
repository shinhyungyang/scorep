/**
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014,
 * Forschungszentrum Juelich GmbH, Germany
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

#define SCOREP_DEBUG_MODULE_NAME PTHREAD
#include <UTILS_Debug.h>

#include <stddef.h>
#include <stdbool.h>


/* *INDENT-OFF* */
static void register_pthread_regions( void );
/* *INDENT-ON* */


struct SCOREP_Location;


bool          scorep_pthread_initialized = false;
bool          scorep_pthread_finalized   = true;
static size_t subsystem_id;


static SCOREP_ErrorCode
scorep_pthread_register( size_t subsystemId )
{
    UTILS_DEBUG_ENTRY();
    subsystem_id = subsystemId;
    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
scorep_pthread_init_adapter( void )
{
    UTILS_DEBUG_ENTRY();
    if ( scorep_pthread_initialized )
    {
        return SCOREP_SUCCESS;
    }
    scorep_pthread_initialized = true;
    scorep_pthread_finalized   = false;

    register_pthread_regions();
    scorep_pthread_mutex_init();

    UTILS_DEBUG_EXIT();
    return SCOREP_SUCCESS;
}


SCOREP_RegionHandle scorep_pthread_regions[ REGION_SENTINEL - 1 ];


static void
register_pthread_regions( void )
{
    SCOREP_SourceFileHandle file = SCOREP_Definitions_NewSourceFile( "PTHREAD" );

#define PTHREAD_REGION( NAME, name, TYPE ) \
    scorep_pthread_regions[ PTHREAD_ ## NAME ] = \
        SCOREP_Definitions_NewRegion( "pthread_" name, \
                                      NULL, \
                                      file, \
                                      SCOREP_INVALID_LINE_NO, \
                                      SCOREP_INVALID_LINE_NO, \
                                      SCOREP_PARADIGM_PTHREAD, \
                                      SCOREP_REGION_ ## TYPE );

    PTHREAD_REGIONS

#undef PTHREAD_REGION

    scorep_pthread_regions[ PTHREAD_START_ROUTINE ] =
        SCOREP_Definitions_NewRegion( "Pthreads",
                                      NULL,
                                      file,
                                      SCOREP_INVALID_LINE_NO,
                                      SCOREP_INVALID_LINE_NO,
                                      SCOREP_PARADIGM_PTHREAD,
                                      SCOREP_REGION_WRAPPER );
}


static void
scorep_pthread_finalize( void )
{
    UTILS_DEBUG_ENTRY();
    if ( scorep_pthread_finalized || !scorep_pthread_initialized )
    {
        return;
    }
    scorep_pthread_finalized   = true;
    scorep_pthread_initialized = false;
    scorep_pthread_mutex_finalize();
    UTILS_DEBUG_EXIT();
}


static void
scorep_pthread_deregister( void )
{
    UTILS_DEBUG_ENTRY();
}


static SCOREP_ErrorCode
scorep_pthread_init_location( struct SCOREP_Location* locationData )
{
    UTILS_DEBUG_ENTRY();
    return SCOREP_SUCCESS;
}


static void
scorep_pthread_finalize_location( struct SCOREP_Location* locationData )
{
    UTILS_DEBUG_ENTRY();
}


/* Implementation of the pthread adapter initialization/finalization struct */
const SCOREP_Subsystem SCOREP_Subsystem_PthreadAdapter =
{
    .subsystem_name              = "PTHREAD",
    .subsystem_register          = &scorep_pthread_register,
    .subsystem_init              = &scorep_pthread_init_adapter,
    .subsystem_init_location     = &scorep_pthread_init_location,
    .subsystem_finalize_location = &scorep_pthread_finalize_location,
    .subsystem_pre_unify         = NULL,
    .subsystem_post_unify        = NULL,
    .subsystem_finalize          = &scorep_pthread_finalize,
    .subsystem_deregister        = &scorep_pthread_deregister,
    .subsystem_control           = NULL
};
