#include <exception>
#include <stdio.h>
extern "C" void
c_library_not_handling_exception();

/**
 * Catch the exception propergated from the first C++ function (emulated library)
 *
 * This function will be called from main
 */
void
test_point()
{
    try
    {
        c_library_not_handling_exception();
    }
    catch ( ... )
    {
    }
}
