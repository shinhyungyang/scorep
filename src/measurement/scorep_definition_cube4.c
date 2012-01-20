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
 * @file scorep_definition_cube4.c
 *
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @status alpha
 * Writes the definitions to Cube 4.
 */

#include <config.h>
#include <inttypes.h>
#include "scorep_definition_cube4.h"

#include <SCOREP_Memory.h>

#include "scorep_mpi.h"
#include "scorep_types.h"
#include "scorep_definitions.h"
#include <cubew_services.h>

extern SCOREP_DefinitionManager  scorep_local_definition_manager;
extern SCOREP_DefinitionManager* scorep_unified_definition_manager;

/* ****************************************************************************
 * Internal system tree representation
 *****************************************************************************/

/** Handles of both default metrics (number of visits and time) */
SCOREP_MetricHandle time_metric_handle   = SCOREP_INVALID_METRIC;
SCOREP_MetricHandle visits_metric_handle = SCOREP_INVALID_METRIC;

/**
   Node type definition for temporary internal system tree structure cor Cube definition
   writing. It is needed to map Score-P and Cube system tree definitions.
 */
typedef struct scorep_cube_system_node
{
    struct scorep_cube_system_node* parent;          /**< Pointer to the parent node */
    SCOREP_SystemTreeNodeHandle     scorep_node;     /**< The Score-P handle */
    cube_machine*                   my_cube_machine; /**< The Cube machine definition */
    cube_node*                      my_cube_node;    /**< The Cube node definition */
} scorep_cube_system_node;

/**
   Finds the node of the system tree mapping that belongs the the Score-P system tree
   node handle.
   @param system_tree Pointer to an array of scorep_cube_system_node nodes that contain
                      the system tree mapping structure.
   @param size        Number of elements in the system tree.
   @param node        Score-P system tree node handle that is seached in the mapping
                      tree.
   @returns A pointer to the mapping tree node that belong to @a node. If no matching
            Node is found, ir returns NULL.
 */
static scorep_cube_system_node*
scorep_cube_find_system_node( scorep_cube_system_node* system_tree, uint32_t size,
                              SCOREP_SystemTreeNodeHandle node )
{
    for ( uint32_t i = 0; i < size; i++ )
    {
        if ( system_tree[ i ].scorep_node == node )
        {
            return &system_tree[ i ];
        }
    }
    return NULL;
}

/**
   Seaches the ancestors of @a node for the machine definition.
   @param node Pointer to the mapping system tree for that the machine is seached.
   @returns A pointer to the Cube definition. If no machine exists in the ancestors of
            @a node, it returns NULL.
 */
static cube_machine*
scorep_cube_get_mach( scorep_cube_system_node* node )
{
    assert( node );

    /* Found machine */
    if ( node->my_cube_machine != NULL )
    {
        return node->my_cube_machine;
    }

    if ( node->parent == NULL )
    {
        return NULL;
    }
    return scorep_cube_get_mach( node->parent );
}

/**
   Lookup the cube node definition for a Score-P system tree handle.
   In cases the Score-P system tree does not provide a node definition, a default node
   is defined to Cube and returned.
   @param my_cube     Pointer to the Cube instance.
   @param system_tree Pointer to an array of scorep_cube_system_node nodes that contain
                      the system tree mapping structure.
   @param size        Number of entries in @a system_tree.
   @returns A pointer to the Cube node definition.
 */
static cube_node*
scorep_cube_get_node(  cube_t* my_cube,
                       scorep_cube_system_node* system_tree,
                       SCOREP_SystemTreeNodeHandle node, uint32_t size )
{
    /* Need a default node, in cases a system tree implmentation does provide no node
       definition. However, Cube insists of a node definition. */
    static cube_node*        default_node = NULL;
    scorep_cube_system_node* scorep_node  =
        scorep_cube_find_system_node( system_tree, size, node );

    if ( scorep_node == NULL )
    {
        if ( default_node == NULL )
        {
            default_node = cube_def_node( my_cube, "default node",
                                          scorep_cube_get_mach( &system_tree[ 0 ] ) );
        }
        return default_node;
    }

    assert( scorep_node );
    assert( scorep_node->my_cube_node );
    return scorep_node->my_cube_node;
}


