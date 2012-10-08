/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
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
 * @status      alpha
 * @file        SCOREP_Events.c
 * @maintainer  Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @brief   Declaration of event recording functions to be used by the
 *          adapter layer.
 *
 *
 */

#include <config.h>

#include <SCOREP_Events.h>

#include <stdlib.h>
#include <stdio.h>

#include <UTILS_Error.h>
#include <UTILS_Debug.h>

#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Properties.h>
#include <SCOREP_Timing.h>
#include <tracing/SCOREP_Tracing_Events.h>
#include <SCOREP_Profile.h>
#include <SCOREP_Profile_Tasking.h>
#include <SCOREP_Profile_MpiEvents.h>
#include <SCOREP_Metric_Management.h>
#include <scorep_openmp.h>

#include "scorep_runtime_management.h"
#include "scorep_types.h"
#include "scorep_handles.h"
#include "scorep_thread.h"
#include "scorep_status.h"
#include "scorep_definition_structs.h"
#include "scorep_definitions.h"

static uint64_t
scorep_get_timestamp( SCOREP_Location* location )
{
    uint64_t timestamp = SCOREP_GetClockTicks();
    SCOREP_Location_SetLastTimestamp( location, timestamp );
    return timestamp;
}



/** Use this predicate to decide, whether to record an event in the trace. */
static inline bool
scorep_tracing_consume_event( void )
{
    return SCOREP_IsTracingEnabled() && SCOREP_RecordingEnabled();
}


/**
 * Process a region enter event in the measurement system.
 */
static void
scorep_enter_region( uint64_t            timestamp,
                     SCOREP_RegionHandle regionHandle,
                     uint64_t*           metricValues,
                     SCOREP_Location*    location )
{
    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Reg:%s",
                        scorep_region_to_string( stringBuffer,
                                                 sizeof( stringBuffer ),
                                                 "%x", regionHandle ) );

    if ( scorep_tracing_consume_event() )
    {
        if ( metricValues )
        {
            /* @todo: Writing metrics to trace file will be improved in the near future */

            SCOREP_Metric_WriteToTrace( location,
                                        timestamp );
        }

        SCOREP_Tracing_Enter( location,
                              timestamp,
                              regionHandle );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_Enter( location, regionHandle,
                              SCOREP_Region_GetType( regionHandle ),
                              timestamp, metricValues );
    }
}


void
SCOREP_EnterRegion( SCOREP_RegionHandle regionHandle )
{
    SCOREP_Location* location      = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp     = scorep_get_timestamp( location );
    uint64_t*        metric_values = SCOREP_Metric_Read( location );

    scorep_enter_region( timestamp, regionHandle, metric_values, location );
}

/*
 * NOTE: If dense metrics are used in the profile,
 * they need to be specified for these two functions!
 */
void
SCOREP_Location_EnterRegion( SCOREP_Location*    location,
                             uint64_t            timestamp,
                             SCOREP_RegionHandle regionHandle )
{
    UTILS_BUG_ON( !location && SCOREP_Location_GetType( location ) == SCOREP_LOCATION_TYPE_CPU_THREAD,
                  "SCOREP_Location_EnterRegion() must not be used for CPU thread locations." );

    if ( !location )
    {
        location = SCOREP_Location_GetCurrentCPULocation();
    }

    UTILS_BUG_ON( timestamp < SCOREP_Location_GetLastTimestamp( location ),
                  "Invalid event order." );

    uint64_t* metric_values = SCOREP_Metric_Read( location );

    scorep_enter_region( timestamp, regionHandle, metric_values, location );

    SCOREP_Location_SetLastTimestamp( location, timestamp );
}


/**
 * Process a region exit event in the measurement system.
 */
static void
scorep_exit_region( uint64_t            timestamp,
                    SCOREP_RegionHandle regionHandle,
                    uint64_t*           metricValues,
                    SCOREP_Location*    location )
{
    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Reg:%s",
                        scorep_region_to_string( stringBuffer,
                                                 sizeof( stringBuffer ),
                                                 "%x", regionHandle ) );

    if ( scorep_tracing_consume_event() )
    {
        if ( metricValues )
        {
            /* @todo: Writing metrics to trace file will be improved in the near future */

            SCOREP_Metric_WriteToTrace( location,
                                        timestamp );
        }

        SCOREP_Tracing_Leave( location,
                              timestamp,
                              regionHandle );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_Exit( location,
                             regionHandle,
                             timestamp,
                             metricValues );
    }
}


