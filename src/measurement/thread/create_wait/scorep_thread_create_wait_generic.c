/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file
 */


#include <config.h>

#include <SCOREP_ThreadCreateWait_Event.h>
#include <scorep_thread_generic.h>
#include <scorep_thread_model_specific.h>
#include <scorep_thread_create_wait_model_specific.h>

#include <SCOREP_Subsystem.h>
#include <SCOREP_Mutex.h>
#include <definitions/SCOREP_Definitions.h>
#include <scorep_events_common.h>
#include <SCOREP_Properties.h>
#include <SCOREP_Profile.h>
#include <tracing/SCOREP_Tracing_Events.h>
#include <scorep_unify_helpers.h>

#define SCOREP_DEBUG_MODULE_NAME THREAD_CREATE_WAIT
#include <UTILS_Debug.h>

#include <UTILS_Error.h>


#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>


/* *INDENT-OFF* */
static SCOREP_ErrorCode subsystem_init( void );
static SCOREP_ErrorCode subsystem_init_location( struct SCOREP_Location* );
static SCOREP_ErrorCode subsystem_pre_unify( void );
static SCOREP_ErrorCode subsystem_post_unify( void );
static void finalize( void );
/* *INDENT-ON*  */


static SCOREP_InterimCommunicatorHandle thread_team = SCOREP_INVALID_INTERIM_COMMUNICATOR;
static bool is_initialized;
static bool is_finalized;

typedef uint64_t location_id;
static size_t       active_locations;
static size_t       n_locations;
static size_t       location_ids_size = 16;
static location_id* location_ids;
static SCOREP_Mutex thread_create_mutex;


const SCOREP_Subsystem SCOREP_Subsystem_ThreadCreateWait =
{
    .subsystem_name              = "THREAD CREATE WAIT",
    .subsystem_register          = NULL,
    .subsystem_init              = &subsystem_init,
    .subsystem_init_location     = NULL,
    .subsystem_finalize_location = NULL,
    .subsystem_pre_unify         = &subsystem_pre_unify,
    .subsystem_post_unify        = &subsystem_post_unify,
    .subsystem_finalize          = &finalize,
    .subsystem_deregister        = NULL,
    .subsystem_control           = NULL
};


