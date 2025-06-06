/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2021-2022, 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 * @file
 *
 * @brief Implements addr2line by matching given addresses with
 * loadtime- (lt_) and runtime-loaded (rt_) shared objects and
 * corresponding libbfd images. Shared objects are obtained via
 * dl_iterate_phdr; the loadtime ones at measurement initialization
 * and the runtime ones via rtdl-audit callbacks.
 */

#include <config.h>

#include <SCOREP_Addr2line.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <link.h>

#if HAVE( STDALIGN_H )
#include <stdalign.h>
#endif

#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Memory.h>
#include <SCOREP_ReaderWriterLock.h>

#include <UTILS_Atomic.h>
#include <UTILS_Error.h>
#include <UTILS_CStr.h>
#include <UTILS_Mutex.h>

#define SCOREP_DEBUG_MODULE_NAME ADDR2LINE
#include <UTILS_Debug.h>

#ifndef PACKAGE
/* Some bfd.h installations require a PACKAGE or PACKAGE_VERSION to be
   defined, see https://sourceware.org/bugzilla/show_bug.cgi?id=14243
   and https://sourceware.org/bugzilla/show_bug.cgi?id=15920 */
#define PACKAGE
#endif /* ! PACKAGE */
#include <bfd.h>
/* Deal with bfd API changes since binutils 2.34 */
#ifndef bfd_get_section_size
#define bfd_get_section_size( asection ) bfd_section_size( asection )
#endif /* ! bfd_get_section_size */
#ifndef bfd_get_section_vma
#define bfd_get_section_vma( abfd, asection ) bfd_section_vma( asection )
#endif /* ! bfd_get_section_vma */
#ifndef bfd_get_section_flags
#define bfd_get_section_flags( abfd, asection ) bfd_section_flags( asection )
#endif /* ! bfd_get_section_flags */


/* *INDENT-OFF* */
static int count_shared_objs( struct dl_phdr_info* info, size_t unused, void* cnt );
static bool is_obj_relevant( const char* name );
static bool iterate_segments( struct dl_phdr_info* info, const char** name, uintptr_t* baseAddr, bfd** abfd, asymbol*** symbols, uintptr_t* beginAddrMin, uintptr_t* endAddrMax, bool adjustName );
static int fill_lt_arrays_cb( struct dl_phdr_info* info, size_t unused, void* cnt );
static void init_abfd( const char* name, bfd** abfd, asymbol*** symbols, long* nSyms );
/* *INDENT-ON* */


/* Loadtime-loaded shared objects correspond to two sorted arrays for
   fast and concurrent lookup. The first one contains just the start
   addresses where the second ones contains the remaining
   metadata. The capacity of the array might be larger than the actual
   element count.
   We bfd-lookup (addr - base_addr) if addr in [begin_addr, end_addr]. */
static size_t lt_objs_capacity;    /* upper bound of loadtime objects */
static size_t     lt_object_count; /* number of loadtime objects used */
static uintptr_t* lt_begin_addrs;
#define SO_OBJECT_COMMON \
    uintptr_t end_addr; \
    uintptr_t   base_addr; \
    bfd*        abfd; \
    asymbol**   symbols; \
    const char* name; \
    uint16_t    token; \
    UTILS_Mutex abfd_mutex
typedef struct lt_object lt_object;
struct lt_object
{
    SO_OBJECT_COMMON;
};
static lt_object* lt_objects;
static uintptr_t  lt_objects_min_addr = UINTPTR_MAX;
static uintptr_t  lt_objects_max_addr = 0;
static uint32_t   n_overlapping_address_ranges;

static bool addr2line_initialized;

void
SCOREP_Addr2line_Initialize( void )
{
    UTILS_DEBUG_ENTRY();
    if ( addr2line_initialized )
    {
        return;
    }
    addr2line_initialized = true;

    /* get upper bound of relevant loadtime shared objects */
    dl_iterate_phdr( count_shared_objs, ( void* )&lt_objs_capacity );
    UTILS_DEBUG( "lt_objs_capacity=%zu", lt_objs_capacity );

    lt_begin_addrs = SCOREP_Memory_AlignedAllocForMisc( SCOREP_CACHELINESIZE,
                                                        lt_objs_capacity * sizeof( uintptr_t ) );
    memset( lt_begin_addrs, 0, lt_objs_capacity * sizeof( uintptr_t ) );
    lt_objects = calloc( lt_objs_capacity, sizeof( lt_object ) );
    UTILS_BUG_ON( !lt_begin_addrs || !lt_objects );

    /* some versions of libbfd have bfd_init to return void, others return unsigned. */
    bfd_init();

    /* fill arrays but ignore shared objects that don't report any symbols */
    dl_iterate_phdr( fill_lt_arrays_cb, ( void* )&lt_object_count );
    UTILS_BUG_ON( lt_object_count > lt_objs_capacity,
                  "Actual count must not exceed capacity." );

    UTILS_DEBUG_EXIT( "lt_object_count=%zu", lt_object_count );
}


static int
count_shared_objs( struct dl_phdr_info* info, size_t unused, void* cnt )
{
    if ( !is_obj_relevant( info->dlpi_name ) )
    {
        return 0;
    }
    /* Consider objects that have at least one loadable (PT_LOAD) and
       readable (PF_R) program segment. */
    for ( int i = 0; i < info->dlpi_phnum; i++ )
    {
        if ( info->dlpi_phdr[ i ].p_type == PT_LOAD
             && info->dlpi_phdr[ i ].p_flags & PF_R )
        {
            ++*( size_t* )cnt;
            return 0;
        }
    }
    return 0;
}


static bool
is_obj_relevant( const char* name )
{
    if ( strstr( name, "linux-vdso" )
         || strstr( name, "libscorep" )
         || strstr( name, "libcube4w" )
         || strstr( name, "libotf2" )
         /* Explicitly ignore ld-linux[x].so, as we encountered cases where
            the address space overlaps with runtime loaded libraries. */
         || strstr( name, "ld-linux" )
         /* Don't be too aggressive about excluding things
            1. Initial parsing time not relevant
            2. Space only relevant if library with line info is never queried
            3. Additional addr-range lookup time negligible
            4. "You think you want source for this address, but you
               don't." might have no use case */
         )
    {
        UTILS_DEBUG( "'%s' not relevant", name );
        return false;
    }
    return true;
}


