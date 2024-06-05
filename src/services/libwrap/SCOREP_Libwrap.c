/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014, 2017, 2020, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2022,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * @brief Handling of symbols in shared libraries, for shared library wrapping
 */

#include <config.h>

#include <SCOREP_Libwrap_Internal.h>

#include <scorep/SCOREP_Libwrap_Plugins.h>

#include <SCOREP_Config.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Filtering.h>
#include <SCOREP_Types.h>
#include <SCOREP_Events.h>
#include <SCOREP_Subsystem.h>

#include <UTILS_Error.h>
#include <UTILS_CStr.h>
#include <UTILS_Atomic.h>
#include <UTILS_Mutex.h>
#include <UTILS_IO.h>

#define SCOREP_DEBUG_MODULE_NAME LIBWRAP
#include <UTILS_Debug.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <gotcha/gotcha.h>
#if !defined( GOTCHA_VERSION )
// introduced in 1.0.7, we check only against 1.0.7, hence we can force values
// which evaluate to false
#define GOTCHA_VERSION 0
#define GOTCHA_GET_VERSION( x, y, z ) 1
#endif

#if HAVE( LIBWRAP_PLUGIN_SUPPORT )
#include <dlfcn.h>
#endif
#include <link.h>

enum wrapping_state
{
    WRAPPING_DISABLED = 0,
    WRAPPING_ENABLED
};

/** Data structure for library wrapper handle */
struct SCOREP_LibwrapHandle
{
    const SCOREP_LibwrapAttributes* attributes;
    SCOREP_LibwrapHandle*           next;
    UTILS_Mutex                     lock;
    enum wrapping_state             wrapping_state;
    gotcha_binding_t*               gotcha_bindings;
    size_t                          gotcha_bindings_actions;  /* in #elements */
    size_t                          gotcha_bindings_capacity; /* in bytes */
    char                            gotcha_tool_name[];
};

/** Library wrapper handles */
static SCOREP_LibwrapHandle* libwrap_handles;

/** Lock for definitions within Score-P library wrapping infrastructure */
static UTILS_Mutex libwrap_object_lock;

static size_t libwrap_subsystem_id;

static char** libwrap_path;

extern const SCOREP_LibwrapAPI scorep_libwrap_plugin_api;

#include "scorep_libwrap_confvars.inc.c"

/* ****************************************************************** */
/* Filtering                                                          */
/* ****************************************************************** */

/* Only "helper" libraries, where we do not expect to get events from anyway */
const static char* filtered_libraries[] =
{
    "/libscorep_",
    "/libotf2.",
    "/libcube4w.",
    "/libgotcha.",
    "/libunwind.",
    "/libhwloc.",
    "/libpfm.",
    "/libpapi.",
    "/libsde.",

    /*
     * Interfers with the libOpenCL ICD dispatcher.
     * Solved in 1.0.7
     * https://github.com/LLNL/GOTCHA/issues/146
     */
/* *INDENT-OFF* */ /* GOTCHA_GET_VERSION( 1,0, 7 ), seriously? */
#if GOTCHA_VERSION < GOTCHA_GET_VERSION( 1, 0, 7 )
    "libOpenCL",
#endif
/* *INDENT-ON* */
    NULL
};

static int
libwrap_library_exclude_filter( struct link_map* target )
{
    const char** it = filtered_libraries;
    while ( *it )
    {
        if ( strstr( target->l_name, *it ) != NULL )
        {
            // filter this library
            return 0;
        }
        it++;
    }

    return 1;
}

/* ****************************************************************** */
/* Management                                                         */
/* ****************************************************************** */

static SCOREP_ErrorCode
libwrap_subsystem_register( size_t subsystemId )
{
    libwrap_subsystem_id = subsystemId;

    return SCOREP_ConfigRegister( "libwrap",
                                  scorep_libwrap_confvars );
}

