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
 *  @status     alpha
 *
 *  @file       scorep_metric_management.c
 *
 *  @author     Ronny Tschueter <ronny.tschueter@zih.tu-dresden.de>
 *  @maintainer Ronny Tschueter <ronny.tschueter@zih.tu-dresden.de>
 *
 *  @brief This module handles implementations of different metric sources.
 *
 *  We have to distinguish different kinds of metrics regarding to their
 *  scope (validity). On the one hand there is a global set of metrics
 *  that is recorded by each location for every enter/leave event. This
 *  kind of metrics is call 'strictly synchronous' metrics. On the other
 *  hand there are additional scoped metrics. The additional scoped
 *  metrics consists of per-process metrics. For per-process metrics only
 *  one location of each location group records this kind of metrics.
 */

#include <config.h>

#include <SCOREP_Metric_Management.h>

#include <stdlib.h>
#include <string.h>

#include "SCOREP_Config.h"
#include <SCOREP_Memory.h>
#include <UTILS_Error.h>
#include <UTILS_Debug.h>
#include <SCOREP_RuntimeManagement.h>
#include <tracing/SCOREP_Tracing_Events.h>


#include <definitions/SCOREP_Definitions.h>
#include "scorep_system_tree.h"

/* RonnyT: is this include valid for a service */
#include "scorep_status.h"

/* Include to implement metric service */
#include "SCOREP_Metric.h"
/* Include to manage different metric sources */
#include "SCOREP_Metric_Source.h"
/* Include to write definitions */
#include "SCOREP_Definitions.h"

/* Include header files of supported metric sources */
#if HAVE( PAPI )
#include "scorep_metric_papi.h"
#endif
#if HAVE( GETRUSAGE )
#include "scorep_metric_rusage.h"
#endif
#if HAVE( METRIC_PLUGINS )
#include "scorep_metric_plugins.h"
#endif

#include <unistd.h>
#include <sys/types.h>


/** List of metric sources. */
static const SCOREP_MetricSource* scorep_metric_sources[] = {
#if HAVE( PAPI )
    &SCOREP_Metric_Papi,
#endif
#if HAVE( GETRUSAGE )
    &SCOREP_Metric_Rusage,
#endif
#if HAVE( METRIC_PLUGINS )
    &SCOREP_Metric_Plugins
#endif
};

/** Number of registered metric sources */
#define SCOREP_NUMBER_OF_METRIC_SOURCES sizeof( scorep_metric_sources ) / sizeof( scorep_metric_sources[ 0 ] )

/** Additional metrics are managed in a per-location based data structure.
 *  There are two different data structures handling synchronous or
 *  asynchronous metrics. This data structure handles additional synchronous
 *  metrics. Please see @SCOREP_Metric_LocationAsynchronousMetricSet for
 *  corresponding data structure of asynchronous metrics.
 *
 *  Need to define sampling set for individual metric, because synchronous metrics may not write values each time they are requested
 *  Therefore each metric must have its own sampling, as sampling sets have to be written completely or not at all.
 *
 *  There will be one @ SCOREP_Metric_LocationSynchronousMetricSet per
 *  metric type (e.g. per-process metric). */
typedef struct SCOREP_Metric_LocationSynchronousMetricSet SCOREP_Metric_LocationSynchronousMetricSet;
struct SCOREP_Metric_LocationSynchronousMetricSet
{
    /** Internal metric set of the metric service */
    SCOREP_Metric_EventSet* event_set[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** The number of sampling set definitions in @ sampling_sets.
     *  It also determines size of @ is_update_available. */
    uint32_t number_of_sampling_sets;

    /** The corresponding definitions */
    SCOREP_SamplingSetHandle* sampling_sets;

    /** Flag to indicate whether value of metric was updated recently */
    bool* is_update_available;

    /** Last written value of a specific metric. Used by
     *  SCOREP_Profile_Trigger* to calculate difference
     *  between current and last metric value. */
    uint64_t* last_written_value;

    /** Number of metrics contained in event sets */
    uint32_t metrics_counts[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** Offsets for metric values in global value array */
    uint32_t metrics_offsets[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** Next additional metric */
    SCOREP_Metric_LocationSynchronousMetricSet* next;
};

/** Additional metrics are managed in a per-location based data structure.
 *  There are two different data structures handling synchronous or
 *  asynchronous metrics. This data structure handles additional asynchronous
 *  metrics. Please see @SCOREP_Metric_LocationSynchronousMetricSet for
 *  corresponding data structure of asynchronous metrics.
 *
 *  There will be one @ SCOREP_Metric_LocationSynchronousMetricSet per
 *  metric type (e.g. per-process metric). */
typedef struct SCOREP_Metric_LocationAsynchronousMetricSet SCOREP_Metric_LocationAsynchronousMetricSet;
struct SCOREP_Metric_LocationAsynchronousMetricSet
{
    /** To distinguish the two (sub-)types of asynchronous metrics:
     *  ASYNCHRONOUS_EVENT and ASYNCHRONOUS */
    SCOREP_MetricSynchronicity synchronicity;

    /** Internal metric set of the metric service */
    SCOREP_Metric_EventSet* event_set[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** The number of sampling set definitions in @ sampling_sets */
    uint32_t number_of_sampling_sets;

    /** The corresponding sampling set definitions */
    SCOREP_SamplingSetHandle* sampling_sets;

    /** Number of metrics contained in event sets */
    uint32_t metrics_counts[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** Additional locations which will store values of asynchronous metrics */
    SCOREP_Location** additional_locations;

    /** Next additional metric */
    SCOREP_Metric_LocationAsynchronousMetricSet* next;
};


/** Type for per-location based data related to metrics. */
typedef struct SCOREP_Metric_LocationData SCOREP_Metric_LocationData;

/** Per-location based data related to metrics.
 *  The value array will be used by all metric kinds. */
struct SCOREP_Metric_LocationData
{
    /** Event sets of 'strictly synchronous' metrics */
    SCOREP_Metric_EventSet* event_set[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** Additional scoped synchronous metrics of location */
    SCOREP_Metric_LocationSynchronousMetricSet* additional_synchronous_metrics;

    /** Additional scoped asynchronous metrics of location */
    SCOREP_Metric_LocationAsynchronousMetricSet* additional_asynchronous_metrics;

    /** This location records any metrics (regardless of metric type,
     * 'strictly synchronous' or additional metric) */
    bool has_metrics;

    /** Size of values array */
    uint64_t size_of_values_array;

    /** Array of all metric values (including 'strictly synchronous' and
     *  additional metrics) */
    uint64_t* values;
};


/** Data structure to manage global set of 'strictly synchronous' metrics */
typedef struct scorep_strictly_synchronous_metrics
{
    /** Number of overall active metrics */
    uint32_t overall_number_of_metrics;

    /** Vector that contains number of requested 'strictly synchronous' metrics in each metric source */
    uint32_t counts[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** Vector of offsets for each metric. Metric source s starts writing its 'strictly synchronous'
     *  metric values to the global array at values[ offset[ s ] ]. */
    uint32_t offsets[ SCOREP_NUMBER_OF_METRIC_SOURCES ];

    /** Array of all metric handles used by the global sampling set of 'strictly synchronous' metrics */
    SCOREP_MetricHandle* metrics;

    /** Global sampling set of 'strictly synchronous' metrics */
    SCOREP_SamplingSetHandle sampling_set;
} scorep_strictly_synchronous_metrics;


/** Flag indicating status of metric management */
static bool scorep_metric_management_initialized = false;

/** 'strictly synchronous' metrics */
static scorep_strictly_synchronous_metrics strictly_synchronous_metrics;

/** Our subsystem id, used to address our per-location metric data */
static size_t scorep_metric_subsystem_id;


/* *********************************************************************
 * Function prototypes
 **********************************************************************/

static bool
initialize_location_metric_cb( SCOREP_Location* location,
                               void*            data );

static bool
initialize_location_metric_after_mpp_init_cb( SCOREP_Location* location,
                                              void*            data );

static bool
finalize_location_metric_cb( SCOREP_Location* location,
                             void*            data );

static int
scorep_metric_finalize_callback( void );

/* *********************************************************************
 * Macros
 **********************************************************************/

/* *INDENT-OFF* */

#define read_strictly_synchronous_metrics()                                                                                                                                 \
    for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )                                                                         \
    {                                                                                                                                                                       \
        if ( strictly_synchronous_metrics.counts[ source_index ] > 0 )                                                                                                      \
        {                                                                                                                                                                   \
            scorep_metric_sources[ source_index ]->metric_source_strictly_synchronous_read( metric_data->event_set[ source_index ],                                         \
                                                                                            &metric_data->values[ strictly_synchronous_metrics.offsets[ source_index ] ] ); \
        }                                                                                                                                                                   \
    }

#define read_synchronous_metrics()                                                                                                                                                          \
    {                                                                                                                                                                                       \
        SCOREP_Metric_LocationSynchronousMetricSet* location_synchronous_metric_set                                                                                                         \
            = metric_data->additional_synchronous_metrics;                                                                                                                                  \
        while ( location_synchronous_metric_set != NULL )                                                                                                                                   \
        {                                                                                                                                                                                   \
            for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )                                                                                 \
            {                                                                                                                                                                               \
                if ( location_synchronous_metric_set->metrics_counts[ source_index ] > 0 )                                                                                                  \
                {                                                                                                                                                                           \
                    scorep_metric_sources[ source_index ]->metric_source_synchronous_read( location_synchronous_metric_set->event_set[ source_index ],                                      \
                                                                                           &( metric_data->values[ location_synchronous_metric_set->metrics_offsets[ source_index ] ] ),    \
                                                                                           location_synchronous_metric_set->is_update_available,                                            \
                                                                                           false );                                                                                         \
                }                                                                                                                                                                           \
            }                                                                                                                                                                               \
                                                                                                                                                                                            \
            location_synchronous_metric_set = location_synchronous_metric_set->next;                                                                                                        \
        }                                                                                                                                                                                   \
    }

#define read_and_write_asynchronous_metrics( FORCE_UPDATE )                                                                                             \
    {                                                                                                                                                   \
        uint32_t recent_metric_index;                                                                                                                   \
                                                                                                                                                        \
        recent_metric_index = 0;                                                                                                                        \
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )                                                 \
        {                                                                                                                                               \
            if ( location_asynchronous_metric_set->metrics_counts[ source_index ] > 0 )                                                                 \
            {                                                                                                                                           \
                SCOREP_MetricTimeValuePair** time_value_pairs;                                                                                          \
                uint64_t*                    num_pairs;                                                                                                 \
                                                                                                                                                        \
                num_pairs        = NULL;                                                                                                                \
                time_value_pairs = malloc( location_asynchronous_metric_set->metrics_counts[ source_index ] * sizeof( SCOREP_MetricTimeValuePair* ) );  \
                UTILS_ASSERT( time_value_pairs != NULL );                                                                                               \
                                                                                                                                                        \
                scorep_metric_sources[ source_index ]->metric_source_asynchronous_read( location_asynchronous_metric_set->event_set[ source_index ],    \
                                                                                        time_value_pairs,                                               \
                                                                                        &num_pairs,                                                     \
                                                                                        FORCE_UPDATE );                                                 \
                for ( uint32_t metric_index = 0;                                                                                                        \
                      metric_index < location_asynchronous_metric_set->metrics_counts[ source_index ];                                                  \
                      metric_index++ )                                                                                                                  \
                {                                                                                                                                       \
                    for ( uint32_t pair_index = 0;                                                                                                      \
                          pair_index < num_pairs[ metric_index ];                                                                                       \
                          pair_index++ )                                                                                                                \
                    {                                                                                                                                   \
                        SCOREP_Tracing_Metric( location_asynchronous_metric_set->additional_locations[ metric_index ],                                  \
                                               time_value_pairs[ metric_index ][ pair_index ].timestamp,                                                \
                                               location_asynchronous_metric_set->sampling_sets[ recent_metric_index ],                                  \
                                               &( time_value_pairs[ metric_index ][ pair_index ].value ) );                                             \
                    }                                                                                                                                   \
                                                                                                                                                        \
                    free( time_value_pairs[ metric_index ] );                                                                                           \
                    time_value_pairs[ metric_index ] = NULL;                                                                                            \
                                                                                                                                                        \
                    recent_metric_index++;                                                                                                              \
                }                                                                                                                                       \
                                                                                                                                                        \
                free( time_value_pairs );                                                                                                               \
                free( num_pairs );                                                                                                                      \
                time_value_pairs = NULL;                                                                                                                \
                num_pairs        = NULL;                                                                                                                \
            }                                                                                                                                           \
        }                                                                                                                                               \
        location_asynchronous_metric_set = location_asynchronous_metric_set->next;                                                                      \
    }

/* *INDENT-ON* */

/* *********************************************************************
 * Service management
 **********************************************************************/

/** @brief Registers configuration variables for the metric services.
 *
 *  @param subsystem_id         Identifier of metric subsystem.
 *
 *  @return It returns SCOREP_SUCCESS if successful,
 *          otherwise an error code will be reported.
 */
static SCOREP_ErrorCode
scorep_metric_register( size_t subsystem_id )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " register metric management." );

    scorep_metric_subsystem_id = subsystem_id;

    /* Register metric sources */
    for ( size_t i = 0; i < SCOREP_NUMBER_OF_METRIC_SOURCES; i++ )
    {
        scorep_metric_sources[ i ]->metric_source_register();
    }

    return SCOREP_SUCCESS;
}

/** @brief Called on deregistration of the metric service.
 */
static void
scorep_metric_deregister( void )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " deregister metric management." );