/* ****************************************************************************
 * Initialization / Finalization
 *****************************************************************************/

/**
   @def SCOREP_CUBE4_INIT_MAP(Type, type, tablesize)
   Defines a macro for the initialization of the mapping tables for one type
   of handles. This macro is used in scorep_cube4_create_definitions_map.
   @param Type      The definitions type with first letter capitalized. Values
                    be 'Region', 'Metric', 'Callpath'.
   @param type      The definition type in small letters. Values can be
                    'region', 'metric', 'callpath'.
   @param tablesize Defines the number of slots for the mapping table.
 */
/* *INDENT-OFF* */
#define SCOREP_CUBE4_INIT_MAP(Type, type, tablesize)                          \
    if ( sizeof( SCOREP_ ## Type ## Handle ) == 8 )                           \
    {                                                                         \
        map-> type ## _table_cube                                             \
            = SCOREP_Hashtab_CreateSize( tablesize,                           \
                                       &SCOREP_Hashtab_HashInt64,             \
                                       &SCOREP_Hashtab_CompareInt64 );        \
    }                                                                         \
    else if ( sizeof( SCOREP_ ## Type ## Handle ) == 4 )                      \
    {                                                                         \
        map-> type ## _table_cube                                             \
             = SCOREP_Hashtab_CreateSize( tablesize,                          \
                                        &SCOREP_Hashtab_HashInt32,            \
                                        &SCOREP_Hashtab_CompareInt32 );       \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        SCOREP_ASSERT( false );                                               \
        goto cleanup;                                                         \
    }                                                                         \
    if ( map-> type ## _table_cube == NULL )                                  \
    {                                                                         \
        SCOREP_ERROR_POSIX( "Unable to create " #type " mapping table" );     \
        goto cleanup;                                                         \
    }                                                                         \
    map-> type ## _table_scorep                                               \
        = SCOREP_Hashtab_CreateSize( tablesize,                               \
                                   &SCOREP_Hashtab_HashPointer,               \
                                   &SCOREP_Hashtab_ComparePointer );          \
    if ( map-> type ## _table_scorep == NULL )                                \
    {                                                                         \
        SCOREP_ERROR_POSIX( "Unable to create " #type " mapping table" );     \
        goto cleanup;                                                         \
    }
/* *INDENT-ON* */

/* Creates a new scorep_cube4_definitions_map. */
scorep_cube4_definitions_map*
scorep_cube4_create_definitions_map()
{
    scorep_cube4_definitions_map* map = NULL;

    /* Allocate memory for the struct */
    map = ( scorep_cube4_definitions_map* )malloc( sizeof( scorep_cube4_definitions_map ) );
    if ( map == NULL )
    {
        SCOREP_ERROR_POSIX( "Unable to create mapping struct" );
        return NULL;
    }

    /* Initialize with NULL */
    map->region_table_cube     = NULL;
    map->metric_table_cube     = NULL;
    map->callpath_table_cube   = NULL;
    map->region_table_scorep   = NULL;
    map->metric_table_scorep   = NULL;
    map->callpath_table_scorep = NULL;

    /* Initialize region table */
    SCOREP_CUBE4_INIT_MAP( Region, region, 128 )

    /* Initialize metric table */
    SCOREP_CUBE4_INIT_MAP( Metric, metric, 8 )

    /* Initialize callpath table */
    SCOREP_CUBE4_INIT_MAP( Callpath, callpath, 256 )

    return map;

cleanup:
    if ( map->region_table_cube != NULL )
    {
        SCOREP_Hashtab_Free( map->region_table_cube );
    }
    if ( map->metric_table_cube != NULL )
    {
        SCOREP_Hashtab_Free( map->metric_table_cube );
    }
    if ( map->callpath_table_cube != NULL )
    {
        SCOREP_Hashtab_Free( map->callpath_table_cube );
    }
    if ( map->region_table_scorep != NULL )
    {
        SCOREP_Hashtab_Free( map->region_table_scorep );
    }
    if ( map->metric_table_scorep != NULL )
    {
        SCOREP_Hashtab_Free( map->metric_table_scorep );
    }
    if ( map->callpath_table_scorep != NULL )
    {
        SCOREP_Hashtab_Free( map->callpath_table_scorep );
    }
    free( map );
    return NULL;
}

#define SCOREP_Hashtab_DeleteRegionHandle SCOREP_Hashtab_DeleteFree
#define SCOREP_Hashtab_DeleteMetricHandle SCOREP_Hashtab_DeleteFree
#define SCOREP_Hashtab_DeleteCallpathHandle SCOREP_Hashtab_DeleteFree

/* Deletes the scorep_cube4_definitions_map */
void
scorep_cube4_delete_definitions_map( scorep_cube4_definitions_map* map )
{
    /* Because the scorep handle is only duplicated once for both mappings, it
       must only be deleted once.
     */
    SCOREP_Hashtab_FreeAll( map->region_table_cube,
                            SCOREP_Hashtab_DeleteRegionHandle,
                            SCOREP_Hashtab_DeleteNone );
    SCOREP_Hashtab_FreeAll( map->metric_table_cube,
                            SCOREP_Hashtab_DeleteMetricHandle,
                            SCOREP_Hashtab_DeleteNone );
    SCOREP_Hashtab_FreeAll( map->callpath_table_cube,
                            SCOREP_Hashtab_DeleteCallpathHandle,
                            SCOREP_Hashtab_DeleteNone );

    SCOREP_Hashtab_Free( map->region_table_scorep );
    SCOREP_Hashtab_Free( map->metric_table_scorep );
    SCOREP_Hashtab_Free( map->callpath_table_scorep );

    free( map );
}

/* ****************************************************************************
 * Add entries
 *****************************************************************************/
void
scorep_cube4_add_region_mapping( scorep_cube4_definitions_map* map,
                                 cube_region*                  cube_handle,
                                 SCOREP_RegionHandle           scorep_handle )
{
    /* Create copy of the SCOREP region handle on the heap */
    SCOREP_RegionHandle* scorep_copy = malloc( sizeof( SCOREP_RegionHandle ) );
    *scorep_copy = scorep_handle;

    /* Store handle in hashtable */
    SCOREP_Hashtab_Insert( map->region_table_cube, scorep_copy,
                           ( void* )cube_handle, NULL );
    SCOREP_Hashtab_Insert( map->region_table_scorep, ( void* )cube_handle,
                           scorep_copy, NULL );
}

void
scorep_cube4_add_callpath_mapping( scorep_cube4_definitions_map* map,
                                   cube_cnode*                   cube_handle,
                                   SCOREP_CallpathHandle         scorep_handle )
{
    /* Create copy of the SCOREP region handle on the heap */
    SCOREP_CallpathHandle* scorep_copy = malloc( sizeof( SCOREP_CallpathHandle ) );
    *scorep_copy = scorep_handle;

    /* Store handle in hashtable */
    SCOREP_Hashtab_Insert( map->callpath_table_cube, scorep_copy,
                           ( void* )cube_handle, NULL );
    SCOREP_Hashtab_Insert( map->callpath_table_scorep, ( void* )cube_handle,
                           scorep_copy, NULL );
}

void
scorep_cube4_add_metric_mapping( scorep_cube4_definitions_map* map,
                                 cube_metric*                  cube_handle,
                                 SCOREP_MetricHandle           scorep_handle )
{
    /* Create copy of the SCOREP region handle on the heap */
    SCOREP_MetricHandle* scorep_copy = malloc( sizeof( SCOREP_MetricHandle ) );
    *scorep_copy = scorep_handle;

    /* Store handle in hashtable */
    SCOREP_Hashtab_Insert( map->metric_table_cube, scorep_copy,
                           ( void* )cube_handle, NULL );
    SCOREP_Hashtab_Insert( map->metric_table_scorep, ( void* )cube_handle,
                           scorep_copy, NULL );
}

/* ****************************************************************************
 * Get mappings
 *****************************************************************************/

/* *INDENT-OFF* */
#define SCOREP_GET_CUBE_MAPPING( ret_type, type, Type )                       \
ret_type *                                                                    \
scorep_get_cube4_ ## type (scorep_cube4_definitions_map* map,                 \
                         SCOREP_ ## Type ## Handle     handle )               \
{                                                                             \
    SCOREP_Hashtab_Entry* entry = NULL;                                       \
    entry = SCOREP_Hashtab_Find( map->type ## _table_cube,                    \
                               &handle, NULL );                               \
    if ( entry == NULL )                                                      \
    {                                                                         \
        return NULL;                                                          \
    }                                                                         \
    return ( ret_type *) entry->value;                                        \
}

#define SCOREP_GET_SCOREP_MAPPING( in_type, type, Type, TYPE )                \
SCOREP_ ## Type ## Handle                                                     \
scorep_get_ ## type ## _from_cube4 (scorep_cube4_definitions_map* map,        \
                                  in_type *                   handle)         \
{                                                                             \
    SCOREP_Hashtab_Entry* entry = NULL;                                       \
    entry = SCOREP_Hashtab_Find( map->type ## _table_scorep,                  \
                               handle, NULL );                                \
    if ( entry == NULL )                                                      \
    {                                                                         \
        return SCOREP_INVALID_ ## TYPE;                                       \
    }                                                                         \
    return *( SCOREP_ ## Type ## Handle *) entry->value;                      \
}
/* *INDENT-ON* */

SCOREP_GET_CUBE_MAPPING( cube_metric, metric, Metric )

SCOREP_GET_CUBE_MAPPING( cube_region, region, Region )

SCOREP_GET_CUBE_MAPPING( cube_cnode, callpath, Callpath )

SCOREP_GET_SCOREP_MAPPING( cube_metric, metric, Metric, METRIC )

SCOREP_GET_SCOREP_MAPPING( cube_region, region, Region, REGION )

SCOREP_GET_SCOREP_MAPPING( cube_cnode, callpath, Callpath, CALLPATH )

uint64_t
scorep_cube4_get_number_of_callpathes( scorep_cube4_definitions_map* map )
{
    return SCOREP_Hashtab_Size( map->callpath_table_cube );
}

/* ****************************************************************************
 * Internal definition writer functions
 *****************************************************************************/

/**
   Writes metric definitions to Cube. The new Cube definitions are added to the
   mapping table @a map.
   @param my_cube Pointer to Cube instance.
   @param manager Pointer to Score-P definition manager with unified definitions.
   @param map     Pointer to mapping instance to map Score-P und Cube definitions.
 */
static void
scorep_write_metric_definitions_to_cube4( cube_t*                       my_cube,
                                          SCOREP_DefinitionManager*     manager,
                                          scorep_cube4_definitions_map* map )
{
    cube_metric* cube_handle;

    /* Add default profiling metrics for number of visits and implicit time */
    time_metric_handle = ( SCOREP_MetricHandle )SCOREP_Memory_AllocForDefinitions( 8 );
    cube_handle        = cube_def_met( my_cube, "Time", "time", "FLOAT", "sec", "",
                                       "@mirror@scorep_metrics-" PACKAGE_VERSION ".html#time",
                                       "Total CPU allocation time", NULL, CUBE_METRIC_INCLUSIVE );
    scorep_cube4_add_metric_mapping( map, cube_handle, time_metric_handle );

    visits_metric_handle = ( SCOREP_MetricHandle )SCOREP_Memory_AllocForDefinitions( 8 );
    cube_handle          = cube_def_met( my_cube, "Visits", "visits", "INTEGER", "occ", "",
                                         "@mirror@scorep_metrics-" PACKAGE_VERSION ".html#visits",
                                         "Number of visits", NULL, CUBE_METRIC_EXCLUSIVE );
    scorep_cube4_add_metric_mapping( map, cube_handle, visits_metric_handle );

    SCOREP_Metric_Definition* metric_definition;
    char*                     metric_name;
    char*                     metric_unit;
    char*                     metric_description;
    enum CubeMetricType       cube_metric_type;

    //for ( uint8_t i = 0; i < num_metrics; i++ )
    SCOREP_DEFINITION_FOREACH_DO( manager, Metric, metric )
    {
        /* Collect necessary data */
        metric_name = SCOREP_UNIFIED_HANDLE_DEREF( definition->name_handle,
                                                   String )->string_data;
        metric_unit = SCOREP_UNIFIED_HANDLE_DEREF( definition->unit_handle,
                                                   String )->string_data;
        metric_description = SCOREP_UNIFIED_HANDLE_DEREF( definition->description_handle,
                                                          String )->string_data;

        switch ( definition->profiling_type )
        {
            case SCOREP_METRIC_PROFILING_TYPE_EXCLUSIVE:
                cube_metric_type = CUBE_METRIC_EXCLUSIVE;
                break;
            case SCOREP_METRIC_PROFILING_TYPE_INCLUSIVE:
                cube_metric_type = CUBE_METRIC_INCLUSIVE;
                break;
            case SCOREP_METRIC_PROFILING_TYPE_SIMPLE:
                cube_metric_type = CUBE_METRIC_SIMPLE;
                break;
            default:
                SCOREP_ERROR( SCOREP_ERROR_UNKNOWN_TYPE,
                              "Metric '%s' has unknown profiling type.",
                              metric_name );
                continue;
        }

        switch ( definition->value_type )
        {
            case SCOREP_METRIC_VALUE_INT64:
            case SCOREP_METRIC_VALUE_UINT64:
                cube_handle = cube_def_met( my_cube, metric_name, metric_name, "INTEGER",
                                            metric_unit, "", "", metric_description, NULL,
                                            cube_metric_type );
                break;
            case SCOREP_METRIC_VALUE_DOUBLE:
                cube_handle = cube_def_met( my_cube, metric_name, metric_name, "FLOAT",
                                            metric_unit, "", "", metric_description, NULL,
                                            cube_metric_type );
                break;
            default:
                SCOREP_ERROR( SCOREP_ERROR_UNKNOWN_TYPE,
                              "Metric '%s' has unknown value type.",
                              metric_name );
                continue;
        }

        scorep_cube4_add_metric_mapping( map, cube_handle, handle );
    }
    SCOREP_DEFINITION_FOREACH_WHILE();
}

SCOREP_MetricHandle
scorep_get_time_metric_handle()
{
    return time_metric_handle;
}

SCOREP_MetricHandle
scorep_get_visits_metric_handle()
{
    return visits_metric_handle;
}

/**
   Writes region definitions to Cube. The new Cube definitions are added to the
   mapping table @a map.
   @param my_cube Pointer to Cube instance.
   @param manager Pointer to Score-P definition manager with unified definitions.
   @param map     Pointer to mapping instance to map Score-P und Cube definitions.
 */
static void
scorep_write_region_definitions_to_cube4( cube_t*                       my_cube,
                                          SCOREP_DefinitionManager*     manager,
                                          scorep_cube4_definitions_map* map )
{
    SCOREP_DEFINITION_FOREACH_DO( manager, Region, region )
    {
        /* Collect necessary data */
        const char* region_name = SCOREP_UNIFIED_HANDLE_DEREF( definition->name_handle,
                                                               String )->string_data;
        const char* adapter   = scorep_adapter_type_to_string( definition->adapter_type );
        const char* file_name = "";
        if ( definition->file_name_handle != SCOREP_INVALID_STRING )
        {
            file_name = SCOREP_UNIFIED_HANDLE_DEREF( definition->file_name_handle, String )->string_data;
        }

        /* Register region to cube */
        cube_region* cube_handle = cube_def_region( my_cube,
                                                    region_name,
                                                    definition->begin_line,
                                                    definition->end_line,
                                                    "", /* URL */
                                                    adapter,
                                                    file_name );

        /* Create entry in mapping table */
        scorep_cube4_add_region_mapping( map, cube_handle, handle );
    }
    SCOREP_DEFINITION_FOREACH_WHILE();
}

/**
   Writes callpath definitions to Cube. The new Cube definitions are added to the
   mapping table @a map.
   @param my_cube Pointer to Cube instance.
   @param manager Pointer to Score-P definition manager with unified definitions.
   @param map     Pointer to mapping instance to map Score-P und Cube definitions.
 */
static void
scorep_write_callpath_definitions_to_cube4( cube_t*                       my_cube,
                                            SCOREP_DefinitionManager*     manager,
                                            scorep_cube4_definitions_map* map )
{
    cube_cnode*           cnode  = NULL;
    cube_region*          region = NULL;
    cube_cnode*           parent = NULL;
    SCOREP_RegionHandle   scorep_region;
    SCOREP_CallpathHandle scorep_callpath;

    /* We must write the callpathes in the order that the sequence_number of the
       unified definitions go from 0 to n-1. The unified definitions on rank zero
       are in the correct order.
     */
    SCOREP_DEFINITION_FOREACH_DO( manager, Callpath, callpath )
    {
        /* Collect necessary data */
        scorep_region   = definition->callpath_argument.region_handle;
        region          = scorep_get_cube4_region( map, scorep_region );
        scorep_callpath = definition->parent_callpath_handle;
        parent          = scorep_get_cube4_callpath( map, scorep_callpath );

        /* Register region to cube */
        cnode = cube_def_cnode( my_cube, region, parent );

        /* Create entry in mapping table */
        scorep_cube4_add_callpath_mapping( map, cnode, handle );
    }
    SCOREP_DEFINITION_FOREACH_WHILE();
}

/**
   Writes the inner nodes of the Score-P system tree definitions to Cube.
   Because Cube assumes a fixed hierarchy the system tree is comressed in the two Cube
   levels machine and node. In order to have a mapping between Cube and Score-P
   definitions a tree is constructed where each node contains the respective definitions.
   The tree's nodes are stored in a newly created array, which is returned from the
   function.
   @param my_cube Pointer to Cube instance.
   @param manager Pointer to Score-P definition manager with unified definitions.
   @returns A Pointer to the array which contains the mapping system tree. This array
            must be freed by the caller of this function.
 */
static scorep_cube_system_node*
scorep_write_cube_system_tree( cube_t*                   my_cube,
                               SCOREP_DefinitionManager* manager )
{
    uint32_t                 end         = 0;
    uint32_t                 nodes       = manager->system_tree_node_definition_counter;
    scorep_cube_system_node* system_tree = ( scorep_cube_system_node* )
                                           malloc( sizeof( scorep_cube_system_node ) * nodes );

    if ( system_tree == NULL )
    {
        SCOREP_ERROR_POSIX( "Failed to allocate memory for system tree translation." );
        return NULL;
    }

    /* The node for rank i is at position i in the system tree array. Inner nodes
       follow afterwards. */
    SCOREP_DEFINITION_FOREACH_DO( manager, SystemTreeNode, system_tree_node )
    {
        const char* class = SCOREP_UNIFIED_HANDLE_DEREF( definition->class_handle,
                                                         String )->string_data;
        const char* name = SCOREP_UNIFIED_HANDLE_DEREF( definition->name_handle,
                                                        String )->string_data;

        system_tree[ end ].scorep_node     = handle;
        system_tree[ end ].my_cube_machine = NULL;
        system_tree[ end ].my_cube_node    = NULL;
        system_tree[ end ].parent          = scorep_cube_find_system_node( system_tree, end,
                                                                           definition->parent_handle );

        /* Register nodes to cube on machine and node level, because Cube has still
           a fixed system tree hiarachy */
        if ( strcmp( "machine", class ) == 0 )
        {
            system_tree[ end ].my_cube_machine = cube_def_mach( my_cube, name, "" );
        }
        else if ( ( strcmp( "node", class ) == 0 ) ||
                  ( strcmp( "nodecard", class ) == 0 ) )
        {
            system_tree[ end ].my_cube_node = cube_def_node( my_cube, name,
                                                             scorep_cube_get_mach( &system_tree[ end ] ) );
        }

        end++;
    }
    SCOREP_DEFINITION_FOREACH_WHILE();
    return system_tree;
}

/**
   Writes location and location group  definitions to Cube.
   @param my_cube Pointer to Cube instance.
   @param manager Pointer to Score-P definition manager with unified definitions.
   @param ranks   Number of MPI ranks. It must equal the number of array elements in
                  @a threads.
   @param threads Array of the number of threads in each rank. The ith entry contains
                  the number of threads in rank i. The number of elements must equal
                  @a ranks.
 */
static void
scorep_write_location_definitions_to_cube4( cube_t*                   my_cube,
                                            SCOREP_DefinitionManager* manager,
                                            uint32_t                  ranks,
                                            int*                      threads )
{
    char                     name[ 32 ];
    cube_process*            process = NULL;
    cube_thread*             thread  = NULL;
    int                      index   = 0;

    scorep_cube_system_node* system_tree = scorep_write_cube_system_tree( my_cube,
                                                                          manager );
    assert( system_tree );

    SCOREP_DEFINITION_FOREACH_DO( manager, LocationGroup, location_group )
    {
        uint32_t   rank = definition->global_location_group_id;
        cube_node* node = scorep_cube_get_node( my_cube, system_tree, definition->parent,
                                                manager->system_tree_node_definition_counter );

        sprintf( name, "rank %u", rank );
        process = cube_def_proc( my_cube, name, rank, node );

        for ( uint32_t loc = 0; loc < threads[ rank ]; loc++ )
        {
            sprintf( name, "thread %" PRIu32, loc ),
            thread = cube_def_thrd( my_cube, name, index, process );
            index++;
        }
    }
    SCOREP_DEFINITION_FOREACH_WHILE();

    free( system_tree );
}

/* ****************************************************************************
 * Main definition writer function
 *****************************************************************************/
void
scorep_write_definitions_to_cube4( cube_t*                       my_cube,
                                   scorep_cube4_definitions_map* map,
                                   uint32_t                      ranks,
                                   int*                          threads )
{
    /* The unification is always processed, even in serial case. Thus, we have
       always access to the unified definitions on rank 0.
       In non-mpi case SCOREP_Mpi_GetRank() returns always 0. Thus, we need only
       to test for the rank. */
    SCOREP_DefinitionManager* manager = scorep_unified_definition_manager;
    if ( SCOREP_Mpi_GetRank() != 0 )
    {
        return;
    }
    assert( scorep_unified_definition_manager );

    scorep_write_metric_definitions_to_cube4( my_cube, manager, map );
    scorep_write_region_definitions_to_cube4( my_cube, manager, map );
    scorep_write_callpath_definitions_to_cube4( my_cube, manager, map );
    scorep_write_location_definitions_to_cube4( my_cube, manager, ranks, threads );
}
