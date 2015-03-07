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
 * @ingroup OPARI2
 *
 * @brief Implementation of the OPARI2 user adapter functions.
 */

#include <config.h>

#include "SCOREP_Opari2_User_Regions.h"

#include <SCOREP_Events.h>
#include <SCOREP_RuntimeManagement.h>

#define SCOREP_DEBUG_MODULE_NAME OPARI2
#include <UTILS_Debug.h>

#include <opari2/pomp2_user_lib.h>

/** @ingroup OPARI2
    @{
 */

/**
   @def SCOREP_OPARI2_USER_ENSURE_INITIALIZED
   Checks whether opari2-user adapter is initialized and if not initializes
   the measurement system.
 */
#define SCOREP_OPARI2_USER_ENSURE_INITIALIZED \
    if ( !scorep_opari2_user_is_initialized ) \
    {                                         \
        SCOREP_InitMeasurement();             \
    }
extern bool scorep_opari2_user_is_initialized;

/* **************************************************************************************
 *                                                               OPARI2 event functions *
 ***************************************************************************************/

void
POMP2_Begin( POMP2_USER_Region_handle* pomp_handle,
             const char                ctc_string[] )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPARI2, "In POMP2_Begin" );
    SCOREP_OPARI2_USER_ENSURE_INITIALIZED;
    if ( scorep_opari2_recording_on )
    {
        SCOREP_OPARI2_USER_HANDLE_UNITIALIZED_REGION( pomp_handle, ctc_string );
        SCOREP_Opari2_User_Region* region = *( SCOREP_Opari2_User_Region** )pomp_handle;
        SCOREP_EnterRegion( region->regionHandle );
    }
}

void
POMP2_End( POMP2_USER_Region_handle* pomp_handle )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPARI2, "In POMP2_End" );
    SCOREP_OPARI2_USER_ENSURE_INITIALIZED;
    if ( scorep_opari2_recording_on )
    {
        SCOREP_Opari2_User_Region* region = *( SCOREP_Opari2_User_Region** )pomp_handle;
        SCOREP_ExitRegion( region->regionHandle );
    }
}

void
POMP2_Init( void )
{
    UTILS_DEBUG_ENTRY();

    /* If adapter is not initialized, it means that the measurement
       system is not initialized. */
    SCOREP_InitMeasurement();

    UTILS_DEBUG_EXIT();
}

void
POMP2_Finalize( void )
{
    UTILS_DEBUG_ENTRY();
    UTILS_DEBUG_EXIT();
}

void
POMP2_On( void )
{
    SCOREP_OPARI2_USER_ENSURE_INITIALIZED;
    scorep_opari2_recording_on = true;
}

void
POMP2_Off( void )
{
    SCOREP_OPARI2_USER_ENSURE_INITIALIZED;
    scorep_opari2_recording_on = false;
}

/** @} */
