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
 * @status     alpha
 * @file       src/measurement/definitions/scorep_definitions_region.c
 * @maintainer Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @brief Declaration of definition functions to be used by the adapter layer.
 *
 */


#include <config.h>


#include <definitions/SCOREP_Definitions.h>


#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>


#include <UTILS_Error.h>
#define SCOREP_DEBUG_MODULE_NAME DEFINITIONS
#include <UTILS_Debug.h>


#include <jenkins_hash.h>


#include <SCOREP_DefinitionHandles.h>
#include <scorep_types.h>
#include <SCOREP_Mutex.h>
#include <SCOREP_Memory.h>


static SCOREP_RegionHandle
define_region( SCOREP_DefinitionManager* definition_manager,
               SCOREP_StringHandle       regionNameHandle,
               SCOREP_StringHandle       regionCanonicalNameHandle,
               SCOREP_StringHandle       descriptionNameHandle,
               SCOREP_StringHandle       fileNameHandle,
               SCOREP_LineNo             beginLine,
               SCOREP_LineNo             endLine,
               SCOREP_AdapterType        adapter,
               SCOREP_RegionType         regionType );


static void
initialize_region( SCOREP_Region_Definition* definition,
                   SCOREP_DefinitionManager* definition_manager,
                   SCOREP_StringHandle       regionNameHandle,
                   SCOREP_StringHandle       regionCanonicalNameHandle,
                   SCOREP_StringHandle       descriptionNameHandle,
                   SCOREP_StringHandle       fileNameHandle,
                   SCOREP_LineNo             beginLine,
                   SCOREP_LineNo             endLine,
                   SCOREP_AdapterType        adapter,
                   SCOREP_RegionType         regionType );


static bool
equal_region( const SCOREP_Region_Definition* existingDefinition,
              const SCOREP_Region_Definition* newDefinition );


SCOREP_RegionHandle
SCOREP_DefineRegion( const char*             regionName,
                     const char*             regionCanonicalName,
                     SCOREP_SourceFileHandle fileHandle,
                     SCOREP_LineNo           beginLine,
                     SCOREP_LineNo           endLine,
                     SCOREP_AdapterType      adapter,
                     SCOREP_RegionType       regionType )
{
    UTILS_DEBUG_ENTRY( "%s", regionName );

    /* resolve the file name early */
    SCOREP_StringHandle file_name_handle = SCOREP_INVALID_STRING;
    if ( fileHandle != SCOREP_INVALID_SOURCE_FILE )
    {
        file_name_handle = SCOREP_LOCAL_HANDLE_DEREF( fileHandle, SourceFile )->name_handle;
    }

    SCOREP_Definitions_Lock();

    SCOREP_RegionHandle new_handle = define_region(
        &scorep_local_definition_manager,
        /* region name (use it for demangled name) */
        scorep_string_definition_define(
            &scorep_local_definition_manager,
            regionName ? regionName : "<unknown region>" ),
        /* canonical region name (use it for mangled name) */
        scorep_string_definition_define(
            &scorep_local_definition_manager,
            regionCanonicalName ? regionCanonicalName :
            regionName ? regionName : "<unknown region>" ),
        /* description currently not used */
        scorep_string_definition_define(
            &scorep_local_definition_manager,
            "" ),
        file_name_handle,
        beginLine,
        endLine,
        adapter,
        regionType );

    SCOREP_Definitions_Unlock();

    return new_handle;
}


void
SCOREP_CopyRegionDefinitionToUnified( SCOREP_Region_Definition*     definition,
                                      SCOREP_Allocator_PageManager* handlesPageManager )
{
    assert( definition );
    assert( handlesPageManager );

    SCOREP_StringHandle unified_file_name_handle = SCOREP_INVALID_STRING;
    if ( definition->file_name_handle != SCOREP_INVALID_STRING )
    {
        unified_file_name_handle = SCOREP_HANDLE_GET_UNIFIED(
            definition->file_name_handle,
            String,
            handlesPageManager );
        assert( unified_file_name_handle != SCOREP_MOVABLE_NULL );
    }

    definition->unified = define_region(
        scorep_unified_definition_manager,
        SCOREP_HANDLE_GET_UNIFIED(
            definition->name_handle,
            String,
            handlesPageManager ),
        SCOREP_HANDLE_GET_UNIFIED(
            definition->canonical_name_handle,
            String,
            handlesPageManager ),
        SCOREP_HANDLE_GET_UNIFIED(
            definition->description_handle,
            String,
            handlesPageManager ),
        unified_file_name_handle,
        definition->begin_line,
        definition->end_line,
        definition->adapter_type,
        definition->region_type );
}


SCOREP_RegionHandle
define_region( SCOREP_DefinitionManager* definition_manager,
               SCOREP_StringHandle       regionNameHandle,
               SCOREP_StringHandle       regionCanonicalNameHandle,
               SCOREP_StringHandle       descriptionNameHandle,
               SCOREP_StringHandle       fileNameHandle,
               SCOREP_LineNo             beginLine,
               SCOREP_LineNo             endLine,
               SCOREP_AdapterType        adapter,
               SCOREP_RegionType         regionType )
{
    assert( definition_manager );

    SCOREP_Region_Definition* new_definition = NULL;
    SCOREP_RegionHandle       new_handle     = SCOREP_INVALID_REGION;

    SCOREP_DEFINITION_ALLOC( Region );
    initialize_region( new_definition,
                       definition_manager,
                       regionNameHandle,
                       regionCanonicalNameHandle,
                       descriptionNameHandle,
                       fileNameHandle,
                       beginLine,
                       endLine,
                       adapter,
                       regionType );

    /* Does return if it is a duplicate */
    SCOREP_DEFINITION_MANAGER_ADD_DEFINITION( Region, region );

    return new_handle;
}


