/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
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
 * @file       src/measurement/scorep_unify.c
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


#include <config.h>
#include "scorep_unify.h"

#include <SCOREP_Config.h>
#include "scorep_environment.h"
#include "scorep_status.h"
#include <definitions/SCOREP_Definitions.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "scorep_subsystem.h"

#include <UTILS_Debug.h>


void
SCOREP_Unify( void )
{
    /* This prepares the unified definition manager */
    SCOREP_Unify_CreateUnifiedDefinitionManager();

    /* Let the subsystems do some stuff */
    scorep_subsystems_pre_unify();

    /* Unify the local definitions */
    SCOREP_Unify_Locally();

    if ( SCOREP_Status_IsMpp() )
    {
        /* unify the definitions with all processes. */
        SCOREP_Unify_Mpp();
    }

    /* Let the subsystems do some stuff */
    scorep_subsystems_post_unify();

    /* fool linker, so that the scorep_unify_helpers.c unit is always linked
       into the library/binary. */
    UTILS_FOOL_LINKER( scorep_unify_helpers );
}


/**
 * Copies all definitions of type @a type to the unified definition manager.
 *
 * @needs Variable named @a definition_manager of type @a SCOREP_DefinitionManager*.
 *        The definitions to be copied live here.
 */
#define UNIFY_DEFINITION( definition_manager, Type, type ) \
    do \
    { \
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_DEFINITIONS, "Copying %s to unified", #type ); \
        SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( definition_manager, Type, type ) \
        { \
            scorep_definitions_unify_ ## type( definition, ( definition_manager )->page_manager ); \
        } \
        SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END(); \
    } \
    while ( 0 )


void
SCOREP_CopyDefinitionsToUnified( SCOREP_DefinitionManager* sourceDefinitionManager )
{
    assert( sourceDefinitionManager );
    UNIFY_DEFINITION( sourceDefinitionManager, String, string );
    UNIFY_DEFINITION( sourceDefinitionManager, SystemTreeNode, system_tree_node );
    UNIFY_DEFINITION( sourceDefinitionManager, SystemTreeNodeProperty, system_tree_node_property );
    UNIFY_DEFINITION( sourceDefinitionManager, LocationGroup, location_group );
    UNIFY_DEFINITION( sourceDefinitionManager, Location, location );
    UNIFY_DEFINITION( sourceDefinitionManager, SourceFile, source_file );
    UNIFY_DEFINITION( sourceDefinitionManager, Region, region );
    UNIFY_DEFINITION( sourceDefinitionManager, Group, group );
    UNIFY_DEFINITION( sourceDefinitionManager, Communicator, communicator );
    UNIFY_DEFINITION( sourceDefinitionManager, RmaWindow, rma_window );
    UNIFY_DEFINITION( sourceDefinitionManager, Metric, metric );
    UNIFY_DEFINITION( sourceDefinitionManager, SamplingSet, sampling_set );
    UNIFY_DEFINITION( sourceDefinitionManager, SamplingSetRecorder, sampling_set_recorder );
    UNIFY_DEFINITION( sourceDefinitionManager, Parameter, parameter );
    UNIFY_DEFINITION( sourceDefinitionManager, Callpath, callpath );
    UNIFY_DEFINITION( sourceDefinitionManager, Property, property );
}


/**
 * Allocates the array member @a type_mappings of struct SCOREP_DefinitionMappings that lives
 * in @a definition_manager. The size of the array equals @a type_definition_counter.
 *
 */
#define ALLOC_MAPPINGS( definition_manager, type ) \
    do \
    { \
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_DEFINITIONS, "Alloc mappings for %s", #type ); \
        scorep_definitions_manager_entry_alloc_mapping( &( definition_manager )->type ); \
    } \
    while ( 0 )


void
SCOREP_CreateDefinitionMappings( SCOREP_DefinitionManager* definitionManager )
{
    assert( definitionManager );

    #define DEF_WITH_MAPPING( Type, type ) \
    ALLOC_MAPPINGS( definitionManager, type );
    SCOREP_LIST_OF_DEFS_WITH_MAPPINGS
    #undef DEF_WITH_MAPPING
}


/**
 * Fill the mapping array member @a type_mappings @a SCOREP_DefinitionMappings with the
 * corresponding sequence numbers of the unified definition.
 */
#define ASSIGN_MAPPING( definition_manager, Type, type ) \
    do \
    { \
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_DEFINITIONS, "Assign mapping for %s", #Type ); \
        if ( ( definition_manager )->type.counter > 0 ) \
        { \
            uint32_t type ## _sequence_number = 0; \
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( definition_manager, Type, type ) \
            { \
                assert( type ## _sequence_number == definition->sequence_number ); \
                ( definition_manager )->type.mapping[ type ## _sequence_number ] = \
                    SCOREP_UNIFIED_HANDLE_DEREF( definition->unified, Type )->sequence_number; \
                ++type ## _sequence_number; \
            } \
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END(); \
        } \
    } \
    while ( 0 )


void
SCOREP_AssignDefinitionMappingsFromUnified( SCOREP_DefinitionManager* definitionManager )
{
    assert( definitionManager );

    #define DEF_WITH_MAPPING( Type, type ) \
    ASSIGN_MAPPING( definitionManager, Type, type );
    SCOREP_LIST_OF_DEFS_WITH_MAPPINGS
    #undef DEF_WITH_MAPPING
}


/**
 * Frees the array member @a type_mappings of struct SCOREP_DefinitionMappings that lives
 * in @a definition_manager. The size of the array equals @a type_definition_counter.
 *
 */
#define FREE_MAPPING( definition_manager, type ) \
    do \
    { \
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_DEFINITIONS, "Free mappings for %s", #type ); \
        scorep_definitions_manager_entry_free_mapping( &( definition_manager )->type ); \
    } \
    while ( 0 )


void
SCOREP_DestroyDefinitionMappings( SCOREP_DefinitionManager* definitionManager )
{
    assert( definitionManager );

    #define DEF_WITH_MAPPING( Type, type ) \
    FREE_MAPPING( definitionManager, type );
    SCOREP_LIST_OF_DEFS_WITH_MAPPINGS
    #undef DEF_WITH_MAPPING

    FREE_MAPPING( definitionManager,
                  interim_communicator );

    FREE_MAPPING( definitionManager, interim_rma_window );
}


void
SCOREP_Unify_CreateUnifiedDefinitionManager( void )
{
    assert( scorep_unified_definition_manager == 0 );

    bool alloc_hash_tables = true;
    SCOREP_Definitions_InitializeDefinitionManager( &scorep_unified_definition_manager,
                                                    SCOREP_Memory_GetLocalDefinitionPageManager(),
                                                    alloc_hash_tables );
}

void
SCOREP_Unify_Locally( void )
{
    SCOREP_CopyDefinitionsToUnified( &scorep_local_definition_manager );
    // The unified definitions might differ from the local ones if there were
    // duplicates in the local ones. By creating mappings we are on the save side.
    SCOREP_CreateDefinitionMappings( &scorep_local_definition_manager );
    SCOREP_AssignDefinitionMappingsFromUnified( &scorep_local_definition_manager );

    /*
     * Allocate also mappings for the interim definitions.
     */
    ALLOC_MAPPINGS( &scorep_local_definition_manager, interim_communicator );
    ALLOC_MAPPINGS( &scorep_local_definition_manager, interim_rma_window );
}