static SCOREP_ErrorCode
subsystem_init( void )
{
    UTILS_DEBUG_ENTRY();
    if ( is_initialized )
    {
        return SCOREP_SUCCESS;
    }
    is_initialized = true;

    SCOREP_ErrorCode result = SCOREP_MutexCreate( &thread_create_mutex );
    UTILS_BUG_ON( result != SCOREP_SUCCESS, "" );

    thread_team = SCOREP_Definitions_NewInterimCommunicator(
        SCOREP_INVALID_INTERIM_COMMUNICATOR,
        scorep_thread_get_paradigm(),
        0,
        NULL );
    struct scorep_thread_private_data* tpd = scorep_thread_get_private_data();
    scorep_thread_set_team( tpd, thread_team );

    active_locations = 1;
    location_ids     = malloc( sizeof( location_id ) * location_ids_size );
    UTILS_BUG_ON( !location_ids, "Cannot allocate memory for location_ids" );
    location_ids[ n_locations++ ] = SCOREP_Location_GetId(
        scorep_thread_get_location( tpd ) );

    UTILS_DEBUG_EXIT();
    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
subsystem_pre_unify( void )
{
    UTILS_DEBUG_ENTRY();
    // todo CR: everything that mentions PTHREAD should come from
    // create_wait_pthread.c. Check fork-join as well.

    /* MPP should be initialized by now so it is safe to obtain global location
     * ids from local ones. */
    for ( size_t i = 0; i < n_locations; i++ )
    {
        location_ids[ i ] = SCOREP_Location_GetGlobalIdFromLocalId( location_ids[ i ] );
    }

    /* Create the group of all pthread locations in all processes. */
    uint32_t offset = scorep_unify_helper_define_comm_locations(
        SCOREP_GROUP_PTHREAD_LOCATIONS,
        "PTHREAD",
        n_locations,
        location_ids );

    /* Assign this process's pthread locations the global position in the
     * SCOREP_GROUP_PTHREAD_LOCATIONS group */
    for ( size_t i = 0; i < n_locations; i++ )
    {
        location_ids[ i ] = i + offset;
    }

    /* In every process, create one local group of all local pthread locations */
    SCOREP_GroupHandle group_handle = SCOREP_Definitions_NewGroup(
        SCOREP_GROUP_PTHREAD_THREAD_TEAM,
        "PTHREAD threads",
        n_locations,
        location_ids );

    /* Define the final communicator over this group */
    SCOREP_CommunicatorHandle communicator_handle =
        SCOREP_Definitions_NewCommunicator(
            group_handle,
            "",
            SCOREP_INVALID_COMMUNICATOR );

    /* Remember this communicator for use in post_unify */
    SCOREP_LOCAL_HANDLE_DEREF( thread_team,
                               InterimCommunicator )->unified =
        communicator_handle;

    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
subsystem_post_unify( void )
{
    UTILS_DEBUG_ENTRY();
    /* Map the local interim communicator to the unified communicator */
    scorep_local_definition_manager.interim_communicator.mapping[
        SCOREP_LOCAL_HANDLE_DEREF( thread_team,
                                   InterimCommunicator )->sequence_number ] =
        scorep_local_definition_manager.communicator.mapping[
            SCOREP_LOCAL_HANDLE_DEREF( SCOREP_LOCAL_HANDLE_DEREF(
                                           thread_team,
                                           InterimCommunicator )->unified,
                                       Communicator )->sequence_number ];
    return SCOREP_SUCCESS;
}


static void
finalize( void )
{
    UTILS_DEBUG_ENTRY();
    if ( !is_initialized )
    {
        return;
    }
    is_finalized = true;

    SCOREP_ErrorCode result = SCOREP_MutexDestroy( &thread_create_mutex );
    UTILS_BUG_ON( result != SCOREP_SUCCESS );
    free( location_ids );
}


bool
SCOREP_Thread_InParallel()
{
    UTILS_DEBUG_ENTRY();
    if ( !is_initialized || is_finalized )
    {
        return false;
    }

    SCOREP_MutexLock( thread_create_mutex );
    bool in_parallel = ( active_locations > 1 );
    SCOREP_MutexUnlock( thread_create_mutex );
    return in_parallel;
}


void
SCOREP_ThreadCreateWait_Create( SCOREP_ParadigmType                 paradigm,
                                struct scorep_thread_private_data** parent,
                                uint32_t*                           sequenceCount )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( paradigm & SCOREP_PARADIGM_THREAD_CREATE_WAIT == 0,
                  "Provided paradigm not in group SCOREP_PARADIGM_THREAD_CREATE_WAIT " );
    /* We are in the creator thread. */

    struct scorep_thread_private_data* tpd       = scorep_thread_get_private_data();
    struct SCOREP_Location*            location  = scorep_thread_get_location( tpd );
    uint64_t                           timestamp = scorep_get_timestamp( location );

    *parent        = tpd;
    *sequenceCount = scorep_thread_get_next_sequence_count();

    scorep_thread_create_wait_on_create( paradigm,
                                         scorep_thread_get_model_data( tpd ),
                                         location );

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_ThreadCreate( location, *sequenceCount );
    }

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ThreadCreate( location,
                                     timestamp,
                                     paradigm,
                                     scorep_thread_get_team( tpd ),
                                     *sequenceCount );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_THREAD_CREATE_WAIT_EVENT_COMPLETE );
    }
}


void
SCOREP_ThreadCreateWait_Wait( SCOREP_ParadigmType paradigm,
                              uint32_t            sequenceCount )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( paradigm & SCOREP_PARADIGM_THREAD_CREATE_WAIT == 0,
                  "Provided paradigm not in group SCOREP_PARADIGM_THREAD_CREATE_WAIT " );

    struct scorep_thread_private_data* tpd       = scorep_thread_get_private_data();
    struct SCOREP_Location*            location  = scorep_thread_get_location( tpd );
    uint64_t                           timestamp = scorep_get_timestamp( location );

    scorep_thread_create_wait_on_wait( paradigm,
                                       scorep_thread_get_model_data( tpd ),
                                       location );

    if ( SCOREP_IsProfilingEnabled() )
    {
        SCOREP_Profile_ThreadWait( location, sequenceCount );
    }

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ThreadWait( location,
                                   timestamp,
                                   paradigm,
                                   scorep_thread_get_team( tpd ),
                                   sequenceCount );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_THREAD_CREATE_WAIT_EVENT_COMPLETE );
    }
}