    /* Deregister metric sources */
    for ( size_t i = 0; i < SCOREP_NUMBER_OF_METRIC_SOURCES; i++ )
    {
        scorep_metric_sources[ i ]->metric_source_deregister();
    }
}

/** @brief Called on initialization of the metric service.
 *
 *  @return It returns SCOREP_SUCCESS if successful,
 *          otherwise an error code will be reported.
 */
static SCOREP_ErrorCode
scorep_metric_initialize_service( void )
{
    /* Call only, if not previously initialized */
    if ( !scorep_metric_management_initialized )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " initialize metric management." );

        /* Register callback of metric management service which will be
         * called at the end of the measurement */
        SCOREP_RegisterExitCallback( scorep_metric_finalize_callback );

        strictly_synchronous_metrics.overall_number_of_metrics = 0;
        strictly_synchronous_metrics.sampling_set              = SCOREP_INVALID_SAMPLING_SET;

        for ( size_t i = 0; i < SCOREP_NUMBER_OF_METRIC_SOURCES; i++ )
        {
            /* Initialize metric source. metric_source_initialize() will return number of'synchronous
             * strict' metrics defined by the user and recorded by every location. */
            strictly_synchronous_metrics.counts[ i ] = scorep_metric_sources[ i ]->metric_source_initialize();

            /* Number of active metrics (valid up to now) indicates the required offset we have to use,
             * if we want to store values from this source in the shared values array at a later time. */
            strictly_synchronous_metrics.offsets[ i ] = strictly_synchronous_metrics.overall_number_of_metrics;

            /* Update number of active metrics */
            strictly_synchronous_metrics.overall_number_of_metrics += strictly_synchronous_metrics.counts[ i ];
        }

        /* Set initialization flag */
        scorep_metric_management_initialized = true;

        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " initialization of metric management done." );
    }

    return SCOREP_SUCCESS;
}

/** @brief Service finalization.
 */
static void
scorep_metric_finalize_service( void )
{
    /* Call only, if previously initialized */
    if ( scorep_metric_management_initialized )
    {
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
        {
            /* Finalize metric source */
            scorep_metric_sources[ source_index ]->metric_source_finalize();

            /* Reset number of requested metrics for recent source */
            strictly_synchronous_metrics.counts[ source_index ] = 0;
        }
        strictly_synchronous_metrics.overall_number_of_metrics = 0;
        strictly_synchronous_metrics.sampling_set              = SCOREP_INVALID_SAMPLING_SET;

        free( strictly_synchronous_metrics.metrics );

        /* Set initialization flag */
        scorep_metric_management_initialized = false;

        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " finalization of metric management done." );
    }
}

/** @brief Initialize per-location data structures.
 *
 *  @param location             Location data.
 *  @param data                 Additional user-defined data (not used yet).
 */