void
SCOREP_ExitRegion( SCOREP_RegionHandle regionHandle )
{
    SCOREP_Location* location      = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp     = scorep_get_timestamp( location );
    uint64_t*        metric_values = SCOREP_Metric_Read( location );

    scorep_exit_region( timestamp, regionHandle, metric_values, location );
}

void
SCOREP_Location_ExitRegion( SCOREP_Location*    location,
                            uint64_t            timestamp,
                            SCOREP_RegionHandle regionHandle )
{
    UTILS_BUG_ON( !location && SCOREP_Location_GetType( location ) == SCOREP_LOCATION_TYPE_CPU_THREAD,
                  "SCOREP_Location_ExitRegion() must not be used for CPU thread locations." );

    if ( !location )
    {
        location = SCOREP_Location_GetCurrentCPULocation();
    }

    UTILS_BUG_ON( timestamp < SCOREP_Location_GetLastTimestamp( location ),
                  "Invalid event order." );

    uint64_t* metric_values = SCOREP_Metric_Read( location );

    scorep_exit_region( timestamp, regionHandle, metric_values, location );

    SCOREP_Location_SetLastTimestamp( location, timestamp );
}


/**
 * Generate a rewind region enter event in the measurement system.
 */
void
SCOREP_EnterRewindRegion( SCOREP_RegionHandle regionHandle )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "RwR:%s",
                        scorep_region_to_string( stringBuffer,
                                                 sizeof( stringBuffer ),
                                                 "%x", regionHandle ) );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_StoreRewindPoint( location, regionHandle, timestamp );
    }
}


/**
 * Generate a rewind region exit event in the measurement system.
 */
void
SCOREP_ExitRewindRegion( SCOREP_RegionHandle regionHandle, bool do_rewind )
{
    SCOREP_Location* location       = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         leavetimestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "RwR:%s",
                        scorep_region_to_string( stringBuffer,
                                                 sizeof( stringBuffer ),
                                                 "%x", regionHandle ) );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ExitRewindRegion( location, regionHandle, leavetimestamp, do_rewind );
    }
}


/**
 * Process an mpi send event in the measurement system.
 */
void
SCOREP_MpiSend( SCOREP_MpiRank                    destinationRank,
                SCOREP_LocalMPICommunicatorHandle communicatorHandle,
                uint32_t                          tag,
                uint64_t                          bytesSent )
{
    UTILS_BUG_ON( destinationRank < 0, "Invalid rank passed to SCOREP_MpiSend" );


    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Dst:%d Comm:%s Tag:%u Bytes:%llu",
                        destinationRank,
                        scorep_comm_to_string( stringBuffer,
                                               sizeof( stringBuffer ),
                                               "%x", communicatorHandle ),
                        tag,
                        ( unsigned long long )bytesSent );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiSend( location,
                                timestamp,
                                destinationRank,
                                communicatorHandle,
                                tag,
                                bytesSent );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_MpiSend( location,
                                destinationRank,
                                communicatorHandle,
                                tag,
                                bytesSent );
    }
}


/**
 * Process an mpi recv event in the measurement system.
 */
void
SCOREP_MpiRecv( SCOREP_MpiRank                    sourceRank,
                SCOREP_LocalMPICommunicatorHandle communicatorHandle,
                uint32_t                          tag,
                uint64_t                          bytesReceived )
{
    UTILS_BUG_ON( sourceRank < 0, "Invalid rank passed to SCOREP_MpiRecv" );


    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Src:%u Comm:%s Tag:%u Bytes:%llu",
                        sourceRank,
                        scorep_comm_to_string( stringBuffer,
                                               sizeof( stringBuffer ),
                                               "%x", communicatorHandle ),
                        tag,
                        ( unsigned long long )bytesReceived );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiRecv( location,
                                timestamp,
                                sourceRank,
                                communicatorHandle,
                                tag,
                                bytesReceived );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_MpiRecv( location,
                                sourceRank,
                                communicatorHandle,
                                tag,
                                bytesReceived );
    }
}

/**
 * Process an mpi collective begin event in the measurement system.
 */
uint64_t
SCOREP_MpiCollectiveBegin( SCOREP_RegionHandle regionHandle )
{
    SCOREP_Location* location      = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp     = scorep_get_timestamp( location );
    uint64_t*        metric_values = SCOREP_Metric_Read( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    scorep_enter_region( timestamp, regionHandle, metric_values, location );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiCollectiveBegin( location,
                                           timestamp );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }

    return timestamp;
}

