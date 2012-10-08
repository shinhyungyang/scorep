/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 *    RWTH Aachen University, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */


/**
 * @status     alpha
 * @file       SCOREP_Compiler_Init.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief Compiler adapter interface support to the measurement system.
 *
 * This file contains compiler adapter initialization and finalization functions
 * which are common for all compiler adapters.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include "SCOREP_Compiler_Init.h"
#include "SCOREP_Types.h"
#include <SCOREP_Location.h>
#include "SCOREP_Config.h"
#define SCOREP_DEBUG_MODULE_NAME COMPILER
#include <UTILS_Debug.h>


#include "scorep_compiler_confvars.inc.c"

/**
   The adapter initialize function is compiler specific. Thus it is contained in each
   compiler adapter implementation.
 */
extern SCOREP_ErrorCode
scorep_compiler_init_adapter();

/**
   The location init function is compiler specific. Thus it is contained in each
   compiler adapter implementation.
 */
extern SCOREP_ErrorCode
scorep_compiler_init_location( SCOREP_Location* location );

/**
   The location finalize function is compiler specific. Thus it is contained in each
   compiler adapter implementation.
 */
extern void
scorep_compiler_finalize_location( SCOREP_Location* location );

/**
   The adapter finalize function is compiler specific. Thus it is contained in each
   compiler adapter implementation.
 */
extern void
scorep_compiler_finalize();

/* Implementation of the compiler adapter initialization/finalization struct */
const SCOREP_Subsystem SCOREP_Compiler_Adapter =
{
    "COMPILER",
    &scorep_compiler_register,
    &scorep_compiler_init_adapter,
    &scorep_compiler_init_location,
    &scorep_compiler_finalize_location,
    &scorep_compiler_finalize,
    &scorep_compiler_deregister
};
