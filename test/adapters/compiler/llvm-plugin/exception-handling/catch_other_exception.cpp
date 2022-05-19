#include <exception>
#include <cstdlib>

/**
 * Check if code in a catch block with no matching exception type is not executed on exception cleanup's clause
 */

/**
 * throw a float exception
 */
// noinline prevents from being inlined into main
void
__attribute__( ( noinline ) )
throws_exception()
{
    throw ( 1.0 );
}

/**
 * catch only int exception, therefore catch block is not executed
 */
void
__attribute__( ( noinline ) )
catches_other_exception()
{
    try {
        throws_exception();
    }
    catch ( int e )
    {
        exit( 1 );
    }
}

/**
 * This function will be called from main
 */
void
test_point()
{
    try
    {
        catches_other_exception();
    }
    catch ( ... )
    {
    }
}
