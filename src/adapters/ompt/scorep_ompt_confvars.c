/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 */

#include <config.h>

#define SCOREP_DEBUG_MODULE_NAME OMPT
#include <UTILS_Debug.h>

#include <SCOREP_Subsystem.h>
#include <SCOREP_Config.h>

#include "scorep_ompt_confvars.inc.c"

/**
 * Registers the required configuration variables of the OMPT adapter
 * to the measurement system.
 */
static SCOREP_ErrorCode
scorep_ompt_register( size_t subsystemId )
{
    UTILS_DEBUG_ENTRY( "Register environment variables" );

    const SCOREP_ErrorCode host_register_result = SCOREP_ConfigRegisterCond( "openmp",
        scorep_ompt_confvars,
        HAVE_BACKEND_SCOREP_OMPT_SUPPORT );
    if ( host_register_result != SCOREP_SUCCESS )
    {
        return host_register_result;
    }

    return SCOREP_ConfigRegisterCond( "openmp",
                                      scorep_ompt_target_confvars,
                                      HAVE_BACKEND_SCOREP_OMPT_SUPPORT );
}

const SCOREP_Subsystem SCOREP_Subsystem_OmptAdapter =
{
    .subsystem_name     = "OMPT",
    .subsystem_register = &scorep_ompt_register,
};
