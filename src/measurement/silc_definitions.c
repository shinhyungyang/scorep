/*
 * This file is part of the SILC project (http://www.silc.de)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */


/**
 * @file       silc_definitions.c
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


#include "silc_definitions.h"

#include "silc_definition_structs.h"
#include "silc_runtime_management.h"
#include "silc_thread.h"
#include <SILC_Omp.h>
#include <SILC_PublicTypes.h>
#include <SILC_Timing.h>
#include <OTF2_Archive.h>
#include <OTF2_DefWriter_inc.h>
#include <stdbool.h>
#include <stdint.h>
#include <SILC_Debug.h>
#include "silc_mpi.h"


SILC_DefinitionManager silc_definition_manager;
static bool            silc_definitions_initialized = false;


/* *INDENT-OFF* */
static OTF2_DefWriter* silc_create_definition_writer();
static OTF2_FlushType silc_on_definitions_pre_flush();
static void silc_delete_definition_writer(OTF2_DefWriter* definitionWriter);
static void silc_write_callpath_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_counter_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_counter_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_definitions(OTF2_DefWriter* definitionWriter);
static void silc_write_io_file_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_io_file_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_marker_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_marker_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_mpi_cartesian_coords_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_mpi_cartesian_topology_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_mpi_window_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_parameter_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_region_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_source_file_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_string_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_write_location_definitions_to_otf2( OTF2_DefWriter* definitionWriter );
static void silc_handle_definition_writing_error( SILC_Error_Code status, const char* definitionType );
static OTF2_RegionType silc_region_type_to_otf_region_type( SILC_RegionType silcType );
/* *INDENT-ON* */


