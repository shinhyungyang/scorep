/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014, 2015
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 *  @file
 *
 *  @brief Implementation of OpenCL API, kernel and transfer recording.
 */

#include <config.h>

#include "scorep_opencl.h"
#include "scorep_opencl_config.h"

#include <SCOREP_Mutex.h>
#include <SCOREP_Memory.h>
#include <SCOREP_Events.h>
#include <SCOREP_Timing.h>

#include <jenkins_hash.h>

#ifdef SCOREP_LIBWRAP_SHARED
#include "scorep_opencl_function_pointers.h"
#endif

#include <string.h>
#include <inttypes.h>


/** @def KERNEL_STRING_LENGTH
 *  maximum number of characters for OpenCL kernels */
#define KERNEL_STRING_LENGTH 64

/** @def KERNEL_HASHTABLE_SIZE
 *  the default size for the OpenCL kernel name hash table */
#define KERNEL_HASHTABLE_SIZE 1024

/**
 * The key of the hash node is a string, the value the corresponding region handle.
 * It is used to store region names with its corresponding region handles.
 * The region name is implemented as C99 flexible array member.
 */
typedef struct scorep_opencl_kernel_hash_node
{
    SCOREP_RegionHandle                    region; /**< associated region handle */
    uint32_t                               hash;   /**< hash for this region */
    struct scorep_opencl_kernel_hash_node* next;   /**< bucket for collision */
    char                                   name[]; /**< name of the symbol */
} scorep_opencl_kernel_hash_node;

/** hash table for OpenCL kernels */
static scorep_opencl_kernel_hash_node* opencl_kernel_hashtab[ KERNEL_HASHTABLE_SIZE ];

/**
 * @def SCOREP_OPENCL_CHECK
 * Check an OpenCL error code and print the error string as warning.
 *
 * @param err OpenCL error code
 */
#define SCOREP_OPENCL_CHECK( err )                              \
    if ( err != CL_SUCCESS )                                    \
    {                                                           \
        UTILS_WARNING( "[OpenCL] Error '%s'",                   \
                       scorep_opencl_get_error_string( err ) ); \
    }

/**
 * @def OPENCL_CALL
 * Call the 'real' OpenCL function (and not the wrapper).
 *
 * @param func OpenCL API function call
 */
#ifdef SCOREP_LIBWRAP_STATIC
#define OPENCL_CALL( func, args ) __real_##func args
#elif SCOREP_LIBWRAP_SHARED
#define OPENCL_CALL( func, args ) scorep_opencl_funcptr_##func args
#endif


/*
 * OpenCL version specifics
 */
#if ( defined( CL_VERSION_1_2 ) && ( CL_VERSION_1_2 == 1 ) ) || ( defined( CL_VERSION_2_0 ) && ( CL_VERSION_2_0 == 1 ) )
/**
 * @def clEnqueueMarker
 * Call clEnqueueMarkerWithWaitList function
 */
#define clEnqueueMarker( _clQueue, _clEvt ) \
    clEnqueueMarkerWithWaitList( _clQueue, 0, NULL, _clEvt )
#endif /* defined(CL_VERSION_1_2) && (CL_VERSION_1_2 == 1) */

// thread (un)locking macros for OpenCL wrapper
/**
 * @def SCOREP_OPENCL_LOCK
 * Lock mutex of the OpenCL adapter
 */
# define SCOREP_OPENCL_LOCK() SCOREP_MutexLock( opencl_mutex )
/**
 * @def SCOREP_OPENCL_UNLOCK
 * Unlock mutex of the OpenCL adapter
 */
# define SCOREP_OPENCL_UNLOCK() SCOREP_MutexUnlock( opencl_mutex )

/** Score-P mutex for the OpenCL wrapper */
static SCOREP_Mutex opencl_mutex = NULL;

/**
 * Internal location mapping for unification (needed for OpenCL communication)
 */
typedef struct scorep_opencl_location
{
    SCOREP_Location*               location;    /**< Score-P location */
    uint32_t                       location_id; /**< wrapper internal location ID */
    struct scorep_opencl_location* next;        /**< pointer to next element */
} scorep_opencl_location;

static scorep_opencl_location* location_list = NULL;

/** count communicating locations (for OpenCL communication unification) */
size_t scorep_opencl_global_location_number = 0;

/** pointer to the array containing all communicating locations */
uint64_t* scorep_opencl_global_location_ids = NULL;

/** handles for OpenCL communication unification */
SCOREP_InterimCommunicatorHandle scorep_opencl_interim_communicator_handle =
    SCOREP_INVALID_INTERIM_COMMUNICATOR;

/** handles for OpenCL RMA window unification */
SCOREP_InterimRmaWindowHandle scorep_opencl_interim_window_handle =
    SCOREP_INVALID_INTERIM_RMA_WINDOW;

/** ID of the OpenCL subsystem */
size_t scorep_opencl_subsystem_id = 0;


