#include "utils.h"

#include <time.h>


int world_rank, world_nprocs;


void
do_nothing()
{
}


void
wait_logical_twice( int steps )
{
    /*
     * Each function call generates an ENTER and an EXIT event
     * which each increment the counter by one.
     * This includes the call to wait_logical_twice itself.
     */
    for ( int k = 0; k < ( steps - 1 ); ++k )
    {
        do_nothing();
    }
}


void
wait_milliseconds( int milliseconds )
{
    clock_t start_time = clock();
    while ( ( clock() - start_time ) / ( CLOCKS_PER_SEC / 1000 ) <  milliseconds )
    {
    }
}