/**
 * Process an mpi collective end event in the measurement system.
 */
void
SCOREP_MpiCollectiveEnd( SCOREP_RegionHandle               regionHandle,
                         SCOREP_LocalMPICommunicatorHandle communicatorHandle,
                         SCOREP_MpiRank                    rootRank,
                         SCOREP_MpiCollectiveType          collectiveType,
                         uint64_t                          bytesSent,
                         uint64_t                          bytesReceived )
{
    UTILS_BUG_ON( ( rootRank < 0 && rootRank != SCOREP_INVALID_ROOT_RANK ),
                  "Invalid rank passed to SCOREP_MpiCollectiveEnd\n" );

    SCOREP_Location* location      = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp     = scorep_get_timestamp( location );
    uint64_t*        metric_values = SCOREP_Metric_Read( location );


    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );


    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiCollectiveEnd( location,
                                         timestamp,
                                         communicatorHandle,
                                         rootRank,
                                         collectiveType,
                                         bytesSent,
                                         bytesReceived );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_CollectiveEnd( location,
                                      communicatorHandle,
                                      rootRank,
                                      collectiveType,
                                      bytesSent,
                                      bytesReceived );
    }

    scorep_exit_region( timestamp, regionHandle, metric_values, location );
}

void
SCOREP_MpiIsendComplete( SCOREP_MpiRequestId requestId )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiIsendComplete( location,
                                         timestamp,
                                         requestId );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}

void
SCOREP_MpiIrecvRequest( SCOREP_MpiRequestId requestId )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiIrecvRequest( location,
                                        timestamp,
                                        requestId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}

void
SCOREP_MpiRequestTested( SCOREP_MpiRequestId requestId )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiRequestTested( location,
                                         timestamp,
                                         requestId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}

void
SCOREP_MpiRequestCancelled( SCOREP_MpiRequestId requestId )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiRequestCancelled( location,
                                            timestamp,
                                            requestId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}

void
SCOREP_MpiIsend(  SCOREP_MpiRank                    destinationRank,
                  SCOREP_LocalMPICommunicatorHandle communicatorHandle,
                  uint32_t                          tag,
                  uint64_t                          bytesSent,
                  SCOREP_MpiRequestId               requestId )
{
    UTILS_BUG_ON( destinationRank < 0, "Invalid rank passed to SCOREP_MpiIsend\n" );


    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiIsend( location,
                                 timestamp,
                                 destinationRank,
                                 communicatorHandle,
                                 tag,
                                 bytesSent,
                                 requestId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_MpiSend( location,
                                destinationRank,
                                communicatorHandle,
                                tag,
                                bytesSent );
    }
}

void
SCOREP_MpiIrecv( SCOREP_MpiRank                    sourceRank,
                 SCOREP_LocalMPICommunicatorHandle communicatorHandle,
                 uint32_t                          tag,
                 uint64_t                          bytesReceived,
                 SCOREP_MpiRequestId               requestId )
{
    UTILS_BUG_ON( sourceRank < 0,  "Invalid rank passed to SCOREP_MpiIrecv\n" );


    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_MpiIrecv( location,
                                 timestamp,
                                 sourceRank,
                                 communicatorHandle,
                                 tag,
                                 bytesReceived,
                                 requestId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_MPI_COMMUNICATION_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_MpiRecv( location,
                                sourceRank,
                                communicatorHandle,
                                tag,
                                bytesReceived );
    }
}

/**
 * Process an OpenMP fork event in the measurement system.
 */
void
SCOREP_OmpFork( uint32_t nRequestedThreads )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpFork( location, timestamp, nRequestedThreads );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_OnFork( location, nRequestedThreads, scorep_thread_get_nesting_level() );
    }

    // Fork last as it modifies TPD state in the OpenMP thread implementation
    // that affects subsequent calls to SCOREP_Location_GetCurrentCPU.
    SCOREP_Thread_OnThreadFork( nRequestedThreads );
}


/**
 * Process an OpenMP join event in the measurement system.
 */