/* ****************************************************************** */
/*          global region IDs for wrapper internal recording          */
/* ****************************************************************** */
static SCOREP_RegionHandle     opencl_sync_region_handle  = SCOREP_INVALID_REGION;
static SCOREP_RegionHandle     opencl_flush_region_handle = SCOREP_INVALID_REGION;
static SCOREP_SourceFileHandle opencl_kernel_file_handle  = SCOREP_INVALID_SOURCE_FILE;


/* ****************************************************************** */
/*                       function declarations                        */
/* ****************************************************************** */

static bool
set_synchronization_point( scorep_opencl_queue* queue );

static bool
add_synchronization_event( scorep_opencl_queue* queue );

static void*
kernel_hash_put( const char*         name,
                 SCOREP_RegionHandle region );

static void*
kernel_hash_get( const char* name );

static void
opencl_create_comm_group( void );


/* ****************************************************************** */
/*             flags to track the state of the wrapper                */
/* ****************************************************************** */

/** Flag indicating whether OpenCL wrapper is initialized */
static bool opencl_wrap_initialized = false;
/** Flag indicating whether OpenCL wrapper is finalized */
static bool opencl_wrap_finalized = false;


/** list of Score-P OpenCL command queues */
static scorep_opencl_queue* cl_queue_list = NULL;

/** maximum number of Score-P command queue buffer entries */
static size_t queue_max_buffer_entries = 0;

/**
 * Initialize the OpenCL adapter.
 *
 * We assume that this function cannot be executed concurrently by
 * multiple threads.
 */
void
scorep_opencl_wrap_init( void )
{
    if ( !opencl_wrap_initialized )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPENCL, "[OpenCL] Initialize wrapper" );

        SCOREP_MutexCreate( &opencl_mutex );

        /* define region for OpenCL synchronization */
        SCOREP_SourceFileHandle cl_sync_file_handle =
            SCOREP_Definitions_NewSourceFile( "OPENCL_SYNC" );

        opencl_sync_region_handle =
            SCOREP_Definitions_NewRegion( "WAIT FOR COMMAND QUEUE",
                                          NULL,
                                          cl_sync_file_handle,
                                          0, 0, SCOREP_PARADIGM_OPENCL,
                                          SCOREP_REGION_ARTIFICIAL );

        /* define region for OpenCL flush */
        SCOREP_SourceFileHandle cl_flush_file_handle =
            SCOREP_Definitions_NewSourceFile( "OPENCL_FLUSH" );

        opencl_flush_region_handle =
            SCOREP_Definitions_NewRegion( "BUFFER FLUSH",
                                          NULL,
                                          cl_flush_file_handle,
                                          0, 0, SCOREP_PARADIGM_OPENCL,
                                          SCOREP_REGION_ARTIFICIAL );

        if ( scorep_opencl_record_kernels )
        {
            opencl_kernel_file_handle = SCOREP_Definitions_NewSourceFile( "OPENCL_KERNEL" );
        }

        if ( scorep_opencl_record_memcpy )
        {
            /* create interim communicator once for a process */
            scorep_opencl_interim_communicator_handle =
                SCOREP_Definitions_NewInterimCommunicator(
                    SCOREP_INVALID_INTERIM_COMMUNICATOR,
                    SCOREP_PARADIGM_OPENCL,
                    0,
                    NULL );

            scorep_opencl_interim_window_handle =
                SCOREP_Definitions_NewInterimRmaWindow(
                    "OPENCL_WINDOW",
                    scorep_opencl_interim_communicator_handle );
        }

        queue_max_buffer_entries = scorep_opencl_queue_size
                                   / sizeof( scorep_opencl_buffer_entry );

        opencl_wrap_initialized = true;
    }
}


/**
 * Finalize the OpenCL adapter.
 */
void
scorep_opencl_wrap_finalize( void )
{
    if ( !opencl_wrap_finalized && opencl_wrap_initialized )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPENCL, "[OpenCL] Finalize wrapper" );

        // flush remaining entries and release command queues
        if ( scorep_opencl_record_memcpy || scorep_opencl_record_kernels )
        {
            scorep_opencl_queue* queue = cl_queue_list;
            while ( queue != NULL )
            {
                if ( queue->buffer < queue->buf_pos )
                {
                    scorep_opencl_queue_flush( queue );
                }

                if ( queue->queue )
                {
                    SCOREP_OPENCL_CALL( clReleaseCommandQueue, ( queue->queue ) );
                }

                queue = queue->next;
            }
        }

        // finalize Score-P OpenCL buffer transfers
        if ( scorep_opencl_record_memcpy )
        {
            // do some unification preparation before destroying these information
            opencl_create_comm_group();

            // destroy RMA windows for queues
            scorep_opencl_queue* queue = cl_queue_list;
            while ( queue != NULL )
            {
                /* destroy window on every location, where it is used */
                if ( SCOREP_OPENCL_NO_ID != queue->device_location_id )
                {
                    SCOREP_Location_RmaWinDestroy( queue->device_location,
                                                   SCOREP_GetClockTicks(),
                                                   scorep_opencl_interim_window_handle );
                }

                queue = queue->next;
            }

            cl_queue_list = NULL;

            // destroy RMA window for host locations
            scorep_opencl_location* location = location_list;
            while ( location != NULL )
            {
                SCOREP_Location_RmaWinDestroy( location->location,
                                               SCOREP_GetClockTicks(),
                                               scorep_opencl_interim_window_handle );

                location = location->next;
            }

            location_list = NULL;
        }

        opencl_wrap_finalized = true;

        SCOREP_MutexDestroy( &opencl_mutex );
    }
}


