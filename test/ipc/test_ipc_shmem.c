/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2012-2014, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2017,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 */

#include <config.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <shmem.h>

void
pstart_pes( int );

int
pshmem_my_pe( void );

int
pshmem_n_pes( void );

void
pshmem_barrier_all( void );

#if defined( SCOREP_SHMEM_INT_MIN_TO_ALL_DECL_ARGS )

/* *INDENT-OFF* */
void
pshmem_int_min_to_all SCOREP_SHMEM_INT_MIN_TO_ALL_DECL_ARGS;
/* *INDENT-ON* */

#else

#error No suitable reduction function available!

#endif


#include "test_ipc.h"

static int pes;
static long pSync[ _SHMEM_REDUCE_SYNC_SIZE ];
static int  pWrk[ _SHMEM_REDUCE_MIN_WRKDATA_SIZE ];
static int  reduce_args[ 2 ];


static void
reduce_results( int* result )
{
    reduce_args[ 0 ] = *result;

    pshmem_barrier_all();

    pshmem_int_min_to_all( &reduce_args[ 1 ],
                           &reduce_args[ 0 ],
                           1,
                           0, 0, pes,
                           pWrk, pSync );

    *result = reduce_args[ 1 ];

    pshmem_barrier_all();
}


int
main( int    argc,
      char** argv )
{
    int me;

    pstart_pes( 0 );
    pes = pshmem_n_pes();
    me  = pshmem_my_pe();

    for ( int i = 0; i < _SHMEM_REDUCE_SYNC_SIZE; i++ )
    {
        pSync[ i ] = _SHMEM_SYNC_VALUE;
    }
    pshmem_barrier_all();

    int ret = test_ipc( me, reduce_results );

    pshmem_barrier_all();

    /*
     * Brutally exit the program on failure.
     * Some stupid runtimes are calling _exit() in their atexit handler
     * but don't have the actually exit value at hand.
     */
    if ( EXIT_FAILURE == ret )
    {
        _exit( ret );
    }

    return ret;
}