static int
fill_lt_arrays_cb( struct dl_phdr_info* info, size_t unused, void* cnt )
{
    if ( !is_obj_relevant( info->dlpi_name ) )
    {
        return 0;
    }

    /* a dl_phdr_info object has one corresponding abfd, symbols, and
       baseAddr, and potentially several beginAddr/endAddr pairs.
       Later on, we will look up every address that is in the range
       [begin_addr_min, end_addr_max] */
    const char* name           = NULL;
    uintptr_t   base_addr      = 0;
    bfd*        abfd           = NULL;
    asymbol**   symbols        = NULL;
    uintptr_t   begin_addr_min = UINTPTR_MAX;
    uintptr_t   end_addr_max   = 0;
    bool        has_symbols    = iterate_segments( info,
                                                   &name,
                                                   &base_addr,
                                                   &abfd,
                                                   &symbols,
                                                   &begin_addr_min,
                                                   &end_addr_max,
                                                   true /* adjustName */ );
    if ( !has_symbols )
    {
        return 0;
    }

    /* add shared object representation to array. No need to keep array
       sorted (e.g., by begin-address). As address ranges may overlap,
       we later on search all load-time and run-time objects. */
    size_t insert = ( *( size_t* )cnt )++;
    lt_begin_addrs[ insert ]        = begin_addr_min;
    lt_objects[ insert ].end_addr   = end_addr_max;
    lt_objects[ insert ].base_addr  = base_addr;
    lt_objects[ insert ].abfd       = abfd;
    lt_objects[ insert ].symbols    = symbols;
    lt_objects[ insert ].name       = name;
    lt_objects[ insert ].token      = SCOREP_ADDR2LINE_LT_OBJECT_TOKEN;
    lt_objects[ insert ].abfd_mutex = UTILS_MUTEX_INIT;

    /* Check for overlapping address ranges (no assumptions made). */
    for ( size_t i = 0; i < insert; ++i )
    {
        if ( lt_begin_addrs[ i ] <= lt_objects[ insert ].end_addr
             && lt_objects[ i ].end_addr >= lt_begin_addrs[ insert ] )
        {
            n_overlapping_address_ranges++;
        }
    }

    /* update lt address interval */
    if ( lt_begin_addrs[ insert ] < lt_objects_min_addr )
    {
        lt_objects_min_addr = lt_begin_addrs[ insert ];
    }
    if ( lt_objects[ insert ].end_addr > lt_objects_max_addr )
    {
        lt_objects_max_addr = lt_objects[ insert ].end_addr;
    }

    UTILS_DEBUG( "Use %s: base=%" PRIuPTR "; begin=%" PRIuPTR "; end=%" PRIuPTR "",
                 name, base_addr, begin_addr_min, end_addr_max );
    return 0;
}


static bool
iterate_segments( struct dl_phdr_info* info,
                  const char**         name,
                  uintptr_t*           baseAddr,
                  bfd**                abfd,
                  asymbol***           symbols,
                  uintptr_t*           beginAddrMin,
                  uintptr_t*           endAddrMax,
                  bool                 adjustName )
{
    *name     = info->dlpi_name;
    *baseAddr = ( uintptr_t )info->dlpi_addr;
    /* iterate over segments to find all loadable program segments (PT_LOAD)
       ones that are readable (PF_R) */
    for ( int i = 0; i < info->dlpi_phnum; i++ )
    {
        if ( info->dlpi_phdr[ i ].p_type == PT_LOAD
             && info->dlpi_phdr[ i ].p_flags & PF_R )
        {
            if ( !*abfd )
            {
                /* A pathname is needed to open a bfd. */
                UTILS_BUG_ON( !*name, "Valid name form dl_phdr_info expected." );

                bool is_executable = false;
                if ( adjustName && *name[ 0 ] == '\0' )
                {
                    /* As dl_iterate_phdr returns "" for the executable itself,
                       we need to acquire the executable name. */
                    bool unused;
                    *name         = SCOREP_GetExecutableName( &unused );
                    is_executable = true;
                }

                long n_syms = 0; /* bfd_canonicalize_symtab returns long */
                init_abfd( *name, abfd, symbols, &n_syms );
                /* objects that don't report symbols are ignored */
                if ( n_syms < 1 )
                {
                    return false;
                }

                if ( adjustName && !is_executable )
                {
                    *name = UTILS_CStr_dup( info->dlpi_name );
                }
            }
            uintptr_t begin_addr = *baseAddr + ( uintptr_t )info->dlpi_phdr[ i ].p_vaddr;
            if ( begin_addr < *beginAddrMin )
            {
                *beginAddrMin = begin_addr;
            }
            uintptr_t end_addr = begin_addr + ( uintptr_t )info->dlpi_phdr[ i ].p_memsz;
            if ( end_addr > *endAddrMax )
            {
                *endAddrMax = end_addr;
            }
        }
    }
    if ( !*abfd )
    {
        UTILS_WARNING( "No readable PT_LOAD segment found for '%s'. "
                       "Is this supposed to happen?",
                       info->dlpi_name );
        return false;
    }
    UTILS_BUG_ON( *beginAddrMin > *endAddrMax );

    /* The memory range [ beginAddrMin, endAddrMax ] might contain segments
       that are not of type PT_LOAD and/or that are not readable (?). Would a
       bfd lookup into these parts of the memory range do any harm? If yes, we
       would need to track begin/end of the individual readable PT_LOAD
       segments. */

    return true;
}


static void
init_abfd( const char* name, bfd** abfd, asymbol*** symbols, long* nSyms )
{
    *abfd = bfd_openr( name, NULL );
    if ( !*abfd )
    {
        UTILS_DEBUG( "Could not bfd-open %s", name );
        return;
    }
    ( *abfd )->flags |= BFD_DECOMPRESS;
    if ( !bfd_check_format( *abfd, bfd_object ) )
    {
        UTILS_DEBUG( "abfd of %s not of type bfd_object", name );
        bfd_close( *abfd ); /* returns bool */
        return;
    }
    if ( !( bfd_get_file_flags( *abfd ) & HAS_SYMS ) )
    {
        UTILS_DEBUG( "abfd of %s has no symbols", name );
        bfd_close( *abfd );
        return;
    }
    long upper_bound = bfd_get_symtab_upper_bound( *abfd );
    if ( upper_bound < 1 )
    {
        UTILS_DEBUG( "abfd of %s reports an symbol upper bound of %ld",
                     name, upper_bound );
        bfd_close( *abfd );
        return;
    }
    *symbols = malloc( upper_bound );
    if ( !*symbols )
    {
        UTILS_BUG( "Could not allocate symbols for abfd of %s", name );
        bfd_close( *abfd );
        return;
    }
    *nSyms = bfd_canonicalize_symtab( *abfd, *symbols );
    if ( *nSyms < 1 )
    {
        UTILS_DEBUG( "No symbols in abfd of %s (n_syms=%ld)", name, *nSyms );
        free( *symbols );
        bfd_close( *abfd );
        return;
    }
    else
    {
        UTILS_DEBUG( "Read %ld symbols for abfd of %s (upper_bound=%ld)",
                     *nSyms, name, upper_bound );
    }
}


