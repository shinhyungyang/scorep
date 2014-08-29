#ifndef SCOREP_THREAD_FORK_JOIN_EVENT_H_
#define SCOREP_THREAD_FORK_JOIN_EVENT_H_

/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013,
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


#include <SCOREP_Types.h>

#include <stdint.h>


/**
 * Notify the measurement system about the creation of a fork-join
 * parallel execution with at max @a nRequestedThreads new
 * threads. This function needs to be triggered for every thread
 * creation in a fork-join model, e.g., #pragma omp parallel in OpenMP
 * (for create-wait models see SCOREP_ThreadCreate()).
 * SCOREP_ThreadForkJoin_Fork() needs to be called outside the parallel
 * execution from the thread creating the parallel region.
 *
 * @param paradigm One of the predefined threading models.
 *
 * @param nRequestedThreads Upper bound of threads that comprise the
 * parallel region to be created.
 *
 * @note All threads in the following parallel region including the
 * master/creator need to call SCOREP_ThreadForkJoin_TeamBegin() and
 * SCOREP_ThreadForkJoin_TeamEnd().
 *
 * @note After execution of the parallel region the master/creator
 * needs to call SCOREP_ThreadForkJoin_Join().
 *
 * @see SCOREP_ThreadCreate()
 */
void
SCOREP_ThreadForkJoin_Fork( SCOREP_ParadigmType paradigm,
                            uint32_t            nRequestedThreads );


/**
 * Notify the measurement system about the completion of a fork-join
 * parallel execution. The parallel execution was started by a call to
 * SCOREP_ThreadForkJoin_Fork() that returned the @a forkSequenceCount that
 * needs to be provided to this function.
 *
 * @param paradigm One of the predefined threading models.
 *
 * @note See the notes to SCOREP_ThreadForkJoin_Fork().
 */
void
SCOREP_ThreadForkJoin_Join( SCOREP_ParadigmType paradigm );


/**
 * Notify the measurement system about the begin of a parallel
 * execution on a thread created by either SCOREP_ThreadForkJoin_Fork() or
 * SCOREP_ThreadCreate(). In case of SCOREP_ThreadForkJoin_Fork() all created
 * threads including the master must call SCOREP_ThreadForkJoin_TeamBegin().
 *
 * @param paradigm One of the predefined threading models.
 *
 * @param threadId Id out of [0..n) within the team of threads that
 * constitute the parallel region.
 *
 * @note The end of the parallel execution will be signaled by a call
 * to SCOREP_ThreadForkJoin_TeamEnd().
 *
 * @note Per convention and as there is no parallelism for the initial
 * thread we don't call SCOREP_ThreadForkJoin_TeamBegin() and SCOREP_ThreadForkJoin_TeamEnd()
 * for the initial thread.
 */
void
SCOREP_ThreadForkJoin_TeamBegin( SCOREP_ParadigmType paradigm,
                                 uint32_t            threadId );


/**
 * Notify the measurement system about the end of a parallel execution
 * on a thread created by either SCOREP_ThreadForkJoin_Fork() or
 * SCOREP_ThreadCreate(). Every thread that started a parallel
 * execution via SCOREP_ThreadForkJoin_TeamBegin() needs to end via
 * SCOREP_ThreadForkJoin_TeamEnd().
 *
 * @param paradigm One of the predefined threading models.
 *
 * @note The begin of the parallel execution was signalled by a call
 * to SCOREP_ThreadForkJoin_TeamBegin().
 *
 * @note Per convention and as there is no parallelism for the initial
 * thread we don't call SCOREP_ThreadForkJoin_TeamBegin() and SCOREP_ThreadForkJoin_TeamEnd()
 * for the initial thread.
 */
void
SCOREP_ThreadForkJoin_TeamEnd( SCOREP_ParadigmType paradigm );




/**
 * Process a task create event in the measurement system.
 *
 * @param paradigm            One of the predefined threading models.
 * @param threadId         Id of the this thread within the team of
 *                         threads that constitute the parallel region.
 * @param generationNumber The sequence number for this task. Each task
 *                         gets a thread private generation number of the
 *                         creating thread attached. Combined with the
 *                         @a threadId, this constitutes a unique task ID
 *                         inside the parallel region.
 */
void
SCOREP_ThreadForkJoin_TaskCreate( SCOREP_ParadigmType paradigm,
                                  uint32_t            threadId,
                                  uint32_t            generationNumber );


/**
 * Process a task switch event in the measurement system.
 *
 * @param paradigm            One of the predefined threading models.
 * @param threadId         Id of the this thread within the team of
 *                         threads that constitute the parallel region.
 * @param generationNumber The sequence number for this task. Each task
 *                         gets a thread private generation number of the
 *                         creating thread attached. Combined with the
 *                         @a threadId, this constitutes a unique task ID
 *                         inside the parallel region.
 */
void
SCOREP_ThreadForkJoin_TaskSwitch( SCOREP_ParadigmType paradigm,
                                  uint32_t            threadId,
                                  uint32_t            generationNumber );


/**
 * Process a task begin event in the measurement system.
 *
 * @param paradigm            One of the predefined threading models.
 * @param regionHandle     Region handle of the task region.
 * @param threadId         Id of the this thread within the team of
 *                         threads that constitute the parallel region.
 * @param generationNumber The sequence number for this task. Each task created
 *                         gets a thread private generation number attached.
 *                         Combined with the @a threadId, this constitutes a
 *                         unique task ID inside the parallel region.
 */
void
SCOREP_ThreadForkJoin_TaskBegin( SCOREP_ParadigmType paradigm,
                                 SCOREP_RegionHandle regionHandle,
                                 uint32_t            threadId,
                                 uint32_t            generationNumber );

/**
 * Process a task end event in the measurement system.
 *
 * @param paradigm            One of the predefined threading models.
 * @param regionHandle     Region handle of the task region.
 * @param threadId         Id of the this thread within the team of
 *                         threads that constitute the parallel region.
 * @param generationNumber The sequence number for this task. Each task
 *                         gets a thread private generation number of the
 *                         creating thread attached. Combined with the
 *                         @a threadId, this constitutes a unique task ID
 *                         inside the parallel region.
 */
void
SCOREP_ThreadForkJoin_TaskEnd( SCOREP_ParadigmType paradigm,
                               SCOREP_RegionHandle regionHandle,
                               uint32_t            threadId,
                               uint32_t            generationNumber );


#endif /* SCOREP_THREAD_FORK_JOIN_EVENT_H_ */
