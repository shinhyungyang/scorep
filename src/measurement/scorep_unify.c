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
 * @file       scorep_unify.c
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


#include <config.h>
#include "scorep_unify.h"

#include <SCOREP_Config.h>
#include "scorep_environment.h"
#include "scorep_mpi.h"
#include "scorep_definitions.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <scorep_utility/SCOREP_Debug.h>

/* *INDENT-OFF* */
/* *INDENT-ON*  */


extern SCOREP_DefinitionManager  scorep_local_definition_manager;
extern SCOREP_DefinitionManager* scorep_unified_definition_manager;
extern SCOREP_DefinitionManager* scorep_remote_definition_manager;


//typedef struct scorep_unify_string_definition scorep_unify_string_definition;
//struct scorep_unify_string_definition
//{
//    SCOREP_String_Definition*       definition;
//    scorep_unify_string_definition* next;
//};


/* *INDENT-OFF* */
/* *INDENT-ON* */


void
SCOREP_Unify( void )
{
    SCOREP_Unify_Locally();
    if ( SCOREP_Mpi_HasMpi() )
    {
        SCOREP_Mpi_Unify();
    }
}


void
SCOREP_CopyDefinitionsToUnified( SCOREP_DefinitionManager* sourceDefinitionManager )
{
    assert( sourceDefinitionManager );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, String, string );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, SystemTreeNode, system_tree_node );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, LocationGroup, location_group );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, Location, location );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, SourceFile, source_file );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, Region, region );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, Group, group );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, Metric, metric );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, SamplingSet, sampling_set );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, Parameter, parameter );
    SCOREP_COPY_DEFINITIONS_TO_UNIFIED_DEFINITION_MANAGER( sourceDefinitionManager, Callpath, callpath );
}


void
SCOREP_CreateDefinitionMappings( SCOREP_DefinitionManager* definitionManager )
{
    assert( definitionManager );
    assert( !definitionManager->mappings );

    definitionManager->mappings = calloc( 1, sizeof( *definitionManager->mappings ) );
    assert( definitionManager->mappings );

    #define DEF_WITH_MAPPING( Type, type ) \
    SCOREP_ALLOC_MAPPINGS_ARRAY( type,  definitionManager );
    SCOREP_LIST_OF_DEFS_WITH_MAPPINGS
    #undef DEF_WITH_MAPPING
}


void
SCOREP_AssignDefinitionMappingsFromUnified( SCOREP_DefinitionManager* definitionManager )
{
    assert( definitionManager );
    assert( definitionManager->mappings );

    #define DEF_WITH_MAPPING( Type, type ) \
    SCOREP_ASSIGN_MAPPINGS( definitionManager, Type, type );
    SCOREP_LIST_OF_DEFS_WITH_MAPPINGS
    #undef DEF_WITH_MAPPING
}


void
SCOREP_DestroyDefinitionMappings( SCOREP_DefinitionManager* definitionManager )
{
    assert( definitionManager );
    assert( definitionManager->mappings );

    #define DEF_WITH_MAPPING( Type, type ) \
    SCOREP_FREE_MAPPINGS_ARRAY( type, definitionManager );
    SCOREP_LIST_OF_DEFS_WITH_MAPPINGS
    #undef DEF_WITH_MAPPING

    // was probably allocated in the MPI unify code, but never freed there
    SCOREP_FREE_MAPPINGS_ARRAY( local_mpi_communicator,
                                definitionManager );

    free( definitionManager->mappings );
    definitionManager->mappings = NULL;
}


void
SCOREP_Unify_Locally( void )
{
    assert( scorep_unified_definition_manager == 0 );

    bool alloc_hash_tables = true;
    SCOREP_InitializeDefinitionManager( &scorep_unified_definition_manager,
                                        SCOREP_Memory_GetLocalDefinitionPageManager(),
                                        alloc_hash_tables );
    // define empty string
    scorep_string_definition_define( scorep_unified_definition_manager, "" );
    SCOREP_CopyDefinitionsToUnified( &scorep_local_definition_manager );
    // The unified definitions might differ from the local ones if there were
    // duplicates in the local ones. By creating mappings we are on the save side.
    SCOREP_CreateDefinitionMappings( &scorep_local_definition_manager );
    SCOREP_AssignDefinitionMappingsFromUnified( &scorep_local_definition_manager );
}