void
SCOREP_ThreadCreateWait_Begin( SCOREP_ParadigmType                paradigm,
                               struct scorep_thread_private_data* parentTpd,
                               uint32_t                           sequenceCount )
{
    UTILS_DEBUG_ENTRY();

    UTILS_ASSERT( paradigm & SCOREP_PARADIGM_THREAD_CREATE_WAIT );
    UTILS_BUG_ON( parentTpd == 0 );
    /* We are in a new thread. */
    struct scorep_thread_private_data* current_tpd         = 0;
    bool                               location_is_created = false;

    scorep_thread_create_wait_on_begin( paradigm,
                                        parentTpd,
                                        sequenceCount,
                                        &current_tpd,
                                        &location_is_created );
    UTILS_BUG_ON( current_tpd == 0, "Failed to create new scorep_thread_private_data object." );

    SCOREP_Location* parent_location  = scorep_thread_get_location( parentTpd );
    SCOREP_Location* current_location = scorep_thread_get_location( current_tpd );
    uint64_t         timestamp        = scorep_get_timestamp( current_location );

    if ( location_is_created )
    {
        SCOREP_Location_CallSubstratesOnNewLocation( current_location,
                                                     0,
                                                     parent_location );

        /* Remember locations ids for unification. */
        SCOREP_MutexLock( thread_create_mutex );
        if ( n_locations >= location_ids_size )
        {
            location_ids_size *= 2;
            location_ids       = realloc( location_ids,
                                          location_ids_size * sizeof( location_id ) );
            UTILS_BUG_ON( !location_ids, "Cannot reallocate memory for location_ids" );
        }
        location_ids[ n_locations++ ] = SCOREP_Location_GetId( current_location );
        active_locations++;
        SCOREP_MutexUnlock( thread_create_mutex );
    }

    scorep_thread_set_team( current_tpd, thread_team );

    SCOREP_Location_CallSubstratesOnActivation( current_location,
                                                parent_location,
                                                sequenceCount );

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ThreadBegin( current_location,
                                    timestamp,
                                    paradigm,
                                    scorep_thread_get_team( current_tpd ),
                                    sequenceCount );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_THREAD_CREATE_WAIT_EVENT_COMPLETE );
    }

    /* Nothing to do for profiling. */
}


void
SCOREP_ThreadCreateWait_End( SCOREP_ParadigmType                paradigm,
                             struct scorep_thread_private_data* parentTpd,
                             uint32_t                           sequenceCount )
{
    UTILS_DEBUG_ENTRY();

    UTILS_ASSERT( paradigm & SCOREP_PARADIGM_THREAD_CREATE_WAIT );
    UTILS_BUG_ON( parentTpd == 0 );

    struct scorep_thread_private_data* current_tpd      = scorep_thread_get_private_data();
    SCOREP_Location*                   current_location = scorep_thread_get_location( current_tpd );
    uint64_t                           timestamp        = scorep_get_timestamp( current_location );

    SCOREP_Location_CallSubstratesOnDeactivation(
        current_location,
        scorep_thread_get_location( parentTpd ) );

    /* Nothing to do for profiling. */

    if ( scorep_tracing_consume_event() )
    {
        SCOREP_Tracing_ThreadEnd( current_location,
                                  timestamp,
                                  paradigm,
                                  scorep_thread_get_team( current_tpd ),
                                  sequenceCount );
    }
    else if ( !SCOREP_RecordingEnabled() )
    {
        SCOREP_InvalidateProperty( SCOREP_PROPERTY_THREAD_CREATE_WAIT_EVENT_COMPLETE );
    }

    scorep_thread_create_wait_on_end( paradigm, parentTpd, current_tpd, sequenceCount );

    SCOREP_MutexLock( thread_create_mutex );
    active_locations--;
    SCOREP_MutexUnlock( thread_create_mutex );
    UTILS_DEBUG_EXIT();
}
