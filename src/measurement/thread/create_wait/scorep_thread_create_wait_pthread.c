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

#include <scorep_thread_model_specific.h>
#include <scorep_thread_generic.h>

#include <scorep_location.h>

#define SCOREP_DEBUG_MODULE_NAME PTHREAD
#include <UTILS_Debug.h>

#include <UTILS_Error.h>

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

typedef struct scorep_thread_private_data scorep_thread_private_data;


/* *INDENT-OFF* */
static void create_tpd_key( void );
/* *INDENT-ON*  */


typedef struct private_data_pthread private_data_pthread;
struct private_data_pthread
{
    /* We don't need any members in this implementation. */
#if defined( _CRAYC )
    /* The cray compiler requires structs to be nonempty (and it is
     * right about that). Fool it be defining an zero sized array as
     * member. */
    char nothing[ 0 ];
#endif
#if defined( __SUNPRO_C )
    /* The above does not work for studio. */
    char nothing[ 1 ];
#endif
};


pthread_key_t  tpd_key;
pthread_once_t tpd_key_once = PTHREAD_ONCE_INIT;


struct SCOREP_Location*
SCOREP_Location_GetCurrentCPULocation()
{
    UTILS_DEBUG_ENTRY();
    scorep_thread_private_data* tpd = scorep_thread_get_private_data();
    UTILS_BUG_ON( tpd == 0, "Invalid Pthread thread specific data object." );
    struct SCOREP_Location* location = scorep_thread_get_location( tpd );
    UTILS_BUG_ON( location == 0, "Invalid location object associated with "
                  "Pthread thread specific data object." );
    return location;
}


scorep_thread_private_data*
scorep_thread_get_private_data()
{
    UTILS_DEBUG_ENTRY();
    return pthread_getspecific( tpd_key );
}


void
scorep_thread_on_initialize( scorep_thread_private_data* initialTpd )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( !initialTpd, "Invalid input data initialTpd" );

    int status = pthread_once( &tpd_key_once, create_tpd_key );
    UTILS_BUG_ON( status != 0, "Failed to create pthread_key_t object via "
                  "pthread_once()." );

    status = pthread_setspecific( tpd_key, initialTpd );
    UTILS_BUG_ON( status != 0, "Failed to store Pthread thread specific data." );
}


static void
create_tpd_key( void )
{
    UTILS_DEBUG_ENTRY();
    int status = pthread_key_create( &tpd_key, NULL );
    UTILS_BUG_ON( status != 0, "Failed to allocate a new pthread_key_t." );
}


void
scorep_thread_on_finalize( scorep_thread_private_data* tpd )
{
    UTILS_DEBUG_ENTRY();
    int status = pthread_key_delete( tpd_key );
    UTILS_BUG_ON( status != 0, "Failed to delete a pthread_key_t." );
}


size_t
scorep_thread_get_sizeof_model_data()
{
    UTILS_DEBUG_ENTRY();
    return sizeof( private_data_pthread );
}


void
scorep_thread_on_create_private_data( scorep_thread_private_data* tpd,
                                      void*                       modelData )
{
    UTILS_DEBUG_ENTRY();
}


void
scorep_thread_delete_private_data( scorep_thread_private_data* initialTpd )
{
    UTILS_DEBUG_ENTRY();
    free( initialTpd );
    // others were already deleted in scorep_thread_create_wait_on_end()
}


void
scorep_thread_create_wait_on_create( SCOREP_ParadigmType     paradigm,
                                     void*                   modelData,
                                     struct SCOREP_Location* location )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( paradigm != SCOREP_PARADIGM_PTHREAD, "Passed paradigm != "
                  "SCOREP_PARADIGM_PTHREAD." );
}


void
scorep_thread_create_wait_on_wait( SCOREP_ParadigmType     paradigm,
                                   void*                   modelData,
                                   struct SCOREP_Location* location )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( paradigm != SCOREP_PARADIGM_PTHREAD, "Passed paradigm != "
                  "SCOREP_PARADIGM_PTHREAD."  );
}


void
scorep_thread_create_wait_on_begin( SCOREP_ParadigmType                 paradigm,
                                    struct scorep_thread_private_data*  parentTpd,
                                    uint32_t                            sequenceCount,
                                    struct scorep_thread_private_data** currentTpd,
                                    bool*                               locationIsCreated )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( paradigm != SCOREP_PARADIGM_PTHREAD, "Passed paradigm != "
                  "SCOREP_PARADIGM_PTHREAD."  );

    *currentTpd =  scorep_thread_create_private_data( parentTpd );
    int status = pthread_setspecific( tpd_key, *currentTpd );
    UTILS_BUG_ON( status != 0, "Failed to store Pthread thread specific data." );

    char location_name[ 80 ];
    int  length = snprintf( location_name, 80, "Pthread thread %d", sequenceCount );
    UTILS_ASSERT( length < 80 );

    scorep_thread_set_location( *currentTpd,
                                SCOREP_Location_CreateCPULocation( scorep_thread_get_location( parentTpd ),
                                                                   location_name,
                                                                   /* deferNewLocationNotification = */ true ) );
    *locationIsCreated = true;
}


void
scorep_thread_create_wait_on_end( SCOREP_ParadigmType                paradigm,
                                  struct scorep_thread_private_data* parentTpd,
                                  struct scorep_thread_private_data* currentTpd,
                                  uint32_t                           sequenceCount )
{
    UTILS_DEBUG_ENTRY();
    UTILS_BUG_ON( paradigm != SCOREP_PARADIGM_PTHREAD, "Passed paradigm != "
                  "SCOREP_PARADIGM_PTHREAD."  );

    int status = pthread_setspecific( tpd_key, NULL );
    UTILS_BUG_ON( status != 0, "Failed to reset Phread thread specific data." );

    free( currentTpd );
    // Once a location pool is available, return
    // scorep_thread_get_location( currentTpd ) back to pool.
    UTILS_DEBUG_EXIT();
}


SCOREP_ParadigmType
scorep_thread_get_paradigm( void )
{
    return SCOREP_PARADIGM_PTHREAD;
}