/**
 * Create a Score-P OpenCL command queue.
 *
 * @param clQueue           OpenCL command queue
 * @param clDeviceID        OpenCL device ID
 *
 * @return pointer to created Score-P OpenCL command queue
 */
scorep_opencl_queue*
scorep_opencl_queue_create( cl_command_queue clQueue,
                            cl_device_id     clDeviceID )
{
    scorep_opencl_queue* queue = NULL;

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPENCL, "[OpenCL] Create command queue %p",
                        clQueue );

    queue = ( scorep_opencl_queue* )SCOREP_Memory_AllocForMisc(
        sizeof( scorep_opencl_queue ) );

    queue->queue         = clQueue;
    queue->host_location = SCOREP_Location_GetCurrentCPULocation();

    /* create Score-P location with name and parent id */
    char thread_name[ 64 ];

    SCOREP_OPENCL_CALL( clGetDeviceInfo, ( clDeviceID, CL_DEVICE_NAME,
                                           sizeof( thread_name ),
                                           thread_name, NULL ) );

    queue->device_location = SCOREP_Location_CreateNonCPULocation(
        queue->host_location,
        SCOREP_LOCATION_TYPE_GPU, thread_name );

    SCOREP_OPENCL_CALL( clRetainCommandQueue, ( clQueue ) );

    /* Get vendor before first call to scorep_opencl_synchronize_event()! */
    cl_platform_id platformID;
    char           vendor[ 32 ];

    SCOREP_OPENCL_CALL( clGetDeviceInfo, ( clDeviceID, CL_DEVICE_PLATFORM,
                                           sizeof( cl_platform_id ),
                                           &platformID, NULL ) );

    SCOREP_OPENCL_CALL( clGetPlatformInfo, ( platformID, CL_PLATFORM_VENDOR,
                                             sizeof( vendor ),
                                             vendor,
                                             NULL ) );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPENCL,
                        "[OpenCL] Vendor '%s' platform ID %d",
                        vendor, platformID );

    if ( strstr( vendor, "Intel" ) )
    {
        queue->vendor = INTEL;
    }
    /* END: get vendor */

    // set initial synchronization point
    add_synchronization_event( queue );

    queue->scorep_last_timestamp = queue->sync.scorep_time;

    queue->device_location_id = SCOREP_OPENCL_NO_ID;

    /* allocate buffer for OpenCL device activities */
    queue->buffer = ( scorep_opencl_buffer_entry* )SCOREP_Memory_AllocForMisc( scorep_opencl_queue_size );

    if ( queue->buffer == NULL )
    {
        UTILS_ERROR( SCOREP_ERROR_UNKNOWN_TYPE,
                     "[OpenCL] malloc of OpenCL queue buffer failed! "
                     "         Change buffer size with SCOPRE_OPENCL_BUFFER!" );
    }

    queue->buf_pos  = queue->buffer;
    queue->buf_last = queue->buffer;

    // append generated queue to global queue list
    SCOREP_OPENCL_LOCK();
    queue->next   = cl_queue_list;
    cl_queue_list = queue;
    SCOREP_OPENCL_UNLOCK();

    return queue;
}

/**
 * Retrieves the Score-P command queue structure for a given
 * OpenCL command queue.
 *
 * @param clQueue           OpenCL command queue
 *
 * @return the Score-P command queue structure
 */
scorep_opencl_queue*
scorep_opencl_queue_get( cl_command_queue clQueue )
{
    scorep_opencl_queue* queue = NULL;

    SCOREP_OPENCL_LOCK();
    queue = cl_queue_list;
    while ( queue != NULL )
    {
        if ( queue->queue == clQueue )
        {
            SCOREP_OPENCL_UNLOCK();
            return queue;
        }
        queue = queue->next;
    }
    SCOREP_OPENCL_UNLOCK();

    return queue;
}

/**
 * Get the OpenCL wrapper internal location ID for a Score-P location.
 *
 * @param hostLocation      Score-P location
 *
 * @return the OpenCL wrapper internal location ID
 */
static uint32_t
opencl_get_cpu_location_id( SCOREP_Location* hostLocation )
{
    scorep_opencl_location* loc_data =
        SCOREP_Location_GetSubsystemData( hostLocation,
                                          scorep_opencl_subsystem_id );

    if ( NULL == loc_data )
    {
        return SCOREP_OPENCL_NO_ID;
    }

    return loc_data->location_id;
}

