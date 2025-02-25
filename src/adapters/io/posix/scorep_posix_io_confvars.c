/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file
 *
 *
 */

#include <config.h>

#define SCOREP_DEBUG_MODULE_NAME IO
#include <UTILS_Debug.h>

#include <SCOREP_Subsystem.h>
#include <SCOREP_Config.h>

#include "scorep_posix_io_confvars.inc.c"

/**
 * Registers the required configuration variables of the POSIX I/O adapter
 * to the measurement system.
 *
 * @param subsystemId       Subsystem identifier
 *
 * @return SCOREP_SUCCESS if successful, otherwise an error code
 *         is returned
 */
static SCOREP_ErrorCode
subsystem_register( size_t subsystemId )
{
    UTILS_DEBUG( "Register environment variables" );

    return SCOREP_ConfigRegisterCond( "io",
                                      scorep_posix_io_confvars,
                                      HAVE_BACKEND_POSIX_IO_SUPPORT );
}

const SCOREP_Subsystem SCOREP_Subsystem_PosixIoAdapter =
{
    .subsystem_name     = "POSIX IO (config variables only)",
    .subsystem_register = subsystem_register
};
