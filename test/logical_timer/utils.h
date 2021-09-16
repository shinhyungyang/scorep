#ifndef LTIMER_TEST_UTILS_H
#define LTIMER_TEST_UTILS_H

#include <stdio.h>

extern int world_rank, world_nprocs;

/**
 * printf only on the master rank
 *
 * Implemented as macro to not generate events in the trace
 */
#define PRINTF_MASTER( FORMAT, ... ) \
    if ( world_rank == 0 ) \
    { \
        printf( "| Master | " ); \
        printf( FORMAT, __VA_ARGS__ ); \
    }


#define PRINTF_RANK( FORMAT, ... ) \
    do \
    { \
        printf( "| Rank %d of %d | ", world_rank, world_nprocs ); \
        printf( FORMAT, __VA_ARGS__ ); \
    } while ( 0 )


/**
 * No-op function. Calls to this will still be picked up by the measurement system
 */
void
do_nothing();

/**
 * Increase the logical timer by twice the given value of @p steps
 *
 * We can only increment the counter by even numbers, because each
 * user function call generates two events (ENTER and EXIT).
 *
 * @param steps: Increase the logical timer by twice this value.
 */
void
wait_logical_twice( int steps );

void
wait_milliseconds( int milliseconds );

#endif