/**
 * Set the OpenCL wrapper internal location ID for the given Score-P location.
 * (only if it has not been set already)
 *
 * @param hostLocation      Score-P location
 */
static void
opencl_set_cpu_location_id( SCOREP_Location* hostLocation )
{
    uint32_t location_id = opencl_get_cpu_location_id( hostLocation );

    // set location data only if they are not available
    if ( SCOREP_OPENCL_NO_ID == location_id )
    {
        // location has not been found, hence create it
        scorep_opencl_location* loc_data =
            ( scorep_opencl_location* )SCOREP_Memory_AllocForMisc(
                sizeof( scorep_opencl_location ) );

        loc_data->location = hostLocation;

        SCOREP_OPENCL_LOCK();

        loc_data->location_id = scorep_opencl_global_location_number++;

        // prepend to communicating location list
        loc_data->next = location_list;
        location_list  = loc_data;

        SCOREP_OPENCL_UNLOCK();

        // create RMA window on host location
        uint64_t time = SCOREP_GetClockTicks();
        SCOREP_Location_RmaWinCreate( hostLocation, time,
                                      scorep_opencl_interim_window_handle );

        SCOREP_Location_SetLastTimestamp( hostLocation, time );

        SCOREP_Location_SetSubsystemData( hostLocation, scorep_opencl_subsystem_id, loc_data );
    }
}

/**
 * Guarantee that the entry can be stored in the queue's buffer. Queue might be
 * flushed, if the buffer limit was exceeded.
 *
 * @param queue Score-P OpenCL command queue
 */
static inline void
guarantee_buffer( scorep_opencl_queue* queue )
{
    /* check if there is enough buffer space for this kernel */
    if ( queue->buf_pos + 1 > queue->buffer + queue_max_buffer_entries )
    {
        UTILS_WARNING( "[OpenCL] Buffer limit exceeded! Flushing queue %p ...",
                       queue->queue );
        scorep_opencl_queue_flush( queue );
    }
}

/**
 * Adds a kernel to the given command queue and initializes the internal
 * kernel structure. Kernel must not yet be enqueued!
 *
 * @param clQueue           OpenCL command queue
 * @param clKernel          OpenCL kernel
 *
 * @return the Score-P kernel structure
 */
scorep_opencl_buffer_entry*
scorep_opencl_enqueue_kernel( cl_command_queue clQueue,
                              cl_kernel        clKernel )
{
    scorep_opencl_queue*        queue  = scorep_opencl_queue_get( clQueue );
    scorep_opencl_buffer_entry* kernel = NULL;

    if ( queue == NULL )
    {
        UTILS_WARNING( "[OpenCL] Add kernel failed!" );
    }

    guarantee_buffer( queue );

    /* create Score-P OpenCL kernel call and initialize it */
    kernel                 = ( scorep_opencl_buffer_entry* )queue->buf_pos;
    kernel->type           = SCOREP_OPENCL_BUF_ENTRY_KERNEL;
    kernel->retained_event = false;
    kernel->u.kernel       = clKernel;

    /* update buffer status */
    queue->buf_last = queue->buf_pos;
    queue->buf_pos++;

    if ( clKernel != NULL )
    {
        SCOREP_OPENCL_CALL( clRetainKernel, ( clKernel ) );
    }

    return kernel;
}

/**
 * Add memory copy to buffer of non-blocking device activities.
 *
 * @param kind          kind/direction of memory copy
 * @param count         number of bytes for this data transfer
 * @param clQueue       pointer to the OpenCL command queue
 *
 * @return pointer to the Score-P memory copy structure
 */
scorep_opencl_buffer_entry*
scorep_opencl_enqueue_buffer( scorep_enqueue_buffer_kind kind,
                              size_t                     count,
                              cl_command_queue           clQueue )
{
    scorep_opencl_buffer_entry* mcpy  = NULL;
    scorep_opencl_queue*        queue = scorep_opencl_queue_get( clQueue );

    guarantee_buffer( queue );

    /* create and initialize asynchronous memory copy entry */
    mcpy                 = ( scorep_opencl_buffer_entry* )queue->buf_pos;
    mcpy->type           = SCOREP_OPENCL_BUF_ENTRY_MEMCPY;
    mcpy->event          = NULL;
    mcpy->retained_event = false;
    mcpy->u.memcpy.bytes = count;
    mcpy->u.memcpy.kind  = kind;

    /* increment buffer of asynchronous calls */
    queue->buf_last = queue->buf_pos;
    queue->buf_pos++;

    // the following might also be executed in scorep_opencl_queue_flush()

    // set host location ID and create RMA window
    if ( kind != SCOREP_ENQUEUE_BUFFER_DEV2DEV )
    {
        opencl_set_cpu_location_id( queue->host_location );
    }

    // set queue location ID and create RMA window, if not already done
    if ( SCOREP_OPENCL_NO_ID == queue->device_location_id )
    {
        SCOREP_OPENCL_LOCK();
        // set location counter and create RMA window
        queue->device_location_id = scorep_opencl_global_location_number++;
        SCOREP_OPENCL_UNLOCK();

        // create window on every location, where it is used
        // use the last written time stamp, as it is not important when exactly
        // the window has been created
        SCOREP_Location_RmaWinCreate( queue->device_location,
                                      queue->scorep_last_timestamp,
                                      scorep_opencl_interim_window_handle );
    }

    return mcpy;
}

