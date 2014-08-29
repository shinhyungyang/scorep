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
 *
 */


/**
 * @file
 *
 *
 */


#include <config.h>


#include <definitions/SCOREP_Definitions.h>


#include "scorep_environment.h"
#include "scorep_runtime_management.h"
#include <scorep/SCOREP_PublicTypes.h>
#include <SCOREP_Mutex.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "scorep_ipc.h"
#include "scorep_status.h"
#include <jenkins_hash.h>
#include <tracing/SCOREP_Tracing.h>

#include <UTILS_Error.h>

SCOREP_DefinitionManager  scorep_local_definition_manager;
SCOREP_DefinitionManager* scorep_unified_definition_manager = 0;
static bool               scorep_definitions_initialized    = false;


/* global definition lock */
static SCOREP_Mutex definitions_lock;

void
SCOREP_Definitions_Lock( void )
{
    SCOREP_MutexLock( definitions_lock );
}

void
SCOREP_Definitions_Unlock( void )
{
    SCOREP_MutexUnlock( definitions_lock );
}

void
SCOREP_Definitions_Initialize()
{
    if ( scorep_definitions_initialized )
    {
        return;
    }
    scorep_definitions_initialized = true;

    SCOREP_MutexCreate( &definitions_lock );
    scorep_definitions_create_interim_communicator_counter_lock();

    SCOREP_DefinitionManager* local_definition_manager = &scorep_local_definition_manager;
    SCOREP_Definitions_InitializeDefinitionManager( &local_definition_manager,
                                                    SCOREP_Memory_GetLocalDefinitionPageManager(),
                                                    false );

    /* ensure, that the empty string gets id 0 */
    SCOREP_Definitions_NewString( "" );
}


void
SCOREP_Definitions_InitializeDefinitionManager( SCOREP_DefinitionManager**    definitionManager,
                                                SCOREP_Allocator_PageManager* pageManager,
                                                bool                          allocHashTables )
{
    UTILS_ASSERT( definitionManager );
    UTILS_ASSERT( pageManager );

    if ( *definitionManager )
    {
        memset( *definitionManager, 0, sizeof( SCOREP_DefinitionManager ) );
    }
    else
    {
        *definitionManager = calloc( 1, sizeof( SCOREP_DefinitionManager ) );
        UTILS_BUG_ON( *definitionManager == 0,
                      "Can't allocate definition manager" );
    }

    ( *definitionManager )->page_manager = pageManager;

    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, string );
    SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, string );
    SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, source_file );

    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, system_tree_node );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, system_tree_node_property );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, location_group );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, location );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, source_file );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, region );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, group );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, interim_communicator );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, communicator );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, interim_rma_window );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, rma_window );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, mpi_cartesian_coords );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, mpi_cartesian_topology );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, metric );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, sampling_set );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, sampling_set_recorder );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, io_file_group );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, io_file );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, marker_group );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, marker );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, parameter );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, callpath );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, property );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, attribute );
    SCOREP_DEFINITIONS_MANAGER_INIT_MEMBER( *definitionManager, location_property );

    if ( allocHashTables )
    {
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, system_tree_node );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, system_tree_node_property );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, region );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, group );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, mpi_cartesian_topology );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, mpi_cartesian_coords );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, metric );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, sampling_set );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, sampling_set_recorder );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, io_file_group );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, io_file );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, marker_group );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, marker );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, parameter );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, callpath );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, property );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, attribute );
        SCOREP_DEFINITIONS_MANAGER_ALLOC_MEMBER_HASH_TABLE( *definitionManager, location_property );
    }
}

void
SCOREP_Definitions_Finalize()
{
    if ( !scorep_definitions_initialized )
    {
        return;
    }

    free( scorep_local_definition_manager.string.hash_table );
    if ( scorep_unified_definition_manager )
    {
        free( scorep_unified_definition_manager->string.hash_table );
        free( scorep_unified_definition_manager->location.hash_table );
        free( scorep_unified_definition_manager->location_group.hash_table );
        free( scorep_unified_definition_manager->system_tree_node.hash_table );
        free( scorep_unified_definition_manager->system_tree_node_property.hash_table );
        free( scorep_unified_definition_manager->source_file.hash_table );
        free( scorep_unified_definition_manager->region.hash_table );
        free( scorep_unified_definition_manager->group.hash_table );
        free( scorep_unified_definition_manager->interim_communicator.hash_table );
        free( scorep_unified_definition_manager->communicator.hash_table );
        free( scorep_unified_definition_manager->interim_rma_window.hash_table );
        free( scorep_unified_definition_manager->rma_window.hash_table );
        free( scorep_unified_definition_manager->mpi_cartesian_topology.hash_table );
        free( scorep_unified_definition_manager->mpi_cartesian_coords.hash_table );
        free( scorep_unified_definition_manager->metric.hash_table );
        free( scorep_unified_definition_manager->sampling_set.hash_table );
        free( scorep_unified_definition_manager->sampling_set_recorder.hash_table );
        free( scorep_unified_definition_manager->io_file_group.hash_table );
        free( scorep_unified_definition_manager->io_file.hash_table );
        free( scorep_unified_definition_manager->marker_group.hash_table );
        free( scorep_unified_definition_manager->marker.hash_table );
        free( scorep_unified_definition_manager->parameter.hash_table );
        free( scorep_unified_definition_manager->callpath.hash_table );
        free( scorep_unified_definition_manager->attribute.hash_table );
        free( scorep_unified_definition_manager->location_property.hash_table );
    }
    free( scorep_unified_definition_manager );
    // the contents of the definition managers is allocated using
    // SCOREP_Memory_AllocForDefinitions, so we don't need to free it
    // explicitly.

    SCOREP_MutexDestroy( &definitions_lock );
    scorep_definitions_destroy_interim_communicator_counter_lock();

    scorep_definitions_initialized = false;
}


void
SCOREP_Definitions_Write()
{
    if ( !scorep_definitions_initialized )
    {
        return;
    }

    if ( SCOREP_IsTracingEnabled() )
    {
        SCOREP_Tracing_WriteDefinitions();
    }

    /// @todo Daniel, what to do here for profiling?
}


/**
 * Returns the number of unified metric definitions.
 */
uint32_t
SCOREP_Definitions_GetNumberOfUnifiedMetricDefinitions( void )
{
    return scorep_unified_definition_manager->metric.counter;
}


/**
 * Returns the number of unified callpath definitions.
 */
uint32_t
SCOREP_Definitions_GetNumberOfUnifiedCallpathDefinitions( void )
{
    return scorep_unified_definition_manager->callpath.counter;
}
