/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 *  @file
 *
 *  This file contains the implementation of the initialization functions of the
 *  HIP adapter.
 */

#include <config.h>

#include "scorep_hip_init.h"

#include <SCOREP_Location.h>

#define SCOREP_DEBUG_MODULE_NAME HIP
#include <UTILS_Debug.h>

#include "scorep_hip.h"

#include "scorep_hip_confvars.inc.c"

size_t scorep_hip_subsystem_id = 0;

/**
 * Registers the required configuration variables of the HIP adapter to the
 * measurement system.
 *
 * @param subsystemId   ID of the subsystem
 *
 * @return SCOREP_SUCCESS on success, otherwise a error code is returned
 */
static SCOREP_ErrorCode
subsystem_register( size_t subsystemId )
{
    UTILS_DEBUG( "Register environment variables" );

    scorep_hip_subsystem_id = subsystemId;

    return SCOREP_ConfigRegister( "hip", scorep_hip_confvars );
}

/**
 * Initializes the HIP subsystem.
 *
 * @return SCOREP_SUCCESS on success, otherwise a error code is returned
 */
static SCOREP_ErrorCode
subsystem_init( void )
{
    return SCOREP_SUCCESS;
}

static SCOREP_ErrorCode
subsystem_begin( void )
{
    return SCOREP_SUCCESS;
}

static void
subsystem_end( void )
{
}

static void
subsystem_finalize( void )
{
}

static SCOREP_ErrorCode
subsystem_init_location( SCOREP_Location* location,
                         SCOREP_Location* parent )
{
    return SCOREP_SUCCESS;
}

static SCOREP_ErrorCode
subsystem_pre_unify( void )
{
    return SCOREP_SUCCESS;
}

static SCOREP_ErrorCode
subsystem_post_unify( void )
{
    return SCOREP_SUCCESS;
}

const SCOREP_Subsystem SCOREP_Subsystem_HipAdapter =
{
    .subsystem_name          = "HIP",
    .subsystem_register      = &subsystem_register,
    .subsystem_begin         = &subsystem_begin,
    .subsystem_end           = &subsystem_end,
    .subsystem_init          = &subsystem_init,
    .subsystem_init_location = &subsystem_init_location,
    .subsystem_finalize      = &subsystem_finalize,
    .subsystem_pre_unify     = &subsystem_pre_unify,
    .subsystem_post_unify    = &subsystem_post_unify
};