static void
libwrap_subsystem_deregister( void )
{
    free( libwrap_confvar_path );
    libwrap_confvar_path = NULL;
    free( libwrap_confvar_enable );
    libwrap_confvar_enable = NULL;
    free( libwrap_confvar_enable_sep );
    libwrap_confvar_enable_sep = NULL;

    free( libwrap_path );
    libwrap_path = NULL;
}

#if HAVE( LIBWRAP_PLUGIN_SUPPORT )

static SCOREP_ErrorCode
prepare_libwrap_path( void )
{
    const char* scorep_prefix = SCOREP_PREFIX;
    size_t      path_copy_len = strlen( libwrap_confvar_path ) + 1 + strlen( scorep_prefix ) + 1;
    char*       path_copy     = malloc( path_copy_len );
    if ( !path_copy )
    {
        UTILS_ERROR_POSIX();
        return SCOREP_ERROR_MEM_ALLOC_FAILED;
    }
    snprintf( path_copy, path_copy_len, "%s:%s", libwrap_confvar_path, scorep_prefix );

    char*  entry;
    size_t num_entries      = 0;
    char*  value_for_strtok = path_copy;
    while ( ( entry = strtok( value_for_strtok, ":" ) ) )
    {
        /* all but the first call to strtok should be NULL */
        value_for_strtok = NULL;

        if ( 0 == strlen( entry ) )
        {
            continue;
        }

        if ( entry[ 0 ] != '/' )
        {
            UTILS_WARNING( "Entry in SCOREP_LIBWRAP_PATH is not an absolute path: '%s'",
                           entry );
            continue;
        }

        num_entries++;
    }

    libwrap_path = calloc( num_entries + 1, sizeof( *libwrap_path ) );
    if ( !libwrap_path )
    {
        UTILS_ERROR_POSIX();
        free( path_copy );
        return SCOREP_ERROR_MEM_ALLOC_FAILED;
    }

    /* reset value and start second pass */
    snprintf( path_copy, path_copy_len, "%s:%s", libwrap_confvar_path, scorep_prefix );
    num_entries      = 0;
    value_for_strtok = path_copy;
    while ( ( entry = strtok( value_for_strtok, ":" ) ) )
    {
        /* all but the first call to strtok should be NULL */
        value_for_strtok = NULL;

        if ( 0 == strlen( entry ) )
        {
            continue;
        }

        if ( entry[ 0 ] != '/' )
        {
            continue;
        }

        UTILS_IO_SimplifyPath( entry );
        libwrap_path[ num_entries++ ] = entry;
    }

    return SCOREP_SUCCESS;
}

static SCOREP_ErrorCode
load_plugin( const char* path )
{
    UTILS_DEBUG_ENTRY( "%s", path );

    void* dl_handle = dlopen( path, RTLD_NOW );

    /* If it is not valid */
    char* dl_lib_error = dlerror();
    if ( dl_lib_error != NULL )
    {
        UTILS_WARNING( "Could not open library wrapper plugin '%s'. Error message was: %s",
                       path, dl_lib_error );
        return SCOREP_ERROR_INVALID_ARGUMENT;
    }

    /* Union to avoid casting and compiler warnings */
    union
    {
        void* void_ptr;
        void  ( * init )( const SCOREP_LibwrapAPI*,
                          size_t );
    } init_function;

    init_function.void_ptr = dlsym( dl_handle, "scorep_libwrap_plugin" );
    dl_lib_error           = dlerror();
    if ( dl_lib_error != NULL )
    {
        UTILS_WARNING( "Could not find symbol 'scorep_libwrap_plugin' of library wrapper plugin %s. Error message was: %s",
                       path, dl_lib_error );
        dlclose( dl_handle );
        return SCOREP_ERROR_INVALID_ARGUMENT;
    }

    init_function.init( &scorep_libwrap_plugin_api, sizeof( scorep_libwrap_plugin_api ) );

    return SCOREP_SUCCESS;
}
#endif

