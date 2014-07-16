/**
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 * Implementation of all __wrap_* functions used by the pthread library wrapper
 */

#include <config.h>

#include "scorep_pthread.h"
#include "scorep_pthread_event_functions.h"
#include "scorep_pthread_mutex.h"

#include <SCOREP_Events.h>
#include <SCOREP_Types.h>
#include <SCOREP_ThreadCreateWait_Event.h>
#include <SCOREP_RuntimeManagement.h>

#define SCOREP_DEBUG_MODULE_NAME PTHREAD
#include <UTILS_Debug.h>

#include <UTILS_Error.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>

/* *INDENT-OFF* */
static void* wrapped_start_routine( void* wrappedArg );
static void once_init( void );
static void cleanup_handler( void* wrappedArg );
/* *INDENT-ON* */

static pthread_key_t tpd_retval_key;
static pthread_once_t once_init_control = PTHREAD_ONCE_INIT;

typedef struct scorep_pthread_wrapped_arg scorep_pthread_wrapped_arg;
struct scorep_pthread_wrapped_arg
{
    void*                              ( *orig_start_routine )( void* );
    void*                              orig_arg;
    struct scorep_thread_private_data* parent_tpd;
    uint32_t                           sequence_count;
};


int
__wrap_pthread_create( pthread_t*            thread,
                       const pthread_attr_t* attr,
                       void* ( *start_routine )( void* ),
                       void*                 arg )
{
    UTILS_DEBUG_ENTRY();

    if ( attr )
    {
        int detach_state;
        int result = pthread_attr_getdetachstate( attr, &detach_state );

        if ( detach_state == PTHREAD_CREATE_DETACHED )
        {
            UTILS_WARN_ONCE( "The current thread is in detached state. "
                             "The usage of pthread_detach is considered dangerous in the "
                             "context of Score-P. If the detached thread is still running "
                             "at the end of main, the measurement will fail." );
        }
    }

    if ( !scorep_pthread_initialized )
    {
        if ( scorep_pthread_finalized )
        {
            return __real_pthread_create( thread,
                                          attr,
                                          start_routine,
                                          arg );
        }
        SCOREP_InitMeasurement();
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_CREATE ] );
    scorep_pthread_wrapped_arg* wrapped_arg = malloc( sizeof( scorep_pthread_wrapped_arg ) );
    UTILS_BUG_ON( !wrapped_arg, "Cannot allocate memory for wrapping argument to "
                  " pthread_create." );

    wrapped_arg->orig_start_routine = start_routine;
    wrapped_arg->orig_arg           = arg;
    SCOREP_ThreadCreateWait_Create( SCOREP_PARADIGM_PTHREAD,
                                    &( wrapped_arg->parent_tpd ),
                                    &( wrapped_arg->sequence_count ) );

    /* wrap __real_pthread_create by SCOREP_Enter|Exit? Could be used to calculate
     * Pthread creation overhead. POMP2 does not have SCOREP_Enter|Exit for fork
     * and join.
     */
    int result = __real_pthread_create( thread,
                                        attr,
                                        &wrapped_start_routine,
                                        ( void* )wrapped_arg );
    UTILS_BUG_ON( result != 0 );
    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_CREATE ] );

    UTILS_DEBUG_EXIT();
    return result;
}


typedef struct wrapped_return_value wrapped_return_value;
struct wrapped_return_value
{
    void*    orig_ret_val;
    uint32_t sequence_count;
};


static void*
wrapped_start_routine( void* wrappedArg )
{
    UTILS_DEBUG_ENTRY();

    pthread_once( &once_init_control, once_init );

    scorep_pthread_wrapped_arg* wrapped_arg = wrappedArg;
    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_PTHREAD, "SequenceCount:%" PRIu32 "",
                        wrapped_arg->sequence_count );

    SCOREP_ThreadCreateWait_Begin( SCOREP_PARADIGM_PTHREAD,
                                   wrapped_arg->parent_tpd,
                                   wrapped_arg->sequence_count );

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_START_ROUTINE ] );

    wrapped_return_value* wrapped_ret_val = calloc( 1, sizeof( wrapped_return_value ) );
    UTILS_BUG_ON( !wrapped_ret_val, "Cannot allocate memory for wrapping return "
                  "value of Pthread start_routine." );
    wrapped_ret_val->sequence_count = wrapped_arg->sequence_count;

    int status = pthread_setspecific( tpd_retval_key, wrapped_ret_val );
    UTILS_BUG_ON( status != 0, "Failed to store Pthread thread specific data." );

    int execute = 1;
    pthread_cleanup_push( cleanup_handler, wrappedArg );

    // Call original start_routine. It might call pthread_exit, i.e. we might
    // not return from here.
    wrapped_ret_val->orig_ret_val = wrapped_arg->orig_start_routine( wrapped_arg->orig_arg );

    pthread_cleanup_pop( execute );

    UTILS_DEBUG_EXIT();
    return wrapped_ret_val;
}