/**
 * Adds a new synchronization event to the given command queue and waits for it.
 * (Generates a synchronization point.)
 *
 * @param queue Score-P OpenCL command queue
 *
 * @return true, if the synchronization was successful (otherwise false)
 */
static bool
add_synchronization_event( scorep_opencl_queue* queue )
{
    cl_int   ret         = CL_SUCCESS;
    cl_event tmpEvt[ 2 ] = { NULL, NULL };

    OPENCL_CALL( clEnqueueMarker, ( queue->queue, tmpEvt ) );
    ret                     = OPENCL_CALL( clWaitForEvents, ( 1, tmpEvt ) );
    queue->sync.scorep_time = SCOREP_GetClockTicks();

    /*
     * For Intel we need submit time so first wait only makes sure
     * we are at the end of the queue
     */
    if ( queue->vendor == INTEL && ret == CL_SUCCESS )
    {
        OPENCL_CALL( clEnqueueMarker, ( queue->queue, tmpEvt ) );
        ret                     = OPENCL_CALL( clWaitForEvents, ( 1, tmpEvt ) );
        queue->sync.scorep_time = SCOREP_GetClockTicks();
    }

    if ( ret == CL_INVALID_EVENT )
    {
        /* OpenCL context might have been released */
        return false;
    }

    SCOREP_OPENCL_CHECK( ret );

    /* use submit time for Intel */
    cl_profiling_info info = ( queue->vendor == INTEL ) ?
                             CL_PROFILING_COMMAND_SUBMIT : CL_PROFILING_COMMAND_END;
    SCOREP_OPENCL_CALL( clGetEventProfilingInfo, ( tmpEvt[ 0 ], info,
                                                   sizeof( cl_ulong ),
                                                   &( queue->sync.cl_time ), NULL ) );

    return true;
}

/**
 * Synchronizes host and OpenCL device to get a synchronous point in time.
 * Uses the last activity in the internal buffer if possible, otherwise it calls
 * add_synchronization_event(queue) to create the synchronization point.
 *
 * @param queue             Score-P OpenCL command queue
 *
 * @return true on success, false on failure
 */
static bool
set_synchronization_point( scorep_opencl_queue* queue )
{
    /* TODO: NVIDIA bug */
    cl_event tmpEvt[ 2 ] = { NULL, NULL };
    cl_int   ret         = CL_SUCCESS;

    scorep_opencl_buffer_entry* lastEntry = queue->buf_last;
    cl_int                      evtState  = 0;


    if ( lastEntry != NULL && lastEntry->event != NULL )
    {
        SCOREP_OPENCL_CALL( clGetEventInfo, ( lastEntry->event,
                                              CL_EVENT_COMMAND_EXECUTION_STATUS,
                                              sizeof( cl_int ), &evtState, NULL ) );
    }
    else
    {
        UTILS_WARNING( "[OpenCL] Synchronization failed for queue %p (last entry %p). "
                       "If last entry is not NULL, then the corresponding clEvent is NULL.",
                       queue->queue, queue->buf_last );

        if ( ( queue->buf_last + 1 ) >= ( queue->buffer + queue_max_buffer_entries ) )
        {
            UTILS_WARNING( "[OpenCL] Buffer out of memory access!" );
        }

        return false;
    }

    /* check whether last command queue activity is already finished or if it can
       be used for synchronization */
    if ( evtState != 0 && evtState != CL_COMPLETE ) /* can be used for synchronization */
    {
        tmpEvt[ 0 ] = lastEntry->event;

        // expose Score-P OpenCL synchronization
        SCOREP_EnterRegion( opencl_sync_region_handle );

        ret                     = OPENCL_CALL( clWaitForEvents, ( 1, &( lastEntry->event ) ) );
        queue->sync.scorep_time = SCOREP_GetClockTicks();

        SCOREP_ExitRegion( opencl_sync_region_handle );

        if ( ret == CL_INVALID_EVENT )
        {
            /* OpenCL context might have been released */
            return false;
        }
        SCOREP_OPENCL_CHECK( ret );
        SCOREP_OPENCL_CALL( clGetEventProfilingInfo, ( tmpEvt[ 0 ], CL_PROFILING_COMMAND_END,
                                                       sizeof( cl_ulong ),
                                                       &( queue->sync.cl_time ), NULL ) );
    }
    else
    {
        /* add additional event */
        return add_synchronization_event( queue );
    }

    return true;
}


/**
 * Write OpenCL activities to Score-P OpenCL locations
 *
 * @param queue             Score-P OpenCL command queue to be flushed
 *
 * @return true on success, false on failure (mostly during synchronize due to OpenCL context already released)
 */