/* data needed for iterating bfd sections, i.e. the actual source code
   location lookup. */
typedef struct lookup_bfd_t lookup_bfd_t;
struct lookup_bfd_t
{
    uintptr_t begin_addr;
    uintptr_t end_addr;
    asymbol** symbols;

    /* OUT members */
    bool*         scl_found_begin_addr;
    bool*         scl_found_end_addr;
    const char**  scl_file_name;
    const char**  scl_function_name;
    unsigned int* scl_begin_lno;
    unsigned int* scl_end_lno;
};


/* An lrt_objects_container is obtained from a thread-safe pool in
   SCOREP_Addr2line_Lookup* and returned immediatly after use. A container
   is needed as address-ranges potentially overlap, thus there is no 1:1
   address to lt/rt_object mapping. */
typedef struct lrt_objects_container lrt_objects_container;
struct lrt_objects_container
{
    size_t                 capacity;  /* number of elements that can be held in
                                         currently allocated objects[] storage */
    size_t                 size;      /* number of elements in objects[] */
    lrt_objects_container* next;      /* linked list */
    bool                   unlock;    /* unlock after use in case of rt_objetcs */
    lt_object*             objects[]; /* Flexible array member */
};


/* *INDENT-OFF* */
static lrt_objects_container* get_lrt_objects_container_from_pool( void );
static inline void release_lrt_objects_container_to_pool( lrt_objects_container* container );
static void lookup_so( uintptr_t addr, lrt_objects_container* matches );
static void section_iterator( bfd* abfd, asection* section, void* payload );
static inline void map_over_sections_locked( lt_object* handle, void* data );
/* *INDENT-ON* */


void
SCOREP_Addr2line_LookupSo( uintptr_t    programCounterAddr,
                           /* shared object OUT parameters */
                           void**       soHandle,
                           const char** soFileName,
                           uintptr_t*   soBaseAddr,
                           uint16_t*    soToken )
{
    UTILS_BUG_ON( soHandle == NULL ||
                  soFileName == NULL ||
                  soBaseAddr == NULL ||
                  soToken == NULL,
                  "Need valid OUT handles but NULL provided." );

    /* We assume an address to be bfd-found in one lt_object only, even if
       address is contained in more than one, potentially overlapping,
       lt_object address-ranges. Obtain address-ranges first, then do
       bfd-lookup to confirm. */
    lrt_objects_container* matches = get_lrt_objects_container_from_pool();
    lookup_so( programCounterAddr, matches );
    bool addr_found = false;
    for ( size_t i = 0; i < matches->size; ++i )
    {
        lt_object* handle = matches->objects[ i ];
        *soFileName = handle->name;
        *soBaseAddr = handle->base_addr;
        *soToken    = handle->token;
        bool         found_begin_addr;
        bool         found_end_addr_unused;
        const char*  function_name_unused;
        unsigned     line_no_unused;
        lookup_bfd_t data = {
            .begin_addr           = programCounterAddr - handle->base_addr,
            .end_addr             = 0,
            .symbols              = handle->symbols,
            .scl_found_begin_addr = &found_begin_addr,
            .scl_found_end_addr   = &found_end_addr_unused,
            .scl_file_name        = soFileName,
            .scl_function_name    = &function_name_unused,
            .scl_begin_lno        = &line_no_unused,
            .scl_end_lno          = NULL
        };
        *( data.scl_found_begin_addr ) = false;
        *( data.scl_found_end_addr )   = false;

        map_over_sections_locked( handle, &data );
        if ( *( data.scl_found_begin_addr ) )
        {
            addr_found = true;
            *soHandle  = handle;
            break;
        }
    }
    release_lrt_objects_container_to_pool( matches );
    if ( !addr_found )
    {
        *soToken  = SCOREP_ADDR2LINE_SO_OBJECT_INVALID_TOKEN;
        *soHandle = NULL;
    }
}


/* The RW lock needed to safely access the linked lists for
   runtime-loaded shared objects. */
struct rwlock
{
    SCOREP_ALIGNAS( SCOREP_CACHELINESIZE ) int16_t pending;
    int16_t     departing;
    int16_t     release_n_readers;
    int16_t     release_writer;
    UTILS_Mutex writer_mutex;
} scorep_rt_objects_rwlock =
{
    .pending           = 0,
    .departing         = 0,
    .release_n_readers = 0,
    .release_writer    = 0,
    .writer_mutex      = UTILS_MUTEX_INIT
};


void
SCOREP_Addr2line_SoLookupAddr( uintptr_t    offset,
                               void*        soHandle,
                               uint16_t     soToken,
                               /* shared object OUT parameters */
                               const char** soFileName,
                               /* Source code location OUT parameters */
                               bool*        sclFound,
                               const char** sclFileName,
                               const char** sclFunctionName,
                               unsigned*    sclLineNo )
{
    UTILS_BUG_ON( soHandle == NULL, "Need valid soHandle but NULL provided" );
    UTILS_BUG_ON( soFileName == NULL ||
                  sclFound == NULL ||
                  sclFileName == NULL ||
                  sclFunctionName == NULL ||
                  sclLineNo == NULL,
                  "Need valid OUT handles but NULL provided." );

    if ( soToken != SCOREP_ADDR2LINE_LT_OBJECT_TOKEN )
    {
        SCOREP_RWLock_ReaderLock( &scorep_rt_objects_rwlock.pending,
                                  &scorep_rt_objects_rwlock.release_n_readers );
        if ( !SCOREP_Addr2line_SoStillLoaded( soToken ) )
        {
            UTILS_WARNING( "Trying to lookup address using invalid token %" PRIu16 ".", soToken );
            *sclFound = false;
            SCOREP_RWLock_ReaderUnlock( &scorep_rt_objects_rwlock.pending,
                                        &scorep_rt_objects_rwlock.departing,
                                        &scorep_rt_objects_rwlock.release_writer );
            return;
        }
    }
    lt_object* so_handle = ( lt_object* )soHandle;
    UTILS_BUG_ON( so_handle->token != soToken, "Provided token does not match soHandle's token" );

    *soFileName = so_handle->name;

    bool         scl_found_unused;
    lookup_bfd_t data = {
        .begin_addr           = offset,
        .end_addr             = 0,
        .symbols              = so_handle->symbols,
        .scl_found_begin_addr = sclFound,
        .scl_found_end_addr   = &scl_found_unused,
        .scl_file_name        = sclFileName,
        .scl_function_name    = sclFunctionName,
        .scl_begin_lno        = sclLineNo,
        .scl_end_lno          = NULL
    };
    *( data.scl_found_begin_addr ) = false;
    *( data.scl_found_end_addr )   = false;

    map_over_sections_locked( so_handle, &data );
    if ( soToken != SCOREP_ADDR2LINE_LT_OBJECT_TOKEN )
    {
        SCOREP_RWLock_ReaderUnlock( &scorep_rt_objects_rwlock.pending,
                                    &scorep_rt_objects_rwlock.departing,
                                    &scorep_rt_objects_rwlock.release_writer );
    }
}


