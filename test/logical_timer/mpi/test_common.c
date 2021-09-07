#include "test_common.h"

#include <stdio.h>

void
do_something()
{
}

void
keep_busy( int k )
{
    for ( int i = 0; i < k; ++i )
    {
        do_something();
    }
}

void
init_const( int* buf, int count, int value )
{
    for ( int i = 0; i < count; ++i )
    {
        buf[ i ] = value;
    }
}

void
print_buf( int* buf, int count )
{
    for ( int i = 0; i < count; ++i )
    {
        printf( "%d ", buf[ i ] );
    }
}