bool
scorep_opencl_queue_flush( scorep_opencl_queue* queue )
{
    scorep_opencl_buffer_entry* buf_entry = NULL;
    uint64_t                    host_sync_stop;
    scorep_opencl_sync          sync_start;
    scorep_opencl_sync*         sync_stop = NULL;
    cl_ulong                    device_sync_stop;
    double                      factor_x;

    /* initialize variables depending on Score-P command queue or return */
    if ( queue == NULL )
    {
        return true;
    }

    buf_entry = queue->buffer;

    /* check if buffer entries available */
    if ( queue->buf_pos == buf_entry )
    {
        return true;
    }

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_OPENCL, "[OpenCL] Flush command queue %p",
                        queue->queue );

    sync_start = queue->sync;

    if ( !set_synchronization_point( queue ) )
    {
        UTILS_WARNING( "[OpenCL] Skip current buffer content." );

        /* reset buffer position */
        queue->buf_pos  = queue->buffer;
        queue->buf_last = queue->buffer;

        return false;
    }

    sync_stop = &( queue->sync );

    host_sync_stop   = sync_stop->scorep_time;
    device_sync_stop = sync_stop->cl_time;

    SCOREP_EnterRegion( opencl_flush_region_handle );

    // get time synchronization factor
    uint64_t device_diff = ( uint64_t )( device_sync_stop - sync_start.cl_time );
    uint64_t host_diff   = host_sync_stop - sync_start.scorep_time;

    if ( device_diff == 0 )
    {
        UTILS_WARNING( "[OpenCL] Did not flush events as device_diff == 0" );
        return false;
    }
    if ( host_diff == 0 )
    {
        UTILS_WARNING( "[OpenCL] Did not flush events as host_diff == 0" );
        return false;
    }

    factor_x = ( double )host_diff / ( double )device_diff;

    // get host location ID, if memory copies are enabled
    uint32_t host_location_id = SCOREP_OPENCL_NO_ID;
    if ( scorep_opencl_record_memcpy )
    {
        host_location_id = opencl_get_cpu_location_id( queue->host_location );
    }

    /* write events for all recorded asynchronous calls */
    while ( buf_entry < queue->buf_pos )
    {
        uint64_t host_start, host_stop;

        /* get profiling information (start and stop time) */
        cl_ulong device_start, device_stop;

        /* get start and stop time of entry */
        SCOREP_OPENCL_CALL( clGetEventProfilingInfo, ( buf_entry->event,
                                                       CL_PROFILING_COMMAND_START, sizeof( cl_ulong ),
                                                       &device_start, NULL ) );
        SCOREP_OPENCL_CALL( clGetEventProfilingInfo, ( buf_entry->event,
                                                       CL_PROFILING_COMMAND_END, sizeof( cl_ulong ),
                                                       &device_stop, NULL ) );

        /* convert from OpenCL to Score-P time (in order queue only!!!) */
        host_start = sync_start.scorep_time +
                     ( uint64_t )( ( double )( device_start - sync_start.cl_time ) * factor_x );

        host_stop = host_start +
                    ( uint64_t )( ( double )( device_stop - device_start ) * factor_x );

        ////////////////////////////////////////////////////////////////////
        // check for ascending timestamp order and time conversion issues //

        // if current activity's start time is before last written timestamp
        if ( host_start < queue->scorep_last_timestamp )
        {
            UTILS_WARN_ONCE( "[OpenCL] Activity start time (%" PRIu64 ") "
                             "< (%" PRIu64 ") last written timestamp!",
                             host_start, queue->scorep_last_timestamp );

            //TODO: provide more debug info like kernel, device, queue

            if ( queue->scorep_last_timestamp < host_stop )
            {
                UTILS_WARN_ONCE( "[OpenCL] Set activity start time to sync-point time"
                                 " (truncate %.4lf%%)",
                                 ( double )( queue->scorep_last_timestamp - host_start ) / ( double )( host_stop - host_start ) );
                host_start = queue->scorep_last_timestamp;
            }
            else
            {
                UTILS_WARN_ONCE( "[OpenCL] Skip recording of activity ..." );

                buf_entry++;
                continue;
            }
        }

        // check if time between start and stop is increasing
        if ( host_stop < host_start )
        {
            UTILS_WARN_ONCE( "[OpenCL] Activity start time > stop time!" );

            //TODO: provide more debug info like kernel, device, queue
            buf_entry++;
        }

        // check if synchronization stop time is before device activity stop time
        if ( host_sync_stop < host_stop )
        {
            UTILS_WARN_ONCE( "[OpenCL] sync-point time "
                             "(%" PRIu64 ") < (%" PRIu64 ") activity stop time",
                             host_sync_stop, host_stop );

            //TODO: provide more debug info like kernel, device, queue

            // Record activity with host_sync_stop stop time stamp, if possible
            if ( host_sync_stop > host_start )
            {
                UTILS_WARN_ONCE( "[OpenCL] Set activity stop time to sync-point time "
                                 "(truncate %.4lf%%)",
                                 ( double )( host_stop - host_sync_stop ) / ( double )( host_stop - host_start ) );

                host_stop = host_sync_stop;
            }
            else
            {
                UTILS_WARN_ONCE( "[OpenCL] Skip recording of activity ..." );

                buf_entry++;
                continue;
            }
        }
        ////////////////////////////////////////////////////////////////////

        // save last time stamp that is written on the queue location
        // (in order queue only!!!)
        queue->scorep_last_timestamp = host_stop;

        if ( buf_entry->type == SCOREP_OPENCL_BUF_ENTRY_KERNEL )
        {
            cl_kernel                       kernel = buf_entry->u.kernel;
            char                            kernel_name[ KERNEL_STRING_LENGTH ];
            scorep_opencl_kernel_hash_node* hashNode = NULL;

            // get kernel name
            if ( kernel )
            {
                // add $ to kernel_name
                kernel_name[ 0 ] = '$';
                SCOREP_OPENCL_CALL( clGetKernelInfo, ( kernel,
                                                       CL_KERNEL_FUNCTION_NAME,
                                                       KERNEL_STRING_LENGTH - 1,
                                                       kernel_name + 1, NULL ) );
            }
            else
            {
                strncpy( kernel_name, "$nativeUserKernel", 17 );
            }

            SCOREP_RegionHandle regionHandle = SCOREP_INVALID_REGION;

            hashNode = kernel_hash_get( kernel_name );
            if ( hashNode )
            {
                regionHandle = hashNode->region;
            }
            else
            {
                // get region ID for current kernel
                regionHandle =
                    SCOREP_Definitions_NewRegion( kernel_name, NULL,
                                                  opencl_kernel_file_handle, 0, 0,
                                                  SCOREP_PARADIGM_OPENCL,
                                                  SCOREP_REGION_FUNCTION );

                hashNode = kernel_hash_put( kernel_name, regionHandle );

                if ( NULL == hashNode )
                {
                    UTILS_WARNING( "[OpenCL] Could not generate hash node for kernel '%s'.", kernel_name );
                }
            }

            // write Score-P events for this kernel
            // TODO: out of order queues!!!
            SCOREP_Location_EnterRegion( queue->device_location, host_start, regionHandle );
            SCOREP_Location_ExitRegion( queue->device_location, host_stop, regionHandle );

            // release kernel that has been retained by this wrapper
            if ( kernel )
            {
                SCOREP_OPENCL_CALL( clReleaseKernel, ( kernel ) );
            }
        }
        else if ( buf_entry->type == SCOREP_OPENCL_BUF_ENTRY_MEMCPY )
        {
            // write communication

            scorep_enqueue_buffer_kind mcpy_kind  = buf_entry->u.memcpy.kind;
            size_t                     mcpy_bytes = buf_entry->u.memcpy.bytes;

            if ( mcpy_kind == SCOREP_ENQUEUE_BUFFER_HOST2DEV )
            {
                SCOREP_Location_RmaGet( queue->device_location, host_start,
                                        scorep_opencl_interim_window_handle,
                                        host_location_id, mcpy_bytes, 42 );
            }
            else if ( mcpy_kind == SCOREP_ENQUEUE_BUFFER_DEV2HOST )
            {
                SCOREP_Location_RmaPut( queue->device_location, host_start,
                                        scorep_opencl_interim_window_handle,
                                        host_location_id, mcpy_bytes, 42 );
            }
            else if ( mcpy_kind == SCOREP_ENQUEUE_BUFFER_DEV2DEV )
            {
                SCOREP_Location_RmaGet( queue->device_location, host_start,
                                        scorep_opencl_interim_window_handle,
                                        queue->device_location_id, mcpy_bytes,
                                        42 );
            }

            if ( mcpy_kind != SCOREP_ENQUEUE_BUFFER_HOST2HOST )
            {
                SCOREP_Location_RmaOpCompleteBlocking(
                    queue->device_location, host_stop,
                    scorep_opencl_interim_window_handle, 42 );
            }
        }
        else
        {
            UTILS_WARNING( "[OpenCL] Unknown buffer entry type found!" );
        }

        // release event that has been retained by this wrapper
        if ( buf_entry->retained_event == true && buf_entry->event )
        {
            SCOREP_OPENCL_CALL( clReleaseEvent, ( buf_entry->event ) );
        }

        // go to next entry in buffer
        buf_entry++;
    }

    // reset buffer pointers
    queue->buf_pos  = queue->buffer;
    queue->buf_last = queue->buffer;

    SCOREP_ExitRegion( opencl_flush_region_handle );

    return true;
}

