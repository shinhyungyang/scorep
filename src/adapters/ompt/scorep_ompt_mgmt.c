/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 */

#include <config.h>

#include "scorep_ompt.h"
#include "scorep_ompt_device.h"
#include "scorep_ompt_callbacks_host.h"
#include "scorep_ompt_callbacks_device.h"
#include "scorep_ompt_confvars.h"
#include "scorep_ompt_confvars.inc.c"

#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>


#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Subsystem.h>
#include <SCOREP_Paradigms.h>
#include <SCOREP_Addr2line.h>
#include <SCOREP_Definitions.h>


/* *INDENT-OFF* */
static int initialize_tool( ompt_function_lookup_t lookup, int initialDeviceNum, ompt_data_t *toolData );
static void register_event_callbacks_host( ompt_set_callback_t setCallback );
static void register_event_callbacks_device( ompt_set_callback_t setCallback );
static void finalize_tool( ompt_data_t *toolData );
/* *INDENT-ON* */


size_t scorep_ompt_subsystem_id;

ompt_get_task_info_t        scorep_ompt_mgmt_get_task_info;
ompt_get_unique_id_t        scorep_ompt_mgmt_get_unique_id;
static ompt_finalize_tool_t ompt_finalize_tool;

static bool tool_initialized;
bool        scorep_ompt_record_events   = false;
bool        scorep_ompt_finalizing_tool = false;


SCOREP_RmaWindowHandle           scorep_ompt_rma_window_handle           = SCOREP_INVALID_RMA_WINDOW;
SCOREP_InterimCommunicatorHandle scorep_ompt_interim_communicator_handle = SCOREP_INVALID_INTERIM_COMMUNICATOR;
uint32_t                         scorep_ompt_global_location_count       = 0;
/* Holds global location ids of host and target locations after unification.
 * scorep_ompt_cpu_location_data and scorep_ompt_device_stream_t objects to
 * index into this array. */
uint64_t* scorep_ompt_global_location_ids = NULL;


/* Called by the OpenMP runtime. Everything starts from here. */
ompt_start_tool_result_t*
ompt_start_tool( unsigned int omp_version, /* == _OPENMP */
                 const char*  runtime_version )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    UTILS_DEBUG( "[%s] omp_version=\"%d\"; runtime_version=\"%s\"",
                 UTILS_FUNCTION_NAME, omp_version, runtime_version );

    /* TODO decide against initialization if env var is set? */
    static ompt_start_tool_result_t tool = { &initialize_tool,
                                             &finalize_tool,
                                             ompt_data_none };

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return &tool;
}


static inline void
init_rma_window( void )
{
    if ( scorep_ompt_interim_communicator_handle != SCOREP_INVALID_INTERIM_COMMUNICATOR )
    {
        return;
    }

    scorep_ompt_interim_communicator_handle =
        SCOREP_Definitions_NewInterimCommunicator(
            SCOREP_INVALID_INTERIM_COMMUNICATOR,
            SCOREP_PARADIGM_OPENMP_TARGET,
            0,
            NULL );
    scorep_ompt_rma_window_handle =
        SCOREP_Definitions_NewRmaWindow(
            "OPENMP_TARGET_WINDOW",
            scorep_ompt_interim_communicator_handle,
            SCOREP_RMA_WINDOW_FLAG_NONE );
}