static bool
initialize_location_metric_cb( SCOREP_Location* location,
                               void*            data )
{
    /* Call only, if previously initialized. Additional check of location
     * type needed in case of SCOREP_Metric_Reinitialize */
    if ( scorep_metric_management_initialized
         && SCOREP_Location_GetType( location ) != SCOREP_LOCATION_TYPE_METRIC )
    {
        /* Get the thread local data related to metrics */
        SCOREP_Metric_LocationData* metric_data =
            SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
        UTILS_ASSERT( metric_data != NULL );

        /* Collection of event sets for all metric sources, metric synchronicities, and metric scopes
         *
         * event_set_collection[ SOURCE ][ SYNCHRONICITY ][ SCOPE ]
         * with
         *  SOURCE        = { PAPI | RUSAGE | PLUGIN }
         *  SYNCHRONICITY = { STRICTLY_SYNCHRONOUS | SYNCHRONOUS | ASYNCHRONOUS_EVENT | ASYNCHRONOUS }
         *  SCOPE         = { PER_THREAD | PER_PROCESS | PER_HOST | ONCE }
         *
         * Note:
         * Some combinations of synchronicity and scope are invalid! For example,
         * STRICTLY_SYNCHRONOUS may only be combined with PER_THREAD.
         */
        SCOREP_Metric_EventSet* event_set_collection[ SCOREP_NUMBER_OF_METRIC_SOURCES ][ SCOREP_METRIC_SYNC_TYPE_MAX ][ SCOREP_METRIC_PER_MAX ];
        memset( &( event_set_collection[ 0 ][ 0 ][ 0 ] ),
                0,
                SCOREP_NUMBER_OF_METRIC_SOURCES * SCOREP_METRIC_SYNC_TYPE_MAX * SCOREP_METRIC_PER_MAX * sizeof( SCOREP_Metric_EventSet* ) );

        /* Number of elements in metric values buffer.
         * The buffer is used to store values of strictly synchronous
         * and synhcronous metrics. All types of asynchronous metrics
         * store their values in additional metric locations. */
        uint64_t buffer_size = 0;

        /* Initialize metric sources for this location */
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
        {
            for ( SCOREP_MetricSynchronicity metric_synchronicity = SCOREP_METRIC_STRICTLY_SYNC;
                  metric_synchronicity <= SCOREP_METRIC_ASYNC;
                  metric_synchronicity++ )
            {
                /* Set up per-location (a.k.a. per-thread) event sets for this location */
                SCOREP_MetricPer metric_type = SCOREP_METRIC_PER_THREAD;
                event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ]
                    = scorep_metric_sources[ source_index ]->metric_source_initialize_location( location,
                                                                                                metric_synchronicity,
                                                                                                metric_type );

                /* Set up per-process event sets for this location */
                metric_type = SCOREP_METRIC_PER_PROCESS;
                /* Metric recorded by main thread only (thread 0 of each process) */
                if ( SCOREP_Location_GetId( location ) == 0 )
                {
                    event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ]
                        = scorep_metric_sources[ source_index ]->metric_source_initialize_location( location,
                                                                                                    metric_synchronicity,
                                                                                                    metric_type );
                }
                /*
                 * Location's responsibility to record PER_HOST and/or ONCE metrics can be decided yet.
                 * These metric types are handled in SCOREP_Metric_InitializeMpp().
                 */
            }

            /* Set link to event set of 'strictly synchronous' metrics */
            metric_data->event_set[ source_index ]
                = event_set_collection[ source_index ][ SCOREP_METRIC_STRICTLY_SYNC ][ SCOREP_METRIC_PER_THREAD ];
        }

        /* First entries in metric values buffer are always used by 'strictly synchronous' metrics which are recorded by each location */
        buffer_size += strictly_synchronous_metrics.overall_number_of_metrics;

        /*
         * (1): Define metric and sampling set handles of global 'strictly synchronous' metrics
         */

        /* Only the master thread defines handles of 'strictly synchronous' metrics */
        if ( strictly_synchronous_metrics.overall_number_of_metrics > 0
             && SCOREP_Location_GetId( location ) == 0 )
        {
            /* Now we know how many metrics are used, so we can allocate memory to store their handles */
            strictly_synchronous_metrics.metrics = malloc( strictly_synchronous_metrics.overall_number_of_metrics * sizeof( SCOREP_MetricHandle ) );
            UTILS_ASSERT( strictly_synchronous_metrics.metrics );

            uint64_t recent_metric_index = 0;
            for ( size_t i = 0; i < SCOREP_NUMBER_OF_METRIC_SOURCES; i++ )
            {
                for ( uint32_t j = 0; j < strictly_synchronous_metrics.counts[ i ]; j++ )
                {
                    /* Get properties of used metrics */
                    SCOREP_Metric_Properties props = scorep_metric_sources[ i ]->metric_source_props( metric_data->event_set[ i ], j );

                    /* Write metric member definition */
                    SCOREP_MetricHandle metric_handle = SCOREP_Definitions_NewMetric( props.name,
                                                                                      props.description,
                                                                                      props.source_type,
                                                                                      props.mode,
                                                                                      props.value_type,
                                                                                      props.base,
                                                                                      props.exponent,
                                                                                      props.unit,
                                                                                      props.profiling_type );
                    strictly_synchronous_metrics.metrics[ recent_metric_index ] = metric_handle;
                    recent_metric_index++;
                }
            }

            /* Write definition of sampling set */
            strictly_synchronous_metrics.sampling_set = SCOREP_Definitions_NewSamplingSet( ( uint8_t )strictly_synchronous_metrics.overall_number_of_metrics,
                                                                                           strictly_synchronous_metrics.metrics,
                                                                                           SCOREP_METRIC_OCCURRENCE_SYNCHRONOUS_STRICT,
                                                                                           SCOREP_SAMPLING_SET_CPU );
        }


        /*
         * (2) Handle different kinds of additional metrics.
         *     Define metric and scoped sampling set handles of
         *     each metric type.
         */

        /* Values of additional metrics are stored after values of
        * 'strictly synchronous' ones. Therefore, number of 'synchronous
        * strict' metrics defines first offset in values array. */
        uint32_t offset = strictly_synchronous_metrics.overall_number_of_metrics;

        /*
         * Next steps differ between synchronous and asynchronous metrics
         */

        /*
         * (2a) Handle additional synchronous metrics
         */

        /* Reference to previously used location metric set data structure */
        SCOREP_Metric_LocationSynchronousMetricSet* previous_location_synchronous_metric_set
            = metric_data->additional_synchronous_metrics;

        for ( uint32_t metric_type = SCOREP_METRIC_PER_THREAD; metric_type <= SCOREP_METRIC_PER_PROCESS; metric_type++ )
        {
            /* Location metric set data structure to work with */
            SCOREP_Metric_LocationSynchronousMetricSet* current_location_metric_set;
            /* Number of requested metrics per source */
            uint32_t current_metrics_vector[ SCOREP_NUMBER_OF_METRIC_SOURCES ];
            /* Sum of requested metrics */
            uint32_t current_overall_number_of_metrics;

            /* Get number of requested metrics */
            current_overall_number_of_metrics = 0;
            current_location_metric_set       = NULL;
            for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
            {
                current_metrics_vector[ source_index ] =
                    scorep_metric_sources[ source_index ]->metric_source_num_of_metrics( event_set_collection[ source_index ][ SCOREP_METRIC_SYNC ][ metric_type ] );
                current_overall_number_of_metrics += current_metrics_vector[ source_index ];
            }

            /* The user requested some metrics of currently processed type (e.g. per-process metrics) */
            if ( current_overall_number_of_metrics > 0 )
            {
                /* Create a new location metric set */
                current_location_metric_set = malloc( sizeof( SCOREP_Metric_LocationSynchronousMetricSet ) );
                UTILS_ASSERT( current_location_metric_set );

                for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                {
                    current_location_metric_set->metrics_counts[ source_index ]  = current_metrics_vector[ source_index ];
                    current_location_metric_set->metrics_offsets[ source_index ] = offset;
                    offset                                                      += current_metrics_vector[ source_index ];
                }
                current_location_metric_set->sampling_sets
                    = malloc( current_overall_number_of_metrics * sizeof( SCOREP_SamplingSetHandle ) );
                UTILS_ASSERT( current_location_metric_set->sampling_sets );
                current_location_metric_set->number_of_sampling_sets = current_overall_number_of_metrics;

                current_location_metric_set->is_update_available
                    = malloc( current_overall_number_of_metrics * sizeof( bool ) );
                UTILS_ASSERT( current_location_metric_set->is_update_available );

                /* This metric can be stored in the profile. The way synchronous metrics are stored in the profile
                 * requires a buffer for the last written value of a metric. Next time this metric is written again,
                 * the buffered value will be used too calculate difference between the last written value and the
                 * recent one. 'last_written_value' is used as this buffer described above. */
                current_location_metric_set->last_written_value
                    = malloc( current_overall_number_of_metrics * sizeof( uint64_t ) );
                UTILS_ASSERT( current_location_metric_set->last_written_value );
                memset( current_location_metric_set->last_written_value, 0, current_overall_number_of_metrics * sizeof( uint64_t ) );

                uint32_t recent_sampling_set_index = 0;
                for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                {
                    /* Store corresponding event sets */
                    current_location_metric_set->event_set[ source_index ]
                        = event_set_collection[ source_index ][ SCOREP_METRIC_SYNC ][ metric_type ];

                    for ( uint32_t metric_index = 0; metric_index < current_metrics_vector[ source_index ]; metric_index++ )
                    {
                        /* Get properties of used metrics */
                        SCOREP_Metric_Properties props =
                            scorep_metric_sources[ source_index ]->metric_source_props( event_set_collection[ source_index ][ SCOREP_METRIC_SYNC ][ metric_type ],
                                                                                        metric_index );

                        /* Write metric member definition */
                        SCOREP_MetricHandle metric_handle = SCOREP_Definitions_NewMetric( props.name,
                                                                                          props.description,
                                                                                          props.source_type,
                                                                                          props.mode,
                                                                                          props.value_type,
                                                                                          props.base,
                                                                                          props.exponent,
                                                                                          props.unit,
                                                                                          props.profiling_type );

                        if ( metric_type == SCOREP_METRIC_PER_THREAD )
                        {
                            /* Write definition of sampling set.
                             * We are only handling synchronous metrics here.
                             * Therefore METRIC_OCCURRENCE type is fixed. */
                            SCOREP_SamplingSetHandle current_sampling_set_handle
                                = SCOREP_Definitions_NewSamplingSet( 1,
                                                                     &metric_handle,
                                                                     SCOREP_METRIC_OCCURRENCE_SYNCHRONOUS,
                                                                     SCOREP_SAMPLING_SET_CPU );
                            /* Special handling of synchronous metrics which are recorded per location.
                             * As their metric values will be valid just for the recording location,
                             * such metrics do not need the definition of a scoped sampling set. */
                            current_location_metric_set->sampling_sets[ recent_sampling_set_index++ ]
                                = current_sampling_set_handle;
                        }
                        else if ( metric_type == SCOREP_METRIC_PER_PROCESS )
                        {
                            /* Write definition of sampling set.
                             * We are only handling synchronous metrics here.
                             * Therefore METRIC_OCCURRENCE type is fixed. */
                            SCOREP_SamplingSetHandle current_sampling_set_handle
                                = SCOREP_Definitions_NewSamplingSet( 1,
                                                                     &metric_handle,
                                                                     SCOREP_METRIC_OCCURRENCE_SYNCHRONOUS,
                                                                     SCOREP_SAMPLING_SET_ABSTRACT );

                            /* Get handle of current location as recorder */
                            SCOREP_LocationHandle current_location_handle = SCOREP_Location_GetLocationHandle( location );
                            SCOREP_MetricScope    scope                   = SCOREP_METRIC_SCOPE_LOCATION_GROUP;
                            SCOREP_AnyHandle      scope_handle            = SCOREP_GetLocationGroup();
                            if ( scope_handle == SCOREP_INVALID_LOCATION_GROUP )
                            {
                                UTILS_WARNING( "Can not get handle for location group." );
                            }

                            UTILS_ASSERT( scope_handle != SCOREP_MOVABLE_NULL );
                            current_location_metric_set->sampling_sets[ recent_sampling_set_index++ ]
                                = SCOREP_Definitions_NewScopedSamplingSet( current_sampling_set_handle,
                                                                           current_location_handle,
                                                                           scope,
                                                                           scope_handle );
                        }
                        else
                        {
                            UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                         "Unknown metric synchronicity %u", metric_type );
                        }
                    }
                }

                /* Synchronous metrics will store their values in the buffer. */
                buffer_size += current_overall_number_of_metrics;

                current_location_metric_set->next        = previous_location_synchronous_metric_set;
                previous_location_synchronous_metric_set = current_location_metric_set;

                /* Update list of additional metrics for this location and  */
                metric_data->additional_synchronous_metrics = current_location_metric_set;
            } // END 'if ( current_overall_number_of_metrics > 0 )'
        }     // END 'for all metric types'

        /*
         * (2b) Handle additional asynchronous metrics
         */

        for ( SCOREP_MetricSynchronicity metric_synchronicity = SCOREP_METRIC_ASYNC_EVENT;
              metric_synchronicity <= SCOREP_METRIC_ASYNC;
              metric_synchronicity++ )
        {
            /* Reference to previously used location metric set data structure */
            SCOREP_Metric_LocationAsynchronousMetricSet* previous_location_asynchronous_metric_set
                = metric_data->additional_asynchronous_metrics;

            for ( SCOREP_MetricPer metric_type = SCOREP_METRIC_PER_THREAD; metric_type <= SCOREP_METRIC_PER_PROCESS; metric_type++ )
            {
                /* Location metric set data structure to work with */
                SCOREP_Metric_LocationAsynchronousMetricSet* current_location_metric_set;
                /* Number of requested metrics per source */
                uint32_t current_metrics_vector[ SCOREP_NUMBER_OF_METRIC_SOURCES ];
                /* Sum of requested metrics */
                uint32_t current_overall_number_of_metrics;
                /* Flag to indicate whether additional locations for asynchronous metrics are needed or not */

                /* Get number of requested metrics */
                current_overall_number_of_metrics = 0;
                current_location_metric_set       = NULL;
                for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                {
                    current_metrics_vector[ source_index ] =
                        scorep_metric_sources[ source_index ]->metric_source_num_of_metrics( event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ] );
                    current_overall_number_of_metrics += current_metrics_vector[ source_index ];
                }

                /* The user requested some metrics of currently processed type (e.g. per-process metrics) */
                if ( current_overall_number_of_metrics > 0 )
                {
                    if ( SCOREP_IsProfilingEnabled() )
                    {
                        UTILS_WARNING( "Asynchronous metrics are not supported in profiling mode and will be skipped." );
                        break;
                    }

                    /* Create a new location metric set */
                    current_location_metric_set = malloc( sizeof( SCOREP_Metric_LocationAsynchronousMetricSet ) );
                    UTILS_ASSERT( current_location_metric_set );

                    /* Store metric synchronicity to distinguish ASYNC_EVENT and ASYNC metrics later on */
                    current_location_metric_set->synchronicity = metric_synchronicity;
                    for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                    {
                        current_location_metric_set->metrics_counts[ source_index ] = current_metrics_vector[ source_index ];
                    }

                    /* Asynchronous metrics will write their values to additional locations.
                     * Create new locations for each asynchronous metric */
                    current_location_metric_set->additional_locations
                        = malloc( current_overall_number_of_metrics * sizeof( SCOREP_LocationHandle* ) );
                    UTILS_ASSERT( current_location_metric_set->additional_locations != NULL );
                    for ( uint32_t loc_num = 0; loc_num < current_overall_number_of_metrics; loc_num++ )
                    {
                        current_location_metric_set->additional_locations[ loc_num ]
                            = SCOREP_Location_CreateNonCPULocation( location,
                                                                    SCOREP_LOCATION_TYPE_METRIC,
                                                                    "" );
                    }
                    current_location_metric_set->sampling_sets
                        = malloc( current_overall_number_of_metrics * sizeof( SCOREP_SamplingSetHandle ) );
                    UTILS_ASSERT( current_location_metric_set->sampling_sets );
                    current_location_metric_set->number_of_sampling_sets = current_overall_number_of_metrics;

                    uint64_t recent_metric_index = 0;
                    for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                    {
                        /* Store corresponding event sets */
                        current_location_metric_set->event_set[ source_index ]
                            = event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ];

                        for ( uint32_t metric_index = 0; metric_index < current_metrics_vector[ source_index ]; metric_index++ )
                        {
                            /* Get properties of used metrics */
                            SCOREP_Metric_Properties props =
                                scorep_metric_sources[ source_index ]->metric_source_props( event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ],
                                                                                            metric_index );

                            /* Write metric member definition */
                            SCOREP_MetricHandle metric_handle = SCOREP_Definitions_NewMetric( props.name,
                                                                                              props.description,
                                                                                              props.source_type,
                                                                                              props.mode,
                                                                                              props.value_type,
                                                                                              props.base,
                                                                                              props.exponent,
                                                                                              props.unit,
                                                                                              props.profiling_type );

                            /* Write definition of sampling set.
                             * SCOREP_METRIC_STRICTLY_SYNC and SCOREP_METRIC_SYNC
                             * will not appear here, because the loop iterates only
                             * over SCOREP_METRIC_ASYNC_EVENT and SCOREP_METRIC_ASYNC.
                             * Therefore, METRIC_OCCURRENCE type is fixed. */
                            SCOREP_SamplingSetHandle current_sampling_set_handle
                                = SCOREP_Definitions_NewSamplingSet( 1,
                                                                     &metric_handle,
                                                                     SCOREP_METRIC_OCCURRENCE_ASYNCHRONOUS,
                                                                     SCOREP_SAMPLING_SET_ABSTRACT );

                            /* As asynchronous metrics will write their values to additional locations,
                             * these metrics always need a definition of a scoped sampling set */

                            /* Determine scope and corresponding handle */
                            SCOREP_MetricScope scope        = SCOREP_INVALID_METRIC_SCOPE;
                            SCOREP_AnyHandle   scope_handle = SCOREP_MOVABLE_NULL;
                            switch ( metric_type )
                            {
                                case SCOREP_METRIC_PER_THREAD:
                                    scope        = SCOREP_METRIC_SCOPE_LOCATION;
                                    scope_handle = SCOREP_Location_GetLocationHandle( location );
                                    if ( scope_handle == SCOREP_INVALID_LOCATION )
                                    {
                                        UTILS_WARNING( "Can not get handle for location." );
                                    }
                                    break;
                                case SCOREP_METRIC_PER_PROCESS:
                                    scope        = SCOREP_METRIC_SCOPE_LOCATION_GROUP;
                                    scope_handle = SCOREP_GetLocationGroup();
                                    if ( scope_handle == SCOREP_INVALID_LOCATION_GROUP )
                                    {
                                        UTILS_WARNING( "Can not get handle for location group." );
                                    }
                                    break;
                                case SCOREP_METRIC_PER_HOST:
                                case SCOREP_METRIC_ONCE:
                                    UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                                 "Metrics of synchronicity 'per-host' and 'once' can not be handled yet" );
                                    break;
                                default:
                                    UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                                 "Unknown metric synchronicity %u", metric_type );
                            }

                            UTILS_ASSERT( scope_handle != SCOREP_MOVABLE_NULL );
                            SCOREP_LocationHandle recorder_location_handle
                                = SCOREP_Location_GetLocationHandle( current_location_metric_set->additional_locations[ recent_metric_index ] );
                            current_location_metric_set->sampling_sets[ recent_metric_index ]
                                = SCOREP_Definitions_NewScopedSamplingSet( current_sampling_set_handle,
                                                                           recorder_location_handle,
                                                                           scope,
                                                                           scope_handle );
                            recent_metric_index++;
                        }
                    }

                    current_location_metric_set->next         = previous_location_asynchronous_metric_set;
                    previous_location_asynchronous_metric_set = current_location_metric_set;

                    /* Update list of additional metrics for this location and  */
                    metric_data->additional_asynchronous_metrics = current_location_metric_set;

                    metric_data->has_metrics = true;
                } // END 'if ( current_overall_number_of_metrics > 0 )'
            }     // END 'for all metric types'
        }         // END 'for all metric synchronicities'

        /* Allocate memory for metric values buffer */
        if ( buffer_size != 0 )
        {
            metric_data->values = malloc( buffer_size * sizeof( uint64_t ) );
            UTILS_ASSERT( metric_data->values );
            metric_data->size_of_values_array = buffer_size;
            metric_data->has_metrics          = true;
        }
        else
        {
            metric_data->values               = NULL;
            metric_data->size_of_values_array = 0;
        }

        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " metric management has initialized location." );
    }

    return false;
}

