#ifndef SCOREP_THREADCREATEWAIT_EVENT_H
#define SCOREP_THREADCREATEWAIT_EVENT_H

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


#include <SCOREP_Types.h>

#include <stdint.h>


struct scorep_thread_private_data;


/**
 *
 *
 * @param paradigm
 * @param parent
 * @param sequenceCount
 */
void
SCOREP_ThreadCreateWait_Create( SCOREP_ParadigmType                 paradigm,
                                struct scorep_thread_private_data** parent,
                                uint32_t*                           sequenceCount );


/**
 *
 *
 * @param paradigm
 * @param sequenceCount
 */
void
SCOREP_ThreadCreateWait_Wait( SCOREP_ParadigmType paradigm,
                              uint32_t            sequenceCount );


/**
 *
 *
 * @param paradigm
 * @param parent
 * @param sequenceCount
 */
void
SCOREP_ThreadCreateWait_Begin( SCOREP_ParadigmType                paradigm,
                               struct scorep_thread_private_data* parent,
                               uint32_t                           sequenceCount );


/**
 *
 *
 * @param paradigm
 * @param parentTpd
 * @param sequenceCount
 */
void
SCOREP_ThreadCreateWait_End( SCOREP_ParadigmType                paradigm,
                             struct scorep_thread_private_data* parentTpd,
                             uint32_t                           sequenceCount );


#endif /* SCOREP_THREADCREATEWAIT_EVENT_H */