static int
initialize_tool( ompt_function_lookup_t lookup,
                 int                    initialDeviceNum,
                 ompt_data_t*           toolData )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    UTILS_DEBUG( "[%s] initial_device_num=%d",
                 UTILS_FUNCTION_NAME, initialDeviceNum );

    /* Lookup runtime entry points: */
    /* According to the specification (Monitoring Activity on the Host with
       OMPT), calling the ompt_set_callback runtime entry point is only safe
       from a tool's initializer. This is unfortunate as registering at
       ompt_subsystem_begin and de-registering at ompt_substytem_end would be
       what we need; now we might get events after subsystem_end that we need
       to take care of by maintaining  'scorep_ompt_record_events'. */
    ompt_set_callback_t set_callback =
        ( ompt_set_callback_t )lookup( "ompt_set_callback" );
    UTILS_BUG_ON( set_callback == 0 );
    scorep_ompt_mgmt_get_task_info =
        ( ompt_get_task_info_t )lookup( "ompt_get_task_info" );
    UTILS_BUG_ON( scorep_ompt_mgmt_get_task_info == 0 );
    scorep_ompt_mgmt_get_unique_id =
        ( ompt_get_unique_id_t )lookup( "ompt_get_unique_id" );
    UTILS_DEBUG( "[%s] Using unique ids of runtime? %s",
                 UTILS_FUNCTION_NAME,
                 scorep_ompt_mgmt_get_unique_id == 0 ? "no" : "yes" );
    ompt_finalize_tool =
        ( ompt_finalize_tool_t )lookup( "ompt_finalize_tool" );
    UTILS_BUG_ON( ompt_finalize_tool == 0 );

    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitConfigVariables();
    }

    /* Use runtime entry point set_callback to register callbacks:
       TODO provide means to selectively register (groups of) callbacks? */
    register_event_callbacks_host( set_callback );
    register_event_callbacks_device( set_callback );

    tool_initialized = true;
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return 1; /* non-zero indicates success */
}


static void
finalize_tool( ompt_data_t* toolData )
{
    UTILS_DEBUG( "[%s]", UTILS_FUNCTION_NAME );
}


/* *INDENT-OFF* */
#if 0
callbacks available in 5.2:
  ompt_callback_thread_begin            = 1,
  ompt_callback_thread_end              = 2,
  ompt_callback_parallel_begin          = 3,
  ompt_callback_parallel_end            = 4,
  ompt_callback_task_create             = 5,
  ompt_callback_task_schedule           = 6,
  ompt_callback_implicit_task           = 7,
  ompt_callback_target                  = 8,
  ompt_callback_target_data_op          = 9,
  ompt_callback_target_submit           = 10,
  ompt_callback_control_tool            = 11,
  ompt_callback_device_initialize       = 12,
  ompt_callback_device_finalize         = 13,
  ompt_callback_device_load             = 14,
  ompt_callback_device_unload           = 15,
  ompt_callback_sync_region_wait        = 16,
  ompt_callback_mutex_released          = 17,
  ompt_callback_dependences             = 18,
  ompt_callback_task_dependence         = 19,
  ompt_callback_work                    = 20,
  ompt_callback_masked                  = 21,
  ompt_callback_master /*(deprecated)*/ = ompt_callback_masked,
  ompt_callback_target_map              = 22,
  ompt_callback_sync_region             = 23,
  ompt_callback_lock_init               = 24,
  ompt_callback_lock_destroy            = 25,
  ompt_callback_mutex_acquire           = 26,
  ompt_callback_mutex_acquired          = 27,
  ompt_callback_nest_lock               = 28,
  ompt_callback_flush                   = 29,
  ompt_callback_cancel                  = 30,
  ompt_callback_reduction               = 31,
  ompt_callback_dispatch                = 32,
  ompt_callback_target_emi              = 33,
  ompt_callback_target_data_op_emi      = 34,
  ompt_callback_target_submit_emi       = 35,
  ompt_callback_target_map_emi          = 36,
  ompt_callback_error                   = 37
#endif
/* *INDENT-ON* */


typedef struct registration_data_t
{
    ompt_callbacks_t  event;
    ompt_callback_t   callback;
    const char*       name;
    ompt_set_result_t expected_result;
    bool              critical_callback; /* If critical, disable all other related callbacks.
                                          * If not, only this callback will be disabled and the others
                                          * will stay active */
} registration_data_t;


static inline bool
register_callback( ompt_set_callback_t       setCallback,
                   const registration_data_t callbackInformation )
{
    ompt_set_result_t result = setCallback( callbackInformation.event, callbackInformation.callback );
    #if HAVE( UTILS_DEBUG )
    char* status_str = NULL;
    switch ( result )
    {
        case ompt_set_error:
            status_str = "error";
            break;
        case ompt_set_never:
            status_str = "never";
            break;
        case ompt_set_impossible:
            status_str = "impossible";
            break;
        case ompt_set_sometimes:
            status_str = "sometimes";
            break;
        case ompt_set_sometimes_paired:
            status_str = "sometimes_paired";
            break;
        case ompt_set_always:
            status_str = "always";
            break;
    }
    #endif /* HAVE( UTILS_DEBUG ) */
    UTILS_DEBUG( "[%s] registering ompt_callback_%s: %s",
                 UTILS_FUNCTION_NAME, callbackInformation.name, status_str );
    return result >= callbackInformation.expected_result;
}