static inline void
map_over_sections_locked( lt_object* handle, void* data )
{
    /* From the BFD documentation: A given BFD cannot safely be used
       from two threads at the same time; it is up to the application
       to do any needed locking. However, it is ok for different
       threads to work on different BFD objects at the same time.

       See #371

       The performance impact should be ok, as the lookup is usually
       triggered from a FastHashtab's value ctor, where concurrent
       threads are already synchronized if they try to lookup the same
       address. However, we need to synchronize lookups from distinct
       FastHashtab's buckets that access the same lt_object.
     */
    UTILS_MutexLock( &handle->abfd_mutex );
    bfd_map_over_sections( handle->abfd, section_iterator, data );
    UTILS_MutexUnlock( &handle->abfd_mutex );
}


void
SCOREP_Addr2line_SoLookupAddrRange( uintptr_t    beginOffset,
                                    uintptr_t    endOffset,
                                    void*        soHandle,
                                    uint16_t     soToken,
                                    /* shared object OUT parameters */
                                    const char** soFileName,
                                    /* Source code location OUT parameters */
                                    bool*        sclFoundBegin,
                                    bool*        sclFoundEnd,
                                    const char** sclFileName,
                                    const char** sclFunctionName,
                                    unsigned*    sclBeginLineNo,
                                    unsigned*    sclEndLineNo )
{
    UTILS_BUG_ON( soHandle == NULL, "Need valid soHandle but NULL provided" );
    UTILS_BUG_ON( soFileName == NULL ||
                  sclFoundBegin == NULL ||
                  sclFoundEnd == NULL ||
                  sclFileName == NULL ||
                  sclFunctionName == NULL ||
                  sclBeginLineNo == NULL ||
                  sclEndLineNo == NULL,
                  "Need valid OUT handles but NULL provided." );

    if ( soToken != SCOREP_ADDR2LINE_LT_OBJECT_TOKEN )
    {
        SCOREP_RWLock_ReaderLock( &scorep_rt_objects_rwlock.pending,
                                  &scorep_rt_objects_rwlock.release_n_readers );
        if ( !SCOREP_Addr2line_SoStillLoaded( soToken ) )
        {
            UTILS_WARNING( "Trying to lookup address using invalid token %" PRIu16 ".", soToken );
            *sclFoundBegin = false;
            *sclFoundEnd   = false;
            SCOREP_RWLock_ReaderUnlock( &scorep_rt_objects_rwlock.pending,
                                        &scorep_rt_objects_rwlock.departing,
                                        &scorep_rt_objects_rwlock.release_writer );
            return;
        }
    }
    lt_object* so_handle = ( lt_object* )soHandle;
    UTILS_BUG_ON( so_handle->token != soToken, "Provided token does not match soHandle's token" );

    *soFileName = so_handle->name;

    lookup_bfd_t data = {
        .begin_addr           = beginOffset,
        .end_addr             = endOffset,
        .symbols              = so_handle->symbols,
        .scl_found_begin_addr = sclFoundBegin,
        .scl_found_end_addr   = sclFoundEnd,
        .scl_file_name        = sclFileName,
        .scl_function_name    = sclFunctionName,
        .scl_begin_lno        = sclBeginLineNo,
        .scl_end_lno          = sclEndLineNo
    };
    *( data.scl_found_begin_addr ) = false;
    *( data.scl_found_end_addr )   = false;

    map_over_sections_locked( so_handle, &data );
    if ( soToken != SCOREP_ADDR2LINE_LT_OBJECT_TOKEN )
    {
        SCOREP_RWLock_ReaderUnlock( &scorep_rt_objects_rwlock.pending,
                                    &scorep_rt_objects_rwlock.departing,
                                    &scorep_rt_objects_rwlock.release_writer );
    }
}


void
SCOREP_Addr2line_LookupAddr( uintptr_t    programCounterAddr,
                             /* shared object OUT parameters */
                             void**       soHandle,
                             const char** soFileName,
                             uintptr_t*   soBaseAddr,
                             uint16_t*    soToken,
                             /* Source code location OUT parameters */
                             bool*        sclFound,
                             const char** sclFileName,
                             const char** sclFunctionName,
                             unsigned*    sclLineNo )
{
    UTILS_BUG_ON( soHandle == NULL ||
                  soFileName == NULL ||
                  soBaseAddr == NULL ||
                  soToken == NULL ||
                  sclFound == NULL ||
                  sclFileName == NULL ||
                  sclFunctionName == NULL ||
                  sclLineNo == NULL,
                  "Need valid OUT handles but NULL provided." );

    lrt_objects_container* matches = get_lrt_objects_container_from_pool();
    lookup_so( programCounterAddr, matches );
    bool addr_found = false;
    for ( size_t i = 0; i < matches->size; ++i )
    {
        lt_object*   handle = matches->objects[ i ];
        bool         found_end_addr_unused;
        lookup_bfd_t data = {
            .begin_addr           = programCounterAddr - handle->base_addr,
            .end_addr             = 0,
            .symbols              = handle->symbols,
            .scl_found_begin_addr = sclFound,
            .scl_found_end_addr   = &found_end_addr_unused,
            .scl_file_name        = sclFileName,
            .scl_function_name    = sclFunctionName,
            .scl_begin_lno        = sclLineNo,
            .scl_end_lno          = NULL
        };
        *( data.scl_found_begin_addr ) = false;
        *( data.scl_found_end_addr )   = false;

        map_over_sections_locked( handle, &data );
        if ( *sclFound )
        {
            addr_found  = true;
            *soHandle   = handle;
            *soFileName = handle->name;
            *soBaseAddr = handle->base_addr;
            *soToken    = handle->token;
            break;
        }
    }
    release_lrt_objects_container_to_pool( matches );
    if ( !addr_found )
    {
        *soHandle = NULL;
        *soToken  = SCOREP_ADDR2LINE_SO_OBJECT_INVALID_TOKEN;
        *sclFound = false;
    }

#if HAVE( SCOREP_DEBUG )
    if ( *sclFound )
    {
        UTILS_DEBUG( "addr %" PRIuPTR ": %s@%s:%d [%s]",
                     programCounterAddr, *sclFunctionName, *sclFileName,
                     *sclLineNo, *soFileName );
    }
    else if ( *soHandle )
    {
        UTILS_DEBUG( "addr %" PRIuPTR " found in %s, but bfd lookup not successful",
                     programCounterAddr, *soFileName );
    }
    else
    {
        UTILS_DEBUG( "addr %" PRIuPTR " not contained in shared objects",
                     programCounterAddr );
    }
#endif /* HAVE( SCOREP_DEBUG ) */
}