static SCOREP_ErrorCode
libwrap_subsystem_initialize( void )
{
    UTILS_DEBUG_ENTRY();

    gotcha_set_library_filter_func( libwrap_library_exclude_filter );

    SCOREP_ErrorCode ret = SCOREP_SUCCESS;

#if HAVE( LIBWRAP_PLUGIN_SUPPORT )
    ret = prepare_libwrap_path();
    if ( ret != SCOREP_SUCCESS )
    {
        return ret;
    }

    char* enable_copy = UTILS_CStr_dup( libwrap_confvar_enable );
    if ( !enable_copy )
    {
        UTILS_ERROR_POSIX();
        free( libwrap_path );
        return SCOREP_ERROR_MEM_ALLOC_FAILED;
    }

    char* entry;
    char* value_for_strtok = enable_copy;
    while ( ( entry = strtok( value_for_strtok, libwrap_confvar_enable_sep ) ) )
    {
        /* all but the first call to strtok should be NULL */
        value_for_strtok = NULL;

        if ( 0 == strlen( entry ) )
        {
            continue;
        }

        if ( UTILS_IO_HasPath( entry ) )
        {
            const char* full_path = UTILS_IO_JoinPath( 2,
                                                       SCOREP_GetWorkingDirectory(),
                                                       entry );
            ret = load_plugin( full_path );
        }
        else
        {
            char plugin_name[ 128 ];
            snprintf( plugin_name, 128, "libscorep_libwrap_%s.so", entry );

            char** path = libwrap_path;
            while ( *path )
            {
                char* full_path = UTILS_IO_JoinPath( 3, *path, "lib" SCOREP_BACKEND_SUFFIX, plugin_name );
                ret = load_plugin( full_path );
                free( full_path );
                if ( ret == SCOREP_SUCCESS )
                {
                    break;
                }
                path++;
            }
        }
    }
#endif

    return ret;
}

static void
libwrap_subsystem_finalize( void )
{
    while ( libwrap_handles != NULL )
    {
        SCOREP_LibwrapHandle* temp = libwrap_handles;
        libwrap_handles = temp->next;

        free( temp->gotcha_bindings );
        free( temp );
    }
}

const SCOREP_Subsystem SCOREP_Subsystem_LibwrapService =
{
    .subsystem_name       = "Libwrap",
    .subsystem_register   = libwrap_subsystem_register,
    .subsystem_deregister = libwrap_subsystem_deregister,
    .subsystem_init       = libwrap_subsystem_initialize,
    .subsystem_finalize   = libwrap_subsystem_finalize
};

/* ****************************************************************** */
/* Internal API                                                       */
/* ****************************************************************** */

void
SCOREP_Libwrap_Enable( SCOREP_LibwrapHandle* handle )
{
    UTILS_ASSERT( handle );
    UTILS_DEBUG_ENTRY( "%s with %zu wrappers",
                       handle->attributes->display_name, handle->gotcha_bindings_actions );

    UTILS_MutexLock( &handle->lock );

    UTILS_BUG_ON( handle->wrapping_state == WRAPPING_ENABLED,
                  "Enabling the already enabled libwrap handle %s",
                  handle->attributes->display_name );

    enum gotcha_error_t ret = gotcha_wrap( handle->gotcha_bindings,
                                           handle->gotcha_bindings_actions,
                                           handle->gotcha_tool_name );
    UTILS_BUG_ON( GOTCHA_INTERNAL == ret,
                  "Unexpected failure when enabling library wrapper %s",
                  handle->attributes->display_name );

    /*
     * gotcha_wrap might return GOTCHA_FUNCTION_NOT_FOUND, but still tries to wrap
     * all symbols, FOUND is only required if the original is actually called.
     */

    handle->wrapping_state = WRAPPING_ENABLED;

    UTILS_MutexUnlock( &handle->lock );

    UTILS_DEBUG_EXIT();
}