#define NO_EVENT ( ( ompt_callbacks_t )( -1 ) )
/* Try to register entire group. If one callback in this group fails
 * to register, abort. Abortion is not expected, as we check for the
 * required group during configure already. Abortion can happen if a
 * different OpenMP runtime is used at measurement time as compared to
 * configure time. Switching the runtime can happen, e.g., on Cray
 * systems, by changing the environment (loading modules) or via
 * additional link flags. */
#define REGISTER_REQUIRED_CALLBACK_GROUP( CALLBACK_GROUP )                                           \
    do                                                                                               \
    {                                                                                                \
        for ( short i = 0; CALLBACK_GROUP[ i ].event != NO_EVENT; ++i )                              \
        {                                                                                            \
            if ( !register_callback( setCallback, CALLBACK_GROUP[ i ] ) )                            \
            {                                                                                        \
                UTILS_BUG( "[OMPT] Failed to register %s, which is mandatory! "                      \
                           "Cannot use OMPT, consider instrumenting `--thread=omp:opari2` instead.", \
                           CALLBACK_GROUP[ i ].name );                                               \
            }                                                                                        \
        }                                                                                            \
    } while ( 0 )


/* Try to register entire group. If one callback fails to register, disable entire group */
#define REGISTER_OPTIONAL_CALLBACK_GROUP( CALLBACK_GROUP )                                                              \
    do                                                                                                                  \
    {                                                                                                                   \
        short i;                                                                                                        \
        for ( i = 0; CALLBACK_GROUP[ i ].event != NO_EVENT; ++i )                                                       \
        {                                                                                                               \
            if ( !register_callback( setCallback, CALLBACK_GROUP[ i ] ) )                                               \
            {                                                                                                           \
                if ( CALLBACK_GROUP[ i ].critical_callback )                                                            \
                {                                                                                                       \
                    UTILS_WARNING( "[OMPT] Failed to register %s, disabling related callbacks.",                        \
                                   CALLBACK_GROUP[ i ].name );                                                          \
                    break;                                                                                              \
                }                                                                                                       \
                else                                                                                                    \
                {                                                                                                       \
                    UTILS_WARNING( "[OMPT] Failed to register %s, but callback is not critical for related callbacks.", \
                                   CALLBACK_GROUP[ i ].name );                                                          \
                    setCallback( CALLBACK_GROUP[ i ].event, NULL );                                                     \
                }                                                                                                       \
            }                                                                                                           \
        }                                                                                                               \
        if ( CALLBACK_GROUP[ i ].event != NO_EVENT )                                                                    \
        {                                                                                                               \
            for ( i = 0; CALLBACK_GROUP[ i ].event != NO_EVENT; ++i )                                                   \
            {                                                                                                           \
                setCallback( CALLBACK_GROUP[ i ].event, NULL );                                                         \
            }                                                                                                           \
        }                                                                                                               \
    } while ( 0 )


/* Try to register callback. If it fails to register, disable it. */
#define REGISTER_SINGLE_CALLBACK( CALLBACK_GROUP )                                  \
    do {                                                                            \
        for ( short i = 0; CALLBACK_GROUP[ i ].event != NO_EVENT; ++i )             \
        {                                                                           \
            if ( !register_callback( setCallback, CALLBACK_GROUP[ i ] ) )           \
            {                                                                       \
                UTILS_WARNING( "[OMPT] Failed to register %s, disabling callback.", \
                               CALLBACK_GROUP[ i ].name );                          \
                setCallback( CALLBACK_GROUP[ i ].event, NULL );                     \
            }                                                                       \
        }                                                                           \
    } while ( 0 )