void
SCOREP_Addr2line_LookupAddrRange( uintptr_t    beginProgramCounterAddr,
                                  uintptr_t    endProgramCounterAddr,
                                  /* shared object OUT parameters */
                                  void**       soHandle,
                                  const char** soFileName,
                                  uintptr_t*   soBaseAddr,
                                  uint16_t*    soToken,
                                  /* Source code location OUT parameters */
                                  bool*        sclFoundBegin,
                                  bool*        sclFoundEnd,
                                  const char** sclFileName,
                                  const char** sclFunctionName,
                                  unsigned*    sclBeginLineNo,
                                  unsigned*    sclEndLineNo )
{
    UTILS_BUG_ON( soHandle == NULL ||
                  soFileName == NULL ||
                  soBaseAddr == NULL ||
                  soToken == NULL ||
                  sclFoundBegin == NULL ||
                  sclFoundEnd == NULL ||
                  sclFileName == NULL ||
                  sclFunctionName == NULL ||
                  sclBeginLineNo == NULL ||
                  sclEndLineNo == NULL,
                  "Need valid OUT handles but NULL provided." );

    lrt_objects_container* matches = get_lrt_objects_container_from_pool();
    lookup_so( beginProgramCounterAddr, matches );
    bool addr_found = false;
    for ( size_t i = 0; i < matches->size; ++i )
    {
        lt_object*   handle = matches->objects[ i ];
        lookup_bfd_t data   = {
            .begin_addr           = beginProgramCounterAddr - handle->base_addr,
            .end_addr             = endProgramCounterAddr - handle->base_addr,
            .symbols              = handle->symbols,
            .scl_found_begin_addr = sclFoundBegin,
            .scl_found_end_addr   = sclFoundEnd,
            .scl_file_name        = sclFileName,
            .scl_function_name    = sclFunctionName,
            .scl_begin_lno        = sclBeginLineNo,
            .scl_end_lno          = sclEndLineNo
        };
        *( data.scl_found_begin_addr ) = false;
        *( data.scl_found_end_addr )   = false;

        map_over_sections_locked( handle, &data );
        if ( *sclFoundBegin &&  *sclFoundEnd )
        {
            addr_found  = true;
            *soHandle   = handle;
            *soFileName = handle->name;
            *soBaseAddr = handle->base_addr;
            *soToken    = handle->token;
            break;
        }
    }
    release_lrt_objects_container_to_pool( matches );
    if ( !addr_found )
    {
        *soHandle      = NULL;
        *soToken       = SCOREP_ADDR2LINE_SO_OBJECT_INVALID_TOKEN;
        *sclFoundBegin = false;
        *sclFoundEnd   = false;
    }

#if HAVE( SCOREP_DEBUG )
    if ( *sclFoundBegin )
    {
        if ( *sclFoundEnd )
        {
            UTILS_DEBUG( "addr range [%" PRIuPTR ", %" PRIuPTR "]: %s@%s:%d-%d [%s]",
                         beginProgramCounterAddr, endProgramCounterAddr,
                         *sclFunctionName, *sclFileName,
                         *sclBeginLineNo, *sclEndLineNo, *soFileName );
        }
        else
        {
            UTILS_DEBUG( "begin_addr of range [%" PRIuPTR ", %" PRIuPTR "]: %s@%s:%d [%s]",
                         beginProgramCounterAddr, endProgramCounterAddr,
                         *sclFunctionName, *sclFileName,
                         *sclBeginLineNo, *soFileName );
        }
    }
    else if ( *soHandle )
    {
        UTILS_DEBUG( "addr range [%" PRIuPTR ", %" PRIuPTR "] found in %s, "
                     "but bfd lookup not successful",
                     beginProgramCounterAddr, endProgramCounterAddr, *soFileName );
    }
    else
    {
        UTILS_DEBUG( "addr range [%" PRIuPTR ", %" PRIuPTR "] not contained "
                     "in shared objects",
                     beginProgramCounterAddr, endProgramCounterAddr );
    }
#endif /* HAVE( SCOREP_DEBUG ) */
}


/* Runtime-loaded shared object representation */
typedef struct rt_object rt_object;
struct rt_object
{
    SO_OBJECT_COMMON;
    uintptr_t  begin_addr;
    rt_object* next;         /* singly-linked list */
    uintptr_t  audit_cookie; /* identify dlopened object in dlclose */
};


/* Runtime-loaded shared objects correspond to a sorted singly linked list
   for lookup and a free list. List access and modification needs (RW) locking.
   We bfd-lookup (addr - base_addr) if addr in [begin_addr, end_addr]. */
/* Must not be static as accessed from ld_audit library */
rt_object* scorep_rt_objects_head     = NULL;
unsigned   scorep_rt_object_count     = 0;
uintptr_t  scorep_rt_objects_min_addr = UINTPTR_MAX;
uintptr_t  scorep_rt_objects_max_addr = 0;


/* free list of lrt_objects_container objects */
static lrt_objects_container* lrt_objects_container_free_list;
static UTILS_Mutex            lrt_objects_container_free_list_mutex = UTILS_MUTEX_INIT;


