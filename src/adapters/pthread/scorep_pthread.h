#ifndef SCOREP_PTHREAD_H
#define SCOREP_PTHREAD_H

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


#include <scorep/SCOREP_PublicTypes.h>
#include <stdbool.h>

#define PTHREAD_REGIONS                                         \
    PTHREAD_REGION( CREATE,         "create",         WRAPPER ) \
    PTHREAD_REGION( JOIN,           "join",           WRAPPER ) \
    PTHREAD_REGION( EXIT,           "exit",           WRAPPER ) \
    PTHREAD_REGION( CANCEL,         "cancel",         WRAPPER ) \
    PTHREAD_REGION( DETACH,         "detach",         WRAPPER ) \
    PTHREAD_REGION( MUTEX_INIT,     "mutex_init",     WRAPPER ) \
    PTHREAD_REGION( MUTEX_DESTROY,  "mutex_destroy",  WRAPPER ) \
    PTHREAD_REGION( MUTEX_LOCK,     "mutex_lock",     WRAPPER ) \
    PTHREAD_REGION( MUTEX_UNLOCK,   "mutex_unlock",   WRAPPER ) \
    PTHREAD_REGION( MUTEX_TRYLOCK,  "mutex_trylock",  WRAPPER ) \
    PTHREAD_REGION( COND_INIT,      "cond_init",      WRAPPER ) \
    PTHREAD_REGION( COND_SIGNAL,    "cond_signal",    WRAPPER ) \
    PTHREAD_REGION( COND_BROADCAST, "cond_broadcast", WRAPPER ) \
    PTHREAD_REGION( COND_WAIT,      "cond_wait",      WRAPPER ) \
    PTHREAD_REGION( COND_TIMEDWAIT, "cond_timedwait", WRAPPER ) \
    PTHREAD_REGION( COND_DESTROY,   "cond_destroy",   WRAPPER )



typedef enum scorep_pthread_region_types
{
#define PTHREAD_REGION( NAME, name, TYPE ) PTHREAD_ ## NAME,

    PTHREAD_REGIONS

        PTHREAD_START_ROUTINE,
        REGION_SENTINEL /**< For internal use only. */

#undef PTHREAD_REGION
} scorep_pthread_region_types;


extern SCOREP_RegionHandle scorep_pthread_regions[ REGION_SENTINEL - 1 ];

/* Flag to indicate if the adapter was initialized */
extern bool scorep_pthread_initialized;
/* Flag to indicate if the adapter was finalized */
extern bool scorep_pthread_finalized;

#endif /* SCOREP_PTHREAD_H */
