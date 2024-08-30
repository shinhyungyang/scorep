/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 *  @brief Implementation of the CUDA NVTX API for Score-P
 *
 *  @file
 */

#include <config.h>

#if HAVE( NVTX_SUPPORT )

#include "scorep_cuda_nvtx_mgmt.h"

#include <SCOREP_InMeasurement.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Events.h>
#include <SCOREP_Filtering.h>
#include <SCOREP_Task.h>

/*************** Macros *******************************************************/

#define SCOREP_CUDA_NVTX_MAKE_ASCII_ATTRIBUTES( message ) \
    { \
        .version       = NVTX_VERSION, \
        .size          = NVTX_EVENT_ATTRIB_STRUCT_SIZE, \
        .messageType   = NVTX_MESSAGE_TYPE_ASCII, \
        .message.ascii = message \
    }

#define SCOREP_CUDA_NVTX_MAKE_UNICODE_ATTRIBUTES( message ) \
    { \
        .version         = NVTX_VERSION, \
        .size            = NVTX_EVENT_ATTRIB_STRUCT_SIZE, \
        .messageType     = NVTX_MESSAGE_TYPE_UNICODE, \
        .message.unicode = message \
    }

static UTILS_Mutex scorep_nvtx_mutex;
#define SCOREP_NVTX_LOCK() UTILS_MutexLock( &scorep_nvtx_mutex )
#define SCOREP_NVTX_UNLOCK() UTILS_MutexUnlock( &scorep_nvtx_mutex )

#if HAVE( NVTX_V3 )
#define DECLARE_NVTX_WRAPPER( returnType, origSignature, ... ) \
    static returnType \
    scorep_cuda_ ## origSignature( __VA_ARGS__ )
#define CALL_NVTX_WRAPPER( call ) scorep_cuda_ ## call
#else
#define DECLARE_NVTX_WRAPPER( returnType, origSignature, ... ) \
    NVTX_DECLSPEC returnType NVTX_API \
    origSignature( __VA_ARGS__ )
#define CALL_NVTX_WRAPPER( call ) call
#endif

/*************** Mark functions ***********************************************/