void
SCOREP_Libwrap_Create( SCOREP_LibwrapHandle**          outHandle,
                       const SCOREP_LibwrapAttributes* attributes )
{
    if ( !outHandle || !attributes )
    {
        UTILS_ERROR( SCOREP_ERROR_INVALID_ARGUMENT, "NULL arguments" );
        return;
    }

    UTILS_DEBUG_ENTRY( "%s", attributes->name );

    /* ABI break with version 2, cannot process version <= 1 */
    if ( attributes->version != SCOREP_LIBWRAP_VERSION )
    {
        UTILS_FATAL( "Incompatible API/ABI version for library wrapper '%s' (expected: %d, actual: %d)",
                     attributes->name,
                     SCOREP_LIBWRAP_VERSION,
                     attributes->version );
    }

    if ( NULL != UTILS_Atomic_LoadN_void_ptr( outHandle,
                                              UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) )
    {
        return;
    }

    UTILS_MutexLock( &libwrap_object_lock );

    if ( NULL != UTILS_Atomic_LoadN_void_ptr( outHandle,
                                              UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) )
    {
        UTILS_MutexUnlock( &libwrap_object_lock );
        return;
    }

    /*
     * Get new library wrapper handle. Do not yet assign it to outHandle,
     * only after it is finished. This prevents a race if multiple threads try
     * to create the wrapper. Other threads may call libwrap_plugin_api_create
     * only if the handle is still NULL.
     */
    size_t                tool_name_length = strlen( "Score-P ()" ) + strlen( attributes->name ) + 1;
    SCOREP_LibwrapHandle* handle           = calloc( 1, sizeof( *handle ) + tool_name_length );
    UTILS_ASSERT( handle );

    snprintf( handle->gotcha_tool_name, tool_name_length, "Score-P (%s)", attributes->name );

    /* Initialize the new library wrapper handle */
    handle->attributes = attributes;

    if ( attributes->init )
    {
        attributes->init( handle );
        SCOREP_Libwrap_Enable( handle );
    }

    /* Enqueue new library wrapper handle */
    handle->next    = libwrap_handles;
    libwrap_handles = handle;

    /* Wrapper was successfully created, make the result visible to others. */
    UTILS_Atomic_StoreN_void_ptr( outHandle, handle, UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );

    UTILS_MutexUnlock( &libwrap_object_lock );

    UTILS_DEBUG_EXIT();
}

void
SCOREP_Libwrap_RegisterWrapper( SCOREP_LibwrapHandle*          handle,
                                const char*                    prettyName,
                                const char*                    symbolName,
                                const char*                    file,
                                int                            line,
                                SCOREP_ParadigmType            paradigm,
                                SCOREP_RegionType              regionType,
                                void*                          wrapper,
                                SCOREP_Libwrap_OriginalHandle* originalHandleOut,
                                SCOREP_RegionHandle*           regionOut )
{
    UTILS_ASSERT( handle && symbolName && wrapper && originalHandleOut );

    UTILS_MutexLock( &handle->lock );

    UTILS_BUG_ON( handle->wrapping_state == WRAPPING_ENABLED,
                  "Registering a wrapper to the already enabled libwrap handle %s",
                  handle->attributes->display_name );

    UTILS_DEBUG_ENTRY( "%s, %s, %s:%d", handle->attributes->name, symbolName, file, line );
    if ( regionOut )
    {
        *regionOut = SCOREP_Definitions_NewRegion( prettyName,
                                                   symbolName,
                                                   file ? SCOREP_Definitions_NewSourceFile( file )
                                                   : SCOREP_INVALID_SOURCE_FILE,
                                                   line,
                                                   SCOREP_INVALID_LINE_NO,
                                                   paradigm,
                                                   regionType );
        symbolName = SCOREP_RegionHandle_GetCanonicalName( *regionOut );
    }
    else
    {
        SCOREP_StringHandle symbol_handle = SCOREP_Definitions_NewString( symbolName );
        symbolName = SCOREP_StringHandle_Get( symbol_handle );
    }

    if ( handle->gotcha_bindings_actions == ( handle->gotcha_bindings_capacity / sizeof( *handle->gotcha_bindings ) ) )
    {
        /* I/O and OpenCL have 100<#symbols<170 each.
           4096 provides 170 elements on LP64, hence only one allocation needed. */
        handle->gotcha_bindings_capacity += 4096;
        handle->gotcha_bindings           = realloc( handle->gotcha_bindings,
                                                     handle->gotcha_bindings_capacity );
        UTILS_ASSERT( handle->gotcha_bindings );
    }

    gotcha_binding_t* wrap_actions = &handle->gotcha_bindings[ handle->gotcha_bindings_actions++ ];
    wrap_actions->name            = symbolName;
    wrap_actions->wrapper_pointer = wrapper;
    wrap_actions->function_handle = originalHandleOut;

    UTILS_MutexUnlock( &handle->lock );

    UTILS_DEBUG_EXIT();
}