/** @brief  Location specific initialization function for metric services.
 *
 *  @param location             Location data.
 *
 *  @return It returns SCOREP_SUCCESS if successful,
 *          otherwise an error code will be reported.
 */
static SCOREP_ErrorCode
scorep_metric_initialize_location( SCOREP_Location* location )
{
    UTILS_ASSERT( location != NULL );

    if ( SCOREP_Location_GetType( location ) == SCOREP_LOCATION_TYPE_METRIC )
    {
        /* No need to handle locations which are just used to store metrics */
        return SCOREP_SUCCESS;
    }

    /* Create per-location metric management data */
    SCOREP_Metric_LocationData* metric_data =
        SCOREP_Location_AllocForMisc( location,
                                      sizeof( *metric_data ) );

    SCOREP_Location_SetSubsystemData( location,
                                      scorep_metric_subsystem_id,
                                      metric_data );

    metric_data->has_metrics                     = false;
    metric_data->additional_synchronous_metrics  = NULL;
    metric_data->additional_asynchronous_metrics = NULL;
    metric_data->values                          = NULL;
    metric_data->size_of_values_array            = 0;

    /* All initialization is done in separate function that is re-used
     * by SCOREP_Metric_Reinitialize() */
    initialize_location_metric_cb( location, NULL );

    if ( !SCOREP_Status_IsMpp() )
    {
        initialize_location_metric_after_mpp_init_cb( location, NULL );
    }

    return SCOREP_SUCCESS;
}