void
SCOREP_OmpJoin( void )
{
    // Join first as it modifies TPD in the OpenMP thread implementation,
    // should not affect calls to SCOREP_Location_GetCurrentCPU though
    // as master thread's location data is reused.
    SCOREP_Thread_OnThreadJoin();

    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpJoin( location,
                                timestamp );
        UTILS_DEBUG_PRINTF( 0, "Only partially implemented." );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_OnJoin( location );
    }

    /*
     * The OpenMP implementation on JUMP uses
     * its own exit handler. This exit handler is registered after our
     * Score-P exit handler in the test cases. Causing segmentation faults
     * during finalization, due to invalid TPD variables. Thus, we register
     * an exit handler in the join event, which ensures that the Score-P
     * finalization can access threadprivate variables.
     */
    static bool is_exit_handler_registered = false;
    if ( !is_exit_handler_registered && !SCOREP_Omp_InParallel() )
    {
        is_exit_handler_registered = true;
        SCOREP_RegisterExitHandler();
    }
}


/**
 * Process an OpenMP acquire lock event in the measurement system.
 */
void
SCOREP_OmpAcquireLock( uint32_t lockId,
                       uint32_t acquisitionOrder )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = SCOREP_Location_GetLastTimestamp( location );            // use the timestamp from the associated enter

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Lock:%x", lockId );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpAcquireLock( location,
                                       timestamp,
                                       lockId,
                                       acquisitionOrder );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}


/**
 * Process an OpenMP release lock event in the measurement system.
 */
void
SCOREP_OmpReleaseLock( uint32_t lockId,
                       uint32_t acquisitionOrder )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = SCOREP_Location_GetLastTimestamp( location );            // use the timestamp from the associated enter

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Lock:%x", lockId );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpReleaseLock( location,
                                       timestamp,
                                       lockId,
                                       acquisitionOrder );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}

void
SCOREP_OmpTaskCreate( uint64_t taskId )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = SCOREP_Location_GetLastTimestamp( location );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpTaskCreate( location,
                                      timestamp,
                                      taskId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_TaskCreate( location, timestamp, taskId );
    }
}

void
SCOREP_OmpTaskSwitch( uint64_t taskId )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpTaskSwitch( location,
                                      timestamp,
                                      taskId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        uint64_t* metric_values = SCOREP_Metric_Read( location );
        SCOREP_Profile_TaskSwitch( location, taskId, timestamp, metric_values );
    }
}


void
SCOREP_OmpTaskBegin( SCOREP_RegionHandle regionHandle,
                     uint64_t            taskId )
{
    SCOREP_Location* location      = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp     = scorep_get_timestamp( location );
    uint64_t*        metric_values = SCOREP_Metric_Read( location );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_OmpTaskSwitch( location, timestamp, taskId );

        if ( metric_values )
        {
            /* @todo: Writing metrics to trace file will be improved in the near future */

            SCOREP_Metric_WriteToTrace( location,
                                        timestamp );
        }

        SCOREP_Tracing_Enter( location, timestamp, regionHandle );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_TaskBegin( location,
                                  regionHandle,
                                  taskId,
                                  timestamp,
                                  metric_values );
    }
}

void
SCOREP_OmpTaskEnd( SCOREP_RegionHandle regionHandle,
                   uint64_t            taskId )
{
    SCOREP_Location* location      = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp     = scorep_get_timestamp( location );
    uint64_t*        metric_values = SCOREP_Metric_Read( location );

    if ( scorep_tracing_consume_event() )
    {
        if ( metric_values )
        {
            /* @todo: Writing metrics to trace file will be improved in the near future */

            SCOREP_Metric_WriteToTrace( location,
                                        timestamp );
        }

        SCOREP_Tracing_Leave( location, timestamp, regionHandle );
        SCOREP_Tracing_OmpTaskComplete( location, timestamp, taskId );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_OPENMP_EVENT_COMPLETE );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_TaskEnd( location,
                                regionHandle,
                                taskId,
                                timestamp,
                                metric_values );
    }
}


/**
 *
 */
void
SCOREP_ExitRegionOnException( SCOREP_RegionHandle regionHandle )
{
    SCOREP_Location* location = SCOREP_Location_GetCurrentCPULocation();
    UTILS_DEBUG_ONLY( char stringBuffer[ 16 ];
                      )

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "Reg:%s",
                        scorep_region_to_string( stringBuffer,
                                                 sizeof( stringBuffer ),
                                                 "%x", regionHandle ) );

    /* DL: My proposal would be to call scorep_exit_region until we have
       a special event for exits on exceptions. However, for the profiling part
       no special event is planned, but I do not know about OTF2.
     */
    UTILS_NOT_YET_IMPLEMENTED();

    if ( scorep_tracing_consume_event() )
    {
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* TODO: The same like for the regular exit */
    }
}