static void
register_event_callbacks_host( ompt_set_callback_t setCallback )
{
    #define CALLBACK( name, result ) { ompt_callback_ ## name, ( ompt_callback_t )&scorep_ompt_cb_host_ ## name, #name, result, true }
    #define CALLBACK_NON_CRITICAL( name, result ) { ompt_callback_ ## name, ( ompt_callback_t )&scorep_ompt_cb_host_ ## name, #name, result, false }
    #define END()         { NO_EVENT, NULL, "" } /* Indicates end of array */

    const registration_data_t required_callbacks[] =
    {
        CALLBACK( thread_begin,   ompt_set_always ),
        CALLBACK( thread_end,     ompt_set_always ),
        CALLBACK( parallel_begin, ompt_set_always ),
        CALLBACK( parallel_end,   ompt_set_always ),
        CALLBACK( task_create,    ompt_set_always ),
        CALLBACK( task_schedule,  ompt_set_always ),
        CALLBACK( implicit_task,  ompt_set_always ),
        CALLBACK( sync_region,    ompt_set_sometimes_paired ),
        END()
    };
    const registration_data_t mutex_callbacks[] =
    {
        CALLBACK( mutex_acquire,  ompt_set_sometimes_paired ),
        CALLBACK( mutex_acquired, ompt_set_sometimes_paired ),
        CALLBACK( mutex_released, ompt_set_sometimes_paired ),
        #if !HAVE( SCOREP_OMPT_WRONG_TEST_LOCK_MUTEX )
        CALLBACK( lock_init,      ompt_set_sometimes_paired ),
        CALLBACK( lock_destroy,   ompt_set_sometimes_paired ),
        CALLBACK( nest_lock,      ompt_set_sometimes_paired ),
        #endif /* !HAVE( SCOREP_OMPT_WRONG_TEST_LOCK_MUTEX ) */
        END()
    };
    const registration_data_t work_callbacks[] =
    {
        CALLBACK( work,                  ompt_set_sometimes_paired ),
        CALLBACK_NON_CRITICAL( dispatch, ompt_set_sometimes_paired ),
        END()
    };
    const registration_data_t single_callbacks[] =
    {
        CALLBACK( masked, ompt_set_sometimes_paired ),
        CALLBACK( flush,  ompt_set_sometimes_paired ),
        // CALLBACK( reduction, ompt_set_sometimes_paired ),
        END()
    };

    #undef END
    #undef CALLBACK_NON_CRITICAL
    #undef CALLBACK

    REGISTER_REQUIRED_CALLBACK_GROUP( required_callbacks );
    REGISTER_OPTIONAL_CALLBACK_GROUP( mutex_callbacks );
    REGISTER_OPTIONAL_CALLBACK_GROUP( work_callbacks );
    REGISTER_SINGLE_CALLBACK( single_callbacks );
}


static void
register_event_callbacks_device( ompt_set_callback_t setCallback )
{
    #define CALLBACK( name, result ) { ompt_callback_ ## name, ( ompt_callback_t )&scorep_ompt_cb_ ## name, #name, result }
    #define CALLBACK_NON_CRITICAL( name, result ) { ompt_callback_ ## name, ( ompt_callback_t )&scorep_ompt_cb_ ## name, #name, result, false }
    #define END()                    { NO_EVENT, NULL, "" } /* Indicate end of section */

    if ( scorep_ompt_target_features == 0 )
    {
        return;
    }
    const registration_data_t required_callbacks[] =
    {
        CALLBACK( device_initialize,            ompt_set_sometimes_paired ),
        CALLBACK( target_emi,                   ompt_set_sometimes_paired ),
        CALLBACK_NON_CRITICAL( device_finalize, ompt_set_sometimes_paired ),
        END()
    };
    REGISTER_OPTIONAL_CALLBACK_GROUP( required_callbacks );

    if ( scorep_ompt_target_features & SCOREP_OMPT_TARGET_FEATURE_MEMORY )
    {
        const registration_data_t single_callbacks[] =
        {
            CALLBACK( target_data_op_emi, ompt_set_sometimes_paired ),
            END()
        };
        REGISTER_SINGLE_CALLBACK( single_callbacks );
    }
    if ( scorep_ompt_target_features & SCOREP_OMPT_TARGET_FEATURE_KERNEL )
    {
        const registration_data_t single_callbacks[] =
        {
            CALLBACK( target_submit_emi, ompt_set_sometimes_paired ),
            END()
        };
        REGISTER_SINGLE_CALLBACK( single_callbacks );
    }

    #undef CALLBACK
    #undef END
}