/** @brief Finalize per-location data structures.
 *
 *  @param location             Location data.
 *  @param data                 Additional user-defined data (not used yet).
 */
static bool
finalize_location_metric_cb( SCOREP_Location* location,
                             void*            data )
{
    UTILS_ASSERT( location != NULL );

    /* Call only, if previously initialized. Additionally, there is no
     * need to handle locations which are just used to store metrics. */
    if ( scorep_metric_management_initialized
         && SCOREP_Location_GetType( location ) != SCOREP_LOCATION_TYPE_METRIC )
    {
        /* Get the thread local data related to metrics */
        SCOREP_Metric_LocationData* metric_data =
            SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
        UTILS_ASSERT( metric_data != NULL );

        /* Handle additional synchronous metrics */
        SCOREP_Metric_LocationSynchronousMetricSet* location_synchronous_metric_set
            = metric_data->additional_synchronous_metrics;
        SCOREP_Metric_LocationSynchronousMetricSet* sync_tmp;
        while ( location_synchronous_metric_set != NULL )
        {
            /* For each metric source (e.g. PAPI or Resource Usage) */
            for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
            {
                /* Check whether this metric source has additional metrics */
                if ( location_synchronous_metric_set->metrics_counts[ source_index ] > 0 )
                {
                    /* Free event set of additional metric */
                    scorep_metric_sources[ source_index ]->metric_source_free_additional_metric_event_set( location_synchronous_metric_set->event_set[ source_index ] );
                }
            }
            free( location_synchronous_metric_set->sampling_sets );
            free( location_synchronous_metric_set->is_update_available );
            free( location_synchronous_metric_set->last_written_value );

            /* Save pointer to currently handled metric set */
            sync_tmp = location_synchronous_metric_set;
            /* Set location_synchronous_metric_set to next metric set*/
            location_synchronous_metric_set = location_synchronous_metric_set->next;
            /* Free currently handled metric set */
            free( sync_tmp );
        }

        /* Handle additional asynchronous metrics */
        SCOREP_Metric_LocationAsynchronousMetricSet* location_asynchronous_metric_set
            = metric_data->additional_asynchronous_metrics;
        SCOREP_Metric_LocationAsynchronousMetricSet* async_tmp;
        while ( location_asynchronous_metric_set != NULL )
        {
            /* For each metric source (e.g. PAPI or Resource Usage) */
            for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
            {
                /* Check whether this metric source has additional metrics */
                if ( location_asynchronous_metric_set->metrics_counts[ source_index ] > 0 )
                {
                    /* Free event set of additional metric */
                    scorep_metric_sources[ source_index ]->metric_source_free_additional_metric_event_set( location_asynchronous_metric_set->event_set[ source_index ] );
                }
            }
            free( location_asynchronous_metric_set->sampling_sets );
            free( location_asynchronous_metric_set->additional_locations );

            /* Save pointer to currently handled metric set */
            async_tmp = location_asynchronous_metric_set;
            /* Set location_asynchronous_metric_set to next metric set*/
            location_asynchronous_metric_set = location_asynchronous_metric_set->next;
            /* Free currently handled metric set */
            free( async_tmp );
        }

        /* Handle strictly synchronous metrics and finalize location in metric source */
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
        {
            scorep_metric_sources[ source_index ]->metric_source_finalize_location( metric_data->event_set[ source_index ] );
        }

        free( metric_data->values );
        metric_data->size_of_values_array = 0;

        metric_data->has_metrics = false;

        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " metric management has finalized location." );
    }

    return false;
}

/** @brief Location specific finalization function for metric services.
 *
 *  @param location             Location data.
 *
 *  @param location Reference to location that will finalize its metric related data structures.
 */
static void
scorep_metric_finalize_location( SCOREP_Location* location )
{
    /* All finalization is done in separate function that is re-used
     * by SCOREP_Metric_Reinitialize() */
    finalize_location_metric_cb( location, NULL );
}

/** @brief This function is called by Score-P infrastructure at the end
 *         of the measurement. Within this function we will read all
 *         asynchronous metrics for the last time. Especially, this
 *         function is relevant for asynchronous post mortem metrics
 *         which will be called ONLY at the end of the measurement.
 *
 * @return 0 if successful.
 */
static int
scorep_metric_finalize_callback( void )
{
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " finalize callback" );

    /* RonnyT: Callback will be triggered on CPU locations only */
    SCOREP_Location* location = SCOREP_Location_GetCurrentCPULocation();
    if ( location == NULL )
    {
        return 0;
    }

    /* Get the thread local data related to metrics */
    SCOREP_Metric_LocationData* metric_data =
        SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
    UTILS_ASSERT( metric_data != NULL );

    if ( !metric_data->has_metrics )
    {
        /* Location does not record any metrics */
        return 0;
    }

    /* Just handle additional asynchronous metrics here ! */
    SCOREP_Metric_LocationAsynchronousMetricSet* location_asynchronous_metric_set
        = metric_data->additional_asynchronous_metrics;

    while ( location_asynchronous_metric_set != NULL )
    {
        if ( location_asynchronous_metric_set->synchronicity != SCOREP_METRIC_ASYNC )
        {
            /* location-specific metric of irrelevant synchronicity, skip it */
            location_asynchronous_metric_set = location_asynchronous_metric_set->next;
            continue;
        }

        if ( SCOREP_IsTracingEnabled() && SCOREP_RecordingEnabled() )
        {
            read_and_write_asynchronous_metrics( true );
        }
        else
        {
            uint32_t recent_metric_index;

            recent_metric_index = 0;
            for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
            {
                if ( location_asynchronous_metric_set->metrics_counts[ source_index ] > 0 )
                {
                    SCOREP_MetricTimeValuePair** time_value_pairs;
                    uint64_t*                    num_pairs;

                    num_pairs        = NULL;
                    time_value_pairs = malloc( location_asynchronous_metric_set->metrics_counts[ source_index ] * sizeof( SCOREP_MetricTimeValuePair* ) );
                    UTILS_ASSERT( time_value_pairs != NULL );

                    scorep_metric_sources[ source_index ]->metric_source_asynchronous_read( location_asynchronous_metric_set->event_set[ source_index ],
                                                                                            time_value_pairs,
                                                                                            &num_pairs,
                                                                                            false );
                    for ( uint32_t metric_index = 0;
                          metric_index < location_asynchronous_metric_set->metrics_counts[ source_index ];
                          metric_index++ )
                    {
                        for ( uint32_t pair_index = 0;
                              pair_index < num_pairs[ metric_index ];
                              pair_index++ )
                        {
                            SCOREP_SamplingSetDef* sampling_set
                                = SCOREP_LOCAL_HANDLE_DEREF( location_asynchronous_metric_set->sampling_sets[ recent_metric_index ], SamplingSet );

                            /* Asynchronous metrics always use scoped sampling sets */
                            UTILS_ASSERT( sampling_set->is_scoped );

                            SCOREP_ScopedSamplingSetDef* scoped_sampling_set =
                                ( SCOREP_ScopedSamplingSetDef* )sampling_set;
                            sampling_set = SCOREP_LOCAL_HANDLE_DEREF( scoped_sampling_set->sampling_set_handle,
                                                                      SamplingSet );

                            /* Make sure that scoped sampling set contains only one metric */
                            UTILS_ASSERT( sampling_set->number_of_metrics == 1 );

                            SCOREP_MetricValueType value_type = SCOREP_MetricHandle_GetValueType( sampling_set->metric_handles[ 0 ] );
                            switch ( value_type )
                            {
                                case SCOREP_METRIC_VALUE_INT64:
                                case SCOREP_METRIC_VALUE_UINT64:
                                    SCOREP_Profile_TriggerInteger( location,
                                                                   sampling_set->metric_handles[ 0 ],
                                                                   time_value_pairs[ metric_index ][ pair_index ].value );
                                    break;
                                case SCOREP_METRIC_VALUE_DOUBLE:
                                    SCOREP_Profile_TriggerDouble( location,
                                                                  sampling_set->metric_handles[ 0 ],
                                                                  time_value_pairs[ metric_index ][ pair_index ].value );
                                    break;
                                default:
                                    UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                                 "Unknown metric value type %u", value_type );
                            }
                        }

                        free( time_value_pairs[ metric_index ] );
                        time_value_pairs[ metric_index ] = NULL;

                        recent_metric_index++;
                    }

                    free( time_value_pairs );
                    free( num_pairs );
                    time_value_pairs = NULL;
                    num_pairs        = NULL;
                }
            }
            location_asynchronous_metric_set = location_asynchronous_metric_set->next;
        }
    }

    return 0;
}

/** @brief Continues initialization of per-location data structures
 *         after multi program paradigm was initialized (e.g. MPI_Init
 *         was called). Here we can decide which locations are
 *         responsible to record PER-HOST resp. ONCE metrics.
 *
 *  @param location             Location data.
 *  @param data                 Additional user-defined data (not used yet).
 */
