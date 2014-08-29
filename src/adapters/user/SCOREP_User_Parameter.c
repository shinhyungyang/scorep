/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011, 2013,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 *  @file
 *
 *  This file contains the implementation of user adapter functions concerning
 *  parameters.
 */

#include <config.h>
#include <scorep/SCOREP_User_Functions.h>
#include "SCOREP_User_Init.h"
#include <SCOREP_Definitions.h>
#include <SCOREP_Events.h>
#include <SCOREP_Types.h>

#define SCOREP_PARAMETER_TO_USER( handle ) ( ( uint64_t )( handle ) )
#define SCOREP_PARAMETER_FROM_USER( handle ) ( ( SCOREP_ParameterHandle )( handle ) )

void
SCOREP_User_ParameterInt64
(
    SCOREP_User_ParameterHandle* handle,
    const char*                  name,
    int64_t                      value
)
{
    /* Check for initialization */
    SCOREP_USER_ASSERT_INITIALIZED;

    if ( handle == NULL )
    {
        return;
    }

    /* Initialize parameter handle if necessary */
    if ( *handle == SCOREP_USER_INVALID_PARAMETER )
    {
        *handle = SCOREP_PARAMETER_TO_USER( SCOREP_Definitions_NewParameter( name, SCOREP_PARAMETER_INT64 ) );
    }

    /* Trigger event */
    SCOREP_TriggerParameterInt64( SCOREP_PARAMETER_FROM_USER( *handle ), value );
}

void
SCOREP_User_ParameterUint64
(
    SCOREP_User_ParameterHandle* handle,
    const char*                  name,
    uint64_t                     value
)
{
    /* Check for initialization */
    SCOREP_USER_ASSERT_INITIALIZED;

    if ( handle == NULL )
    {
        return;
    }

    /* Initialize parameter handle if necessary */
    if ( *handle == SCOREP_USER_INVALID_PARAMETER )
    {
        *handle = SCOREP_PARAMETER_TO_USER( SCOREP_Definitions_NewParameter( name, SCOREP_PARAMETER_UINT64 ) );
    }

    /* Trigger event */
    SCOREP_TriggerParameterUint64( SCOREP_PARAMETER_FROM_USER( *handle ), value );
}


void
SCOREP_User_ParameterString
(
    SCOREP_User_ParameterHandle* handle,
    const char*                  name,
    const char*                  value
)
{
    /* Check for initialization */
    SCOREP_USER_ASSERT_INITIALIZED;

    if ( handle == NULL )
    {
        return;
    }

    /* Initialize parameter handle if necessary */
    if ( *handle == SCOREP_USER_INVALID_PARAMETER )
    {
        *handle = SCOREP_PARAMETER_TO_USER( SCOREP_Definitions_NewParameter( name, SCOREP_PARAMETER_STRING ) );
    }

    /* Trigger event */
    SCOREP_TriggerParameterString( SCOREP_PARAMETER_FROM_USER( *handle ), value );
}
