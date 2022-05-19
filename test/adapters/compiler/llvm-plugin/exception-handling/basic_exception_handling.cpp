#include <exception>

/**
 * Basic basic handling with one function throwing an exception and another catching it
 */

// noinline prevents from being inlined into main
void
__attribute__( ( noinline ) )
throws_exception()
{
    throw ( std::exception() );
}

void
__attribute__( ( noinline ) )
catches_exception()
{
    try
    {
        throws_exception();
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
    catches_exception();
}
