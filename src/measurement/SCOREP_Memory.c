/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012, 2014,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2012,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
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
#include <SCOREP_Memory.h>
#include <UTILS_Error.h>
#include <SCOREP_Mutex.h>
#include "scorep_environment.h"
#include "scorep_location.h"
#include "scorep_status.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/* *INDENT-OFF* */
/* *INDENT-ON* */


static SCOREP_Mutex memory_lock;


/// @todo implement memory statistics


/// The one and only allocator for the measurement and the adapters
static SCOREP_Allocator_Allocator* allocator;
static size_t                      total_memory;

static bool is_initialized;

static SCOREP_Allocator_PageManager* definitions_page_manager;

void
SCOREP_Memory_Initialize( size_t totalMemory,
                          size_t pageSize )
{
    assert( totalMemory >= pageSize );

    if ( is_initialized )
    {
        return;
    }
    is_initialized = true;

    SCOREP_MutexCreate( &memory_lock );

    assert( allocator == 0 );
    allocator = SCOREP_Allocator_CreateAllocator(
        totalMemory,
        pageSize,
        ( SCOREP_Allocator_Guard )SCOREP_MutexLock,
        ( SCOREP_Allocator_Guard )SCOREP_MutexUnlock,
        ( SCOREP_Allocator_GuardObject )memory_lock );

    if ( !allocator )
    {
        SCOREP_MutexDestroy( &memory_lock );
        is_initialized = false;
        assert( false );
    }

    assert( definitions_page_manager == 0 );
    definitions_page_manager = SCOREP_Allocator_CreatePageManager( allocator );
    if ( !definitions_page_manager )
    {
        SCOREP_MutexDestroy( &memory_lock );
        is_initialized = false;
        SCOREP_Memory_HandleOutOfMemory();
    }

    total_memory = totalMemory;
}


void
SCOREP_Memory_Finalize( void )
{
    if ( !is_initialized )
    {
        return;
    }
    is_initialized = false;

    assert( definitions_page_manager );
#if defined ( __INTEL_COMPILER ) && ( __INTEL_COMPILER < 1120 )
    // Do nothing here. Intel OpenMP RTL shuts down at the end of main
    // function, so omp_set/unset_lock, which is called after the end
    // of main from the atexit handler, causes segmentation fault. The
    // problem will be fixed in  Intel Compiler 11.1 update 6.
    // See http://software.intel.com/en-us/forums/showpost.php?p=110592
#else
    SCOREP_Allocator_DeletePageManager( definitions_page_manager );
#endif
    definitions_page_manager = 0;

    assert( allocator );
    SCOREP_Allocator_DeleteAllocator( allocator );
    allocator = 0;

    SCOREP_MutexDestroy( &memory_lock );
}

void
SCOREP_Memory_HandleOutOfMemory( void )
{
    UTILS_ERROR( SCOREP_ERROR_MEMORY_OUT_OF_PAGES,
                 "Out of memory. Please increase SCOREP_TOTAL_MEMORY=%zu and try again.",
                 total_memory );
    if ( SCOREP_Env_DoTracing() )
    {
        UTILS_ERROR( SCOREP_ERROR_MEMORY_OUT_OF_PAGES,
                     "Please ensure that there are at least 2MB available for each intended thread." );
        const char* omp_num_threads = getenv( "OMP_NUM_THREADS" );
        if ( omp_num_threads && strlen( omp_num_threads ) )
        {
            UTILS_ERROR( SCOREP_ERROR_MEMORY_OUT_OF_PAGES,
                         "And you seem to have requested %s threads for the measurement.",
                         omp_num_threads );
        }
    }
    abort();
}

SCOREP_Allocator_PageManager*
SCOREP_Memory_CreateTracingPageManager( void )
{
    SCOREP_Allocator_PageManager* page_manager =
        SCOREP_Allocator_CreatePageManager( allocator );
    if ( !page_manager )
    {
        /* aborts */
        SCOREP_Memory_HandleOutOfMemory();
    }
    return page_manager;
}


void
SCOREP_Memory_CreatePageManagers( SCOREP_Allocator_PageManager** pageManagers )
{
    for ( int i = 0; i < SCOREP_NUMBER_OF_MEMORY_TYPES; ++i )
    {
        if ( i == SCOREP_MEMORY_TYPE_PROFILING && !SCOREP_IsProfilingEnabled() )
        {
            pageManagers[ i ] = 0;
            continue;
        }
        pageManagers[ i ] =
            SCOREP_Allocator_CreatePageManager( allocator );
        if ( !pageManagers[ i ] )
        {
            /* aborts */
            SCOREP_Memory_HandleOutOfMemory();
        }
    }
}