#undef REGISTER_REQUIRED_CALLBACK_GROUP
#undef REGISTER_OPTIONAL_CALLBACK_GROUP
#undef REGISTER_SINGLE_CALLBACK
#undef NO_EVENT


static bool
iterate_assign_cpu_locations( SCOREP_Location* location,
                              void*            arg )
{
    if ( SCOREP_Location_GetType( location ) == SCOREP_LOCATION_TYPE_CPU_THREAD )
    {
        scorep_ompt_cpu_location_data* location_data = SCOREP_Location_GetSubsystemData(
            location, scorep_ompt_subsystem_id );
        if ( location_data->local_rank != SCOREP_OMPT_INVALID_LOCAL_RANK )
        {
            UTILS_BUG_ON( location_data->local_rank >= scorep_ompt_global_location_count,
                          "CPU location ID %" PRIu32 " exceeds the expected number of OMPT locations (%" PRIu32 ")!",
                          location_data->local_rank, scorep_ompt_global_location_count );

            scorep_ompt_global_location_ids[ location_data->local_rank ] =
                SCOREP_Location_GetGlobalId( location );
        }
    }

    return false;
}


static void
iterate_finalize_trace( scorep_ompt_device_table_key_t key, scorep_ompt_device_table_value_t value, void* cbData )
{
    scorep_ompt_device_tracing_finalize( value );
}


static void
iterate_report_leaked( scorep_ompt_device_table_key_t key, scorep_ompt_device_table_value_t value, void* cbData )
{
    if ( !value || !value->alloc_metric )
    {
        return;
    }
    SCOREP_AllocMetric_ReportLeaked( value->alloc_metric );
}


static void
iterate_collect_comm_locations( scorep_ompt_device_table_key_t key, scorep_ompt_device_table_value_t value, void* cbData )
{
    if ( !value )
    {
        return;
    }
    for ( scorep_ompt_device_stream_t* ptr = value->streams; ptr != NULL; ptr = ptr->next )
    {
        scorep_ompt_global_location_ids[ ptr->local_rank ] = SCOREP_Location_GetGlobalId( ptr->scorep_location );
    }
}


static void
collect_comm_locations( void )
{
    scorep_ompt_global_location_ids = SCOREP_Memory_AllocForMisc( sizeof( uint64_t ) * scorep_ompt_global_location_count );
    SCOREP_Location_ForAll( iterate_assign_cpu_locations, NULL );
    scorep_ompt_device_table_iterate_key_value_pairs( &iterate_collect_comm_locations, NULL );
}


static SCOREP_ErrorCode
ompt_subsystem_register( size_t id )
{
    UTILS_DEBUG_ENTRY( "OMPT subsystem id: %zu", id );
    scorep_ompt_subsystem_id = id;

    const SCOREP_ErrorCode host_register_result = SCOREP_ConfigRegister( "openmp", scorep_ompt_confvars );
    if ( host_register_result != SCOREP_SUCCESS )
    {
        return host_register_result;
    }

    return SCOREP_ConfigRegister( "openmp", scorep_ompt_target_confvars );
}