void
initialize_region( SCOREP_Region_Definition* definition,
                   SCOREP_DefinitionManager* definition_manager,
                   SCOREP_StringHandle       regionNameHandle,
                   SCOREP_StringHandle       regionCanonicalNameHandle,
                   SCOREP_StringHandle       descriptionNameHandle,
                   SCOREP_StringHandle       fileNameHandle,
                   SCOREP_LineNo             beginLine,
                   SCOREP_LineNo             endLine,
                   SCOREP_AdapterType        adapter,
                   SCOREP_RegionType         regionType )
{
    definition->name_handle = regionNameHandle;
    HASH_ADD_HANDLE( definition, name_handle, String );

    definition->canonical_name_handle = regionCanonicalNameHandle;
    HASH_ADD_HANDLE( definition, canonical_name_handle, String );

    definition->description_handle = descriptionNameHandle;
    HASH_ADD_HANDLE( definition, description_handle, String );

    definition->region_type = regionType;
    HASH_ADD_POD( definition, region_type );

    definition->file_name_handle = fileNameHandle;
    if ( fileNameHandle != SCOREP_INVALID_STRING )
    {
        HASH_ADD_HANDLE( definition, file_name_handle, String );
    }

    definition->begin_line = beginLine;
    HASH_ADD_POD( definition, begin_line );
    definition->end_line = endLine;
    HASH_ADD_POD( definition, end_line );
    definition->adapter_type = adapter;
    HASH_ADD_POD( definition, adapter_type );
}


bool
equal_region( const SCOREP_Region_Definition* existingDefinition,
              const SCOREP_Region_Definition* newDefinition )
{
    return existingDefinition->name_handle           == newDefinition->name_handle &&
           existingDefinition->canonical_name_handle == newDefinition->canonical_name_handle &&
           existingDefinition->description_handle    == newDefinition->description_handle &&
           existingDefinition->region_type           == newDefinition->region_type &&
           existingDefinition->file_name_handle      == newDefinition->file_name_handle &&
           existingDefinition->begin_line            == newDefinition->begin_line &&
           existingDefinition->end_line              == newDefinition->end_line &&
           existingDefinition->adapter_type          == newDefinition->adapter_type;
}


uint32_t
SCOREP_GetRegionHandleToID( SCOREP_RegionHandle handle )
{
    return SCOREP_LOCAL_HANDLE_TO_ID( handle, Region );
}


/**
 * Gets read-only access to the name of the region.
 *
 * @param handle A handle to the region.
 *
 * @return region name.
 */
const char*
SCOREP_Region_GetName( SCOREP_RegionHandle handle )
{
    SCOREP_Region_Definition* region = SCOREP_LOCAL_HANDLE_DEREF( handle, Region );

    return SCOREP_LOCAL_HANDLE_DEREF( region->name_handle, String )->string_data;
}


/**
 * Gets read-only access to the canonical name of the region.
 *
 * @param handle A handle to the region.
 *
 * @return region name.
 */
const char*
SCOREP_Region_GetCanonicalName( SCOREP_RegionHandle handle )
{
    SCOREP_Region_Definition* region = SCOREP_LOCAL_HANDLE_DEREF( handle, Region );

    return SCOREP_LOCAL_HANDLE_DEREF( region->canonical_name_handle, String )->string_data;
}


/**
 * Gets read-only access to the file name of the region.
 *
 * @param handle A handle to the region.
 *
 * @return region file name.
 */
const char*
SCOREP_Region_GetFileName( SCOREP_RegionHandle handle )
{
    SCOREP_Region_Definition* region = SCOREP_LOCAL_HANDLE_DEREF( handle, Region );

    return SCOREP_LOCAL_HANDLE_DEREF( region->file_name_handle, String )->string_data;
}


/**
 * Gets the type of the region.
 *
 * @param handle A handle to the region.
 *
 * @return region type.
 */
SCOREP_RegionType
SCOREP_Region_GetType( SCOREP_RegionHandle handle )
{
    return SCOREP_LOCAL_HANDLE_DEREF( handle, Region )->region_type;
}


/**
 * Gets the adapter type of the region.
 *
 * @param handle A handle to the region.
 *
 * @return regions adapter type.
 */
SCOREP_AdapterType
SCOREP_Region_GetAdapterType( SCOREP_RegionHandle handle )
{
    return SCOREP_LOCAL_HANDLE_DEREF( handle, Region )->adapter_type;
}


/**
 * Gets the region first line of the region
 *
 * @param handle A handle to the region.
 *
 * @return regions first line.
 */
SCOREP_LineNo
SCOREP_Region_GetBeginLine( SCOREP_RegionHandle handle )
{
    return SCOREP_LOCAL_HANDLE_DEREF( handle, Region )->begin_line;
}


/**
 * Gets the region end line of the region.
 *
 * @param handle A handle to the region.
 *
 * @return regions end line.
 */
SCOREP_LineNo
SCOREP_Region_GetEndLine( SCOREP_RegionHandle handle )
{
    return SCOREP_LOCAL_HANDLE_DEREF( handle, Region )->end_line;
}
