/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2014,
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
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */


/**
 * @file
 *
 * @brief Initialization of the PGI compiler adapter.
 */

#include <config.h>

#include <UTILS_Debug.h>

#include "SCOREP_Compiler_Init.h"
#include "scorep_compiler_data.h"
#include "scorep_compiler_pgi_data.h"
#include <SCOREP_Location.h>
#include <SCOREP_Memory.h>


extern size_t scorep_compiler_subsystem_id;


/* **************************************************************************************
 * Initialization / Finalization
 ***************************************************************************************/

/* Location initialization */
SCOREP_ErrorCode
scorep_compiler_init_location( struct SCOREP_Location* locationData )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "PGI compiler adapter init location!" );
    /* Create location struct */
    pgi_location_data* pgi_data = SCOREP_Location_AllocForMisc( locationData,
                                                                sizeof( *pgi_data ) );

    pgi_data->callstack_top   = 0;
    pgi_data->callstack_count = 0;

    SCOREP_Location_SetSubsystemData( locationData,
                                      scorep_compiler_subsystem_id,
                                      pgi_data );

    return SCOREP_SUCCESS;
}

/* Adapter initialization */
SCOREP_ErrorCode
scorep_compiler_init_adapter( void )
{
    if ( !scorep_compiler_initialized )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, " inititialize PGI compiler adapter!" );

        /* Initialize region mutex */
        SCOREP_MutexCreate( &scorep_compiler_region_mutex );

        /* Set flag */
        scorep_compiler_initialized = true;
    }

    return SCOREP_SUCCESS;
}

/* Adapter finalization */
void
scorep_compiler_finalize( void )
{
    /* call only, if previously initialized */
    if ( scorep_compiler_initialized )
    {
        /* Delete region mutex */
        SCOREP_MutexDestroy( &scorep_compiler_region_mutex );

        scorep_compiler_initialized = false;
        scorep_compiler_finalized   = true;
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, " finalize PGI compiler adapter!" );
    }
}