static lrt_objects_container*
get_lrt_objects_container_from_pool( void )
{
    bool unlock = false;
    if ( UTILS_Atomic_LoadN_uint32( &scorep_rt_object_count,
                                    UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) > 0 )
    {
        SCOREP_RWLock_ReaderLock( &scorep_rt_objects_rwlock.pending,
                                  &scorep_rt_objects_rwlock.release_n_readers );
        unlock = true;
    }
    size_t needed_capacity = UTILS_Atomic_LoadN_uint32( &n_overlapping_address_ranges,
                                                        UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) + 1;
    lrt_objects_container* container;

    UTILS_MutexLock( &lrt_objects_container_free_list_mutex );
    bool alloc_container_required = lrt_objects_container_free_list == NULL;
    if ( !alloc_container_required )
    {
        container                       =  lrt_objects_container_free_list;
        lrt_objects_container_free_list =  lrt_objects_container_free_list->next;
        if ( container->capacity < needed_capacity )
        {
            SCOREP_Memory_AlignedFree( container );
            alloc_container_required = true;
        }
    }
    if ( alloc_container_required )
    {
        container = SCOREP_Memory_AlignedMalloc( SCOREP_CACHELINESIZE,
                                                 sizeof( lrt_objects_container ) + needed_capacity * sizeof( lt_object* ) );
        container->capacity = needed_capacity;
    }
    container->size   = 0;
    container->next   = NULL;
    container->unlock = unlock;
    UTILS_MutexUnlock( &lrt_objects_container_free_list_mutex );

    UTILS_DEBUG( "container->capacity=%zu", container->capacity );
    return container;
}


static inline void
release_lrt_objects_container_to_pool( lrt_objects_container* container )
{
    if ( container->unlock )
    {
        SCOREP_RWLock_ReaderUnlock( &scorep_rt_objects_rwlock.pending,
                                    &scorep_rt_objects_rwlock.departing,
                                    &scorep_rt_objects_rwlock.release_writer );
    }
    /* return to pool */
    UTILS_MutexLock( &lrt_objects_container_free_list_mutex );
    container->next                 =  lrt_objects_container_free_list;
    lrt_objects_container_free_list = container;
    UTILS_MutexUnlock( &lrt_objects_container_free_list_mutex );
}


/* lt_object/rt_object address ranges might overlap. Thus, we need to
   map_over_sections-search all lt_object/rt_object that contain addr.
   Here we collect all matching objects in @a matches array that is
   guaranteed to be large enough. The actual search is done be the caller.
 */
static void
lookup_so( uintptr_t addr, lrt_objects_container* matches )
{
    /* search in loadtime objects */
    if ( lt_objects_min_addr <= addr && addr <= lt_objects_max_addr )
    {
        for ( size_t i = 0; i < lt_object_count; ++i )
        {
            if ( lt_begin_addrs[ i ] <= addr && addr <= lt_objects[ i ].end_addr )
            {
                /* found addr in loadtime objects */
                UTILS_BUG_ON( matches->size + 1 > matches->capacity,
                              "More address-ranges found than expected (%zu).",
                              matches->capacity );
                matches->objects[ matches->size++ ] = &lt_objects[ i ];
            }
        }
    }

    if ( matches->unlock ) /* search rt objects only if caller decided to */
    {
        /* search in runtime objects */
        if ( scorep_rt_objects_min_addr <= addr && addr <= scorep_rt_objects_max_addr )
        {
            rt_object* obj = scorep_rt_objects_head;
            while ( obj )
            {
                if ( obj->begin_addr <= addr && addr <= obj->end_addr )
                {
                    /* found addr in runtime objects */
                    UTILS_BUG_ON( matches->size + 1 > matches->capacity,
                                  "More address-ranges found than expected (%zu).",
                                  matches->capacity );
                    matches->objects[ matches->size++ ] = ( lt_object* )obj;
                }
                obj = obj->next;
            }
        }
    }
}


static void
section_iterator( bfd* abfd, asection* section, void* payload )
{
    lookup_bfd_t* data = payload;
    if ( *( data->scl_found_begin_addr ) )
    {
        return;
    }
    if ( !( bfd_get_section_flags( abfd, section ) & SEC_ALLOC ) )
    {
        return;
    }
    /* consider address if in [vma, vma+size] */
    bfd_vma vma = bfd_get_section_vma( abfd, section );
    if ( data->begin_addr < vma
         || ( data->end_addr != 0 && data->end_addr < vma ) )
    {
        return;
    }
    bfd_size_type size = bfd_get_section_size( section );
    if ( data->begin_addr >= vma + size
         || ( data->end_addr != 0
              && data->end_addr >= vma + size ) )
    {
        return;
    }
    *( data->scl_found_begin_addr ) = bfd_find_nearest_line( abfd,
                                                             section,
                                                             data->symbols,
                                                             data->begin_addr - vma,
                                                             data->scl_file_name,
                                                             data->scl_function_name,
                                                             data->scl_begin_lno );
    /* Calling bfd_find_inliner_info seems, after superficial testing,
       to not alter the result for function addressees provided by
       __cyg_profile_func_enter/exit. For callsite addressees, e.g.,
       provided via __builtin_return_address(0) or
       __cyg_profile_func_enter/exit's callsite parameter, walking the
       inline chain brings source code location information back from
       unexpected locations to user code. */
    while ( bfd_find_inliner_info( abfd,
                                   data->scl_file_name,
                                   data->scl_function_name,
                                   data->scl_begin_lno ) )
    {
    }

    if ( *( data->scl_found_begin_addr ) && data->end_addr != 0 )
    {
        const char* filename_unused;
        const char* funcname_unused;
        *( data->scl_found_end_addr ) = bfd_find_nearest_line( abfd,
                                                               section,
                                                               data->symbols,
                                                               data->end_addr - vma,
                                                               &filename_unused,
                                                               &funcname_unused,
                                                               data->scl_end_lno );
        while ( bfd_find_inliner_info( abfd,
                                       &filename_unused,
                                       &funcname_unused,
                                       data->scl_end_lno ) )
        {
        }
    }
    return;
}


/* Keep dlcosed objects in as they might be accessed via
   SCOREP_Addr2line_SoLookup* */
/* Must not be static as accessed from ld_audit library */
rt_object*  scorep_rt_objects_dlclosed_head  = NULL;
UTILS_Mutex scorep_rt_objects_dlclosed_mutex = UTILS_MUTEX_INIT;


