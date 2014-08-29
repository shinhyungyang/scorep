#ifndef SCOREP_TIMING_INTERNAL_H
#define SCOREP_TIMING_INTERNAL_H

/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 *    RWTH Aachen, Germany
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
 * @file
 *
 * Provide timing macros, variables, and utilities for timing the Score-P
 * initialization and finalization functions. You need to have
 * HAVE_SCOREP_RUNTIME_MANAGEMENT_TIMINGS defined to non-zero to activate the
 * macros. @see SCOREP_RuntimeManagement.c, scorep_mpi_enabled, and
 * SCOREP_Mpi_Init.c.
 *
 *
 *
 */


#if HAVE( SCOREP_RUNTIME_MANAGEMENT_TIMINGS )

#include <stdint.h>
#include "scorep_ipc.h"

#define SCOREP_TIMING_FUNCTIONS                                         \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_ConfigInit )                     \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_RegisterAllConfigVariables )     \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_ConfigApplyEnv )                 \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Status_Initialize )              \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_CreateExperimentDir )            \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Memory_Initialize )              \
    SCOREP_TIMING_TRANSFORM_OP( scorep_otf2_initialize )                \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Definitions_Initialize )         \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Location_Initialize )            \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_ThreadForkJoin_Initialize )      \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Filter_Initialize )              \
    SCOREP_TIMING_TRANSFORM_OP( scorep_subsystems_initialize )          \
    SCOREP_TIMING_TRANSFORM_OP( scorep_subsystems_initialize_location ) \
    SCOREP_TIMING_TRANSFORM_OP( scorep_profile_initialize )             \
    SCOREP_TIMING_TRANSFORM_OP( scorep_properties_initialize )          \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_BeginEpoch )                     \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_InitMeasurement )                \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_InitMppMeasurement )             \
    SCOREP_TIMING_TRANSFORM_OP( MeasurementDuration  )                  \
    SCOREP_TIMING_TRANSFORM_OP( scorep_trigger_exit_callbacks )         \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_SynchronizeClocks )              \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_EndEpoch )                       \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Filter_Finalize )                \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Location_FinalizeDefinitions )   \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Tracing_FinalizeEventWriters )   \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Profile_Process )                \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_FinalizeLocationGroup )          \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Unify )                          \
    SCOREP_TIMING_TRANSFORM_OP( scorep_properties_write )               \
    SCOREP_TIMING_TRANSFORM_OP( scorep_profile_finalize )               \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Definitions_Write )              \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Definitions_Finalize )           \
    SCOREP_TIMING_TRANSFORM_OP( scorep_otf2_finalize )                  \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Location_FinalizeLocations )     \
    SCOREP_TIMING_TRANSFORM_OP( scorep_subsystems_finalize )            \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Location_Finalize )              \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_ConfigFini )                     \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_RenameExperimentDir )            \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Status_Finalize )                \
    SCOREP_TIMING_TRANSFORM_OP( scorep_subsystems_deregister )          \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_ThreadForkJoin_Finalize )        \
    SCOREP_TIMING_TRANSFORM_OP( SCOREP_Memory_Finalize )                \
    SCOREP_TIMING_TRANSFORM_OP( scorep_finalize )

enum scorep_timing_functions
{
    #define SCOREP_TIMING_TRANSFORM_OP( function ) function ## _,
    SCOREP_TIMING_FUNCTIONS
    #undef SCOREP_TIMING_TRANSFORM_OP
    scorep_timing_num_entries
};

extern const char* scorep_timing_function_names[ scorep_timing_num_entries ];
extern double      scorep_timing_sendbuf_durations[ scorep_timing_num_entries ];
extern double      scorep_timing_recvbuf_durations_mean[ scorep_timing_num_entries ];
extern double      scorep_timing_recvbuf_durations_max[ scorep_timing_num_entries ];
extern double      scorep_timing_recvbuf_durations_min[ scorep_timing_num_entries ];

#define SCOREP_TIME( function, args ) \
    do { \
        scorep_timing_sendbuf_durations[ function ## _ ] = \
            SCOREP_GetClockTicks(); \
        function args; \
        scorep_timing_sendbuf_durations[ function ## _ ] = \
            SCOREP_GetClockTicks() - \
            scorep_timing_sendbuf_durations[ function ## _ ]; \
    } while ( 0 )

#define SCOREP_TIME_START_TIMING( function ) \
    do { \
        scorep_timing_sendbuf_durations[ function ## _ ] = \
            SCOREP_GetClockTicks(); \
    } while ( 0 )

#define SCOREP_TIME_STOP_TIMING( function ) \
    do { \
        scorep_timing_sendbuf_durations[ function ## _ ] = \
            SCOREP_GetClockTicks() - \
            scorep_timing_sendbuf_durations[ function ## _ ]; \
    } while ( 0 )


#define SCOREP_TIME_PRINT_TIMINGS() \
    if ( SCOREP_Status_GetRank() == 0 ) \
    { \
        double clock_resolution = SCOREP_GetClockResolution(); \
        if ( SCOREP_Status_IsMpp() ) \
        { \
            printf( "[Score-P] Score-P runtime-management timings: function              min[s]         mean[s]          max[s]\n" ); \
        } \
        else \
        { \
            printf( "[Score-P] Score-P runtime-management timings: function             time[s]\n" ); \
        } \
        for ( int i = 0; i < scorep_timing_num_entries; ++i ) \
        { \
            if ( SCOREP_Status_IsMpp() ) \
            { \
                /* special handling for times that are measured only */ \
                /* after PMPI_Finalize, i.e. they don't take part in */ \
                /* the reduce step and always belong to rank 0. */ \
                if ( i >= scorep_subsystems_deregister_ ) \
                { \
                    printf( "[Score-P] %-48s                 %15.6f\n", \
                            scorep_timing_function_names[ i ], \
                            scorep_timing_sendbuf_durations[ i ] / clock_resolution ); \
                } \
                else \
                { \
                    printf( "[Score-P] %-48s %15.6f %15.6f %15.6f\n", \
                            scorep_timing_function_names[ i ], \
                            scorep_timing_recvbuf_durations_min[ i ] / clock_resolution, \
                            scorep_timing_recvbuf_durations_mean[ i ] / clock_resolution, \
                            scorep_timing_recvbuf_durations_max[ i ] / clock_resolution ); \
                } \
            } \
            else \
            { \
                printf( "[Score-P] %-48s %15.6f\n", \
                        scorep_timing_function_names[ i ], \
                        scorep_timing_sendbuf_durations[ i ] / clock_resolution ); \
            } \
        } \
    }

#else

#define SCOREP_TIME( function, args )        function args
#define SCOREP_TIME_START_TIMING( function ) do { } while ( 0 )
#define SCOREP_TIME_STOP_TIMING( function )  do { } while ( 0 )
#define SCOREP_TIME_PRINT_TIMINGS()          do { } while ( 0 )

#endif

extern void
scorep_timing_reduce_runtime_management_timings( void );


#endif /* SCOREP_TIMING_INTERNAL_H */
