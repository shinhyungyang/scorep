#ifndef SCOREP_MEMORY_WRAPPER
#error Macro 'SCOREP_MEMORY_WRAPPER' was not defined
#endif

SCOREP_MEMORY_WRAPPER( void*, malloc,         ( size_t ) );
SCOREP_MEMORY_WRAPPER( void,  free,           ( void* ) );
SCOREP_MEMORY_WRAPPER( void*, calloc,         ( size_t, size_t ) );
SCOREP_MEMORY_WRAPPER( void*, realloc,        ( void*, size_t ) );
SCOREP_MEMORY_WRAPPER( void*, memalign,       ( size_t, size_t ) );
SCOREP_MEMORY_WRAPPER( int,   posix_memalign, ( void**, size_t, size_t ) );
SCOREP_MEMORY_WRAPPER( void*, valloc,         ( size_t ) );
SCOREP_MEMORY_WRAPPER( void*, aligned_alloc,  ( size_t, size_t ) );

/* allocation functions of hbwmalloc hbw_* */

SCOREP_MEMORY_WRAPPER( void*, hbw_malloc,  ( size_t ) );
SCOREP_MEMORY_WRAPPER( void,  hbw_free,    ( void* ) );
SCOREP_MEMORY_WRAPPER( void*, hbw_calloc,  ( size_t, size_t ) );
SCOREP_MEMORY_WRAPPER( void*, hbw_realloc, ( void*, size_t ) );
SCOREP_MEMORY_WRAPPER( int,   hbw_posix_memalign,       ( void**, size_t, size_t ) );
SCOREP_MEMORY_WRAPPER( int,   hbw_posix_memalign_psize, ( void**, size_t, size_t, int ) );

/* Declaration of the mangled real functions new and delete */

SCOREP_MEMORY_WRAPPER( void*, _Znwm,   ( size_t ) );
SCOREP_MEMORY_WRAPPER( void*, _Znwj,   ( size_t ) );
SCOREP_MEMORY_WRAPPER( void,  _ZdlPv,  ( void* ) );
SCOREP_MEMORY_WRAPPER( void,  _ZdlPvm, ( void*, size_t ) );
SCOREP_MEMORY_WRAPPER( void,  _ZdlPvj, ( void*, size_t ) );
SCOREP_MEMORY_WRAPPER( void*, _Znam,   ( size_t ) );
SCOREP_MEMORY_WRAPPER( void*, _Znaj,   ( size_t ) );
SCOREP_MEMORY_WRAPPER( void,  _ZdaPv,  ( void* ) );
SCOREP_MEMORY_WRAPPER( void,  _ZdaPvm, ( void*, size_t ) );
SCOREP_MEMORY_WRAPPER( void,  _ZdaPvj, ( void*, size_t ) );

/* Declaration of the mangled real functions new and delete (old PGI/EDG C++ ABI) */

SCOREP_MEMORY_WRAPPER( void*, __nw__FUi,  ( size_t ) );
SCOREP_MEMORY_WRAPPER( void*, __nw__FUl,  ( size_t ) );
SCOREP_MEMORY_WRAPPER( void,  __dl__FPv,  ( void* ) );
SCOREP_MEMORY_WRAPPER( void*, __nwa__FUi, ( size_t ) );
SCOREP_MEMORY_WRAPPER( void*, __nwa__FUl, ( size_t ) );
SCOREP_MEMORY_WRAPPER( void,  __dla__FPv, ( void* ) );

#undef SCOREP_MEMORY_WRAPPER
