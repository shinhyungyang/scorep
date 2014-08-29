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
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */



/**
 * @file
 *
 *
 */


#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>

#include <SCOREP_Types.h>
#include <SCOREP_Timing.h>
#include <SCOREP_Memory.h>
#include <SCOREP_Config.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Definitions.h>

#include <definitions/SCOREP_Definitions.h>
#include <scorep_subsystem.h>
#include <scorep_ipc.h>

void
SCOREP_RegisterAllConfigVariables( void );
void
scorep_mpi_register_regions( void );
void
SCOREP_Status_OnMppInit( void );
void
SCOREP_Status_OnMppFinalize( void );
void
SCOREP_Unify( void );
void
SCOREP_Definitions_Initialize( void );
void
scorep_mpi_setup_world( void );
uint64_t
SCOREP_Env_GetTotalMemory();
uint64_t
SCOREP_Env_GetPageSize();


static uint64_t              scorep_test_mpi_unify_verbose;
static bool                  scorep_test_mpi_unify_define_mpi_group;
static bool                  scorep_test_mpi_unify_define_mpi_regions;
static SCOREP_ConfigVariable scorep_test_mpi_unify_config_variables[] = {
    {
        "mpi_unify_verbose",
        SCOREP_CONFIG_TYPE_NUMBER,
        &scorep_test_mpi_unify_verbose,
        NULL,
        "0",
        "verbosity",
        ""
    },
    {
        "mpi_unify_define_mpi_regions",
        SCOREP_CONFIG_TYPE_BOOL,
        &scorep_test_mpi_unify_define_mpi_regions,
        NULL,
        "false",
        "",
        ""
    },
    {
        "mpi_unify_define_mpi_group",
        SCOREP_CONFIG_TYPE_BOOL,
        &scorep_test_mpi_unify_define_mpi_group,
        NULL,
        "true",
        "",
        ""
    },
    SCOREP_CONFIG_TERMINATOR
};

int
main( int argc, char* argv[] )
{
    SCOREP_ConfigInit();
    SCOREP_RegisterAllConfigVariables();
    SCOREP_ConfigRegister( "test", scorep_test_mpi_unify_config_variables );
    SCOREP_ConfigApplyEnv();

    SCOREP_Timer_Initialize();

    SCOREP_Memory_Initialize( SCOREP_Env_GetTotalMemory(),
                              SCOREP_Env_GetPageSize() );


    SCOREP_Definitions_Initialize();

    PMPI_Init( &argc, &argv );

    SCOREP_Status_OnMppInit();
    scorep_mpi_setup_world();

    int size;
    PMPI_Comm_size( MPI_COMM_WORLD, &size );

    int rank;
    PMPI_Comm_rank( MPI_COMM_WORLD, &rank );

    char rank_buffer[ 32 ];
    sprintf( rank_buffer, "Rank %d", rank );

    SCOREP_Definitions_NewRegion( rank_buffer,
                                  NULL,
                                  0, //SCOREP_Definitions_NewSourceFile( __FILE__ ),
                                  51,
                                  80,
                                  SCOREP_PARADIGM_USER,
                                  SCOREP_REGION_FUNCTION );

    SCOREP_Definitions_NewRegion( rank_buffer,
                                  NULL,
                                  0, //SCOREP_Definitions_NewSourceFile( __FILE__ ),
                                  51,
                                  80,
                                  SCOREP_PARADIGM_USER,
                                  SCOREP_REGION_FUNCTION );

    if ( scorep_test_mpi_unify_define_mpi_regions )
    {
        scorep_mpi_register_regions();
    }

    if ( scorep_test_mpi_unify_define_mpi_group )
    {
        /* build an MPI group */
        int32_t* group_members;
        group_members = calloc( size, sizeof( *group_members ) );
        assert( group_members );
        for ( int i = 0; i < size; i++ )
        {
            group_members[ i ] = i;
        }
        SCOREP_Definitions_NewGroupFrom32( SCOREP_GROUP_MPI_GROUP,
                                           "",
                                           size,
                                           ( const uint32_t* )group_members );
        free( group_members );
    }

    double timing = SCOREP_GetClockTicks();
    SCOREP_Unify();
    timing = ( SCOREP_GetClockTicks() - timing ) / SCOREP_GetClockResolution();

    char result_name[ 64 ];
    sprintf( result_name, "mpi_unification_test.%d.result", size );

    if ( rank == 0 )
    {
        FILE* result = fopen( result_name, "w" );
        assert( result );

        fprintf( result, "t: %f\n", timing );

        //assert( scorep_unified_definition_manager->region.counter == ( uint32_t )size );
        if ( scorep_test_mpi_unify_verbose >= 1 )
        {
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                                 Region, region )
            {
                fprintf( result, "ur: %u (%s)\n",
                         definition->sequence_number,
                         SCOREP_HANDLE_DEREF( definition->name_handle, String, scorep_unified_definition_manager->page_manager )->string_data );
                fflush( result );
            }
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();
        }

        /*
         * our mpi group (which will also be used fo MPI_COMM_WORLD),
         * the MPI locations
         * the MPI comm self group
         */
        if ( scorep_test_mpi_unify_verbose >= 2 )
        {
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                                 Group, group )
            {
                fprintf( result, "ug: %u %u #%" PRIu64 "\n",
                         definition->sequence_number,
                         definition->group_type,
                         definition->number_of_members );
                fflush( result );
            }
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                                 Communicator, communicator )
            {
                fprintf( result, "uc: %u ug:%u\n",
                         definition->sequence_number,
                         SCOREP_HANDLE_TO_ID( definition->group_handle,
                                              Group,
                                              scorep_unified_definition_manager->page_manager ) );
                fflush( result );
            }
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();
        }
        assert( 3 == scorep_unified_definition_manager->group.counter );

        fclose( result );
    }
    SCOREP_Ipc_Barrier();

    if ( scorep_test_mpi_unify_verbose >= 2 )
    {
        for ( int the_rank = 0; the_rank < size; the_rank++ )
        {
            if ( the_rank == rank )
            {
                FILE* result = fopen( result_name, "a" );
                assert( result );

                SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( &scorep_local_definition_manager,
                                                                     Region, region )
                {
                    //assert( ( uint64_t )rank == scorep_local_definition_manager.region.mapping[ definition->sequence_number ] );
                    fprintf( result, "%d:r: %u -> %u\n",
                             rank,
                             definition->sequence_number,
                             scorep_local_definition_manager.region.mapping[ definition->sequence_number ] );
                    fflush( result );
                }
                SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

                SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( &scorep_local_definition_manager,
                                                                     Group, group )
                {
                    fprintf( result, "%d:g: %u -> %u\n",
                             rank,
                             definition->sequence_number,
                             scorep_local_definition_manager.group.mapping[ definition->sequence_number ] );
                    fflush( result );
                }
                SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

                SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( &scorep_local_definition_manager,
                                                                     InterimCommunicator,
                                                                     interim_communicator )
                {
                    fprintf( result, "%d:c: %u -> %u\n",
                             rank,
                             definition->sequence_number,
                             scorep_local_definition_manager.interim_communicator.mapping[ definition->sequence_number ] );
                    fflush( result );
                }
                SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

                fclose( result );
            }

            SCOREP_Ipc_Barrier();
        }
    }

    SCOREP_Status_OnMppFinalize();

    PMPI_Finalize();

    return 0;
}
