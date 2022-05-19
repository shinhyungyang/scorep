#include <exception>

/**
 * C++ exception should be propagated throw the C interface, despite C has no exception handling
 * we use a C++ compiler
 */
extern "C" void
cpp_library_throwing_exception()
{
    throw std::exception();
}