/**
 * Flush all listed Score-P OpenCL queues.
 */
void
scorep_opencl_flush_all()
{
    if ( scorep_opencl_record_memcpy || scorep_opencl_record_kernels )
    {
        SCOREP_OPENCL_LOCK();

        scorep_opencl_queue* queue = cl_queue_list;
        while ( queue != NULL )
        {
            scorep_opencl_queue_flush( queue );
            queue = queue->next;
        }

        SCOREP_OPENCL_UNLOCK();
    }
}

/**
 * Returns the OpenCL error string for the given error code
 *
 * @param error         the error code
 *
 * @return the error string
 */
const char*
scorep_opencl_get_error_string( cl_int error )
{
    switch ( ( int )error )
    {
        case CL_DEVICE_NOT_FOUND:
            return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE:
            return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES:
            return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY:
            return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP:
            return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH:
            return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE:
            return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE:
            return "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_INVALID_VALUE:
            return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE:
            return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM:
            return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE:
            return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT:
            return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES:
            return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE:
            return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR:
            return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT:
            return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE:
            return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER:
            return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY:
            return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS:
            return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM:
            return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME:
            return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION:
            return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL:
            return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX:
            return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE:
            return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE:
            return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS:
            return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION:
            return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE:
            return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE:
            return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET:
            return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST:
            return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT:
            return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION:
            return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT:
            return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE:
            return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL:
            return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY:
            return "CL_INVALID_PROPERTY";
        default:
            return "unknown error code";
    }

    return "unknown error code";
}

