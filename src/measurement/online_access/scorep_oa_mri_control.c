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
 * @file scorep_oa_mri_control.c
 * @maintainer Yury Oleynik <oleynik@in.tum.de>
 * @status alpha
 *
 * This file contains implementation of OA measurement configuration and application execution control functions
 */

#include <config.h>

#include "scorep_oa_mri_control.h"

#include <stdio.h>
#include <strings.h>
#ifdef WITH_MPI
        #include <mpi.h>
        #include <SCOREP_Mpi.h>
#endif
#include <ctype.h>

#include <SCOREP_Profile.h>
#include <SCOREP_Profile_OAConsumer.h>
#include <SCOREP_RuntimeManagement.h>

#include "scorep_oa_connection.h"
#include "scanner.h"
#include "scorep_profile_node.h"
#include "scorep_profile_definition.h"




static scorep_oa_mri_app_control_type appl_control = SCOREP_OA_MRI_STATUS_UNDEFINED;

static int32_t                        scorep_phase_fileid, scorep_phase_rfl;

static SCOREP_RegionHandle            phase_handle = SCOREP_INVALID_REGION;

typedef struct Measurement
{
    int     rank;       ///<MPI process
    int     thread;     ///<thread
    int     file;       ///<region fileId
    int     rfl;        ///<region first line number
    int     regionType; ///<regionType, e.g., loop, subroutine
    int     samples;    ///<number of measurements
    int     metric;     ///<metric, e.g., execution time
    int     ignore;     ///<number of measurements that could not be performed due to shortage of counters
    double  fpVal;      ///<value as floating point number
    int64_t intVal;     ///<value as integer number
} MeasurementType;

static void
scorep_oa_mri_restart_profiling
(
);

static void
scorep_profile_print_func
(
    scorep_profile_node* node,
    void*                param
);

static void
scorep_oa_mri_parse_subtree( scorep_profile_node* node,
                             uint32_t             level );

static void
scorep_oa_mri_dump_profile();

SCOREP_Error_Code
scorep_oa_mri_receive_and_process_requests
(
    int connection
)
{
    int  length, i;
    char buffer[ 2000 ];
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
    buffer[ 0 ] = 0;
    bzero( buffer, 2000 );

    while ( scorep_oa_mri_get_appl_control() != SCOREP_OA_MRI_STATUS_RUNNING_TO_END
            && scorep_oa_mri_get_appl_control() != SCOREP_OA_MRI_STATUS_RUNNING_TO_BEGINNING
            && scorep_oa_mri_get_appl_control() != SCOREP_OA_MRI_EXEC_REQUEST_TERMINATE )
    {
        bzero( buffer, 2000 );

        while ( ( length = scorep_oa_connection_read_string( connection, buffer, 2000 ) ) == 0 )
        {
        }

        for ( i = 0; i < length; i++ )
        {
            buffer[ i ] = toupper( buffer[ i ] );
        }
        SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Received from socket: %s", buffer );

        if ( scorep_oa_mri_parse( buffer ) != SCOREP_SUCCESS )
        {
            SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "ERROR in parsing MRI command" );
        }
        if ( scorep_oa_mri_get_appl_control() == SCOREP_OA_MRI_EXEC_REQUEST_TERMINATE )
        {
            SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Application Terminating!" );
            //cleanup_registry();
            //PMPI_Finalize();
            //exit(0);
            SCOREP_FinalizeMeasurement();
            _Exit( EXIT_SUCCESS );
        }
    }
#ifdef WITH_MPI
    PMPI_Barrier( MPI_COMM_WORLD );
#endif
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Leaving %s", __FUNCTION__ );
    return SCOREP_SUCCESS;
}

void
scorep_oa_mri_set_appl_control
(
    scorep_oa_mri_app_control_type command,
    uint8_t                        file_id,
    uint8_t                        region_line
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Execution control is SET TO: %d", ( int )command );
    appl_control = command;
    // appl_control_file=file_id;
    //appl_control_region=region_line;
}