DECLARE_NVTX_WRAPPER( void,
                      nvtxDomainMarkEx,
                      nvtxDomainHandle_t domain,
                      const nvtxEventAttributes_t * eventAttrib )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    const char* name = scorep_cuda_nvtx_get_name_from_attributes( eventAttrib );
    if ( SCOREP_Filtering_MatchFunction( name, NULL ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    SCOREP_RegionHandle region_handle = scorep_cuda_nvtx_get_user_region( domain, eventAttrib );

    scorep_cuda_nvtx_apply_payload( eventAttrib );
    SCOREP_EnterRegion( region_handle );
    scorep_cuda_nvtx_apply_category( domain, eventAttrib );
    SCOREP_ExitRegion( region_handle );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxMarkEx,
                      const nvtxEventAttributes_t * eventAttrib )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    CALL_NVTX_WRAPPER( nvtxDomainMarkEx( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN, eventAttrib ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void, nvtxMarkA, const char* message )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxEventAttributes_t attributes = SCOREP_CUDA_NVTX_MAKE_ASCII_ATTRIBUTES( message );
    CALL_NVTX_WRAPPER( nvtxMarkEx( &attributes ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxMarkW,
                      const wchar_t* message )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxEventAttributes_t attributes = SCOREP_CUDA_NVTX_MAKE_UNICODE_ATTRIBUTES( message );
    CALL_NVTX_WRAPPER( nvtxMarkEx( &attributes ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Range start/stop functions ***********************************/

DECLARE_NVTX_WRAPPER( nvtxRangeId_t,
                      nvtxDomainRangeStartEx,
                      nvtxDomainHandle_t domain,
                      const nvtxEventAttributes_t * eventAttrib )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return 0;
    }

    // Look up metric from domain, attributes
    // Increment metric value
    // Return metric ID

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return 0;
}

DECLARE_NVTX_WRAPPER( nvtxRangeId_t,
                      nvtxRangeStartEx,
                      const nvtxEventAttributes_t * eventAttrib )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxRangeId_t result = CALL_NVTX_WRAPPER( nvtxDomainRangeStartEx( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN, eventAttrib ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( nvtxRangeId_t,
                      nvtxRangeStartA,
                      const char* message )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxEventAttributes_t attributes = SCOREP_CUDA_NVTX_MAKE_ASCII_ATTRIBUTES( message );
    nvtxRangeId_t         result     = CALL_NVTX_WRAPPER( nvtxRangeStartEx( &attributes ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( nvtxRangeId_t,
                      nvtxRangeStartW,
                      const wchar_t* message )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxEventAttributes_t attributes = SCOREP_CUDA_NVTX_MAKE_UNICODE_ATTRIBUTES( message );
    nvtxRangeId_t         result     = CALL_NVTX_WRAPPER( nvtxRangeStartEx( &attributes ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxDomainRangeEnd,
                      nvtxDomainHandle_t domain,
                      nvtxRangeId_t id )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    // Decrement metric based on domain/ID

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxRangeEnd,
                      nvtxRangeId_t id )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    CALL_NVTX_WRAPPER( nvtxDomainRangeEnd( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN, id ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Range push/pop functions *************************************/

DECLARE_NVTX_WRAPPER( int,
                      nvtxDomainRangePushEx,
                      nvtxDomainHandle_t domain,
                      const nvtxEventAttributes_t * eventAttrib )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return 0;
    }

    const char* name = scorep_cuda_nvtx_get_name_from_attributes( eventAttrib );
    if ( SCOREP_Filtering_MatchFunction( name, NULL ) )
    {
        SCOREP_Task_Enter( SCOREP_Location_GetCurrentCPULocation(), SCOREP_FILTERED_REGION );
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return 0;
    }

    SCOREP_RegionHandle region_handle = scorep_cuda_nvtx_get_user_region( domain, eventAttrib );

    scorep_cuda_nvtx_apply_payload( eventAttrib );
    SCOREP_EnterRegion( region_handle );
    scorep_cuda_nvtx_apply_category( domain, eventAttrib );

    SCOREP_IN_MEASUREMENT_DECREMENT();
    // Return 0 on success, negative on error (compatible with SCOREP_Error values?)
    return 0;
}

DECLARE_NVTX_WRAPPER( int,
                      nvtxRangePushEx,
                      const nvtxEventAttributes_t * eventAttrib )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int result = CALL_NVTX_WRAPPER( nvtxDomainRangePushEx( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN, eventAttrib ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( int,
                      nvtxRangePushA,
                      const char* message )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxEventAttributes_t attributes = SCOREP_CUDA_NVTX_MAKE_ASCII_ATTRIBUTES( message );
    int                   result     = CALL_NVTX_WRAPPER( nvtxRangePushEx( &attributes ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( int,
                      nvtxRangePushW,
                      const wchar_t* message )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    nvtxEventAttributes_t attributes = SCOREP_CUDA_NVTX_MAKE_UNICODE_ATTRIBUTES( message );
    int                   result     = CALL_NVTX_WRAPPER( nvtxRangePushEx( &attributes ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( int,
                      nvtxDomainRangePop,
                      nvtxDomainHandle_t domain )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();

    SCOREP_Location* location = SCOREP_Location_GetCurrentCPULocation();
    UTILS_ASSERT( location != NULL );

    SCOREP_RegionHandle region_handle =
        SCOREP_Task_GetTopRegion( SCOREP_Task_GetCurrentTask( location ) );
    UTILS_ASSERT( region_handle != SCOREP_INVALID_REGION );

    if ( region_handle != SCOREP_FILTERED_REGION )
    {
        SCOREP_ExitRegion( region_handle );
    }
    else
    {
        SCOREP_Task_Exit( location );
    }

    SCOREP_IN_MEASUREMENT_DECREMENT();

    // Return 0 on success, negative on error (compatible with SCOREP_Error values?)
    return 0;
}

DECLARE_NVTX_WRAPPER( int,
                      nvtxRangePop,
                      void )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    int result = CALL_NVTX_WRAPPER( nvtxDomainRangePop( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

/*************** Domain management ********************************************/

DECLARE_NVTX_WRAPPER( nvtxDomainHandle_t,
                      nvtxDomainCreateA,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_create_domain needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return NULL;
    }

    nvtxDomainHandle_t result = scorep_cuda_nvtx_create_domain( name );

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( nvtxDomainHandle_t,
                      nvtxDomainCreateW,
                      const wchar_t* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return NULL;
    }

    nvtxDomainHandle_t result = CALL_NVTX_WRAPPER( nvtxDomainCreateA( scorep_cuda_nvtx_unicode_to_ascii( name ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxDomainDestroy,
                      nvtxDomainHandle_t domain )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Domain resource management ***********************************/

DECLARE_NVTX_WRAPPER( nvtxResourceHandle_t,
                      nvtxDomainResourceCreate,
                      nvtxDomainHandle_t domain,
                      nvtxResourceAttributes_t * attribs )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return NULL;
    }

    // Not implemented

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return NULL;
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxDomainResourceDestroy,
                      nvtxResourceHandle_t resource )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    // Not implemented
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Category naming **********************************************/

DECLARE_NVTX_WRAPPER( void,
                      nvtxDomainNameCategoryA,
                      nvtxDomainHandle_t domain,
                      uint32_t category,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_name_category needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    scorep_cuda_nvtx_name_category( domain, category, name );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxDomainNameCategoryW,
                      nvtxDomainHandle_t domain,
                      uint32_t category,
                      const wchar_t* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    CALL_NVTX_WRAPPER( nvtxDomainNameCategoryA( domain, category, scorep_cuda_nvtx_unicode_to_ascii( name ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCategoryA,
                      uint32_t category,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    CALL_NVTX_WRAPPER( nvtxDomainNameCategoryA( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN, category, name ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCategoryW,
                      uint32_t category,
                      const wchar_t* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    CALL_NVTX_WRAPPER( nvtxDomainNameCategoryW( SCOREP_CUDA_NVTX_DEFAULT_DOMAIN, category, name ) );
    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Thread naming ************************************************/

/* NVTX documentation:
 *
 * Annotate an OS thread.
 *
 * Allows the user to name an active thread of the current process. If an
 * invalid thread ID is provided or a thread ID from a different process is
 * used the behavior of the tool is implementation dependent.
 *
 * The thread name is associated to the default domain.  To support domains
 * use resource objects via ::nvtxDomainResourceCreate.
 *
 * \param threadId - The ID of the thread to name.
 * \param name     - The name of the thread.
 *
 * \par Example:
 * \code
 * nvtxNameOsThread(GetCurrentThreadId(), "MAIN_THREAD");
 * \endcode
 */

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameOsThreadA,
                      uint32_t threadId,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    SCOREP_Location_SetNameByThreadId( threadId, name );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameOsThreadW,
                      uint32_t threadId,
                      const wchar_t* name )

{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    CALL_NVTX_WRAPPER( nvtxNameOsThreadA( threadId, scorep_cuda_nvtx_unicode_to_ascii( name ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Stream naming ************************************************/

/*
 * From cuda_runtime_api.h:
 * > The types ::CUstream and ::cudaStream_t are identical and may be used interchangeably.
 * They are just pointers, thus we can use `void*` to avoid additional includes
 */

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCuStreamA,
                      void* stream,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    scorep_cuda_nvtx_set_stream_name( stream, name );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCuStreamW,
                      void* stream,
                      const wchar_t* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    CALL_NVTX_WRAPPER( nvtxNameCuStreamA( stream, scorep_cuda_nvtx_unicode_to_ascii( name ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCudaStreamA,
                      void* stream,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    scorep_cuda_nvtx_set_stream_name( stream, name );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCudaStreamW,
                      void* stream,
                      const wchar_t* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    CALL_NVTX_WRAPPER( nvtxNameCudaStreamA( stream, scorep_cuda_nvtx_unicode_to_ascii( name ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** Context naming ************************************************/

/*
 * CUcontext is just a pointer, thus we can use `void*` to avoid additional includes
 */

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCuContextA,
                      void* context,
                      const char* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    scorep_cuda_nvtx_set_context_name( context, name );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

DECLARE_NVTX_WRAPPER( void,
                      nvtxNameCuContextW,
                      void* context,
                      const wchar_t* name )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return;
    }

    CALL_NVTX_WRAPPER( nvtxNameCuContextA( context, scorep_cuda_nvtx_unicode_to_ascii( name ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
}

/*************** String registration ******************************************/

DECLARE_NVTX_WRAPPER( nvtxStringHandle_t,
                      nvtxDomainRegisterStringA,
                      nvtxDomainHandle_t domain,
                      const char* string )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_create_string needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return NULL;
    }

    nvtxStringHandle_t result = scorep_cuda_nvtx_create_string( string );

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

DECLARE_NVTX_WRAPPER( nvtxStringHandle_t,
                      nvtxDomainRegisterStringW,
                      nvtxDomainHandle_t domain,
                      const wchar_t* string )
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        /* scorep_cuda_nvtx_unicode_to_ascii needs Score-P memory */
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return NULL;
    }

    nvtxStringHandle_t result =
        CALL_NVTX_WRAPPER( nvtxDomainRegisterStringA( domain, scorep_cuda_nvtx_unicode_to_ascii( string ) ) );

    SCOREP_IN_MEASUREMENT_DECREMENT();
    return result;
}

/*************** Init functions ***********************************************/

#if HAVE( NVTX_V3 )
static void
scorep_cuda_nvtxInitialize( const void* reserved )
#else
NVTX_DECLSPEC int NVTX_API
nvtxInitialize( const nvtxInitializationAttributes_t* initAttrib )
#endif /* HAVE( NVTX_V3 ) */
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
    }
    #if !HAVE( NVTX_V3 )
    return 0;
    #endif
}

#if HAVE( NVTX_V3 )
int
#ifdef SCOREP_SHARED_BUILD
InitializeInjectionNvtx2( NvtxGetExportTableFunc_t getExportTable )
#elif defined( SCOREP_STATIC_BUILD )
scorep_cuda_initialize_injection_nvtx2( NvtxGetExportTableFunc_t getExportTable )
#endif /* SCOREP_SHARED_BUILD */
{
    SCOREP_IN_MEASUREMENT_INCREMENT();
    if ( SCOREP_IS_MEASUREMENT_PHASE( PRE ) )
    {
        SCOREP_InitMeasurement();
    }
    if ( !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )
    {
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return 0;
    }
    SCOREP_NVTX_LOCK();

    const NvtxExportTableCallbacks* export_table = getExportTable( NVTX_ETID_CALLBACKS );
    if ( !export_table )
    {
        UTILS_WARNING( "[%s] Could not get NVTX export table to inject functions.", UTILS_FUNCTION_NAME );
        SCOREP_NVTX_UNLOCK();
        SCOREP_IN_MEASUREMENT_DECREMENT();
        return 0;
    }

    const NvtxExportTableVersionInfo* version_info = getExportTable( NVTX_ETID_VERSIONINFO );
    if ( !version_info )
    {
        UTILS_WARNING( "[%s] Could not get NVTX version info. NVTX injection might cause issues.",
                       UTILS_FUNCTION_NAME );
    }
    else if ( version_info->version != NVTX_VERSION )
    {
        UTILS_WARNING( "[%s] NVTX version mismatch: %u != %u. NVTX injection might cause issues.",
                       UTILS_FUNCTION_NAME, version_info->version, NVTX_VERSION );
    }

    NvtxFunctionTable function_table;
    unsigned int      out_size;

    /* Test size of NVTX function tables first, to ensure that it matches our expectations */
    #define CHECK_FUNCTION_TABLE_SIZE( module, size_name ) \
    do \
    { \
        export_table->GetModuleFunctionTable( ( module ), NULL, &out_size ); \
        if ( out_size != ( size_name ) ) \
        { \
            UTILS_WARNING( "[%s] NVTX function table %s size mismatch: %u != %u. " \
                           "NVTX injection might cause issues.", \
                           UTILS_FUNCTION_NAME, #module, out_size, size_name ); \
        } \
    } \
    while ( 0 )
    CHECK_FUNCTION_TABLE_SIZE( NVTX_CB_MODULE_CORE, NVTX_CBID_CORE_SIZE );
    CHECK_FUNCTION_TABLE_SIZE( NVTX_CB_MODULE_CUDA, NVTX_CBID_CUDA_SIZE );
    // not implemented: CHECK_FUNCTION_TABLE_SIZE( NVTX_CB_MODULE_OPENCL, NVTX_CBID_OPENCL_SIZE );
    CHECK_FUNCTION_TABLE_SIZE( NVTX_CB_MODULE_CUDART, NVTX_CBID_CUDART_SIZE );
    CHECK_FUNCTION_TABLE_SIZE( NVTX_CB_MODULE_CORE2, NVTX_CBID_CORE2_SIZE );
    // not implemented: CHECK_FUNCTION_TABLE_SIZE( NVTX_CB_MODULE_SYNC, NVTX_CBID_SYNC_SIZE );
    #undef CHECK_FUNCTION_TABLE_SIZE

    #define INJECT_FUNCTION_IF_POSSBILE( id, func ) \
    do \
    { \
        if ( function_table[ id ] ) \
        { \
            *function_table[ id ] = ( NvtxFunctionPointer )func; \
        } \
        else \
        { \
            UTILS_WARNING( "[%s] Could not inject %s.", UTILS_FUNCTION_NAME, #id ); \
        } \
    } while ( 0 )

    export_table->GetModuleFunctionTable( NVTX_CB_MODULE_CORE, &function_table, NULL );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_MarkEx, scorep_cuda_nvtxMarkEx );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_MarkA, scorep_cuda_nvtxMarkA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_MarkW, scorep_cuda_nvtxMarkW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangeStartEx, scorep_cuda_nvtxRangeStartEx );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangeStartA, scorep_cuda_nvtxRangeStartA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangeStartW, scorep_cuda_nvtxRangeStartW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangeEnd, scorep_cuda_nvtxRangeEnd );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangePushEx, scorep_cuda_nvtxRangePushEx );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangePushA, scorep_cuda_nvtxRangePushA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangePushW, scorep_cuda_nvtxRangePushW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_RangePop, scorep_cuda_nvtxRangePop );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_NameCategoryA, scorep_cuda_nvtxNameCategoryA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_NameCategoryW, scorep_cuda_nvtxNameCategoryW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_NameOsThreadA, scorep_cuda_nvtxNameOsThreadA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE_NameOsThreadW, scorep_cuda_nvtxNameOsThreadW );

    export_table->GetModuleFunctionTable( NVTX_CB_MODULE_CUDA, &function_table, NULL );
    // not implemented: INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuDeviceA, scorep_cuda_nvtxNameCuDeviceA );
    // not implemented: INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuDeviceW, scorep_cuda_nvtxNameCuDeviceW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuContextA, scorep_cuda_nvtxNameCuContextA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuContextW, scorep_cuda_nvtxNameCuContextW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuStreamA, scorep_cuda_nvtxNameCuStreamA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuStreamW, scorep_cuda_nvtxNameCuStreamW );
    // not implemented: INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuEventA, scorep_cuda_nvtxNameCuEventA );
    // not implemented: INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDA_NameCuEventW, scorep_cuda_nvtxNameCuEventW );

    /* OpenCL, not yet implemented
     * export_table->GetModuleFunctionTable( NVTX_CB_MODULE_OPENCL, function_table, &out_size );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClDeviceA, scorep_cuda_nvtxNameClDeviceA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClDeviceW, scorep_cuda_nvtxNameClDeviceW );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClContextA, scorep_cuda_nvtxNameClContextA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClContextW, scorep_cuda_nvtxNameClContextW );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClCommandQueueA, scorep_cuda_nvtxNameClCommandQueueA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClCommandQueueW, scorep_cuda_nvtxNameClCommandQueueW );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClMemObjectA, scorep_cuda_nvtxNameClMemObjectA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClMemObjectW, scorep_cuda_nvtxNameClMemObjectW );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClSamplerA, scorep_cuda_nvtxNameClSamplerA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClSamplerW, scorep_cuda_nvtxNameClSamplerW );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClProgramA, scorep_cuda_nvtxNameClProgramA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClProgramW, scorep_cuda_nvtxNameClProgramW );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClEventA, scorep_cuda_nvtxNameClEventA );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_OPENCL_NameClEventW, scorep_cuda_nvtxNameClEventW ); */

    export_table->GetModuleFunctionTable( NVTX_CB_MODULE_CUDART, &function_table, NULL );
    // not implemented: *function_table[ NVTX_CBID_CUDART_NameCudaDeviceA ] = (NvtxFunctionPointer) scorep_cuda_nvtxNameCudaDeviceA;
    // not implemented: *function_table[ NVTX_CBID_CUDART_NameCudaDeviceW ] = (NvtxFunctionPointer) scorep_cuda_nvtxNameCudaDeviceW;
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDART_NameCudaStreamA, scorep_cuda_nvtxNameCudaStreamA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CUDART_NameCudaStreamW, scorep_cuda_nvtxNameCudaStreamW );
    // not implemented: *function_table[ NVTX_CBID_CUDART_NameCudaEventA ] = (NvtxFunctionPointer) scorep_cuda_nvtxNameCudaEventA;
    // not implemented: *function_table[ NVTX_CBID_CUDART_NameCudaEventW ] = (NvtxFunctionPointer) scorep_cuda_nvtxNameCudaEventW;

    export_table->GetModuleFunctionTable( NVTX_CB_MODULE_CORE2, &function_table, NULL );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainMarkEx, scorep_cuda_nvtxDomainMarkEx );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainRangeStartEx, scorep_cuda_nvtxDomainRangeStartEx );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainRangeEnd, scorep_cuda_nvtxDomainRangeEnd );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainRangePushEx, scorep_cuda_nvtxDomainRangePushEx );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainRangePop, scorep_cuda_nvtxDomainRangePop );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainResourceCreate, scorep_cuda_nvtxDomainResourceCreate );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainResourceDestroy, scorep_cuda_nvtxDomainResourceDestroy );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainNameCategoryA, scorep_cuda_nvtxDomainNameCategoryA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainNameCategoryW, scorep_cuda_nvtxDomainNameCategoryW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainRegisterStringA, scorep_cuda_nvtxDomainRegisterStringA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainRegisterStringW, scorep_cuda_nvtxDomainRegisterStringW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainCreateA, scorep_cuda_nvtxDomainCreateA );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainCreateW, scorep_cuda_nvtxDomainCreateW );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_DomainDestroy, scorep_cuda_nvtxDomainDestroy );
    INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_CORE2_Initialize, scorep_cuda_nvtxInitialize );

    /* Sync, not yet implemented
     * export_table->GetModuleFunctionTable( NVTX_CB_MODULE_SYNC, function_table, &out_size );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_SYNC_DomainSyncUserCreate, nvtxDomainSyncUserCreate );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_SYNC_DomainSyncUserDestroy, nvtxDomainSyncUserDestroy );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_SYNC_DomainSyncUserAcquireStart, nvtxDomainSyncUserAcquireStart );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_SYNC_DomainSyncUserAcquireFailed, nvtxDomainSyncUserAcquireFailed );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_SYNC_DomainSyncUserAcquireSuccess, nvtxDomainSyncUserAcquireSuccess );
     * INJECT_FUNCTION_IF_POSSBILE( NVTX_CBID_SYNC_DomainSyncUserReleasing, nvtxDomainSyncUserReleasing ); */

    #undef INJECT_FUNCTION_IF_POSSBILE
    SCOREP_NVTX_UNLOCK();
    SCOREP_IN_MEASUREMENT_DECREMENT();
    return 1;
}

#ifdef SCOREP_SHARED_BUILD
int
InitializeInjectionNvtx( NvtxGetExportTableFunc_t getExportTable )
{
    return InitializeInjectionNvtx2( getExportTable );
}
#endif /* SCOREP_SHARED_BUILD */
#endif /* HAVE( NVTX_V3 ) */
#endif /* HAVE( NVTX_SUPPORT ) */