/** @brief
 * Puts a string into the hash table
 *
 * @param name              Pointer to a string to be stored in the hash table.
 * @param region            Region handle.
 *
 * @return Return pointer to the created hash node.
 */
static void*
kernel_hash_put( const char*         name,
                 SCOREP_RegionHandle region )
{
    if ( NULL == name )
    {
        return NULL;
    }

    size_t len = strlen( name );

    scorep_opencl_kernel_hash_node* add =
        ( scorep_opencl_kernel_hash_node* )SCOREP_Memory_AllocForMisc(
            sizeof( scorep_opencl_kernel_hash_node ) + ( len + 1 ) * sizeof( char ) );

    memcpy( add->name, name, len );
    ( add->name )[ len ] = '\0';

    add->region = region;

    uint32_t hash_value = jenkins_hash( name, len, 0 );
    uint32_t id         = hash_value & ( KERNEL_HASHTABLE_SIZE - 1 );

    add->hash = hash_value;
    add->next = opencl_kernel_hashtab[ id ];

    opencl_kernel_hashtab[ id ] = add;

    return add;
}

/** @brief
 * Get a string from the hash table
 *
 * @param name              Pointer to a string to be retrieved from the hash table.
 *
 * @return Return pointer to the retrieved hash node.
 */
static inline void*
kernel_hash_get( const char* name )
{
    if ( NULL == name )
    {
        return NULL;
    }

    // get hash and corresponding table index of the given string
    uint32_t hash = jenkins_hash( name, strlen( name ), 0 );
    uint32_t id   = hash & ( KERNEL_HASHTABLE_SIZE - 1 );

    // get the bucket
    scorep_opencl_kernel_hash_node* curr = opencl_kernel_hashtab[ id ];

    // iterate over bucket
    while ( curr )
    {
        // check if bucket entry has the same hash as the given string
        // hash match does not ensure that strings are equal, therefore compare strings
        if ( ( curr->hash == hash ) && ( strcmp( curr->name, name ) == 0 ) )
        {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

/**
 * Collect all Score-P locations, which are involved in OpenCL communication and
 * store their global location ID into an the Score-P OpenCL global location ID
 * array.
 */
static void
opencl_create_comm_group()
{
    if ( scorep_opencl_global_location_number == 0 )
    {
        return;
    }

    // allocate the OpenCL communication group array
    scorep_opencl_global_location_ids = ( uint64_t* )malloc(
        scorep_opencl_global_location_number * sizeof( uint64_t ) );

    /* Add all queue and host locations that are involved in OpenCL
       communication. Ensure the array boundary (count). */

    size_t count = 0;

    // add locations for queues that are involved in communication
    scorep_opencl_queue* queue = cl_queue_list;
    while ( queue != NULL )
    {
        if ( count < scorep_opencl_global_location_number )
        {
            if ( SCOREP_OPENCL_NO_ID != queue->device_location_id )
            {
                scorep_opencl_global_location_ids[ queue->device_location_id ] =
                    SCOREP_Location_GetGlobalId( queue->device_location );

                count++;
            }
        }
        else
        {
            UTILS_WARNING( "[OpenCL] Error in creating communication group! "
                           "Communication information might be missing." );

            return;
        }

        queue = queue->next;
    }

    // add host locations
    scorep_opencl_location* location = location_list;
    while ( location != NULL )
    {
        if ( count < scorep_opencl_global_location_number )
        {
            scorep_opencl_global_location_ids[ location->location_id ] =
                SCOREP_Location_GetGlobalId( location->location );

            count++;
        }
        else
        {
            UTILS_WARNING( "[OpenCL] Error in creating communication group! "
                           "Communication information might be missing." );

            return;
        }

        location = location->next;
    }
}
