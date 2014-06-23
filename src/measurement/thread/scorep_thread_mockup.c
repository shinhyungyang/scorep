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
 * Copyright (c) 2009-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */


/**
 * @file
 *
 *
 */

#include <config.h>

#include <SCOREP_Thread_Mgmt.h>
#include <scorep_location.h>

#include <UTILS_Error.h>


static SCOREP_Location* scorep_thread_sole_cpu_location;


void
SCOREP_Thread_Initialize()
{
    UTILS_ASSERT( scorep_thread_sole_cpu_location == 0 );
    scorep_thread_sole_cpu_location =
        SCOREP_Location_CreateCPULocation( NULL, "Master thread",
                                           /* deferNewLocationNotification = */ false );
    UTILS_ASSERT( scorep_thread_sole_cpu_location );
}


void
SCOREP_Thread_Finalize()
{
}


bool
SCOREP_Thread_InParallel()
{
    return false;
}


SCOREP_Location*
SCOREP_Location_GetCurrentCPULocation()
{
    return scorep_thread_sole_cpu_location;
}
