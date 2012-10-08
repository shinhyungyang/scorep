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
 * @status     alpha
 * @file       scorep_compiler_sun.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief SUN f90 compiler PHAT interface.
 * Compiler adapter for the SUN F90 compiler.
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>

#include <SCOREP_Types.h>
#include <UTILS_Error.h>
#include <UTILS_Debug.h>
#include <SCOREP_Events.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Compiler_Init.h>
#include <SCOREP_Mutex.h>
#include <SCOREP_Filter.h>

/* *INDENT-OFF* */
static int on_scorep_finalize( void );
/* *INDENT-ON* */

/**
 * @def SCOREP_SUN_INVALID_REGION
 * Defines the value with which the compiler pre-initializes the pointer to the id
 */
#define SCOREP_SUN_INVALID_REGION -1

/**
 * @def SCOREP_FILTERED_REGION
 * Constant id to mark filtered functions
 */
#define SCOREP_FILTERED_REGION SCOREP_INVALID_REGION

/* Check that we can distinguish filtered functions from uninitialized functions */
#if SCOREP_FILTERED_REGION == SCOREP_SUN_INVALID_REGION
#error "SCOREP_FILTERED_REGION must not equal SCOREP_SUN_INVALID_REGION"
#endif

/**
 * Flag that indicates that the compiler is finalized.
 */
static int scorep_compiler_finalized = 0;

/**
 * Handle for the main region, which is not instrumented by the compiler.
 */
static SCOREP_RegionHandle scorep_compiler_main_handle = SCOREP_INVALID_REGION;

/**
 * Lock used for function registration
 */
static SCOREP_Mutex scorep_compiler_hash_lock;

/**
 * Register new region with Score-P system
 * 'str' is passed in from SUN compiler
 */
static SCOREP_RegionHandle
scorep_compiler_register_region( char* region_name )
{
    SCOREP_RegionHandle handle = SCOREP_FILTERED_REGION;

    /* register region with Score-P and store region identifier */
    if ( ( strchr( region_name, '$' ) == NULL ) &&     /* SUN OMP runtime functions */
         ( strncmp( region_name, "__mt_", 5 ) != 0 ) &&
         ( strncmp( region_name, "POMP", 4 ) != 0 ) &&
         ( strncmp( region_name, "Pomp", 4 ) != 0 ) &&
         ( strncmp( region_name, "pomp", 4 ) != 0 ) &&
         ( !SCOREP_Filter_Match( NULL, region_name, true ) ) )
    {
        handle = SCOREP_DefineRegion( region_name,
                                      NULL,
                                      SCOREP_INVALID_SOURCE_FILE,
                                      SCOREP_INVALID_LINE_NO,
                                      SCOREP_INVALID_LINE_NO,
                                      SCOREP_ADAPTER_COMPILER,
                                      SCOREP_REGION_FUNCTION );
    }
    return handle;
}

/**
 * This function is called at the entry of each function
 * The call is generated by the SUN f90 compilers
 */
void
phat_enter( char* name,
            int*  id )
{
    if ( !SCOREP_IsInitialized() )
    {
        if ( scorep_compiler_finalized )
        {
            return;
        }
        SCOREP_InitMeasurement();
    }

    if ( *id == SCOREP_SUN_INVALID_REGION )
    {
        /* region entered the first time, register region */
        SCOREP_MutexLock( scorep_compiler_hash_lock );
        if ( *id == SCOREP_SUN_INVALID_REGION )
        {
            *id = scorep_compiler_register_region( name );
        }
        SCOREP_MutexUnlock( scorep_compiler_hash_lock );
    }

    if ( *id != SCOREP_FILTERED_REGION )
    {
        SCOREP_EnterRegion( *id );
    }
}


/**
 * This function is called at the exit of each function
 * The call is generated by the SUN F90 compilers
 */
void
phat_exit( char* name,
           int*  id )
{
    if ( scorep_compiler_finalized )
    {
        return;
    }

    if ( ( *id != SCOREP_SUN_INVALID_REGION ) && ( *id != SCOREP_FILTERED_REGION ) )
    {
        SCOREP_ExitRegion( *id );
    }
}

SCOREP_ErrorCode
scorep_compiler_init_adapter()
{
    SCOREP_MutexCreate( &scorep_compiler_hash_lock  );
    scorep_compiler_main_handle = scorep_compiler_register_region( "main" );
    SCOREP_RegisterExitCallback( &on_scorep_finalize );
    return SCOREP_SUCCESS;
}

SCOREP_ErrorCode
scorep_compiler_init_location( SCOREP_Location* locationData )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "studio compiler adapter init location!" );
    /* The studio compiler does not instrument "main" but we want to have a
       main. Note that this main is triggered by the first event that arrives
       at the measurement system. */
    if ( 0 == SCOREP_Location_GetId( locationData ) )
    {
        /* I would like to call SCOREP_Location_EnterRegion() here,
           but we prevent this for CPU locations. We could check
           the passed locationData against
           SCOREP_Location_GetCurrentCPULocation(). */
        SCOREP_EnterRegion( scorep_compiler_main_handle );
    }
    return SCOREP_SUCCESS;
}

/* Location finalization */
void
scorep_compiler_finalize_location( SCOREP_Location* locationData )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "studio compiler adapter finalize location!" );
}

int
on_scorep_finalize()
{
    /* We manually entered the artificial "main" region. We also need to exit
       it manually. See also scorep_compiler_init_adapter().
       Still no SCOREP_Location_ExitRegion() here.
     */
    SCOREP_ExitRegion( scorep_compiler_main_handle );
    return 0;
}

void
scorep_compiler_finalize()
{
    SCOREP_MutexDestroy( &scorep_compiler_hash_lock );
    scorep_compiler_finalized = 1;
}
