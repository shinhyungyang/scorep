/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2012,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */


#ifndef SCOREP_TRACING_THREAD_INTERACTION_H
#define SCOREP_TRACING_THREAD_INTERACTION_H


/**
 * @file
 *
 */


#include <SCOREP_Location.h>


typedef struct SCOREP_TracingData SCOREP_TracingData;


/**
 * Allocate a valid SCOREP_TracingData object, initialize it in
 * SCOREP_Tracing_OnLocationCreation().
 *
 */
SCOREP_TracingData*
SCOREP_Tracing_CreateLocationData( SCOREP_Location* locationData );


/**
 * Clean up @a traceLocationData at the end of a phase or at the end of the
 * measurement.
 *
 * @param traceLocationData The object to be deleted
 */
void
SCOREP_Tracing_DeleteLocationData( SCOREP_TracingData* traceLocationData );


/**
 * Triggered on thread creation, i.e. when a thread is encountered the first
 * time. Note that several thread can share teh same location data.
 *
 * @param locationData Location data of the current thread.
 * @param parentLocationData Location data of the parent thread, may equal @a
 * locationData.
 */
void
SCOREP_Tracing_OnThreadCreation( SCOREP_Location* locationData,
                                 SCOREP_Location* parentLocationData );

/**
 * Triggered at the start of every thread/parallel region. Always triggered,
 * even after thread creation. In contrast to creation this function may be
 * triggered multiple times, e.g. if we reenter a parallel region again or if
 * we reuse the location/thread in a different parallel region.
 *
 * @param locationData Location data of the current thread inside the paralell
 * region.
 * @param parentLocationData Location data of the parent thread, may equal @a
 * locationData.
 */
void
SCOREP_Tracing_OnLocationActivation( SCOREP_Location* locationData,
                                     SCOREP_Location* parentLocationData );


/**
 * Triggered after the end of every thread/parallel region, i.e. in the join
 * event.
 *
 * @param locationData Location data of the deactivated thread inside the
 * parallel region.
 * @param parentLocationData Location data of the parent thread, may equal @a
 * locationData.
 */
void
SCOREP_Tracing_OnLocationDeactivation( SCOREP_Location* locationData,
                                       SCOREP_Location* parentLocationData );


/**
 * Triggered on location creation, i.e. when a location is encountered the first
 * time. Note that several threads can share teh same location data.
 *
 * @param locationData Location data of the current thread.
 * @param parentLocationData Location data of the parent thread, may equal @a
 * locationData.
 */
void
SCOREP_Tracing_OnLocationCreation( SCOREP_Location* locationData,
                                   SCOREP_Location* parentLocationData );


void
SCOREP_Tracing_AssignLocationId( SCOREP_Location* threadLocationData );


#endif /* SCOREP_TRACING_THREAD_INTERACTION_H */
