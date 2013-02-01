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
 * @file scorep_profile_debug.c
 * Provides extended output on errors in the profile.
 *
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @status alpha
 *
 */

#include <config.h>
#include <scorep_profile_debug.h>
#include <scorep_profile_definition.h>
#include <scorep_profile_location.h>

#include <scorep_mpi.h>
#include <scorep_runtime_management.h>
#include <scorep_openmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*----------------------------------------------------------------------------------------
   internal helper functions
   --------------------------------------------------------------------------------------*/
static void
scorep_dump_node( FILE* file, scorep_profile_node* node )
{
    static char* type_name_map[] =
    {
        "regular region",
        "paramater string",
        "parameter integer",
        "thread root",
        "thread start",
        "collapse",
        "task root"
    };

    if ( node == NULL )
    {
        return;
    }

    if ( node->node_type >= sizeof( type_name_map ) / sizeof( char* ) )
    {
        fprintf( file, "unknown type: %d", node->node_type );
        return;
    }

    fprintf( file, "type: %s\t", type_name_map[ node->node_type ] );
    if ( node->node_type == scorep_profile_node_regular_region )
    {
        fprintf( file, "name: %s", SCOREP_Region_GetName( scorep_profile_type_get_region_handle( node->type_specific_data ) ) );
    }
    else if ( node->node_type == scorep_profile_node_thread_start )
    {
        fprintf( file, "fork node: %p",
                 scorep_profile_type_get_fork_node( node->type_specific_data ) );
    }
}

static void
scorep_dump_stack( FILE* file, SCOREP_Profile_LocationData* location )
{
    if ( location == NULL )
    {
        return;
    }
    fprintf( file, "Current stack of failing thread:\n" );
    uint32_t i = 0;
    fprintf( file, "\n" );
    for ( scorep_profile_node* current = location->current_task_node;
          current != NULL;
          current = current->parent, i++ )
    {
        fprintf( file, "%u %p\t", i, current );
        scorep_dump_node( file, current );
        fprintf( file, "\n" );
    }
    fprintf( file, "\n" );
}


static void
scorep_dump_subtree(  FILE*                file,
                      scorep_profile_node* node,
                      uint32_t             level )
{
    if ( node == NULL )
    {
        return;
    }

    fprintf( file, "%p ", node );
    for ( int i = 0; i < level; i++ )
    {
        fprintf( file, "| " );
    }
    fprintf( file, "+ " );
    scorep_dump_node( file, node );
    fprintf( file, "\n" );
    if ( node->first_child != NULL )
    {
        scorep_dump_subtree( file, node->first_child, level + 1 );
    }
    if ( node->next_sibling != NULL )
    {
        scorep_dump_subtree( file, node->next_sibling, level );
    }
}

void
scorep_profile_dump( FILE* file, SCOREP_Profile_LocationData* location )
{
    fprintf( file, "\n" );
    if ( SCOREP_Omp_InParallel() )
    {
        if ( location != NULL && location->root_node != NULL )
        {
            fprintf( file, "Current status of failing profile:\n" );
            scorep_dump_subtree( file, location->root_node->first_child, 0 );
        }
    }
    else
    {
        fprintf( file, "Current state of the profile of all threads:\n" );
        scorep_dump_subtree( file, scorep_profile.first_root_node, 0 );
    }
    fprintf( file, "\n" );
}

/*----------------------------------------------------------------------------------------
   visible functions
   --------------------------------------------------------------------------------------*/

void
scorep_profile_on_error( SCOREP_Profile_LocationData* location )
{
    /* Diable further profiling */
    scorep_profile.is_initialized = false;

    /* If core files are enabled, write a core file */
    if ( scorep_profile_do_core_files() &&
         ( !SCOREP_Omp_InParallel() || location != NULL ) )
    {
        uint32_t    thread   = 0;
        const char* dirname  = SCOREP_GetExperimentDirName();
        const char* basename = scorep_profile_get_basename();
        char*       filename = NULL;

        filename = ( char* )malloc( strlen( dirname ) + /* Directory      */
                                    strlen( basename )  /* basename       */
                                    + 32 );             /* constant stuff */

        if ( filename == NULL )
        {
            return;
        }

        if ( location != NULL )
        {
            thread = SCOREP_Location_GetId( location->location_data );
        }

        sprintf( filename, "%s/%s.%d.%u.core", dirname, basename,
                 SCOREP_Mpi_GetRank(), thread );

        FILE* file = fopen( filename, "a" );
        free( filename );

        if ( file == NULL )
        {
            return;
        }

        fprintf( file, "ERROR on rank %d, thread %u\n\n",
                 SCOREP_Mpi_GetRank(), thread );

        scorep_dump_stack( file, location );
        scorep_profile_dump( file, location );

        fclose( file );
    }
}