void
SILC_Definitions_Initialize()
{
    if ( silc_definitions_initialized )
    {
        return;
    }
    silc_definitions_initialized = true;

    memset( &silc_definition_manager, 0, sizeof( silc_definition_manager ) );

    // note, only lower-case type needed
    #define SILC_INIT_DEFINITION_LIST( type ) \
    do { \
        SILC_ALLOCATOR_MOVABLE_INIT_NULL( \
            silc_definition_manager.type ## _definition_head ); \
        silc_definition_manager.type ## _definition_tail_pointer = \
            &silc_definition_manager.type ## _definition_head; \
    } while ( 0 )

    SILC_INIT_DEFINITION_LIST( string );
    SILC_INIT_DEFINITION_LIST( location );
    SILC_INIT_DEFINITION_LIST( source_file );
    SILC_INIT_DEFINITION_LIST( region );
    SILC_INIT_DEFINITION_LIST( group );
    SILC_INIT_DEFINITION_LIST( mpi_window );
    SILC_INIT_DEFINITION_LIST( mpi_cartesian_topology );
    SILC_INIT_DEFINITION_LIST( mpi_cartesian_coords );
    SILC_INIT_DEFINITION_LIST( counter_group );
    SILC_INIT_DEFINITION_LIST( counter );
    SILC_INIT_DEFINITION_LIST( io_file_group );
    SILC_INIT_DEFINITION_LIST( io_file );
    SILC_INIT_DEFINITION_LIST( marker_group );
    SILC_INIT_DEFINITION_LIST( marker );
    SILC_INIT_DEFINITION_LIST( parameter );
    SILC_INIT_DEFINITION_LIST( callpath );

    #undef SILC_INIT_DEFINITION_LIST

    // No need the create the definition writer, its only needed in the
    // finalization phase and will be created there.
}


void
SILC_Definitions_Finalize()
{
    if ( !silc_definitions_initialized )
    {
        return;
    }
    silc_definitions_initialized = false;

    OTF2_DefWriter* definition_writer = silc_create_definition_writer();
    silc_write_definitions( definition_writer );
    silc_delete_definition_writer( definition_writer );
}


static OTF2_DefWriter*
silc_create_definition_writer()
{
    SILC_CreateExperimentDir();
    uint64_t otf2_location = SILC_Thread_GetTraceLocationData(
        SILC_Thread_GetLocationData() )->otf_location;

    OTF2_DefWriter* definition_writer =
        OTF2_Archive_GetDefWriter( silc_otf2_archive,
                                   otf2_location,
                                   silc_on_definitions_pre_flush,
                                   SILC_OnTraceAndDefinitionPostFlush );

    assert( definition_writer );
    return definition_writer;
}


OTF2_FlushType
silc_on_definitions_pre_flush()
{
    if ( !SILC_Mpi_IsInitialized )
    {
        // flush before MPI_Init, we are lost.
        assert( false );
    }
    // master/slave already set during initialization
    return OTF2_FLUSH;
}


static void
silc_write_definitions( OTF2_DefWriter* definitionWriter )
{
    silc_write_string_definitions_to_otf2( definitionWriter );
    silc_write_location_definitions_to_otf2( definitionWriter );
    silc_write_source_file_definitions_to_otf2( definitionWriter );
    silc_write_region_definitions_to_otf2( definitionWriter );
    silc_write_group_definitions_to_otf2( definitionWriter );
    silc_write_mpi_window_definitions_to_otf2( definitionWriter );
    silc_write_mpi_cartesian_topology_definitions_to_otf2( definitionWriter );
    silc_write_mpi_cartesian_coords_definitions_to_otf2( definitionWriter );
    silc_write_counter_group_definitions_to_otf2( definitionWriter );
    silc_write_counter_definitions_to_otf2( definitionWriter );
    silc_write_io_file_group_definitions_to_otf2( definitionWriter );
    silc_write_io_file_definitions_to_otf2( definitionWriter );
    silc_write_marker_group_definitions_to_otf2( definitionWriter );
    silc_write_marker_definitions_to_otf2( definitionWriter );
    silc_write_parameter_definitions_to_otf2( definitionWriter );
    silc_write_callpath_definitions_to_otf2( definitionWriter );
}


static void
silc_delete_definition_writer( OTF2_DefWriter* definitionWriter )
{
    SILC_Error_Code status = OTF2_DefWriter_Delete( definitionWriter );
    if ( status != SILC_SUCCESS )
    {
        assert( 0 );
    }
}


SILC_StringHandle
SILC_DefineString( const char* str )
{
    SILC_String_Definition*         new_definition = NULL;
    SILC_String_Definition_Movable* new_movable    = NULL;

    #pragma omp critical (define_string)
    {
        SILC_ALLOC_NEW_DEFINITION( String, string );

        SILC_Memory_AllocForDefinitionsRaw( strlen( str ) + 1,
                                            ( SILC_Allocator_MovableMemory* )&new_definition->str );
        strcpy( SILC_MEMORY_DEREF_MOVABLE( &new_definition->str, char* ), str );
    }

    return new_movable;
}


/**
 * Registers a new local location into the definitions.
 *
 * @in internal
 */
SILC_LocationHandle
SILC_DefineLocation( uint64_t    locationId,
                     const char* name )
{
    SILC_Location_Definition*         new_definition = NULL;
    SILC_Location_Definition_Movable* new_movable    = NULL;

    #pragma omp critical (define_location)
    {
        SILC_ALLOC_NEW_DEFINITION( Location, location );

        /* important: overwrite id with the actual location id */
        /** @todo: convert to global locationId */
        new_definition->id          = locationId;
        new_definition->name_handle = *SILC_DefineString( name );

        /** @todo: this needs clarification after the location hierarchy
                   has settled */
        new_definition->location_type = SILC_LOCATION_OMP_THREAD;
    }

    return new_movable;
}


SILC_CallpathHandle
SILC_DefineCallpath( SILC_CallpathHandle parent,
                     SILC_RegionHandle   region )
{
    SILC_Callpath_Definition*         new_definition = NULL;
    SILC_Callpath_Definition_Movable* new_movable    = NULL;

    SILC_ALLOC_NEW_DEFINITION( Callpath, callpath );

    // Init new_definition

    return new_movable;
}


SILC_CallpathHandle
SILC_DefineCallpathParameterInteger( SILC_CallpathHandle  parent,
                                     SILC_ParameterHandle param,
                                     int64_t              value )
{
    SILC_Callpath_Definition*         new_definition = NULL;
    SILC_Callpath_Definition_Movable* new_movable    = NULL;

    SILC_ALLOC_NEW_DEFINITION( Callpath, callpath );

    // Init new_definition

    return new_movable;
}


SILC_CallpathHandle
SILC_DefineCallpathParameterString( SILC_CallpathHandle  parent,
                                    SILC_ParameterHandle param,
                                    SILC_StringHandle    value )
{
    SILC_Callpath_Definition*         new_definition = NULL;
    SILC_Callpath_Definition_Movable* new_movable    = NULL;

    SILC_ALLOC_NEW_DEFINITION( Callpath, callpath );

    // Init new_definition

    return new_movable;
}


static void
silc_handle_definition_writing_error( SILC_Error_Code status,
                                      const char*     definitionType )
{
    assert( false ); // implement me
}


static OTF2_LocationType
silc_location_type_to_otf_location_type( SILC_LocationType silcType )
{
    /* see SILC_Types.h
       SILC_LOCATION_UNKNOWN = 0,
       SILC_LOCATION_OMP_THREAD,
     */

    static OTF2_LocationType type_map[ SILC_INVALID_LOCATION_TYPE ] = {
        OTF2_LOCATION_TYPE_UNKNOWN,
        OTF2_LOCATION_TYPE_THREAD,

        /* unused */
        /*
           OTF2_LOCATION_TYPE_MACHINE,
           OTF2_LOCATION_TYPE_NODE,
           OTF2_LOCATION_TYPE_PROCESS,
         */
    };

    return type_map[ silcType ];
}

static OTF2_RegionType
silc_region_type_to_otf_region_type( SILC_RegionType silcType )
{
    /* see SILC_Types.h
       SILC_REGION_UNKNOWN = 0,
       SILC_REGION_FUNCTION,
       SILC_REGION_LOOP,
       SILC_REGION_USER,
       SILC_REGION_PHASE,
       SILC_REGION_DYNAMIC,

       SILC_REGION_DYNAMIC_PHASE,
       SILC_REGION_DYNAMIC_LOOP,
       SILC_REGION_DYNAMIC_FUNCTION,
       SILC_REGION_DYNAMIC_LOOP_PHASE,

       SILC_REGION_MPI_COLL_BARRIER,
       SILC_REGION_MPI_COLL_ONE2ALL,
       SILC_REGION_MPI_COLL_ALL2ONE,
       SILC_REGION_MPI_COLL_ALL2ALL,
       SILC_REGION_MPI_COLL_OTHER,

       SILC_REGION_OMP_PARALLEL,
       SILC_REGION_OMP_LOOP,
       SILC_REGION_OMP_SECTIONS,
       SILC_REGION_OMP_SECTION,
       SILC_REGION_OMP_WORKSHARE,
       SILC_REGION_OMP_SINGLE,
       SILC_REGION_OMP_MASTER,
       SILC_REGION_OMP_CRITICAL,
       SILC_REGION_OMP_ATOMIC,
       SILC_REGION_OMP_BARRIER,
       SILC_REGION_OMP_IMPLICIT_BARRIER,
       SILC_REGION_OMP_FLUSH,
       SILC_REGION_OMP_CRITICAL_SBLOCK, /// @todo what is SBLOCK?
       SILC_REGION_OMP_SINGLE_SBLOCK,
       SILC_REGION_OMP_WRAPPER,

       SILC_INVALID_REGION_TYPE
     */

    /* see Records/OTF2_LocalDefinitions.c */

    static OTF2_RegionType type_map[ SILC_INVALID_REGION_TYPE ] = {
        OTF2_REGION_TYPE_UNKNOWN,
        OTF2_REGION_TYPE_FUNCTION,
        OTF2_REGION_TYPE_LOOP,
        OTF2_REGION_TYPE_USER_REGION,
        OTF2_REGION_PHASE,
        OTF2_REGION_DYNAMIC,
        OTF2_REGION_DYNAMIC_PHASE,
        OTF2_REGION_DYNAMIC_LOOP,
        OTF2_REGION_DYNAMIC_FUNCTION,
        OTF2_REGION_DYNAMIC_LOOP_PHASE,
        OTF2_REGION_TYPE_MPI_COLL_BARRIER,
        OTF2_REGION_TYPE_MPI_COLL_ONE2ALL,
        OTF2_REGION_TYPE_MPI_COLL_ALL2ONE,
        OTF2_REGION_TYPE_MPI_COLL_ALL2ALL,
        OTF2_REGION_TYPE_MPI_COLL_OTHER,
        OTF2_REGION_TYPE_OMP_PARALLEL,
        OTF2_REGION_TYPE_OMP_LOOP,
        OTF2_REGION_TYPE_OMP_SECTIONS,
        OTF2_REGION_TYPE_OMP_SECTION,
        OTF2_REGION_TYPE_OMP_WORKSHARE,
        OTF2_REGION_TYPE_OMP_SINGLE,
        OTF2_REGION_TYPE_OMP_MASTER,
        OTF2_REGION_TYPE_OMP_CRITICAL,
        OTF2_REGION_TYPE_OMP_ATOMIC,
        OTF2_REGION_TYPE_OMP_BARRIER,
        OTF2_REGION_TYPE_OMP_IBARRIER,
        OTF2_REGION_TYPE_OMP_FLUSH,
        OTF2_REGION_TYPE_OMP_CRITICAL_SBLOCK,
        OTF2_REGION_TYPE_OMP_SINGLE_SBLOCK,
        OTF2_REGION_TYPE_OMP_WRAPPER,
    };

    return type_map[ silcType ];
}

static OTF2_GroupType
silc_group_type_to_otf_group_type( SILC_GroupType silcType )
{
    /* see SILC_Types.h
       SILC_GROUP_UNKNOWN      = 0,
       SILC_GROUP_LOCATIONS    = 1,
       SILC_GROUP_REGIONS      = 2,
       SILC_GROUP_COMMUNICATOR = 3,
       SILC_GROUP_METRIC       = 4,
     */

    static OTF2_GroupType type_map[ SILC_INVALID_GROUP_TYPE ] = {
        OTF2_GROUPTYPE_NON,
        OTF2_GROUPTYPE_LOCATIONS,
        OTF2_GROUPTYPE_REGIONS,
        OTF2_GROUPTYPE_COMMUNICATOR,
        OTF2_GROUPTYPE_METRIC,
    };

    return type_map[ silcType ];
}

static void
silc_write_string_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager, String, string )
    {
        SILC_Error_Code status = OTF2_DefWriter_DefString(
            definitionWriter,
            definition->id,
            SILC_MEMORY_DEREF_MOVABLE( &definition->str, char* ) );
        if ( status != SILC_SUCCESS )
        {
            silc_handle_definition_writing_error( status, "SILC_String_Definition" );
        }
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_location_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager, Location, location )
    {
        /** @todo add definition count */
        SILC_Error_Code status = OTF2_DefWriter_DefLocation(
            definitionWriter,
            definition->id,
            SILC_MEMORY_DEREF_MOVABLE( &definition->name_handle, char* ),
            silc_location_type_to_otf_location_type( definition->location_type ),
            0 );
        if ( status != SILC_SUCCESS )
        {
            silc_handle_definition_writing_error( status, "SILC_String_Definition" );
        }
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_source_file_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    // nothing to be done here
    // SourceFile_Definitions are only available as String_Definition in
    // OTF2, and the string itself for this definition was already passed to
    // OTF2
}


