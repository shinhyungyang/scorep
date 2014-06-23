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

#ifndef SCOREP_PTHREAD_MUTEX_H
#define SCOREP_PTHREAD_MUTEX_H

/**
 * @file
 * @ingroup    PTHREAD
 *
 * @brief Declaration of internal functions for lock management.
 */


#include <stdint.h>
#include <pthread.h>


typedef struct scorep_pthread_mutex scorep_pthread_mutex;
struct scorep_pthread_mutex
{
    scorep_pthread_mutex* next; /* Separate chaining with linked lists. */
    void*                 key;  /* Used as key to hash function.
                                 * uintptr_t also possible */

    uint32_t id;                /* [0, N[ */
    uint32_t acquisition_order;
};


void
scorep_pthread_mutex_init( void );


void
scorep_pthread_mutex_finalize( void );


scorep_pthread_mutex*
scorep_pthread_mutex_hash_put( pthread_mutex_t* pthreadMutex );


scorep_pthread_mutex*
scorep_pthread_mutex_hash_get( pthread_mutex_t* pthreadMutex );


void
scorep_pthread_mutex_hash_remove( pthread_mutex_t* pthreadMutex );



#if 0

#include <scorep/SCOREP_PublicTypes.h>
#include <SCOREP_Mutex.h>
#include <SCOREP_Config.h>
#include <stdint.h>
#include <pthread.h>

typedef struct SCOREP_PthreadLockNode  SCOREP_PthreadLockNode;
typedef struct SCOREP_PthreadHashtable SCOREP_PthreadHashtable;

typedef uint32_t                       SCOREP_Pthread_LockHandleType;

#define SCOREP_PTHREAD_GOLDEN_RATIO 0.61803398875
#define SCOREP_PTHREAD_INVALID_LOCK -1
#define SCOREP_MAX_PTHREAD_LOCKS 32
extern SCOREP_PthreadHashtable* SCOREP_PTHREAD_HASH_TABLE;
extern SCOREP_Mutex             SCOREP_PTHREAD_MUTEX;

enum SCOREP_Pthread_LockRegion_Index
{
    SCOREP_PTHREAD_INIT_LOCK = 0,
    SCOREP_PTHREAD_DESTROY_LOCK,
    SCOREP_PTHREAD_SET_LOCK,
    SCOREP_PTHREAD_UNSET_LOCK,
    SCOREP_PTHREAD_TEST_LOCK,

    SCOREP_PTHREAD_REGION_LOCK_NUM
};

extern SCOREP_RegionHandle scorep_pthread_lock_region_handles[ SCOREP_PTHREAD_REGION_LOCK_NUM ];

struct SCOREP_PthreadLockNode
{
    uintptr_t        id;
    pthread_mutex_t* mutex;
    uint32_t         acquisition_order;
};

struct SCOREP_PthreadHashtable
{
    uint32_t                 table_size;
    SCOREP_PthreadLockNode** table;
    uint32_t                 size;
};

void
scorep_init_pthread_hashtable( SCOREP_PthreadHashtable* hashtable,
                               uint32_t                 size );

int
scorep_pthread_hash_function( int, uintptr_t );


SCOREP_PthreadLockNode*
scorep_pthread_put_in_hashtable( SCOREP_PthreadHashtable*,
                                 pthread_mutex_t* mutex );

SCOREP_PthreadLockNode*
scorep_pthread_get_in_hashtable( SCOREP_PthreadHashtable*,
                                 pthread_mutex_t* mutex );

void
scorep_pthread_destroy_hashtable( SCOREP_PthreadHashtable* );

SCOREP_ErrorCode
scorep_pthread_remove_hashtable_entry( SCOREP_PthreadHashtable*,
                                       pthread_mutex_t* mutex );

void
scorep_pthread_removeall_hashtable_entry( SCOREP_PthreadHashtable* );

#endif // 0

#endif // SCOREP_PTHREAD_MUTEX_H