static bool
initialize_location_metric_after_mpp_init_cb( SCOREP_Location* location,
                                              void*            data )
{
    /* Call only, if location is not a metric-only one and was
     * previously initialized. */
    if ( scorep_metric_management_initialized
         && SCOREP_Location_GetType( location ) != SCOREP_LOCATION_TYPE_METRIC )
    {
        /* Get the thread local data related to metrics */
        SCOREP_Metric_LocationData* metric_data =
            SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
        UTILS_ASSERT( metric_data != NULL );

        /* Collection of event sets for all metric sources, metric synchronicities, and metric scopes
         *
         * event_set_collection[ SOURCE ][ SYNCHRONICITY ][ SCOPE ]
         * with
         *  SOURCE        = { PAPI | RUSAGE | PLUGIN }
         *  SYNCHRONICITY = { STRICTLY_SYNCHRONOUS | SYNCHRONOUS | ASYNCHRONOUS_EVENT | ASYNCHRONOUS }
         *  SCOPE         = { PER_THREAD | PER_PROCESS | PER_HOST | ONCE }
         *
         * Note:
         * Some combinations of synchronicity and scope are invalid! For example,
         * STRICTLY_SYNCHRONOUS may only be combined with PER_THREAD.
         */
        SCOREP_Metric_EventSet* event_set_collection[ SCOREP_NUMBER_OF_METRIC_SOURCES ][ SCOREP_METRIC_SYNC_TYPE_MAX ][ SCOREP_METRIC_PER_MAX ];
        memset( &( event_set_collection[ 0 ][ 0 ][ 0 ] ),
                0,
                SCOREP_NUMBER_OF_METRIC_SOURCES * SCOREP_METRIC_SYNC_TYPE_MAX * SCOREP_METRIC_PER_MAX * sizeof( SCOREP_Metric_EventSet* ) );

        /* Initialize metric sources for this location */
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
        {
            /* Location's responsibility to record PER_HOST and/or ONCE metrics
             * can not be decided before multi program paradigm (MPP) was initialized
             * (e.g. MPI_Init() was called). Certainly, there are function calls
             * before MPP initialization. So, we've lost some enter/leave events
             * for reecording these metrics. That's why, PER_HOST resp. ONCE metrics
             * can't be 'strictly synchronous', which have to be recorded for all
             * enter/leave events of a location. */
            for ( SCOREP_MetricSynchronicity metric_synchronicity = SCOREP_METRIC_SYNC;
                  metric_synchronicity <= SCOREP_METRIC_ASYNC;
                  metric_synchronicity++ )
            {
                /*
                 * Metric types PER_THREAD and PER_PROCESS are
                 * handled in finalize_location_metric_cb().
                 */
                for ( uint32_t metric_type = SCOREP_METRIC_PER_HOST; metric_type < SCOREP_METRIC_PER_MAX; metric_type++ )
                {
                    switch ( metric_type )
                    {
                        case SCOREP_METRIC_PER_HOST:
                            if ( !SCOREP_Status_IsProcessMasterOnNode()
                                 || SCOREP_Location_GetId( location ) != 0 )
                            {
                                continue;
                            }
                            break;
                        case SCOREP_METRIC_ONCE:
                            if ( metric_synchronicity == SCOREP_METRIC_STRICTLY_SYNC )
                            {
                                UTILS_WARNING( "Invalid sync type for 'SCOREP_METRIC_ONCE' metric\n." );
                                continue;
                            }

                            if ( SCOREP_Status_GetRank() != 0
                                 || SCOREP_Location_GetId( location ) != 0 )
                            {
                                continue;
                            }
                            break;
                        default:
                            UTILS_WARNING( "Unknown metric scope\n." );
                            continue;
                    }
                    /* Set up event sets for this location */
                    event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ]
                        = scorep_metric_sources[ source_index ]->metric_source_initialize_location( location,
                                                                                                    metric_synchronicity,
                                                                                                    metric_type );
                }
            }
        }

        /*
         * (2) Handle different kinds of additional metrics.
         *     Define metric and scoped sampling set handles of
         *     each metric type.
         */

        /* First entries in metric values buffer are always used by 'strictly synchronous' metrics which are recorded by each location */
        uint64_t buffer_size = metric_data->size_of_values_array;

        /*
         * Values of additional metrics are stored after values of
         * 'strictly synchronous' ones. Therefore, number of 'strictly
         * synchronous' metrics defines first offset in values array.
         */
        uint32_t offset = buffer_size;

        /*
         * Next steps differ between synchronous and asynchronous metrics
         */

        /*
         * (2a) Handle additional synchronous metrics
         */

        /* Reference to previously used location metric set data structure */
        SCOREP_Metric_LocationSynchronousMetricSet* previous_location_synchronous_metric_set
            = metric_data->additional_synchronous_metrics;

        for ( SCOREP_MetricPer metric_type = SCOREP_METRIC_PER_HOST; metric_type < SCOREP_METRIC_PER_MAX; metric_type++ )
        {
            /* Location metric set data structure to work with */
            SCOREP_Metric_LocationSynchronousMetricSet* current_location_metric_set;
            /* Number of requested metrics per source */
            uint32_t current_metrics_vector[ SCOREP_NUMBER_OF_METRIC_SOURCES ];
            /* Sum of requested metrics */
            uint32_t current_overall_number_of_metrics;

            /* Get number of requested metrics */
            current_overall_number_of_metrics = 0;
            current_location_metric_set       = NULL;
            for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
            {
                current_metrics_vector[ source_index ] =
                    scorep_metric_sources[ source_index ]->metric_source_num_of_metrics( event_set_collection[ source_index ][ SCOREP_METRIC_SYNC ][ metric_type ] );
                current_overall_number_of_metrics += current_metrics_vector[ source_index ];
            }

            /* The user requested some metrics of currently processed type (e.g. per-process metrics) */
            if ( current_overall_number_of_metrics > 0 )
            {
                if ( SCOREP_IsProfilingEnabled() )
                {
                    UTILS_WARNING( "Metrics recorded per host or system-wide are not supported in profiling mode and will be skipped." );
                    break;
                }

                /* Create a new location metric set */
                current_location_metric_set = malloc( sizeof( SCOREP_Metric_LocationSynchronousMetricSet ) );
                UTILS_ASSERT( current_location_metric_set );

                for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                {
                    current_location_metric_set->metrics_counts[ source_index ]  = current_metrics_vector[ source_index ];
                    current_location_metric_set->metrics_offsets[ source_index ] = offset;
                    offset                                                      += current_metrics_vector[ source_index ];
                }
                current_location_metric_set->sampling_sets
                    = malloc( current_overall_number_of_metrics * sizeof( SCOREP_SamplingSetHandle ) );
                UTILS_ASSERT( current_location_metric_set->sampling_sets );
                current_location_metric_set->number_of_sampling_sets = current_overall_number_of_metrics;

                current_location_metric_set->is_update_available
                    = malloc( current_overall_number_of_metrics * sizeof( bool ) );
                UTILS_ASSERT( current_location_metric_set->is_update_available );

                /* At the moment metrics of kind SCOREP_METRIC_PER_HOST and SCOREP_METRIC_ONCE
                 * are not stored in the profile. Therefore, no buffer for old metric values
                 * is required. */
                current_location_metric_set->last_written_value = NULL;

                uint32_t recent_sampling_set_index = 0;
                for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                {
                    /* Store corresponding event sets */
                    current_location_metric_set->event_set[ source_index ]
                        = event_set_collection[ source_index ][ SCOREP_METRIC_SYNC ][ metric_type ];

                    for ( uint32_t metric_index = 0; metric_index < current_metrics_vector[ source_index ]; metric_index++ )
                    {
                        /* Get properties of used metrics */
                        SCOREP_Metric_Properties props =
                            scorep_metric_sources[ source_index ]->metric_source_props( event_set_collection[ source_index ][ SCOREP_METRIC_SYNC ][ metric_type ],
                                                                                        metric_index );

                        /* Write metric member definition */
                        SCOREP_MetricHandle metric_handle = SCOREP_Definitions_NewMetric( props.name,
                                                                                          props.description,
                                                                                          props.source_type,
                                                                                          props.mode,
                                                                                          props.value_type,
                                                                                          props.base,
                                                                                          props.exponent,
                                                                                          props.unit,
                                                                                          props.profiling_type );

                        /* Write definition of sampling set.
                         * We are only handling synchronous metrics here.
                         * Therefore METRIC_OCCURRENCE type is fixed. */
                        SCOREP_SamplingSetHandle current_sampling_set_handle
                            = SCOREP_Definitions_NewSamplingSet( 1,
                                                                 &metric_handle,
                                                                 SCOREP_METRIC_OCCURRENCE_SYNCHRONOUS,
                                                                 SCOREP_SAMPLING_SET_ABSTRACT );

                        /* Get handle of current location as recorder */
                        SCOREP_LocationHandle current_location_handle = SCOREP_Location_GetLocationHandle( location );
                        SCOREP_MetricScope    scope                   = SCOREP_INVALID_METRIC_SCOPE;
                        SCOREP_AnyHandle      scope_handle            = SCOREP_MOVABLE_NULL;

                        /* Determine scope and corresponding handle */
                        switch ( metric_type )
                        {
                            case SCOREP_METRIC_PER_HOST:
                                scope        = SCOREP_METRIC_SCOPE_SYSTEM_TREE_NODE;
                                scope_handle = SCOREP_GetSystemTreeNodeHandleForSharedMemory();
                                if ( scope_handle == SCOREP_INVALID_SYSTEM_TREE_NODE )
                                {
                                    UTILS_WARNING( "Can not get handle for system tree root node." );
                                }
                                break;
                            case SCOREP_METRIC_ONCE:
                                scope        = SCOREP_METRIC_SCOPE_SYSTEM_TREE_NODE;
                                scope_handle = SCOREP_GetSystemTreeRootNodeHandle();
                                if ( scope_handle == SCOREP_INVALID_SYSTEM_TREE_NODE )
                                {
                                    UTILS_WARNING( "Can not get handle for system tree node of shared memory domain." );
                                }
                                break;
                            default:
                                UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                             "Unknown metric synchronicity %u", metric_type );
                        }

                        UTILS_ASSERT( scope_handle != SCOREP_MOVABLE_NULL );
                        current_location_metric_set->sampling_sets[ recent_sampling_set_index++ ]
                            = SCOREP_Definitions_NewScopedSamplingSet( current_sampling_set_handle,
                                                                       current_location_handle,
                                                                       scope,
                                                                       scope_handle );
                    }
                }

                /* Synchronous metrics will store their values in the buffer. */
                buffer_size += current_overall_number_of_metrics;

                current_location_metric_set->next        = previous_location_synchronous_metric_set;
                previous_location_synchronous_metric_set = current_location_metric_set;

                /* Update list of additional metrics for this location and  */
                metric_data->additional_synchronous_metrics = current_location_metric_set;
            } // END 'if ( current_overall_number_of_metrics > 0 )'
        }     // END 'for all metric types'

        /*
         * (2b) Handle additional asynchronous metrics
         */

        for ( SCOREP_MetricSynchronicity metric_synchronicity = SCOREP_METRIC_ASYNC_EVENT;
              metric_synchronicity <= SCOREP_METRIC_ASYNC;
              metric_synchronicity++ )
        {
            /* Reference to previously used location metric set data structure */
            SCOREP_Metric_LocationAsynchronousMetricSet* previous_location_asynchronous_metric_set
                = metric_data->additional_asynchronous_metrics;

            for ( uint32_t metric_type = SCOREP_METRIC_PER_HOST; metric_type < SCOREP_METRIC_PER_MAX; metric_type++ )
            {
                /* Location metric set data structure to work with */
                SCOREP_Metric_LocationAsynchronousMetricSet* current_location_metric_set;
                /* Number of requested metrics per source */
                uint32_t current_metrics_vector[ SCOREP_NUMBER_OF_METRIC_SOURCES ];
                /* Sum of requested metrics */
                uint32_t current_overall_number_of_metrics;
                /* Flag to indicate whether additional locations for asynchronous metrics are needed or not */

                /* Get number of requested metrics */
                current_overall_number_of_metrics = 0;
                current_location_metric_set       = NULL;
                for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                {
                    current_metrics_vector[ source_index ] =
                        scorep_metric_sources[ source_index ]->metric_source_num_of_metrics( event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ] );
                    current_overall_number_of_metrics += current_metrics_vector[ source_index ];
                }

                /* The user requested some metrics of currently processed type (e.g. per-process metrics) */
                if ( current_overall_number_of_metrics > 0 )
                {
                    if ( SCOREP_IsProfilingEnabled() )
                    {
                        UTILS_WARNING( "Metrics recorded per host or system-wide are not supported in profiling mode and will be skipped." );
                        break;
                    }

                    /* Create a new location metric set */
                    current_location_metric_set = malloc( sizeof( SCOREP_Metric_LocationAsynchronousMetricSet ) );
                    UTILS_ASSERT( current_location_metric_set );

                    /* Store metric synchronicity to distinguish ASYNC_EVENT and ASYNC metrics later on */
                    current_location_metric_set->synchronicity = metric_synchronicity;
                    for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                    {
                        current_location_metric_set->metrics_counts[ source_index ] = current_metrics_vector[ source_index ];
                    }

                    /* Asynchronous metrics will write their values to additional locations.
                     * Create new locations for each asynchronous metric */
                    current_location_metric_set->additional_locations
                        = malloc( current_overall_number_of_metrics * sizeof( SCOREP_LocationHandle* ) );
                    UTILS_ASSERT( current_location_metric_set->additional_locations != NULL );
                    for ( uint32_t loc_num = 0; loc_num < current_overall_number_of_metrics; loc_num++ )
                    {
                        current_location_metric_set->additional_locations[ loc_num ]
                            = SCOREP_Location_CreateNonCPULocation( location,
                                                                    SCOREP_LOCATION_TYPE_METRIC,
                                                                    "" );
                    }
                    current_location_metric_set->sampling_sets
                        = malloc( current_overall_number_of_metrics * sizeof( SCOREP_SamplingSetHandle ) );
                    UTILS_ASSERT( current_location_metric_set->sampling_sets );
                    current_location_metric_set->number_of_sampling_sets = current_overall_number_of_metrics;

                    uint64_t recent_metric_index = 0;
                    for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
                    {
                        /* Store corresponding event sets */
                        current_location_metric_set->event_set[ source_index ]
                            = event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ];

                        for ( uint32_t metric_index = 0; metric_index < current_metrics_vector[ source_index ]; metric_index++ )
                        {
                            /* Get properties of used metrics */
                            SCOREP_Metric_Properties props =
                                scorep_metric_sources[ source_index ]->metric_source_props( event_set_collection[ source_index ][ metric_synchronicity ][ metric_type ],
                                                                                            metric_index );

                            /* Write metric member definition */
                            SCOREP_MetricHandle metric_handle = SCOREP_Definitions_NewMetric( props.name,
                                                                                              props.description,
                                                                                              props.source_type,
                                                                                              props.mode,
                                                                                              props.value_type,
                                                                                              props.base,
                                                                                              props.exponent,
                                                                                              props.unit,
                                                                                              props.profiling_type );

                            /* Write definition of sampling set.
                             * SCOREP_METRIC_STRICTLY_SYNC and SCOREP_METRIC_SYNC
                             * will not appear here, because the loop iterates only
                             * over SCOREP_METRIC_ASYNC_EVENT and SCOREP_METRIC_ASYNC.
                             * Therefore, METRIC_OCCURRENCE type is fixed. */
                            SCOREP_SamplingSetHandle current_sampling_set_handle
                                = SCOREP_Definitions_NewSamplingSet( 1,
                                                                     &metric_handle,
                                                                     SCOREP_METRIC_OCCURRENCE_ASYNCHRONOUS,
                                                                     SCOREP_SAMPLING_SET_ABSTRACT );

                            /* As asynchronous metrics will write their values to additional locations,
                             * these metrics always need a definition of a scoped sampling set */

                            /* Determine scope and corresponding handle */
                            SCOREP_MetricScope scope        = SCOREP_INVALID_METRIC_SCOPE;
                            SCOREP_AnyHandle   scope_handle = SCOREP_MOVABLE_NULL;
                            switch ( metric_type )
                            {
                                case SCOREP_METRIC_PER_HOST:
                                    scope        = SCOREP_METRIC_SCOPE_SYSTEM_TREE_NODE;
                                    scope_handle = SCOREP_GetSystemTreeNodeHandleForSharedMemory();
                                    if ( scope_handle == SCOREP_INVALID_SYSTEM_TREE_NODE )
                                    {
                                        UTILS_WARNING( "Can not get handle for system tree node of shared memory domain." );
                                    }
                                    break;
                                case SCOREP_METRIC_ONCE:
                                    scope        = SCOREP_METRIC_SCOPE_SYSTEM_TREE_NODE;
                                    scope_handle = SCOREP_GetSystemTreeRootNodeHandle();
                                    if ( scope_handle == SCOREP_INVALID_SYSTEM_TREE_NODE )
                                    {
                                        UTILS_WARNING( "Can not get handle for system tree root node." );
                                    }
                                    break;
                                default:
                                    UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                                 "Unknown metric synchronicity %u", metric_type );
                            }

                            UTILS_ASSERT( scope_handle != SCOREP_MOVABLE_NULL );
                            SCOREP_LocationHandle recorder_location_handle
                                = SCOREP_Location_GetLocationHandle( current_location_metric_set->additional_locations[ recent_metric_index ] );
                            current_location_metric_set->sampling_sets[ recent_metric_index ]
                                = SCOREP_Definitions_NewScopedSamplingSet( current_sampling_set_handle,
                                                                           recorder_location_handle,
                                                                           scope,
                                                                           scope_handle );
                            recent_metric_index++;
                        }
                    }

                    current_location_metric_set->next         = previous_location_asynchronous_metric_set;
                    previous_location_asynchronous_metric_set = current_location_metric_set;

                    /* Update list of additional metrics for this location and  */
                    metric_data->additional_asynchronous_metrics = current_location_metric_set;

                    metric_data->has_metrics = true;
                } // END 'if ( current_overall_number_of_metrics > 0 )'
            }     // END 'for all metric types'
        }         // END 'for all metric synchronicities'

        /* Allocate memory for metric values buffer */
        if ( buffer_size > metric_data->size_of_values_array )
        {
            metric_data->values = realloc( metric_data->values, buffer_size * sizeof( uint64_t ) );
            UTILS_ASSERT( metric_data->values );
            metric_data->size_of_values_array = buffer_size;
            metric_data->has_metrics          = true;
        }

        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " metric management has initialized location." );
    }

    return false;
}