static void
silc_write_region_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager, Region, region )
    {
        uint32_t source_file_id = OTF2_UNDEFINED_UINT32;
        /*
         * OTF2's source_file_identifier argument is only a string.
         * Therefore extract the string id from the source file definition.
         */
        if ( !SILC_ALLOCATOR_MOVABLE_IS_NULL( definition->file_handle ) )
        {
            SILC_SourceFile_Definition* source_file_definition =
                SILC_MEMORY_DEREF_MOVABLE( &definition->file_handle,
                                           SILC_SourceFile_Definition* );
            source_file_id =
                SILC_HANDLE_TO_ID( &source_file_definition->name_handle,
                                   String );
        }

        SILC_Error_Code status = OTF2_DefWriter_DefRegion(
            definitionWriter,
            definition->id,
            SILC_HANDLE_TO_ID( &definition->name_handle, String ),
            SILC_HANDLE_TO_ID( &definition->description_handle, String ),
            silc_region_type_to_otf_region_type( definition->region_type ),
            source_file_id,
            definition->begin_line,
            definition->end_line );

        if ( status != SILC_SUCCESS )
        {
            silc_handle_definition_writing_error( status, "SILC_Region_Definition" );
        }
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager, Group, group )
    {
        SILC_Error_Code status = OTF2_DefWriter_DefGroup(
            definitionWriter,
            definition->id,
            silc_group_type_to_otf_group_type( definition->group_type ),
            definition->number_of_members,
            definition->members );
        if ( status != SILC_SUCCESS )
        {
            silc_handle_definition_writing_error( status, "SILC_Group_Definition" );
        }
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_mpi_window_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                MPIWindow,
                                mpi_window )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_MPIWindow_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_mpi_cartesian_topology_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                MPICartesianTopology,
                                mpi_cartesian_topology )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_MPICartesianTopology_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_mpi_cartesian_coords_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                MPICartesianCoords,
                                mpi_cartesian_coords )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_MPICartesianCoords_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_counter_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                CounterGroup,
                                counter_group )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_CounterGroup_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_counter_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                Counter,
                                counter )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_Counter_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_io_file_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                IOFileGroup,
                                io_file_group )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_IOFileGroup_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_io_file_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                IOFile,
                                io_file )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_IOFile_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_marker_group_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                MarkerGroup,
                                marker_group )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_MarkerGroup_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_marker_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                Marker,
                                marker )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_Marker_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_parameter_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                Parameter,
                                parameter )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_Parameter_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}


static void
silc_write_callpath_definitions_to_otf2( OTF2_DefWriter* definitionWriter )
{
    SILC_DEFINITION_FOREACH_DO( &silc_definition_manager,
                                Callpath,
                                callpath )
    {
        //SILC_Error_Code status = OTF2_DefWriter_Def...(definitionWriter, ...);
        //if ( status != SILC_SUCCESS )
        //{
        //    silc_handle_definition_writing_error( status, "SILC_Callpath_Definition" );
        //}
        SILC_DEBUG_PRINTF( SILC_WARNING | SILC_DEBUG_DEFINITIONS, "Not yet implemented." );
    }
    SILC_DEFINITION_FOREACH_WHILE();
}
