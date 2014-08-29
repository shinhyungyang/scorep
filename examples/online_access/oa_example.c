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

/* *INDENT-OFF* */
#include <config.h>
#include <stdio.h>

#include "SCOREP_User.h"

int
main( int    argc,
      char** argv )
{
    int	retVal = 0; /* return value */
    int k;

    for(k=0;k<30;k++)
    {
        SCOREP_USER_REGION_DEFINE( mainRegion );
        SCOREP_USER_ONLINE_ACCESS_PHASE_BEGIN( mainRegion, "mainRegion", SCOREP_USER_REGION_TYPE_COMMON);

        printf("OA_TEST_C: Inside Online Access phase\n");

        SCOREP_USER_ONLINE_ACCESS_PHASE_END( mainRegion );

    }

    return retVal;
}
