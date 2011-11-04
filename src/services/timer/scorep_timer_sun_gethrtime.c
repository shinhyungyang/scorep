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
 * @file       scorep_timer_sun_gethrtime.c
 * @author     Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 * This is the implementation of the SCOREP_Timing.h interface that uses the
 * sun_gethrtime timer.
 */


#include <config.h>
#include "SCOREP_Timing.h"

#include <assert.h>
#include <stdbool.h>

static bool isInitialized = false;

void
SCOREP_Timer_Initialize()
{
    assert( false ); // implement me
    if ( isInitialized )
    {
        return;
    }
    isInitialized = true;
}


uint64_t
SCOREP_GetClockTicks()
{
    assert( false ); // implement me
    return 0;
}


uint64_t
SCOREP_GetClockResolution()
{
    assert( false ); // implement me
    return 0;
}


bool
SCOREP_ClockIsGlobal()
{
    return false;
}
