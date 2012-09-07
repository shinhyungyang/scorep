/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
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
 * @file       src/measurement/scorep_environment.h
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


#ifndef SCOREP_INTERNAL_ENVIRONMENT_H
#define SCOREP_INTERNAL_ENVIRONMENT_H


#include <stdbool.h>
#include <stdint.h>

UTILS_BEGIN_C_DECLS


void
SCOREP_RegisterAllConfigVariables( void );


//bool
//SCOREP_Env_CoreEnvironmentVariablesInitialized();


bool
SCOREP_Env_RunVerbose();


bool
SCOREP_Env_DoTracing();


bool
SCOREP_Env_DoProfiling();


uint64_t
SCOREP_Env_GetTotalMemory();


uint64_t
SCOREP_Env_GetPageSize();


const char*
SCOREP_Env_ExperimentDirectory();


bool
SCOREP_Env_OverwriteExperimentDirectory();


UTILS_END_C_DECLS


#endif /* SCOREP_INTERNAL_ENVIRONMENT_H */
