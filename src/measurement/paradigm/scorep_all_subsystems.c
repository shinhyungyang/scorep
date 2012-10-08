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
 * @file        src/measurement/paradigm/scorep_all_subsystems.c
 * @maintainer  Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @status      alpha
 *
 * @brief       Holds the list of adapters linked into the measurement system.
 */


#include <config.h>

#include <scorep_subsystem.h>

#include <SCOREP_Metric.h>
#include <SCOREP_Compiler_Init.h>
#include <SCOREP_User_Init.h>
#include <SCOREP_Pomp_Init.h>
#include <SCOREP_Mpi_Init.h>
#include <SCOREP_Cuda_Init.h>

/**
 * List of subsystems.
 */
const SCOREP_Subsystem* scorep_subsystems[] = {
    &SCOREP_Metric_Service,
    &SCOREP_Compiler_Adapter,
    &SCOREP_User_Adapter,
    &SCOREP_Pomp_Adapter,
    &SCOREP_Mpi_Adapter,
    &SCOREP_Cuda_Adapter,
};

const size_t scorep_number_of_subsystems = sizeof( scorep_subsystems ) /
                                           sizeof( scorep_subsystems[ 0 ] );
