/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
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
 *
 */


#include <config.h>


#include <UTILS_Error.h>


#include <UTILS_Debug.h>


#include "scorep_ipc.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

bool
SCOREP_Status_IsMpp( void )
{
    return false;
}


bool
scorep_create_experiment_dir( void ( * createDir )( void ) )
{
    createDir();
    return true;
}

void
scorep_timing_reduce_runtime_management_timings( void )
{
}
