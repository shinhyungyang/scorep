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

#include <SCOREP_Subsystem.h>
#include <SCOREP_Config.h>

#include "scorep_libwrap_confvars.inc.c"

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
libwrap_register( size_t subsystemId )
{
    return SCOREP_ConfigRegisterCond( "libwrap",
                                      scorep_libwrap_confvars,
                                      HAVE_BACKEND_LIBWRAP_PLUGIN_SUPPORT );
}

const SCOREP_Subsystem SCOREP_Subsystem_LibwrapService =
{
    .subsystem_name     = "LIBWRAP Service (config variables only)",
    .subsystem_register = libwrap_register
};
