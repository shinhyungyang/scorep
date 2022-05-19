#include <stdio.h>
#include <stdint.h>

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


static int uid = 10;

void
scorep_plugin_register_region( scorep_compiler_region_description* description )
{
    *description->handle_ref = uid;
    uid++;
    printf( "plugin registered with handle ref '%1d', canonical name '%20s' and name '%30s'\n", *( description->handle_ref ), description->canonical_name, description->name );
    return;
}

void
scorep_plugin_enter_region( uint32_t __scorep_region_handle )
{
    printf( "plugin entered with '%1d'\n", __scorep_region_handle );
    return;
}

void
scorep_plugin_exit_region( uint32_t __scorep_region_handle )
{
    printf( "plugin exited with '%1d'\n", __scorep_region_handle );
    return;
}
