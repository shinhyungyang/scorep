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


#include <stdlib.h>
#include <stdio.h>


/**
 * @file        SILC_Events.c
 * @maintainer  Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @brief   Declaration of event recording functions to be used by the
 *          adapter layer.
 *
 *
 */


#include <SILC_Debug.h>
#include <SILC_Events.h>
#include <SILC_Timing.h>
#include <OTF2_EvtWriter.h>


#include "silc_runtime_management.h"
#include "silc_types.h"


/**
 * Generate a region enter event in the measurement system.
 */
void
SILC_EnterRegion
(
    SILC_RegionHandle regionHandle
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Reg:%s",
                       silc_region_to_string( stringBuffer,
                                              sizeof( stringBuffer ),
                                              "%x", regionHandle ) );

    OTF2_EvtWriter_Enter( silc_local_event_writer, NULL,
                          SILC_GetClockTicks(),
                          regionHandle );
}


/**
 * Generate a region exit event in the measurement system.
 */
void
SILC_ExitRegion
(
    SILC_RegionHandle regionHandle
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Reg:%s",
                       silc_region_to_string( stringBuffer,
                                              sizeof( stringBuffer ),
                                              "%x", regionHandle ) );

    OTF2_EvtWriter_Leave( silc_local_event_writer, NULL,
                          SILC_GetClockTicks(),
                          regionHandle );
}


/**
 * Generate an mpi send event in the measurement system.
 */
void
SILC_MpiSend
(
    uint32_t                   globalDestinationRank,
    SILC_MPICommunicatorHandle communicatorHandle,
    uint32_t                   tag,
    uint64_t                   bytesSent
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Dst:%u Comm:%s Tag:%u Bytes:%llu",
                       globalDestinationRank,
                       silc_comm_to_string( stringBuffer,
                                            sizeof( stringBuffer ),
                                            "%x", communicatorHandle ),
                       tag,
                       ( unsigned long long )bytesSent );

    OTF2_EvtWriter_MpiSend( silc_local_event_writer, NULL,
                            SILC_GetClockTicks(),
                            OTF2_MPI_BLOCK,
                            silc_local_id,
                            globalDestinationRank,
                            communicatorHandle,
                            tag,
                            bytesSent );
}


/**
 * Generate an mpi recv event in the measurement system.
 */
void
SILC_MpiRecv
(
    uint32_t                   globalSourceRank,
    SILC_MPICommunicatorHandle communicatorHandle,
    uint32_t                   tag,
    uint64_t                   bytesReceived
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Src:%u Comm:%s Tag:%u Bytes:%llu",
                       globalSourceRank,
                       silc_comm_to_string( stringBuffer,
                                            sizeof( stringBuffer ),
                                            "%x", communicatorHandle ),
                       tag,
                       ( unsigned long long )bytesReceived );

    OTF2_EvtWriter_MpiRecv( silc_local_event_writer, NULL,
                            SILC_GetClockTicks(),
                            OTF2_MPI_BLOCK,
                            globalSourceRank,
                            silc_local_id,
                            communicatorHandle,
                            tag,
                            bytesReceived );
}


/**
 * Generate an mpi collective event in the measurement system.
 */
void
SILC_MpiCollective
(
    SILC_RegionHandle          regionHandle,
    SILC_MPICommunicatorHandle communicatorHandle,
    uint32_t                   globalRootRank,
    uint64_t                   bytesSent,
    uint64_t                   bytesReceived
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 3 ][ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Reg:%s Comm:%s Root:%s Sent:%llu Recv:%llu",
                       silc_region_to_string( stringBuffer[ 0 ],
                                              sizeof( stringBuffer[ 0 ] ),
                                              "%x", regionHandle ),
                       silc_comm_to_string( stringBuffer[ 1 ],
                                            sizeof( stringBuffer[ 1 ] ),
                                            "%x", communicatorHandle ),
                       silc_uint32_to_string( stringBuffer[ 2 ],
                                              sizeof( stringBuffer[ 2 ] ),
                                              "%x", globalRootRank,
                                              SILC_INVALID_ROOT_RANK ),
                       ( unsigned long long )bytesSent,
                       ( unsigned long long )bytesReceived );

    OTF2_EvtWriter_MpiCollective( silc_local_event_writer, NULL,
                                  SILC_GetClockTicks(),
                                  OTF2_MPI_BARRIER,
                                  communicatorHandle,
                                  globalRootRank,
                                  bytesSent,
                                  bytesReceived );
}


/**
 * Generate an OpenMP fork event in the measurement system.
 */
void
SILC_OmpFork
(
    SILC_RegionHandle regionHandle
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Reg:%s",
                       silc_region_to_string( stringBuffer,
                                              sizeof( stringBuffer ),
                                              "%x", regionHandle ) );
}


/**
 * Generate an OpenMP join event in the measurement system.
 */
void
SILC_OmpJoin
(
    SILC_RegionHandle regionHandle
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Reg:%s",
                       silc_region_to_string( stringBuffer,
                                              sizeof( stringBuffer ),
                                              "%x", regionHandle ) );
}


/**
 * Generate an OpenMP acquire lock event in the measurement system.
 */
void
SILC_OmpAcquireLock
(
    uint32_t lockId
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Lock:%x", lockId );
}


/**
 * Generate an OpenMP release lock event in the measurement system.
 */
void
SILC_OmpReleaseLock
(
    uint32_t lockId
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Lock:%x", lockId );
}


/**
 *
 */
void
SILC_ExitRegionOnException
(
    SILC_RegionHandle regionHandle
)
{
    SILC_DEBUG_ONLY( char stringBuffer[ 16 ];
                     )

    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "Reg:%s",
                       silc_region_to_string( stringBuffer,
                                              sizeof( stringBuffer ),
                                              "%x", regionHandle ) );
}


/**
 *
 */
void
SILC_TriggerCounterInt64
(
    SILC_CounterHandle counterHandle,
    int64_t            value
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "" );
}


/**
 *
 */
void
SILC_TriggerCounterDouble
(
    SILC_CounterHandle counterHandle,
    double             value
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "" );
}


/**
 *
 */
void
SILC_TriggerMarker
(
    SILC_MarkerHandle markerHandle
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "" );
}


/**
 *
 */
void
SILC_TriggerParameterInt64
(
    SILC_ParameterHandle parameterHandle,
    int64_t              value
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "" );
}


/**
 *
 */
void
SILC_TriggerParameterDouble
(
    SILC_ParameterHandle parameterHandle,
    double               value
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "" );
}


/**
 *
 */
void
SILC_TriggerParameterString
(
    SILC_ParameterHandle parameterHandle,
    const char*          value
)
{
    SILC_DEBUG_PRINTF( SILC_DEBUG_EVENTS, "" );
}
