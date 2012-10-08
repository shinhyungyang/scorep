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
 * @file        scorep_platform_bgq.c
 * @maintainer  Alexandre Strube <a.strube@fz-juelich.de>
 *
 * @status alpha
 *
 * Implementation to obtain the system tree information from a Blue Gene/Q
 * system.
 */


#include <config.h>

#include <UTILS_Error.h>
#include <SCOREP_Platform.h>
#include "scorep_platform_system_tree.h"


SCOREP_ErrorCode
SCOREP_Platform_GetPathInSystemTree( SCOREP_Platform_SystemTreePathElement** root )
{
    if ( !root )
    {
        return UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                            "Invalid system tree root reference given." );
    }
    *root = NULL;
    SCOREP_Platform_SystemTreePathElement** tail = root;
    SCOREP_Platform_SystemTreePathElement*  node;

    node = scorep_platform_system_tree_top_down_add( &tail,
                                                     "machine",
                                                     0, "Blue Gene/Q" );
    if ( !node )
    {
        goto fail;
    }

    return SCOREP_SUCCESS;

fail:
    SCOREP_Platform_FreePath( *root );

    return UTILS_ERROR( SCOREP_ERROR_PROCESSED_WITH_FAULTS,
                        "Failed to build system tree path" );
}