void
SCOREP_Addr2line_Finalize( void )
{
    UTILS_DEBUG_ENTRY();

    if ( !addr2line_initialized )
    {
        return;
    }
    addr2line_initialized = false;

    bool        unused;
    const char* exe_name = SCOREP_GetExecutableName( &unused );
    for ( size_t i = 0; i < lt_object_count; i++ )
    {
        free( lt_objects[ i ].symbols );
        bfd_close( lt_objects[ i ].abfd );
        if ( strcmp( lt_objects[ i ].name, exe_name ) != 0 )
        {
            free( ( char* )lt_objects[ i ].name );
        }
    }
    free( lt_objects );
    lt_objects      = NULL;
    lt_object_count = 0;

    /* No need to lock as we are in serial context */
    while ( scorep_rt_objects_head )
    {
        free( scorep_rt_objects_head->symbols );
        bfd_close( scorep_rt_objects_head->abfd );
        free( ( char* )scorep_rt_objects_head->name );
        scorep_rt_objects_head = scorep_rt_objects_head->next;
    }
    scorep_rt_object_count = 0;

    while ( scorep_rt_objects_dlclosed_head )
    {
        free( scorep_rt_objects_dlclosed_head->symbols );
        bfd_close( scorep_rt_objects_dlclosed_head->abfd );
        free( ( char* )scorep_rt_objects_dlclosed_head->name );
        scorep_rt_objects_dlclosed_head = scorep_rt_objects_dlclosed_head->next;
    }

    UTILS_DEBUG_EXIT();
}


void
scorep_la_preinit( uintptr_t* cookie )
{
    /* Called after ctors. Load-time shared objects already loaded and
       dealt with at SCOREP_Addr2line_Initialize(). From now on,
       scorep_la_objopen and scorep_la_objclose are called. */
    UTILS_DEBUG_ENTRY();
}


/* dl_iterate_phdr helper */
typedef struct rt_obj_identifier rt_obj_identifier;
struct rt_obj_identifier
{
    size_t      cnt; /* ignore up to lt_objs_capacity objects */
    const char* name;
    uintptr_t   cookie;
};


/* *INDENT-OFF* */
static int insert_rt_object_cb( struct dl_phdr_info* info, size_t unused, void* data );
/* *INDENT-ON* */


/* called only after la_preinit() */
void
scorep_la_objopen( const char* name,
                   uintptr_t*  cookie /* corresponds to la_objclose */ )
{
    UTILS_DEBUG_ENTRY( "name=%s; cookie=%" PRIuPTR "; scorep_rt_object_count=%d",
                       name,  cookie != NULL ? *cookie : 0,
                       scorep_rt_object_count );

    /* iterate over all shared objects and compare against provided name */
    rt_obj_identifier search_for = { .cnt = 0, .name = name, .cookie = *cookie };
    dl_iterate_phdr( insert_rt_object_cb, &search_for );
}


#ifndef UINT8_WIDTH
#define UINT8_WIDTH 8
#endif
#define BITSET_NSLOTS( n_bits ) ( ( n_bits + UINT8_WIDTH - 1 ) / UINT8_WIDTH )
#define BITSET_MASK( bit ) ( UINT8_C( 1 ) << ( ( bit ) % UINT8_WIDTH ) )
#define BITSET_SLOT( bit ) ( ( bit ) / UINT8_WIDTH )


/* bitset to keep track of no more than MAX_RT_OBJOPEN_CALLS_TRACKED
   scorep_la_objopen() calls. The n-th calls to scorep_la_objopen()
   sets the n-th bit, while a corresponding call to
   scorep_la_objclose() clears the n-th bit. */
/* Must not be static as accessed from ld_audit library */
#define MAX_RT_OBJOPEN_CALLS_TRACKED ( UINT16_C( 1 ) << ( 13 ) )
SCOREP_ALIGNAS( SCOREP_CACHELINESIZE )
uint8_t scorep_rt_objects_loaded[ BITSET_NSLOTS( MAX_RT_OBJOPEN_CALLS_TRACKED ) ];


