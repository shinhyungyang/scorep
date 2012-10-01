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
 * @file       scorep_compiler_ibm.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief Support for XL IBM-Compiler
 * Will be triggered by the frunctrion trace option by the xl
 * compiler.
 */

#include <config.h>
#include <stdio.h>
#include <string.h>

#include <UTILS_Error.h>
#include <UTILS_Debug.h>
#include <UTILS_IO.h>
#include <SCOREP_Location.h>
#include <SCOREP_Events.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Mutex.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Filter.h>

#include <SCOREP_Compiler_Init.h>
#include <scorep_compiler_data.h>

#if __IBMC__ > 1100
#define SCOREP_FILTER_REGION ( SCOREP_INVALID_REGION - 1 )
#endif

/**
 * static variable to control initialize status of compiler adapter.
 */
static int scorep_compiler_initialize = 1;

/**
 * flag that indicates whether the compiler adapter is finalized
 */
static int scorep_compiler_finalized = 0;

/**
 * Mutex for exclusive access to the region hash table.
 */
static SCOREP_Mutex scorep_compiler_region_mutex;

/**
 * Looks up the region name in the hash table, registers the region
 * if it is not alread registered and returns the region handle.
 * If the region is filtered it returns SCOREP_INVALID_REGION.
 * @ param region_name function name
 * @ param file_name   file name
 * @ param line_no     line number
 */
static inline SCOREP_RegionHandle
get_region_handle( char* region_name,
                   char* file_name,
                   int   line_no )
{
    scorep_compiler_hash_node* hash_node;

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER,
                        " function name: %s %s", region_name, file_name );

    if ( scorep_compiler_initialize )
    {
        if ( scorep_compiler_finalized )
        {
            return SCOREP_INVALID_REGION;
        }
        SCOREP_InitMeasurement();
    }

    /* put function to list */
    if ( ( hash_node = scorep_compiler_hash_get( ( long )region_name ) ) == 0 )
    {
        /* The IBM compiler instruments outlined functions of OpenMP parallel regions.
           These functions are called at a stage, where locks do not yet work. Thus,
           make sure that in case of race conditions, functions can only get filtered.
         */
        SCOREP_MutexLock( scorep_compiler_region_mutex );
        if ( ( hash_node = scorep_compiler_hash_get( ( long )region_name ) ) == 0 )
        {
            char* file = UTILS_CStr_dup( file_name );
            UTILS_IO_SimplifyPath( file );
            hash_node = scorep_compiler_hash_put( ( long )region_name,
                                                  region_name,
                                                  region_name,
                                                  file, line_no );
            UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER,
                                " number %ld and put name -- %s -- to list",
                                ( long )region_name, region_name );

            /* Check for filters:
                 1. In case OpenMP is used, the XL compiler creates some
                    functions like <func_name>:<func_name>$OL$OL.1 on BG/P or
                    <func_name>@OL@1 on AIX which cause the measurement system
                    to crash. Thus, filter functions which names contain a '$'
                    or '@' symbol.
                 2. POMP and POMP2 functions.
             */
            bool is_filtered = true;
            if ( ( strchr( region_name, '$' ) == NULL ) &&
                 ( strchr( region_name, '@' ) == NULL ) &&
                 ( strncmp( region_name, "POMP", 4 ) != 0 ) &&
                 ( strncmp( region_name, "Pomp", 4 ) != 0 ) &&
                 ( strncmp( region_name, "pomp", 4 ) != 0 ) &&
                 !SCOREP_Filter_Match( file, region_name, true ) )
            {
                is_filtered = false;
            }

            /* If not filtered register region */
            if ( !is_filtered )
            {
                scorep_compiler_register_region( hash_node );
            }
            else
            {
                hash_node->region_handle = SCOREP_INVALID_REGION;
            }
            free( file );
        }
        SCOREP_MutexUnlock( scorep_compiler_region_mutex );
    }
    return hash_node->region_handle;
}

/**
 * @ brief This function is called at the entry of each function.
 * The call is generated by the IBM xl compilers
 *
 * @ param region_name function name
 * @ param file_name   file name
 * @ param line_no     line number
 */
#if __IBMC__ > 1100
void
__func_trace_enter( char*                region_name,
                    char*                file_name,
                    int                  line_no,
                    SCOREP_RegionHandle* handle )
{
    /* The IBM compiler instruments outlined functions of OpenMP parallel regions.
       These functions are called at a stage, where locks do not yet work. Thus,
       make sure that only final valid values are assigned to *handle.
     */
    if ( *handle == 0 )
    {
        SCOREP_RegionHandle region = get_region_handle( region_name, file_name, line_no );
        if ( region == SCOREP_INVALID_REGION )
        {
            *handle = SCOREP_FILTER_REGION;
        }
        else
        {
            *handle = region;
        }
    }
    if ( *handle != SCOREP_FILTER_REGION )
    {
        SCOREP_EnterRegion( *handle );
    }
}
#else
void
__func_trace_enter( char* region_name,
                    char* file_name,
                    int   line_no )
{
    SCOREP_RegionHandle handle = get_region_handle( region_name, file_name, line_no );
    if ( handle != SCOREP_INVALID_REGION )
    {
        SCOREP_EnterRegion( handle );
    }
}
#endif

/**
 * @ brief This function is called at the exit of each function.
 * The call is generated by the IBM xl compilers
 *
 * @ param region_name function name
 * @ param file_name   file name
 * @ param line_no     line number
 */
#if __IBMC__ > 1100
void
__func_trace_exit( char*                region_name,
                   char*                file_name,
                   int                  line_no,
                   SCOREP_RegionHandle* handle )
{
    if ( *handle != SCOREP_FILTER_REGION )
    {
        SCOREP_ExitRegion( *handle );
    }
}
#else
void
__func_trace_exit( char* region_name,
                   char* file_name,
                   int   line_no )
{
    scorep_compiler_hash_node* hash_node;
    if ( scorep_compiler_finalized )
    {
        return;
    }
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "call function exit!!!" );
    if ( hash_node = scorep_compiler_hash_get( ( long )region_name ) )
    {
        /* Invalid handle marks filtered regions */
        if ( ( hash_node->region_handle != SCOREP_INVALID_REGION ) )
        {
            SCOREP_ExitRegion( hash_node->region_handle );
        }
    }
}
#endif

/* Initialize adapter */
SCOREP_ErrorCode
scorep_compiler_init_adapter()
{
    if ( scorep_compiler_initialize )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER,
                            " inititialize IBM xl compiler adapter!" );

        /* Initialize region mutex */
        SCOREP_MutexCreate( &scorep_compiler_region_mutex );

        /* Initialize hash table */
        scorep_compiler_hash_init();

        /* Sez flag */
        scorep_compiler_initialize = 0;
    }
    return SCOREP_SUCCESS;
}

SCOREP_ErrorCode
scorep_compiler_init_location( SCOREP_Location* locationData )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "IBM xl compiler adapter init location!" );
    return SCOREP_SUCCESS;
}

/* Location finalization */
void
scorep_compiler_finalize_location( SCOREP_Location* locationData )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "IBM xlcompiler adapter finalize location!" );
}

/* Finalize adapter */
void
scorep_compiler_finalize()
{
    /* call only, if previously initialized */
    if ( !scorep_compiler_initialize )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, " finalize IBM xl compiler adapter!" );

        /* Delete hash table */
        scorep_compiler_hash_free();

        /* Set flag to not initialized */
        scorep_compiler_initialize = 1;
        scorep_compiler_finalized  = 1;

        /* Delete region mutex */
        SCOREP_MutexDestroy( &scorep_compiler_region_mutex );
    }
}