static SCOREP_ErrorCode
ompt_subsystem_init( void )
{
    UTILS_DEBUG( "[%s] register paradigm", UTILS_FUNCTION_NAME );

    SCOREP_Paradigms_RegisterParallelParadigm(
        SCOREP_PARADIGM_OPENMP,
        SCOREP_PARADIGM_CLASS_THREAD_FORK_JOIN,
        "OpenMP",
        SCOREP_PARADIGM_FLAG_NONE );

    /* Only register the parallel paradigm if at least one
     * target feature is enabled. With this, the paradigm
     * will not show up at all if the user chose to disable
     * targeting. */
    if ( scorep_ompt_target_features > 0 )
    {
        SCOREP_Paradigms_RegisterParallelParadigm(
            SCOREP_PARADIGM_OPENMP_TARGET,
            SCOREP_PARADIGM_CLASS_ACCELERATOR,
            "OpenMP Target",
            SCOREP_PARADIGM_FLAG_RMA_ONLY );
    }
    SCOREP_Paradigms_SetStringProperty( SCOREP_PARADIGM_OPENMP,
                                        SCOREP_PARADIGM_PROPERTY_COMMUNICATOR_TEMPLATE,
                                        "Thread team ${id}" );

    SCOREP_Addr2line_RegisterObjcloseCb( scorep_ompt_codeptr_hash_dlclose_cb );

    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
ompt_subsystem_begin( void )
{
    /* Only initialize RMA window if OpenMP target recording is enabled.
     * The host side of OMPT does not require having an RMA window. */
    if ( scorep_ompt_target_features > 0 )
    {
        init_rma_window();
    }

    UTILS_DEBUG( "[%s] start recording OMPT events", UTILS_FUNCTION_NAME );
    scorep_ompt_record_events = true;
    return SCOREP_SUCCESS;
}


static void
ompt_subsystem_end( void )
{
    if ( !tool_initialized )
    {
        UTILS_DEBUG( "tool wasn't initialized" );
        return;
    }

    UTILS_DEBUG( "[%s] finalizing tool, might trigger overdue OMPT events",
                 UTILS_FUNCTION_NAME );

    scorep_ompt_finalizing_tool = true;
    /* Since iterate_finalize_trace will call functions of the OMPT device tracing interface,
     * we may need to call this function before ompt_finalize_tool(). In theory, ompt_finalize_tool()
     * should do the same job, but fails to do so with LLVM based runtimes.
     * Issue link: https://github.com/RadeonOpenCompute/ROCm/issues/2056 */
    scorep_ompt_device_table_iterate_key_value_pairs( &iterate_finalize_trace, NULL );
    ompt_finalize_tool();
    scorep_ompt_device_table_iterate_key_value_pairs( &iterate_report_leaked, NULL );

    /* ignore subsequent events */
    scorep_ompt_record_events = false;

    // TODO: free task and parallel (+task_generation_numbers) pools at finalization

    UTILS_DEBUG( "[%s] stop recording OMPT events", UTILS_FUNCTION_NAME );
}


static SCOREP_ErrorCode
ompt_subsystem_init_location( struct SCOREP_Location* newLocation,
                              struct SCOREP_Location* parentLocation )
{
    static bool already_called = false;
    if ( !already_called )
    {
        already_called = true;
        UTILS_DEBUG( "[%s] initial location %p", UTILS_FUNCTION_NAME, newLocation );
    }
    else
    {
        UTILS_DEBUG( "[%s] new location %p", UTILS_FUNCTION_NAME, newLocation );
    }

    if ( SCOREP_Location_GetType( newLocation ) == SCOREP_LOCATION_TYPE_CPU_THREAD )
    {
        scorep_ompt_cpu_location_data* data =
            SCOREP_Memory_AlignedAllocForMisc( SCOREP_CACHELINESIZE, sizeof( *data ) );
        memset( data, 0, sizeof( *data ) );
        data->local_rank = SCOREP_OMPT_INVALID_LOCAL_RANK;
        SCOREP_Location_SetSubsystemData( newLocation,
                                          scorep_ompt_subsystem_id,
                                          data );
    }
    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
ompt_subsystem_pre_unify( void )
{
    if ( scorep_ompt_interim_communicator_handle == SCOREP_INVALID_INTERIM_COMMUNICATOR )
    {
        return SCOREP_SUCCESS;
    }
    collect_comm_locations();
    scorep_ompt_unify_pre();
    return SCOREP_SUCCESS;
}


static SCOREP_ErrorCode
ompt_subsystem_post_unify( void )
{
    if ( scorep_ompt_interim_communicator_handle == SCOREP_INVALID_INTERIM_COMMUNICATOR )
    {
        return SCOREP_SUCCESS;
    }
    scorep_ompt_unify_post();
    return SCOREP_SUCCESS;
}


const SCOREP_Subsystem SCOREP_Subsystem_OmptAdapter =
{
    .subsystem_name                   = "OMPT",
    .subsystem_register               = &ompt_subsystem_register,
    .subsystem_init                   = &ompt_subsystem_init,
    .subsystem_begin                  = &ompt_subsystem_begin,
    .subsystem_end                    = &ompt_subsystem_end,
    .subsystem_init_location          = &ompt_subsystem_init_location,
    .subsystem_trigger_overdue_events = &scorep_ompt_subsystem_trigger_overdue_events,
    .subsystem_pre_unify              = &ompt_subsystem_pre_unify,
    .subsystem_post_unify             = &ompt_subsystem_post_unify
};