/* *********************************************************************
 * Functions called directly by measurement environment
 **********************************************************************/

/** @brief Runs initialization task which has to be executed after multi
 *         program paradigm was initialized.
 */
void
SCOREP_Metric_InitializeMpp( void )
{
    /* Reinitialize each location */
    SCOREP_Location_ForAll( initialize_location_metric_after_mpp_init_cb, NULL );
}

/** @brief  Get recent values of all metrics.
 *
 *  @param location             Location data.
 *
 *  @return Returns pointer to value array filled with recent metric values,
 *          or NULL if we don't have metrics to read from.
 */
uint64_t*
SCOREP_Metric_Read( SCOREP_Location* location )
{
    /* Call only if previously initialized */
    if ( !scorep_metric_management_initialized )
    {
        return NULL;
    }

    /* Get the thread local data related to metrics */
    SCOREP_Metric_LocationData* metric_data =
        SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
    UTILS_ASSERT( metric_data != NULL );

    if ( !metric_data->has_metrics )
    {
        /* Location does not record any metrics */
        return NULL;
    }

    /* (1) Handle 'strictly synchronous' metrics */
    read_strictly_synchronous_metrics();

    /* (2) Handle additional synchronous metric */
    read_synchronous_metrics();

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_METRIC, " metric management has read metric values." );

    return metric_data->values;
}

