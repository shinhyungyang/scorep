/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2012,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Muenchen, Germany
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


/**
   Contains the name of the executable.
 */
char* scorep_compiler_executable = NULL;

/**
   Configuration variables for the compiler adapter.
   Current configuration variables are:
   @li executable: Executable of the application. Preferrably, with full path. It is used
                   by some compiler adapters (GNU) to evaluate the symbol table.
 */
SCOREP_ConfigVariable scorep_compiler_configs[] = {
    {
        "executable",
        SCOREP_CONFIG_TYPE_STRING,
        &scorep_compiler_executable,
        NULL,
        "",
        "Executable of the application",
        "File name, preferrrably with full path, of the application's executable.\n"
        "It is used for evaluating the symbol table of the application, which is\n"
        "required by some compiler adapters."
    },
    SCOREP_CONFIG_TERMINATOR
};


size_t scorep_compiler_subsystem_id;

/**
   Registers configuration variables for the compiler adapters.
 */
static SCOREP_ErrorCode
scorep_compiler_register( size_t subsystem_id )
{
    UTILS_DEBUG_ENTRY();

    scorep_compiler_subsystem_id = subsystem_id;

    return SCOREP_ConfigRegister( "", scorep_compiler_configs );
}

/**
   Called on dereigstration of the compiler adapter. Currently, no action is performed
   on deregistration.
 */
static void
scorep_compiler_deregister()
{
    UTILS_DEBUG_ENTRY();

    free( scorep_compiler_executable );
}