/* ****************************************************************** */
/* Plug-in API                                                        */
/* ****************************************************************** */

static void
libwrap_plugin_api_register_wrapper( SCOREP_LibwrapHandle* handle,
                                     const char*           prettyName,
                                     const char*           symbolName,
                                     const char*           file,
                                     int                   line,
                                     void*                 wrapper,
                                     void**                originalHandleOut,
                                     SCOREP_RegionHandle*  regionOut )
{
    if ( !handle || !symbolName || !wrapper || !originalHandleOut || !regionOut )
    {
        return;
    }

    if ( SCOREP_Filtering_Match( file, prettyName, symbolName ) )
    {
        return;
    }

    SCOREP_Libwrap_RegisterWrapper(
        handle,
        prettyName,
        symbolName,
        file,
        line,
        SCOREP_PARADIGM_LIBWRAP,
        SCOREP_REGION_WRAPPER,
        wrapper,
        originalHandleOut,
        regionOut );
    SCOREP_RegionHandle_SetGroup( *regionOut, handle->attributes->display_name );
}

static void*
libwrap_plugin_api_get_original( SCOREP_Libwrap_OriginalHandle originalHandle )
{
    return gotcha_get_wrappee( originalHandle );
}

static int
libwrap_plugin_api_enter_measurement( void )
{
    return SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT()
           && SCOREP_IS_MEASUREMENT_PHASE( WITHIN );
}

static void
libwrap_plugin_api_exit_measurement( void )
{
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

static void
libwrap_plugin_api_enter_region( SCOREP_RegionHandle region )
{
    /* The increment is needed, to account this function also as part of the wrapper */
#if HAVE( RETURN_ADDRESS )
    SCOREP_IN_MEASUREMENT_INCREMENT();
#endif
    SCOREP_EnterWrappedRegion( region );
#if HAVE( RETURN_ADDRESS )
    SCOREP_IN_MEASUREMENT_DECREMENT();
#endif
}

static void
libwrap_plugin_api_exit_region( SCOREP_RegionHandle region )
{
    SCOREP_ExitRegion( region );
}

static int
libwrap_plugin_api_enter_wrapped_region( void )
{
    SCOREP_ENTER_WRAPPED_REGION();
#if HAVE( THREAD_LOCAL_STORAGE )
    return scorep_in_measurement_save;
#else /* !HAVE( THREAD_LOCAL_STORAGE ) */
    return 0;
#endif
}

static void
libwrap_plugin_api_exit_wrapped_region( int previous )
{
#if HAVE( THREAD_LOCAL_STORAGE )
    sig_atomic_t scorep_in_measurement_save = previous;
#endif
    SCOREP_EXIT_WRAPPED_REGION();
}

const SCOREP_LibwrapAPI scorep_libwrap_plugin_api =
{
    .create               = SCOREP_Libwrap_Create,
    .register_wrapper     = libwrap_plugin_api_register_wrapper,
    .get_original         = libwrap_plugin_api_get_original,
    .enter_measurement    = libwrap_plugin_api_enter_measurement,
    .exit_measurement     = libwrap_plugin_api_exit_measurement,
    .enter_region         = libwrap_plugin_api_enter_region,
    .exit_region          = libwrap_plugin_api_exit_region,
    .enter_wrapped_region = libwrap_plugin_api_enter_wrapped_region,
    .exit_wrapped_region  = libwrap_plugin_api_exit_wrapped_region
};
