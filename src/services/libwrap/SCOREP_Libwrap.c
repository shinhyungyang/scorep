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

#include <SCOREP_Config.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_InMeasurement.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Filtering.h>
#include <SCOREP_Types.h>
#include <SCOREP_Events.h>
#include <SCOREP_Subsystem.h>
#include <scorep/SCOREP_Libwrap.h>

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

#if HAVE( LIBWRAP_PLUGIN_SUPPORT )
#include <dlfcn.h>
#endif

struct scorep_gotcha_handle
{
    struct scorep_gotcha_handle* next;
    gotcha_wrappee_handle_t      wrappee_handle;
    gotcha_binding_t             wrap_actions;
    char                         name[];
};

/** Data structure for library wrapper handle */
struct SCOREP_LibwrapHandle
{
    const SCOREP_LibwrapAttributes* attributes;
    SCOREP_LibwrapHandle*           next;
    UTILS_Mutex                     lock;
    struct scorep_gotcha_handle*    gotcha_head;
    struct scorep_gotcha_handle**   gotcha_tail;
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

/**
 * This function will free the allocated memory and delete the wrapper
 * handle.
 *
 * @param handle            Library wrapper handle
 */
static void
delete_libwrap_handle( SCOREP_LibwrapHandle* handle )
{
    UTILS_ASSERT( handle );

    while ( handle->gotcha_head )
    {
        struct scorep_gotcha_handle* gotcha_handle = handle->gotcha_head;
        handle->gotcha_head = gotcha_handle->next;
        free( gotcha_handle );
    }
    handle->gotcha_tail = &handle->gotcha_head;
}


static void
libwrap_subsystem_finalize( void )
{
    while ( libwrap_handles != NULL )
    {
        SCOREP_LibwrapHandle* temp = libwrap_handles;
        libwrap_handles = temp->next;

        delete_libwrap_handle( temp );
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
    SCOREP_LibwrapHandle* handle = calloc( 1, sizeof( *handle ) );
    UTILS_ASSERT( handle );

    /* Initialize the new library wrapper handle */
    handle->attributes  = attributes;
    handle->gotcha_head = NULL;
    handle->gotcha_tail = &handle->gotcha_head;

    if ( attributes->init )
    {
        attributes->init( handle );
    }

    /* Enqueue new library wrapper handle */
    handle->next    = libwrap_handles;
    libwrap_handles = handle;

    /* Wrapper was successfully created, make the result visible to others. */
    UTILS_Atomic_StoreN_void_ptr( outHandle, handle, UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );

    UTILS_MutexUnlock( &libwrap_object_lock );

    UTILS_DEBUG_EXIT();
}

SCOREP_LibwrapEnableErrorCode
SCOREP_Libwrap_EnableWrapper( SCOREP_LibwrapHandle* handle,
                              const char*           prettyName,
                              const char*           symbolName,
                              const char*           file,
                              int                   line,
                              SCOREP_ParadigmType   paradigm,
                              SCOREP_RegionType     regionType,
                              void*                 wrapper,
                              void**                funcPtrOut,
                              SCOREP_RegionHandle*  region )
{
    UTILS_ASSERT( handle && symbolName && wrapper && funcPtrOut );

    /* shoule only be called once */
    if ( NULL != UTILS_Atomic_LoadN_void_ptr( funcPtrOut,
                                              UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) )
    {
        return SCOREP_LIBWRAP_ENABLED_SUCCESS;
    }

    UTILS_MutexLock( &handle->lock );

    if ( NULL != UTILS_Atomic_LoadN_void_ptr( funcPtrOut,
                                              UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) )
    {
        UTILS_MutexUnlock( &handle->lock );
        return SCOREP_LIBWRAP_ENABLED_SUCCESS;
    }

    UTILS_DEBUG_ENTRY( "%s, %s, %s:%d", handle->attributes->name, symbolName, file, line );

    size_t                       funcname_length = strlen( symbolName ) + 1;
    struct scorep_gotcha_handle* gotcha_handle   =
        calloc( 1, sizeof( *gotcha_handle ) + funcname_length );
    memcpy( gotcha_handle->name, symbolName, funcname_length );
    gotcha_handle->next = NULL;

    gotcha_binding_t* wrap_actions = &gotcha_handle->wrap_actions;
    wrap_actions->name            = gotcha_handle->name;
    wrap_actions->wrapper_pointer = wrapper;
    wrap_actions->function_handle = &gotcha_handle->wrappee_handle;
    enum gotcha_error_t ret = gotcha_wrap( wrap_actions, 1, "Score-P" );
    if ( GOTCHA_INTERNAL == ret )
    {
        /* some internal error, wrapping not done and wrap_actions not referenced */
        free( gotcha_handle );
        UTILS_MutexUnlock( &handle->lock );
        UTILS_DEBUG_EXIT( "gotcha_wrap failed" );
        return SCOREP_LIBWRAP_ENABLED_ERROR_NOT_WRAPPED;
    }
    /* wrap_actions now referenced in GOTCHA internal data structures */

    *handle->gotcha_tail = gotcha_handle;
    handle->gotcha_tail  = &gotcha_handle->next;

    if ( region )
    {
        *region = SCOREP_Definitions_NewRegion( prettyName,
                                                symbolName,
                                                file ? SCOREP_Definitions_NewSourceFile( file )
                                                : SCOREP_INVALID_SOURCE_FILE,
                                                line,
                                                SCOREP_INVALID_LINE_NO,
                                                paradigm,
                                                regionType );
    }

    if ( ret != GOTCHA_FUNCTION_NOT_FOUND )
    {
        UTILS_Atomic_StoreN_void_ptr( funcPtrOut,
                                      gotcha_get_wrappee( gotcha_handle->wrappee_handle ),
                                      UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
    }

    UTILS_MutexUnlock( &handle->lock );

    UTILS_DEBUG_EXIT();

    return SCOREP_LIBWRAP_ENABLED_SUCCESS;
}

/* ****************************************************************** */
/* Plug-in API                                                        */
/* ****************************************************************** */

static SCOREP_LibwrapEnableErrorCode
libwrap_plugin_api_enable_wrapper( SCOREP_LibwrapHandle* handle,
                                   const char*           prettyName,
                                   const char*           symbolName,
                                   const char*           file,
                                   int                   line,
                                   void*                 wrapper,
                                   void**                funcPtrOut,
                                   SCOREP_RegionHandle*  region )
{
    if ( !handle || !symbolName || !wrapper || !funcPtrOut || !region )
    {
        return SCOREP_LIBWRAP_ENABLED_ERROR_INVALID_ARGUMENTS;
    }

    if ( SCOREP_Filtering_Match( file, prettyName, symbolName ) )
    {
        return SCOREP_LIBWRAP_ENABLED_FILTERED;
    }

    SCOREP_LibwrapEnableErrorCode ret = SCOREP_Libwrap_EnableWrapper(
        handle,
        prettyName,
        symbolName,
        file,
        line,
        SCOREP_PARADIGM_LIBWRAP,
        SCOREP_REGION_WRAPPER,
        wrapper,
        funcPtrOut,
        region );
    if ( ret == SCOREP_LIBWRAP_ENABLED_SUCCESS )
    {
        SCOREP_RegionHandle_SetGroup( *region, handle->attributes->display_name );
    }

    return ret;
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
    .enable_wrapper       = libwrap_plugin_api_enable_wrapper,
    .enter_measurement    = libwrap_plugin_api_enter_measurement,
    .exit_measurement     = libwrap_plugin_api_exit_measurement,
    .enter_region         = libwrap_plugin_api_enter_region,
    .exit_region          = libwrap_plugin_api_exit_region,
    .enter_wrapped_region = libwrap_plugin_api_enter_wrapped_region,
    .exit_wrapped_region  = libwrap_plugin_api_exit_wrapped_region
};
