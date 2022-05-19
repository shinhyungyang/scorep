#include <exception>

/**
 * Tests if the exit-function is called if there is no exception handling in a function which gets an exception (b).
 * The exception should be propagated through this function into c and handled there.
 */

/** noinline prevents from being inlined into main */
void
__attribute__( ( noinline ) )
throws_exception()
{
    throw ( std::exception() );
}

void
__attribute__( ( noinline ) )
does_not_handle_exception()
{
    throws_exception();
}

void
__attribute__( ( noinline ) )
catch_exception()
{
    try {
        does_not_handle_exception();
    }
    catch ( ... )
    {
    }
}

/**
 * This function will be called from main
 */
void
test_point()
{
    catch_exception();
}
