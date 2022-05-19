extern void
cpp_library_throwing_exception();

/**
 * Propergate C++ exception through C despite C has no exception handling
 * we use a C compiler
 */
void
c_library_not_handling_exception()
{
    cpp_library_throwing_exception();
}