static void
once_init( void )
{
    int status = pthread_key_create( &tpd_retval_key, NULL );
    UTILS_BUG_ON( status != 0, "Failed to allocate a new pthread_key_t." );

    status = pthread_setspecific( tpd_retval_key, NULL );
    UTILS_BUG_ON( status != 0, "Failed to store Pthread thread specific data." );
}


static void
cleanup_handler( void* wrappedArg )
{
    UTILS_DEBUG_ENTRY();

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_START_ROUTINE ] );

    scorep_pthread_wrapped_arg* wrapped_arg = wrappedArg;
    SCOREP_ThreadCreateWait_End( SCOREP_PARADIGM_PTHREAD,
                                 wrapped_arg->parent_tpd,
                                 wrapped_arg->sequence_count );
    free( wrappedArg );

    UTILS_DEBUG_EXIT();
}


int
__wrap_pthread_join( pthread_t thread,
                     void**    valuePtr )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_join( thread, valuePtr );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_JOIN ] );

    void* result;
    int   status = __real_pthread_join( thread, &result );
    UTILS_BUG_ON( status != 0, "__real_pthread_join failed." );

    if ( result == PTHREAD_CANCELED )
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_PTHREAD, "Thread was cancelled." );
        // Can we provide sequence count to pthread_exit?
        // Do we want to see
        // SCOREP_ThreadCreateWait_Wait( SCOREP_PARADIGM_PTHREAD, sequence_count );???
    }
    else
    {
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_PTHREAD, "result :%p", result );
        if ( valuePtr )
        {
            *valuePtr = ( ( wrapped_return_value* )result )->orig_ret_val;
        }
        uint32_t sequence_count = ( ( wrapped_return_value* )result )->sequence_count;
        UTILS_DEBUG_PRINTF( SCOREP_DEBUG_PTHREAD, "sequence_count :%" PRIu32 "",
                            sequence_count );
        free( result );

        SCOREP_ThreadCreateWait_Wait( SCOREP_PARADIGM_PTHREAD, sequence_count );
    }

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_JOIN ] );

    UTILS_DEBUG_EXIT();
    return status;
}


void
__wrap_pthread_exit( void* valuePtr )
{
    UTILS_DEBUG_ENTRY();

    UTILS_FATAL( "The usage of pthread_exit is not supported by this version of "
                 "Score-P as it will produce inconsistent profiles and traces. "
                 "We will support this feature in a subsequent release." );

    if ( scorep_pthread_finalized )
    {
        __real_pthread_exit( valuePtr );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_EXIT ] );

    // TLS access still possible.
    wrapped_return_value* wrapped_ret_val = pthread_getspecific( tpd_retval_key );
    UTILS_BUG_ON( wrapped_ret_val == 0 );
    UTILS_BUG_ON( wrapped_ret_val->orig_ret_val != 0 );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_PTHREAD, "sequence_count :%" PRIu32 "",
                        wrapped_ret_val->sequence_count );

    wrapped_ret_val->orig_ret_val = valuePtr;

    // Exit events needs to happen earlier than __real_pthread_exit as this
    // function does not return.
    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_EXIT ] );
    UTILS_DEBUG_EXIT();
    __real_pthread_exit( wrapped_ret_val );
}


