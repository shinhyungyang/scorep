/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2023,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 */

#ifndef SCOREP_OMPT_H
#define SCOREP_OMPT_H

#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <omp-tools.h>

#define SCOREP_DEBUG_MODULE_NAME OMPT
#include <UTILS_Debug.h>

#include <UTILS_Error.h>
#include <UTILS_Mutex.h>

#include <SCOREP_DefinitionHandles.h>


#define SCOREP_OMPT_INVALID_LOCAL_RANK ( UINT32_MAX )


struct SCOREP_Location;


extern uint32_t                         scorep_ompt_my_location_count;
extern uint64_t*                        scorep_ompt_my_location_ids;
extern SCOREP_RmaWindowHandle           scorep_ompt_rma_window_handle;
extern SCOREP_InterimCommunicatorHandle scorep_ompt_interim_communicator_handle;


typedef struct scorep_ompt_cpu_location_data
{
    struct task_t* task;
    UTILS_Mutex    protect_task_exchange; /* task and potentially task members
                                             need to be modified in an atomic way;
                                             cannot be done with UTILS_Atomic_ExchangeN. */

    /* Have the mutexes on different cache lines */
    char pad[ SCOREP_CACHELINESIZE -
              ( sizeof( struct task_t* ) + sizeof( UTILS_Mutex ) ) ];
    /* preserve event order by preventing trigger_overdue to write itask_begin of new
       parallel region into location before ibarrier_end and itask_end of previous
       region are processed. */
    SCOREP_ALIGNAS( SCOREP_CACHELINESIZE ) UTILS_Mutex preserve_order;

    bool     is_ompt_location; /* ignore non-ompt location in overdue processing */
    uint32_t local_rank;       /* Local rank used for processing RMA records,
                                  and later in unification */
} scorep_ompt_cpu_location_data;


static inline size_t
scorep_ompt_get_subsystem_id( void )
{
    extern size_t scorep_ompt_subsystem_id;
    return scorep_ompt_subsystem_id;
}

/* We want to record events between ompt_subsystem_begin and ompt_substytem_end
   only, but cannot register and deregister callbacks there. Thus, pass events
   to the measurement core only if scorep_ompt_record_event() return true. */
static inline bool
scorep_ompt_record_event( void )
{
    extern bool scorep_ompt_record_events;
    return scorep_ompt_record_events;
}

/* Ignore events before ompt_subsystem_begin. Events during POST are ok when
   triggered within ompt_subsystem_end. */
#define SCOREP_OMPT_RETURN_ON_INVALID_EVENT() \
    do \
    { \
        if ( !scorep_ompt_record_event() ) \
        { \
            UTILS_DEBUG_EXIT( "Event ignored (either PRE, or POST after ompt_finalize_tool)." ); \
            SCOREP_IN_MEASUREMENT_DECREMENT(); \
            return; \
        } \
    } while ( 0 )


static inline bool
scorep_ompt_finalizing_tool_in_progress( void )
{
    extern bool scorep_ompt_finalizing_tool;
    return scorep_ompt_finalizing_tool;
}


static inline int
scorep_ompt_get_task_info( int            ancestor_level,
                           int*           flags,
                           ompt_data_t**  task_data,
                           ompt_frame_t** task_frame,
                           ompt_data_t**  parallel_data,
                           int*           thread_num )
{
    extern ompt_get_task_info_t scorep_ompt_mgmt_get_task_info;
    return scorep_ompt_mgmt_get_task_info( ancestor_level,
                                           flags,
                                           task_data,
                                           task_frame,
                                           parallel_data,
                                           thread_num );
}


static inline uint64_t
scorep_ompt_get_unique_id( void )
{
    /* Use OpenMP tools interface runtime entry point if available,
     * as runtimes might provide a more efficient way to get unique IDs.
     * LLVM-based runtimes provide 2^48 unique IDs, which is more
     * than enough for us. */
    extern ompt_get_unique_id_t scorep_ompt_mgmt_get_unique_id;
    if ( scorep_ompt_mgmt_get_unique_id )
    {
        return scorep_ompt_mgmt_get_unique_id();
    }
    /* Do not start at 0, as ompt_id_none is defined as 0. */
    static uint64_t next_unique_id = 1;
    return UTILS_Atomic_FetchAdd_uint64( &next_unique_id, 1, UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
}


SCOREP_ErrorCode
scorep_ompt_subsystem_trigger_overdue_events( struct SCOREP_Location* location );

void
scorep_ompt_codeptr_hash_dlclose_cb( void*       soHandle,
                                     const char* soFileName,
                                     uintptr_t   soBaseAddr,
                                     uint16_t    soToken );

void
scorep_ompt_unify_pre( void );

void
scorep_ompt_unify_post( void );

#endif /* SCOREP_OMPT_H */