void
scorep_oa_mri_set_phase
(
    SCOREP_RegionHandle handle
)
{
    phase_handle = handle;
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Phase set to region (handle = %ld )\n", phase_handle );
}

scorep_oa_mri_app_control_type
scorep_oa_mri_get_appl_control
(
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
    return appl_control;
}

void
scorep_oa_mri_return_summary_data
(
    int connection
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );

    /** Initialize OA Consumer interface and index Profile data */
    SCOREP_OAConsumer_Initialize( phase_handle );

    /** Get number of merged regions definitions*/
    int                          region_defs_size = ( int )SCOREP_OAConsumer_GetDataSize( MERGED_REGION_DEFINITIONS );
    /** Generate merged regions definitions buffer*/
    SCOREP_OA_CallPathRegionDef* region_defs = ( SCOREP_OA_CallPathRegionDef* )SCOREP_OAConsumer_GetData(
        MERGED_REGION_DEFINITIONS );

    /** Send merged region definitions to the agent*/
    //printf( "Sending MERGED_REGION_DEFINITIONS size: %d elements of size %d\n", region_defs_size, sizeof( SCOREP_OA_CallPathRegionDef ) );
    scorep_oa_connection_send_string( connection, "MERGED_REGION_DEFINITIONS\n" );
    scorep_oa_connection_send_data( connection, region_defs, region_defs_size, sizeof( SCOREP_OA_CallPathRegionDef ) );

    /** Get number of static profile records*/
    int                               static_profile_size = ( int )SCOREP_OAConsumer_GetDataSize( FLAT_PROFILE );
    /** Get static profile buffer*/
    SCOREP_OA_FlatProfileMeasurement* static_profile = ( SCOREP_OA_FlatProfileMeasurement* )SCOREP_OAConsumer_GetData(
        FLAT_PROFILE );
    /** Send static profile to the agent*/
    //printf( "Sending STATIC_PROFILE size: %d elements of size %d\n", static_profile_size, sizeof( SCOREP_OA_FlatProfileMeasurement ) );
    scorep_oa_connection_send_string( connection, "FLAT_PROFILE\n" );
    scorep_oa_connection_send_data( connection, static_profile, static_profile_size, sizeof( SCOREP_OA_FlatProfileMeasurement ) );


    /** Dissmiss the data*/
    SCOREP_OAConsumer_DismissData();
}





void
scorep_oa_mri_noop
(
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
}


SCOREP_Error_Code
scorep_oa_mri_parse
(
    char* buffer
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
    SCOREP_Error_Code return_status = SCOREP_SUCCESS;

    YY_BUFFER_STATE   my_string_buffer;

    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "parse_mri_cmd: %s", buffer );
    my_string_buffer = yy_scan_string( buffer );
    if ( yyparse() != 0 )
    {
        return_status = SCOREP_ERROR_OA_PARSE_MRI;
    }
    yy_delete_buffer( my_string_buffer );

    return return_status;
}

void
scorep_oa_mri_set_mpiprofiling
(
    int value
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
#ifdef WITH_MPI
    SCOREP_MPI_HOOKS_SET( value );
#else
    scorep_oa_connection_send_string( connection, "this is serial version of Score-P, no MPI available\n" );
#endif
}

void
scorep_oa_mri_set_profiling
(
    int value
)
{
    SCOREP_DEBUG_RAW_PRINTF( SCOREP_DEBUG_OA, "Entering %s", __FUNCTION__ );
    //SCOREP_MPI_HOOKS_SET(value);
}

void
scorep_oa_mri_restart_profiling
(
)
{
    SCOREP_Profile_Finalize();
    SCOREP_Profile_Initialize( 0, NULL );
}

void
scorep_oa_mri_setphase
(
    int32_t file_id,
    int32_t rfl
)
{
    scorep_phase_fileid = file_id;
    scorep_phase_rfl    = rfl;
}