/**
 *
 */
void
SCOREP_TriggerCounterInt64( SCOREP_SamplingSetHandle counterHandle,
                            int64_t                  value )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    SCOREP_SamplingSet_Definition* sampling_set
        = SCOREP_LOCAL_HANDLE_DEREF( counterHandle, SamplingSet );
    UTILS_BUG_ON( sampling_set->number_of_metrics != 1,
                  "User sampling set with more than one metric" );

    if ( scorep_tracing_consume_event() )
    {
        union
        {
            uint64_t uint64;
            int64_t  int64;
        } union_value;
        union_value.int64 = value;

        SCOREP_Tracing_Metric( location,
                               timestamp,
                               counterHandle,
                               &union_value.uint64 );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_TriggerInteger( location,
                                       sampling_set->metric_handles[ 0 ],
                                       value );
    }
}


/**
 *
 */
void
SCOREP_TriggerCounterUint64( SCOREP_SamplingSetHandle counterHandle,
                             uint64_t                 value )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    SCOREP_SamplingSet_Definition* sampling_set
        = SCOREP_LOCAL_HANDLE_DEREF( counterHandle, SamplingSet );
    UTILS_BUG_ON( sampling_set->number_of_metrics != 1,
                  "User sampling set with more than one metric" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_Metric( location,
                               timestamp,
                               counterHandle,
                               &value );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_TriggerInteger( location,
                                       sampling_set->metric_handles[ 0 ],
                                       value );
    }
}


/**
 *
 */
void
SCOREP_TriggerCounterDouble( SCOREP_SamplingSetHandle counterHandle,
                             double                   value )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    SCOREP_SamplingSet_Definition* sampling_set
        = SCOREP_LOCAL_HANDLE_DEREF( counterHandle, SamplingSet );
    UTILS_BUG_ON( sampling_set->number_of_metrics != 1,
                  "User sampling set with more than one metric" );

    if ( scorep_tracing_consume_event() )
    {
        union
        {
            uint64_t uint64;
            double   float64;
        } union_value;
        union_value.float64 = value;

        SCOREP_Tracing_Metric( location,
                               timestamp,
                               counterHandle,
                               &union_value.uint64 );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_TriggerDouble( location,
                                      sampling_set->metric_handles[ 0 ],
                                      value );
    }
}


/**
 *
 */
void
SCOREP_TriggerMarker( SCOREP_MarkerHandle markerHandle )
{
    SCOREP_Location* location = SCOREP_Location_GetCurrentCPULocation();
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    UTILS_NOT_YET_IMPLEMENTED();

    if ( scorep_tracing_consume_event() )
    {
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* No action necessary */
    }
}


/**
 *
 */
void
SCOREP_TriggerParameterInt64( SCOREP_ParameterHandle parameterHandle,
                              int64_t                value )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ParameterInt64( location,
                                       timestamp,
                                       parameterHandle,
                                       value );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_ParameterInteger( location,
                                         parameterHandle,
                                         value );
    }
}


/**
 *
 */
void
SCOREP_TriggerParameterUint64( SCOREP_ParameterHandle parameterHandle,
                               uint64_t               value )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ParameterUint64( location,
                                        timestamp,
                                        parameterHandle,
                                        value );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        /* The SCOREP_Profile_ParameterInteger handles unsigned and signed integers */
        SCOREP_Profile_ParameterInteger( location,
                                         parameterHandle,
                                         value );
    }
}


/**
 *
 */
void
SCOREP_TriggerParameterString( SCOREP_ParameterHandle parameterHandle,
                               const char*            value )
{
    SCOREP_Location* location  = SCOREP_Location_GetCurrentCPULocation();
    uint64_t         timestamp = scorep_get_timestamp( location );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_EVENTS, "" );

    SCOREP_StringHandle string_handle = SCOREP_DefineString( value );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ParameterString( location,
                                        timestamp,
                                        parameterHandle,
                                        string_handle );
    }

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_ParameterString( location,
                                        parameterHandle,
                                        string_handle );
    }
}

/**
 * Returns the timestamp of the last triggered event on the current location.
 *
 */
uint64_t
SCOREP_GetLastTimeStamp()
{
    return SCOREP_Location_GetLastTimestamp( SCOREP_Location_GetCurrentCPULocation() );
}