/** @brief  Reinitialize metric management. This functionality is used by
 *          Score-P Online Access to change recorded metrics between
 *          separate phases of program execution.
 *
 *  @return It returns SCOREP_SUCCESS if successful,
 *          otherwise an error code will be reported.
 */
SCOREP_ErrorCode
SCOREP_Metric_Reinitialize( void )
{
    /* Finalize each location (frees internal buffers) */
    SCOREP_Location_ForAll( finalize_location_metric_cb, NULL );

    /* Finalize metric service */
    scorep_metric_finalize_service();

    /* Reinitialize metric service */
    scorep_metric_initialize_service();

    /* Reinitialize each location */
    SCOREP_Location_ForAll( initialize_location_metric_cb, NULL );
    SCOREP_Location_ForAll( initialize_location_metric_after_mpp_init_cb, NULL );

    return SCOREP_SUCCESS;
}

/** @brief  Returns the sampling set handle to the measurement system.
 *
 *  @return Returns the sampling set handle to the measurement system.
 */
SCOREP_SamplingSetHandle
SCOREP_Metric_GetStrictlySynchronousSamplingSet( void )
{
    return strictly_synchronous_metrics.sampling_set;
}

/** @brief  Returns the handle of a synchronous metric to the measurement system.
 *
 *  @return Returns the handle of a synchronous metric to the measurement system.
 */
SCOREP_MetricHandle
SCOREP_Metric_GetStrictlySynchronousMetricHandle( uint32_t index )
{
    UTILS_ASSERT( index < strictly_synchronous_metrics.overall_number_of_metrics );

    return strictly_synchronous_metrics.metrics[ index ];
}

/** @brief  Returns the number of a synchronous metrics.
 *
 *  @return Returns the number of a synchronous metrics.
 */
uint32_t
SCOREP_Metric_GetNumberOfStrictlySynchronousMetrics( void )
{
    return strictly_synchronous_metrics.overall_number_of_metrics;
}

/** @brief Writes all metrics of a location in tracing mode.
 *
 *  @param location             Location data.
 *  @param timestamp            Time when event occurred.
 */
void
SCOREP_Metric_WriteToTrace( SCOREP_Location* location,
                            uint64_t         timestamp )
{
    /* Get the thread local data related to metrics */
    SCOREP_Metric_LocationData* metric_data =
        SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
    UTILS_ASSERT( metric_data != NULL );

    if ( !metric_data->has_metrics )
    {
        /* Location does not record any metrics */
        return;
    }

    /* (1) Handle 'strictly synchronous' metrics */
    if ( strictly_synchronous_metrics.sampling_set != SCOREP_INVALID_SAMPLING_SET )
    {
        SCOREP_Tracing_Metric( location,
                               timestamp,
                               strictly_synchronous_metrics.sampling_set,
                               metric_data->values );
    }

    /* (2) Handle additional synchronous metrics */
    SCOREP_Metric_LocationSynchronousMetricSet* location_synchronous_metric_set
        = metric_data->additional_synchronous_metrics;
    while ( location_synchronous_metric_set != NULL )
    {
        uint32_t sampling_set_index;

        sampling_set_index = 0;
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
        {
            for ( uint32_t metric_index = 0;
                  metric_index < location_synchronous_metric_set->metrics_counts[ source_index ];
                  metric_index++ )
            {
                if ( location_synchronous_metric_set->is_update_available[ sampling_set_index ] )
                {
                    SCOREP_Tracing_Metric( location,
                                           timestamp,
                                           location_synchronous_metric_set->sampling_sets[ sampling_set_index ],
                                           &( metric_data->values[ location_synchronous_metric_set->metrics_offsets[ source_index ] + metric_index ] ) );
                }
                sampling_set_index++;
            }
        }

        location_synchronous_metric_set = location_synchronous_metric_set->next;
    }

    /* (3) Handle additional asynchronous metrics */
    SCOREP_Metric_LocationAsynchronousMetricSet* location_asynchronous_metric_set
        = metric_data->additional_asynchronous_metrics;

    while ( location_asynchronous_metric_set != NULL )
    {
        read_and_write_asynchronous_metrics( false );
    }
}

void
SCOREP_Metric_WriteToProfile( SCOREP_Location* location )
{
    /* Get the thread local data related to metrics */
    SCOREP_Metric_LocationData* metric_data =
        SCOREP_Location_GetSubsystemData( location, scorep_metric_subsystem_id );
    UTILS_ASSERT( metric_data != NULL );

    if ( !metric_data->has_metrics )
    {
        /* Location does not record any metrics */
        return;
    }

    /* 'Strictly synchronous' are handled at enter/leave events.
     * So this type of metric can be skipped here. */

    /* Handle additional synchronous metrics */
    SCOREP_Metric_LocationSynchronousMetricSet* location_synchronous_metric_set
        = metric_data->additional_synchronous_metrics;
    while ( location_synchronous_metric_set != NULL )
    {
        uint32_t sampling_set_index;

        sampling_set_index = 0;
        for ( size_t source_index = 0; source_index < SCOREP_NUMBER_OF_METRIC_SOURCES; source_index++ )
        {
            for ( uint32_t metric_index = 0;
                  metric_index < location_synchronous_metric_set->metrics_counts[ source_index ];
                  metric_index++ )
            {
                if ( location_synchronous_metric_set->is_update_available[ sampling_set_index ] )
                {
                    SCOREP_SamplingSetDef* sampling_set
                        = SCOREP_LOCAL_HANDLE_DEREF( location_synchronous_metric_set->sampling_sets[ sampling_set_index ], SamplingSet );
                    if ( sampling_set->is_scoped )
                    {
                        SCOREP_ScopedSamplingSetDef* scoped_sampling_set =
                            ( SCOREP_ScopedSamplingSetDef* )sampling_set;
                        sampling_set = SCOREP_LOCAL_HANDLE_DEREF( scoped_sampling_set->sampling_set_handle,
                                                                  SamplingSet );
                    }

                    /* Make sure that sampling set contains only one metric.
                     * We handle each synchronous metric in its individual
                     * sampling sets to permit synchronous metrics to skip
                     * writing a value if the metric was not updated. Therefore,
                     * a sampling set of a synchronous metric needs to contain
                     * exactly one metric. */
                    UTILS_ASSERT( sampling_set->number_of_metrics == 1 );

                    SCOREP_MetricValueType value_type =
                        SCOREP_MetricHandle_GetValueType( sampling_set->metric_handles[ 0 ] );
                    uint64_t diff_int;
                    double   diff_dbl;
                    switch ( value_type )
                    {
                        case SCOREP_METRIC_VALUE_INT64:
                        case SCOREP_METRIC_VALUE_UINT64:
                            diff_int                                                                  = metric_data->values[ location_synchronous_metric_set->metrics_offsets[ source_index ] + metric_index ] - location_synchronous_metric_set->last_written_value[ sampling_set_index ];
                            location_synchronous_metric_set->last_written_value[ sampling_set_index ] = metric_data->values[ location_synchronous_metric_set->metrics_offsets[ source_index ] + metric_index ];
                            SCOREP_Profile_TriggerInteger( location,
                                                           sampling_set->metric_handles[ 0 ],
                                                           diff_int );
                            break;
                        case SCOREP_METRIC_VALUE_DOUBLE:
                            diff_dbl                                                                  = ( double )metric_data->values[ location_synchronous_metric_set->metrics_offsets[ source_index ] + metric_index ] - ( double )location_synchronous_metric_set->last_written_value[ sampling_set_index ];
                            location_synchronous_metric_set->last_written_value[ sampling_set_index ] = metric_data->values[ location_synchronous_metric_set->metrics_offsets[ source_index ] + metric_index ];
                            SCOREP_Profile_TriggerDouble( location,
                                                          sampling_set->metric_handles[ 0 ],
                                                          diff_dbl );
                            break;
                        default:
                            UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT,
                                         "Unknown metric value type %u", value_type );
                    }
                }
                sampling_set_index++;
            }
        }

        location_synchronous_metric_set = location_synchronous_metric_set->next;
    }

    /* At the moment additional asynchronous metrics are not stored in the profile */
}

#undef read_strictly_synchronous_metrics
#undef read_synchronous_metrics
#undef read_and_write_asynchronous_metrics

/* *********************************************************************
 * Subsystem declaration
 **********************************************************************/

/**
 * Implementation of the metric service initialization/finalization struct
 */
const SCOREP_Subsystem SCOREP_Subsystem_MetricService =
{
    .subsystem_name              = "METRIC",
    .subsystem_register          = &scorep_metric_register,
    .subsystem_init              = &scorep_metric_initialize_service,
    .subsystem_init_location     = &scorep_metric_initialize_location,
    .subsystem_finalize_location = &scorep_metric_finalize_location,
    .subsystem_pre_unify         = NULL,
    .subsystem_post_unify        = NULL,
    .subsystem_finalize          = &scorep_metric_finalize_service,
    .subsystem_deregister        = &scorep_metric_deregister,
    .subsystem_control           = NULL
};