int
__wrap_pthread_cancel( pthread_t thread )
{
    UTILS_DEBUG_ENTRY();

    UTILS_FATAL( "The usage of pthread_cancel is not supported by this version of "
                 "Score-P as it will produce inconsistent profiles and traces. "
                 "We will support this feature in a subsequent release." );

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_cancel( thread );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_CANCEL ] );

    int result = __real_pthread_cancel( thread );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_CANCEL ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_detach( pthread_t thread )
{
    UTILS_DEBUG_ENTRY();

    UTILS_WARN_ONCE( "The usage of pthread_detach is considered dangerous in the "
                     "context of Score-P. If the detached thread is still running "
                     "at the end of main, the measurement will fail." );

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_detach( thread );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_DETACH ] );

    // If detached thread still runs at finalization time, finalization will
    // fail because it requires serial execution.
    int result = __real_pthread_detach( thread );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_DETACH ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_mutex_init( pthread_mutex_t*           pthreadMutex,
                           const pthread_mutexattr_t* pthreadAttr )
{
    UTILS_DEBUG_ENTRY();

    if ( !scorep_pthread_initialized )
    {
        if ( scorep_pthread_finalized )
        {
            return __real_pthread_mutex_init( pthreadMutex, pthreadAttr );
        }
        SCOREP_InitMeasurement();
    }

    // check if the mutex is process shared one
    int process_shared;
    if ( pthreadAttr )
    {
        int result = pthread_mutexattr_getpshared( pthreadAttr, &process_shared );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_INIT ] );

    int result = __real_pthread_mutex_init( pthreadMutex, pthreadAttr );

    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
    if ( !scorep_mutex )
    {
        scorep_mutex = scorep_pthread_mutex_hash_put( pthreadMutex );
        UTILS_BUG_ON( scorep_mutex == 0 );
        if ( process_shared == PTHREAD_PROCESS_SHARED )
        {
            UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                             "which is currently not supported. "
                             "No trace event will be recorded." );
            scorep_mutex->process_shared = true;
        }
    }
    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_INIT ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_mutex_destroy( pthread_mutex_t* pthreadMutex )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_mutex_destroy( pthreadMutex );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_DESTROY ] );
    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
    scorep_pthread_mutex_hash_remove( pthreadMutex );

    int result = __real_pthread_mutex_destroy( pthreadMutex );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_DESTROY ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_mutex_lock( pthread_mutex_t* pthreadMutex )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_mutex_lock( pthreadMutex );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_LOCK ] );

    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
    if ( !scorep_mutex  )
    {
        /* Mutex initialized statically via PTHREAD_MUTEX_INITIALIZER and friends. */
        scorep_mutex = scorep_pthread_mutex_hash_put( pthreadMutex );
        UTILS_BUG_ON( scorep_mutex == 0 );
    }

    int result = __real_pthread_mutex_lock( pthreadMutex );

    if ( scorep_mutex->process_shared == false )
    {
        /* Inside the lock, save to increase acquisition_order. */
        scorep_mutex->acquisition_order++;
        SCOREP_ThreadAcquireLock( SCOREP_PARADIGM_PTHREAD,
                                  scorep_mutex->id,
                                  scorep_mutex->acquisition_order );
    }
    else
    {
        UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                         "which is currently not supported. "
                         "No trace event will be recorded." );
    }

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_LOCK ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_mutex_unlock( pthread_mutex_t* pthreadMutex )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_mutex_unlock( pthreadMutex );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_UNLOCK ] );

    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
    UTILS_BUG_ON( scorep_mutex == 0 );

    if ( scorep_mutex->process_shared == false )
    {
        SCOREP_ThreadReleaseLock( SCOREP_PARADIGM_PTHREAD,
                                  scorep_mutex->id,
                                  scorep_mutex->acquisition_order );
    }
    else
    {
        UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                         "which is currently not supported. "
                         "No trace event will be recorded." );
    }

    int result = __real_pthread_mutex_unlock( pthreadMutex );
    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_UNLOCK ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_mutex_trylock( pthread_mutex_t* pthreadMutex )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_mutex_trylock( pthreadMutex );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_TRYLOCK ] );

    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );

    int result = __real_pthread_mutex_trylock( pthreadMutex );
    if ( result != EBUSY )
    {
        scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
        if ( !scorep_mutex  )
        {
            /* Mutex initialized statically via PTHREAD_MUTEX_INITIALIZER and friends. */
            scorep_mutex = scorep_pthread_mutex_hash_put( pthreadMutex );
            UTILS_BUG_ON( scorep_mutex == 0 );
        }

        if ( scorep_mutex->process_shared == false )
        {
            /* Inside the lock, save to increase acquisition_order. */
            scorep_mutex->acquisition_order++;
            SCOREP_ThreadAcquireLock( SCOREP_PARADIGM_PTHREAD,
                                      scorep_mutex->id,
                                      scorep_mutex->acquisition_order );
        }
        else
        {
            UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                             "which is currently not supported. "
                             "No trace event will be recorded." );
        }
    }

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_MUTEX_TRYLOCK ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_cond_init( pthread_cond_t* cond, pthread_condattr_t* attr )
{
    UTILS_DEBUG_ENTRY();

    if ( !scorep_pthread_initialized )
    {
        if ( scorep_pthread_finalized )
        {
            return __real_pthread_cond_init( cond, attr );
        }
        SCOREP_InitMeasurement();
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_INIT ] );

    int result = __real_pthread_cond_init( cond, attr );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_INIT ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_cond_signal( pthread_cond_t* cond )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_cond_signal( cond );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_SIGNAL ] );

    int result = __real_pthread_cond_signal( cond );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_SIGNAL ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_cond_broadcast( pthread_cond_t* cond )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_cond_broadcast( cond );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_BROADCAST ] );

    int result = __real_pthread_cond_broadcast( cond );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_BROADCAST ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_cond_wait( pthread_cond_t* cond, pthread_mutex_t* pthreadMutex )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_cond_wait( cond, pthreadMutex );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_WAIT ] );

    // do we want to see enter/exit for implicit pthread_mutex_unlock here?

    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
    UTILS_BUG_ON( scorep_mutex == 0 );

    if ( scorep_mutex->process_shared == false )
    {
        SCOREP_ThreadReleaseLock( SCOREP_PARADIGM_PTHREAD,
                                  scorep_mutex->id,
                                  scorep_mutex->acquisition_order );
    }
    else
    {
        UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                         "which is currently not supported. "
                         "No trace event will be recorded." );
    }

    // Will unlock mutex, wait for cond, then lock mutex again.
    int result = __real_pthread_cond_wait( cond, pthreadMutex );

    if ( scorep_mutex->process_shared == false )
    {
        scorep_mutex->acquisition_order++;
        SCOREP_ThreadAcquireLock( SCOREP_PARADIGM_PTHREAD,
                                  scorep_mutex->id,
                                  scorep_mutex->acquisition_order );
    }
    else
    {
        UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                         "which is currently not supported. "
                         "No trace event will be recorded." );
    }

    // do we want to see enter/exit for implicit pthread_mutex_lock here?

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_WAIT ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_cond_timedwait( pthread_cond_t*        cond,
                               pthread_mutex_t*       pthreadMutex,
                               const struct timespec* time )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_cond_timedwait( cond, pthreadMutex, time );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_TIMEDWAIT ] );

    // do we want to see enter/exit for implicit pthread_mutex_unlock here?

    scorep_pthread_mutex* scorep_mutex = scorep_pthread_mutex_hash_get( pthreadMutex );
    UTILS_BUG_ON( scorep_mutex == 0 );

    if ( scorep_mutex->process_shared == false )
    {
        SCOREP_ThreadReleaseLock( SCOREP_PARADIGM_PTHREAD,
                                  scorep_mutex->id,
                                  scorep_mutex->acquisition_order );
    }
    else
    {
        UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                         "which is currently not supported. "
                         "No trace event will be recorded." );
    }

    // Will unlock mutex, wait for cond until timeout, then lock mutex again.

    int result = __real_pthread_cond_timedwait( cond, pthreadMutex, time );
    if ( scorep_mutex->process_shared == false )
    {
        scorep_mutex->acquisition_order++;
        SCOREP_ThreadAcquireLock( SCOREP_PARADIGM_PTHREAD,
                                  scorep_mutex->id,
                                  scorep_mutex->acquisition_order );
    }
    else
    {
        UTILS_WARN_ONCE( "The current mutex is a process shared mutex "
                         "which is currently not supported. "
                         "No trace event will be recorded." );
    }

    // do we want to see enter/exit for implicit pthread_mutex_lock here?

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_TIMEDWAIT ] );

    UTILS_DEBUG_EXIT();
    return result;
}


int
__wrap_pthread_cond_destroy( pthread_cond_t* cond )
{
    UTILS_DEBUG_ENTRY();

    if ( scorep_pthread_finalized )
    {
        return __real_pthread_cond_destroy( cond );
    }

    SCOREP_EnterRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_DESTROY ] );

    int result = __real_pthread_cond_destroy( cond );

    SCOREP_ExitRegion( scorep_pthread_regions[ SCOREP_PTHREAD_COND_DESTROY ] );

    UTILS_DEBUG_EXIT();
    return result;
}