void
SCOREP_Memory_DeletePageManagers( SCOREP_Allocator_PageManager** pageManagers )
{
    // is there a need to free pages before deleting them?
    for ( int i = 0; i < SCOREP_NUMBER_OF_MEMORY_TYPES; ++i )
    {
        if ( pageManagers[ i ] )
        {
#if defined ( __INTEL_COMPILER ) && ( __INTEL_COMPILER < 1120 )
            // Do nothing here. Intel OpenMP RTL shuts down at the end of main
            // function, so omp_set/unset_lock, which is called after the end
            // of main from the atexit handler, causes segmentation fault. The
            // problem will be fixed in  Intel Compiler 11.1 update 6.
            // See http://software.intel.com/en-us/forums/showpost.php?p=110592
#else
            SCOREP_Allocator_DeletePageManager( pageManagers[ i ] );
#endif
        }
    }
}


void*
SCOREP_Location_AllocForMisc( SCOREP_Location* locationData, size_t size )
{
    // collect statistics
    if ( size == 0 )
    {
        return NULL;
    }

    void* mem = SCOREP_Allocator_Alloc(
        SCOREP_Location_GetMemoryPageManager( locationData,
                                              SCOREP_MEMORY_TYPE_MISC ),
        size );
    if ( !mem )
    {
        /* aborts */
        SCOREP_Memory_HandleOutOfMemory();
    }
    return mem;
}


void*
SCOREP_Memory_AllocForMisc( size_t size )
{
    return SCOREP_Location_AllocForMisc( SCOREP_Location_GetCurrentCPULocation(),
                                         size );
}

static bool
free_memory_type_for_location( SCOREP_Location* location,
                               void*            arg )
{
    SCOREP_MemoryType type = *( SCOREP_MemoryType* )arg;
    SCOREP_Allocator_Free( SCOREP_Location_GetMemoryPageManager( location, type ) );
    return false;
}

void
SCOREP_Memory_FreeMiscMem( void )
{
    // print mem usage statistics
    SCOREP_MemoryType type = SCOREP_MEMORY_TYPE_MISC;
    SCOREP_Location_ForAll( free_memory_type_for_location, &type );
}


void*
SCOREP_Location_AllocForProfile( SCOREP_Location* location, size_t size )
{
    // collect statistics
    if ( size == 0 )
    {
        return NULL;
    }

    void* mem = SCOREP_Allocator_Alloc(
        SCOREP_Location_GetMemoryPageManager( location,
                                              SCOREP_MEMORY_TYPE_PROFILING ),
        size );
    if ( !mem )
    {
        /* aborts */
        SCOREP_Memory_HandleOutOfMemory();
    }
    return mem;
}


void
SCOREP_Memory_FreeProfileMem( void )
{
    // print mem usage statistics
    SCOREP_MemoryType type = SCOREP_MEMORY_TYPE_PROFILING;
    SCOREP_Location_ForAll( free_memory_type_for_location, &type );
}


SCOREP_Allocator_MovableMemory
SCOREP_Memory_AllocForDefinitions( SCOREP_Location* location,
                                   size_t           size )
{
    // collect statistics
    if ( size == 0 )
    {
        return SCOREP_MOVABLE_NULL;
    }

    SCOREP_Allocator_PageManager* page_manager = definitions_page_manager;
    if ( location )
    {
        page_manager = SCOREP_Location_GetMemoryPageManager(
            location,
            SCOREP_MEMORY_TYPE_DEFINITIONS );
    }

    SCOREP_Allocator_MovableMemory mem =
        SCOREP_Allocator_AllocMovable( page_manager, size );
    if ( mem == SCOREP_MOVABLE_NULL )
    {
        /* aborts */
        SCOREP_Memory_HandleOutOfMemory();
    }
    return mem;
}


void
SCOREP_Memory_FreeDefinitionMem( void )
{
    // print mem usage statistics
    SCOREP_Allocator_Free( definitions_page_manager );
}


void*
SCOREP_Memory_GetAddressFromMovableMemory( SCOREP_Allocator_MovableMemory movableMemory,
                                           SCOREP_Allocator_PageManager*  movablePageManager )
{
    return SCOREP_Allocator_GetAddressFromMovableMemory(
               movablePageManager,
               movableMemory );
}


SCOREP_Allocator_PageManager*
SCOREP_Memory_CreateMovedPagedMemory( void )
{
    return SCOREP_Allocator_CreateMovedPageManager( allocator );
}

SCOREP_Allocator_PageManager*
SCOREP_Memory_GetLocalDefinitionPageManager( void )
{
    assert( is_initialized );
    return definitions_page_manager;
}
