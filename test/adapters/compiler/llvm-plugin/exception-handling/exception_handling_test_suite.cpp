#include <cstdlib>
#include <stdint.h>
#include <exception>
#include <vector>

extern void
test_point();

typedef struct
{
    uint32_t*   handle_ref;
    const char* name;
    const char* canonical_name;
    const char* file;
    int         begin_lno;
    int         end_lno;
    unsigned    flags;
} scorep_compiler_region_description;

static std::vector<int> func_vec;

extern "C" void
scorep_plugin_register_region( scorep_compiler_region_description* description )
{
    func_vec.push_back( 0 );
    *description->handle_ref = func_vec.size() - 1;
    return;
}

extern "C" void
scorep_plugin_enter_region( uint32_t __scorep_region_handle )
{
    if ( __scorep_region_handle > func_vec.size() )
    {
        ;//   exit( 1 );
    }

    func_vec[ __scorep_region_handle ]++;
}

extern "C" void
scorep_plugin_exit_region( uint32_t __scorep_region_handle )
{
    if ( __scorep_region_handle > func_vec.size() )
    {
        ;//  exit( 1 );
    }
    func_vec[ __scorep_region_handle ]--;
}

int
main()
{
    test_point();
    if ( func_vec.empty() )
    {
        return 1;
    }

    for ( int el : func_vec )
    {
        if ( el != 0 )
        {
            return 1;
        }
    }
    return 0;
}
