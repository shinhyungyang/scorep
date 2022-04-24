/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011, 2020,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

#ifndef SCOREP_OPARI2_OPENMP_H
#define SCOREP_OPARI2_OPENMP_H


#include <SCOREP_Timer_Utils.h>
#include <SCOREP_Timer_Ticks.h>
#include <SCOREP_Mutex.h>

/* Caveat: Order shall be respected */
enum omp_lamport_events
{
    SCOREP_OPARI2_OMP_LAMPORT_EVENT_BARRIER = 0,
    SCOREP_OPARI2_OMP_LAMPORT_EVENT_CRITICAL,
    SCOREP_OPARI2_OMP_LAMPORT_EVENT_FORK,
    SCOREP_OPARI2_OMP_LAMPORT_EVENT_JOIN
};

/**
   Lock to protect shared global variable assignment.
 */
SCOREP_Mutex scorep_opari2_omp_lock[] =
{
    SCOREP_MUTEX_INIT,
    SCOREP_MUTEX_INIT,
    SCOREP_MUTEX_INIT,
    SCOREP_MUTEX_INIT
};

/* global variable shared among all threads */
/* protected by mutex                       */
/* updated on logical sync events           */
uint64_t scorep_opari2_omp_timer_max[ 4 ];


#define SCOREP_OPARI2_OMP_UPDATE_LOGICAL_MAX( omp_lamport_event )          \
{                                                                          \
    uint8_t  event_type        = ( int )( omp_lamport_event );             \
    uint64_t current_timer_val = SCOREP_Timer_GetLogical();                \
    SCOREP_MutexLock( &scorep_opari2_omp_lock[ event_type ] );             \
    scorep_opari2_omp_timer_max[ event_type ] =                            \
        scorep_opari2_omp_timer_max[ event_type ] < current_timer_val ?    \
        current_timer_val : scorep_opari2_omp_timer_max[ event_type ];     \
    SCOREP_MutexUnlock( &scorep_opari2_omp_lock[ event_type ] );           \
}

#define SCOREP_OPARI2_OMP_SET_LOGICAL_TIMER( omp_lamport_event )            \
{                                                                           \
    uint8_t event_type = ( int )( omp_lamport_event );                      \
    SCOREP_Timer_SetLogical( scorep_opari2_omp_timer_max[ event_type ] );   \
}

/* called inside mutex context                                   */
/* update logic max inside lock struct and in thread's timestamp */
#define SCOREP_OPARI2_OMP_UPDATE_LOGICAL_MAX_LOCK( lock )                      \
{                                                                              \
    uint64_t current_timer_val = SCOREP_Timer_GetLogical();                    \
    /* get timer max value stored inside lock struct   */                      \
    uint64_t lock_timer_max = SCOREP_Opari2_Openmp_LockGetLogicTimer( lock );  \
    lock_timer_max = lock_timer_max < current_timer_val?                       \
                     current_timer_val : lock_timer_max;                       \
    /* update the lock struct with max value   */                              \
    SCOREP_Opari2_Openmp_LockSetLogicTimer( lock, lock_timer_max );            \
    /* update the timestamp for the thread  */                                 \
    SCOREP_Timer_SetLogical( lock_timer_max );                                 \
}


#endif /* SCOREP_OPARI2_OPENMP_H */