/* atomic bitset operations */
static inline void
bitset_set( uint8_t* bitset, uint16_t bit )
{
    UTILS_Atomic_OrFetch_uint8( &( bitset[ BITSET_SLOT( bit ) ] ),
                                BITSET_MASK( bit ),
                                UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
}


static inline void
bitset_clear( uint8_t* bitset, uint16_t bit )
{
    UTILS_Atomic_AndFetch_uint8( &( bitset[ BITSET_SLOT( bit ) ] ),
                                 ~BITSET_MASK( bit ),
                                 UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
}


static inline bool
bitset_test( uint8_t* bitset, uint16_t bit )
{
    return UTILS_Atomic_LoadN_uint8( &( bitset[ BITSET_SLOT( bit ) ] ),
                                     UTILS_ATOMIC_SEQUENTIAL_CONSISTENT )
           & BITSET_MASK( bit );
}


/* Must not be static as accessed from ld_audit library */
uint16_t scorep_rt_objopen_calls_tracked = 0;


static int
insert_rt_object_cb( struct dl_phdr_info* info, size_t unused, void* data )
{
    /* return 0 to continue iteration over shared objects, 1 to stop
       iteration. */
    rt_obj_identifier* id = ( rt_obj_identifier* )data;
    /* ignore loadtime objects */
    if ( id->cnt < lt_objs_capacity )
    {
        id->cnt++;
        return 0;
    }
    if ( strcmp( id->name, info->dlpi_name ) != 0 )
    {
        return 0;
    }
    if ( !is_obj_relevant( id->name ) )
    {
        return 0;
    }

    const char* name           = NULL;
    uintptr_t   base_addr      = 0;
    bfd*        abfd           = NULL;
    asymbol**   symbols        = NULL;
    uintptr_t   begin_addr_min = UINTPTR_MAX;
    uintptr_t   end_addr_max   = 0;
    bool        has_symbols    = iterate_segments( info,
                                                   &name,
                                                   &base_addr,
                                                   &abfd,
                                                   &symbols,
                                                   &begin_addr_min,
                                                   &end_addr_max,
                                                   false /* adjustName */ );
    if ( !has_symbols )
    {
        return 1;
    }

    /* create shared object representation */
    rt_object* new = SCOREP_Memory_AllocForMisc( sizeof( rt_object ) );
    memset( new, 0, sizeof( rt_object ) );

    new->end_addr     = end_addr_max;
    new->base_addr    = base_addr;
    new->abfd         = abfd;
    new->symbols      = symbols;
    new->name         = UTILS_CStr_dup( name );
    new->token        = ++scorep_rt_objopen_calls_tracked;
    new->abfd_mutex   = UTILS_MUTEX_INIT;
    new->begin_addr   = begin_addr_min;
    new->next         = NULL;
    new->audit_cookie = id->cookie;

    UTILS_BUG_ON( scorep_rt_objopen_calls_tracked >= MAX_RT_OBJOPEN_CALLS_TRACKED );
    bitset_set( scorep_rt_objects_loaded, new->token );

    SCOREP_RWLock_WriterLock( &scorep_rt_objects_rwlock.writer_mutex,
                              &scorep_rt_objects_rwlock.pending,
                              &scorep_rt_objects_rwlock.departing,
                              &scorep_rt_objects_rwlock.release_writer );

    /* Check for overlapping address ranges (no assumptions made). */
    rt_object* obj = scorep_rt_objects_head;
    while ( obj )
    {
        if ( obj->begin_addr <= new->end_addr
             && obj->end_addr >= new->begin_addr )
        {
            UTILS_Atomic_AddFetch_uint32( &n_overlapping_address_ranges, 1,
                                          UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
        }

        obj = obj->next;
    }

    /* Add shared object representation to list. No need to keep list
       sorted (e.g., by begin-address). As address ranges may overlap,
       we later on search all load-time and run-time objects. */
    new->next              = scorep_rt_objects_head;
    scorep_rt_objects_head = new;
    scorep_rt_object_count++;
    /* update rt address interval */
    if ( new->begin_addr < scorep_rt_objects_min_addr )
    {
        scorep_rt_objects_min_addr = new->begin_addr;
    }
    if ( new->end_addr > scorep_rt_objects_max_addr )
    {
        scorep_rt_objects_max_addr = new->end_addr;
    }
    SCOREP_RWLock_WriterUnlock( &scorep_rt_objects_rwlock.writer_mutex,
                                &scorep_rt_objects_rwlock.pending,
                                &scorep_rt_objects_rwlock.release_n_readers );

    UTILS_DEBUG( "Insert %s; base=%" PRIuPTR "; begin=%" PRIuPTR "; "
                 "end=%" PRIuPTR "; cookie=%" PRIuPTR "",
                 new->name, base_addr, begin_addr_min, end_addr_max,
                 new->audit_cookie );
    return 1;
}


/* Singly-linked list to store objclose callbacks */
/* Must not be static as accessed from ld_audit library */
typedef struct rt_objclose_cb rt_objclose_cb;
struct rt_objclose_cb
{
    SCOREP_Addr2line_ObjcloseCb cb;
    rt_objclose_cb*             next;
};
rt_objclose_cb* scorep_rt_objclose_cb_head  = NULL;
UTILS_Mutex     scorep_rt_objclose_cb_mutex = UTILS_MUTEX_INIT;


void
SCOREP_Addr2line_RegisterObjcloseCb( SCOREP_Addr2line_ObjcloseCb cb )
{
    rt_objclose_cb* new = SCOREP_Memory_AllocForMisc( sizeof( rt_objclose_cb ) );
    new->cb = cb;
    UTILS_MutexLock( &scorep_rt_objclose_cb_mutex );
    new->next                  = scorep_rt_objclose_cb_head;
    scorep_rt_objclose_cb_head = new;
    UTILS_MutexUnlock( &scorep_rt_objclose_cb_mutex );
}


/* called only after la_preinit() */
void
scorep_la_objclose( uintptr_t* cookie /* corresponds to la_objopen */ )
{
    UTILS_DEBUG_ENTRY( "cookie=%" PRIuPTR "; scorep_rt_object_count=%d",
                       cookie != NULL ? *cookie : 0, scorep_rt_object_count );

    /* ignore objclose for loadtime objects */
    if ( scorep_rt_object_count > 0 )
    {
        UTILS_BUG_ON( !scorep_rt_objects_head );

        rt_object* remove = NULL;
        SCOREP_RWLock_WriterLock( &scorep_rt_objects_rwlock.writer_mutex,
                                  &scorep_rt_objects_rwlock.pending,
                                  &scorep_rt_objects_rwlock.departing,
                                  &scorep_rt_objects_rwlock.release_writer );
        rt_object** obj = &scorep_rt_objects_head;
        while ( *obj && ( *obj )->audit_cookie != *cookie )
        {
            obj = &( *obj )->next;
        }
        if ( !*obj || ( *obj )->audit_cookie != *cookie )
        {
            UTILS_DEBUG_EXIT( "Audit cookie %ld not found. Likely that it provided no symbols",
                              *cookie );
            SCOREP_RWLock_WriterUnlock( &scorep_rt_objects_rwlock.writer_mutex,
                                        &scorep_rt_objects_rwlock.pending,
                                        &scorep_rt_objects_rwlock.release_n_readers );
            return;
        }

        remove = *obj;
        *obj   = ( *obj )->next;

        /* trigger objclose callbacks */
        UTILS_MutexLock( &scorep_rt_objclose_cb_mutex );
        rt_objclose_cb* objclose_cb = scorep_rt_objclose_cb_head;
        while ( objclose_cb )
        {
            objclose_cb->cb( remove, remove->name, remove->base_addr, remove->token );
            objclose_cb = objclose_cb->next;
        }
        UTILS_MutexUnlock( &scorep_rt_objclose_cb_mutex );

        bitset_clear( scorep_rt_objects_loaded, remove->token );
        scorep_rt_object_count--;
        /* update rt address interval */
        if ( remove->begin_addr == scorep_rt_objects_min_addr
             || remove->end_addr == scorep_rt_objects_max_addr )
        {
            scorep_rt_objects_min_addr = UINTPTR_MAX;
            scorep_rt_objects_max_addr = 0;
            rt_object* rto = scorep_rt_objects_head;
            while ( rto )
            {
                if ( rto->begin_addr < scorep_rt_objects_min_addr )
                {
                    scorep_rt_objects_min_addr = rto->begin_addr;
                }
                if ( rto->end_addr > scorep_rt_objects_max_addr )
                {
                    scorep_rt_objects_max_addr = rto->end_addr;
                }
                rto = rto->next;
            }
        }
        SCOREP_RWLock_WriterUnlock( &scorep_rt_objects_rwlock.writer_mutex,
                                    &scorep_rt_objects_rwlock.pending,
                                    &scorep_rt_objects_rwlock.release_n_readers );

        UTILS_DEBUG( "dlclose %s; cookie=%" PRIuPTR "", remove->name, *cookie );

        /* keep dlclosed object representations, potentially accessed via
           SCOREP_Addr2line_SoLookup* later on */
        UTILS_MutexLock( &scorep_rt_objects_dlclosed_mutex );
        remove->next                    = scorep_rt_objects_dlclosed_head;
        scorep_rt_objects_dlclosed_head = remove;
        UTILS_MutexUnlock( &scorep_rt_objects_dlclosed_mutex );
    }
}


bool
scorep_addr2line_so_still_loaded( uint16_t soToken )
{
    UTILS_BUG_ON( soToken >= MAX_RT_OBJOPEN_CALLS_TRACKED,
                  "soToken %" PRIu16 " not in range [0, %d]",
                  soToken, MAX_RT_OBJOPEN_CALLS_TRACKED - 1 );
    return bitset_test( scorep_rt_objects_loaded, soToken );
}
