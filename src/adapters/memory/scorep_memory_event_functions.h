/**
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016-2017, 2019, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2016-2017,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

#ifndef SCOREP_MEMORY_EVENT_FUNCTIONS_H
#define SCOREP_MEMORY_EVENT_FUNCTIONS_H

/**
 * Declaration of wrapper functions used by the memory library wrapper
 */

#include <malloc.h>

#include "scorep_memory_mgmt.h"
#include "scorep_memory_attributes.h"

#include <SCOREP_InMeasurement.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Libwrap_Internal.h>

#define SCOREP_DEBUG_MODULE_NAME MEMORY
#include <UTILS_Debug.h>

/* *INDENT-OFF* */
#define SCOREP_MEMORY_WRAP_MALLOC( FUNCTION, REGION ) \
void* \
SCOREP_LIBWRAP_WRAPPER( FUNCTION )( size_t size ) \
{ \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( !trigger || \
         !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        return SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( size ); \
    } \
 \
    UTILS_DEBUG_ENTRY( "%zu", size ); \
 \
    scorep_memory_attributes_add_enter_alloc_size( size ); \
    SCOREP_EnterWrappedRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    SCOREP_ENTER_WRAPPED_REGION(); \
    void* result = SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( size ); \
    SCOREP_EXIT_WRAPPED_REGION(); \
 \
    if ( result ) \
    { \
        SCOREP_AllocMetric_HandleAlloc( scorep_memory_metric, \
                                        ( uint64_t )result, \
                                        size ); \
    } \
\
    scorep_memory_attributes_add_exit_return_address( ( uint64_t )result ); \
    SCOREP_ExitRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    UTILS_DEBUG_EXIT( "%zu, %p", size, result ); \
    SCOREP_IN_MEASUREMENT_DECREMENT(); \
    return result; \
}
/* *INDENT-ON* */

/* *INDENT-OFF* */
#define SCOREP_MEMORY_WRAP_FREE( FUNCTION, REGION ) \
void \
SCOREP_LIBWRAP_WRAPPER( FUNCTION )( void* ptr ) \
{ \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( !trigger || \
         !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr ); \
        return; \
    } \
 \
    UTILS_DEBUG_ENTRY( "%p", ptr ); \
 \
    scorep_memory_attributes_add_enter_argument_address( ( uint64_t )ptr ); \
    SCOREP_EnterWrappedRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
\
    void* allocation = NULL; \
    if ( ptr ) \
    { \
        SCOREP_AllocMetric_AcquireAlloc( scorep_memory_metric, \
                                         ( uint64_t )ptr, &allocation ); \
    } \
 \
    SCOREP_ENTER_WRAPPED_REGION(); \
    SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr ); \
    SCOREP_EXIT_WRAPPED_REGION(); \
 \
    uint64_t dealloc_size = 0; \
    if ( ptr ) \
    { \
        SCOREP_AllocMetric_HandleFree( scorep_memory_metric, \
                                       allocation, &dealloc_size ); \
    } \
\
    scorep_memory_attributes_add_exit_dealloc_size( dealloc_size ); \
    SCOREP_ExitRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    UTILS_DEBUG_EXIT(); \
    SCOREP_IN_MEASUREMENT_DECREMENT(); \
}
/* *INDENT-ON* */


/* *INDENT-OFF* */
#define SCOREP_MEMORY_WRAP_FREE_SIZE( FUNCTION, REGION ) \
void \
SCOREP_LIBWRAP_WRAPPER( FUNCTION )( void*  ptr, \
                                    size_t size ) \
{ \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( !trigger || \
         !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr, size ); \
        return; \
    } \
 \
    UTILS_DEBUG_ENTRY( "%p, %zu", ptr, size ); \
 \
    scorep_memory_attributes_add_enter_argument_address( ( uint64_t )ptr ); \
    scorep_memory_attributes_add_exit_dealloc_size( size ); \
    SCOREP_EnterWrappedRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
\
    void* allocation = NULL; \
    if ( ptr ) \
    { \
        SCOREP_AllocMetric_AcquireAlloc( scorep_memory_metric, \
                                         ( uint64_t )ptr, &allocation ); \
    } \
 \
    SCOREP_ENTER_WRAPPED_REGION(); \
    SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr, size ); \
    SCOREP_EXIT_WRAPPED_REGION(); \
 \
    uint64_t dealloc_size = 0; \
    if ( ptr ) \
    { \
        SCOREP_AllocMetric_HandleFree( scorep_memory_metric, \
                                       allocation, &dealloc_size ); \
    } \
\
    scorep_memory_attributes_add_exit_dealloc_size( dealloc_size ); \
    SCOREP_ExitRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    UTILS_DEBUG_EXIT(); \
    SCOREP_IN_MEASUREMENT_DECREMENT(); \
}
/* *INDENT-ON* */


/* *INDENT-OFF* */
#define SCOREP_MEMORY_WRAP_CALLOC( FUNCTION, REGION ) \
void* \
SCOREP_LIBWRAP_WRAPPER( FUNCTION )( size_t nmemb, \
                                    size_t size ) \
{ \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( !trigger || \
         !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        return SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( nmemb, size ); \
    } \
 \
    UTILS_DEBUG_ENTRY( "%zu, %zu", nmemb, size ); \
 \
    scorep_memory_attributes_add_enter_alloc_size( nmemb * size ); \
    SCOREP_EnterWrappedRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    SCOREP_ENTER_WRAPPED_REGION(); \
    void* result = SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( nmemb, size ); \
    SCOREP_EXIT_WRAPPED_REGION(); \
 \
    if ( result ) \
    { \
        SCOREP_AllocMetric_HandleAlloc( scorep_memory_metric, \
                                        ( uint64_t )result, \
                                        nmemb * size ); \
    } \
\
    scorep_memory_attributes_add_exit_return_address( ( uint64_t )result ); \
    SCOREP_ExitRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    UTILS_DEBUG_EXIT( "%zu, %zu, %p", nmemb, size, result ); \
 \
    SCOREP_IN_MEASUREMENT_DECREMENT(); \
    return result; \
}
/* *INDENT-ON* */

/* *INDENT-OFF* */
#define SCOREP_MEMORY_WRAP_REALLOC( FUNCTION, REGION ) \
void* \
SCOREP_LIBWRAP_WRAPPER( FUNCTION )( void*  ptr, \
                                    size_t size ) \
{ \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( !trigger || \
         !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        return SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr, size ); \
    } \
 \
    UTILS_DEBUG_ENTRY( "%p, %zu", ptr, size ); \
 \
    /* The size belongs to to the result ptr in \
     * scorep_memory_attributes_add_exit_return_address */ \
    scorep_memory_attributes_add_enter_alloc_size( size ); \
    /* The ptr belongs to dealloc_size in \
     * scorep_memory_attributes_add_exit_dealloc_size. */ \
    scorep_memory_attributes_add_enter_argument_address( (  uint64_t )ptr ); \
    SCOREP_EnterWrappedRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
\
    void* allocation = NULL; \
    if ( ptr ) \
    { \
        SCOREP_AllocMetric_AcquireAlloc( scorep_memory_metric, \
                                         ( uint64_t )ptr, &allocation ); \
    } \
 \
    SCOREP_ENTER_WRAPPED_REGION(); \
    void* result = SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr, size ); \
    SCOREP_EXIT_WRAPPED_REGION(); \
 \
    /* \
     * If ptr is a null pointer, then it is like malloc. \
     */ \
    if ( ptr == NULL && result ) \
    { \
        SCOREP_AllocMetric_HandleAlloc( scorep_memory_metric, \
                                        ( uint64_t )result, \
                                        size ); \
        scorep_memory_attributes_add_exit_return_address( ( uint64_t )result ); \
    } \
    /* \
     * If size equals zero and ptr != NULL, then it is like free. \
     */ \
    else if ( ptr != NULL && size == 0 ) \
    { \
        uint64_t dealloc_size = 0; \
        SCOREP_AllocMetric_HandleFree( scorep_memory_metric, \
                                       allocation, &dealloc_size ); \
        scorep_memory_attributes_add_exit_dealloc_size( dealloc_size ); \
    } \
    /* Otherwise it is a realloc, treat as realloc on success, ... */ \
    else if ( result ) \
    { \
        uint64_t dealloc_size = 0; \
        SCOREP_AllocMetric_HandleRealloc( scorep_memory_metric, \
                                          ( uint64_t )result, \
                                          size, \
                                          allocation, \
                                          &dealloc_size ); \
        scorep_memory_attributes_add_exit_dealloc_size( dealloc_size ); \
        scorep_memory_attributes_add_exit_return_address( ( uint64_t )result ); \
    } \
    /* ... otherwise, realloc failed, ptr is not touched. * \
     */ \
    else \
    { \
        scorep_memory_attributes_add_exit_return_address( 0 ); \
    } \
\
    SCOREP_ExitRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    UTILS_DEBUG_EXIT( "%p, %zu, %p", ptr, size, result ); \
    SCOREP_IN_MEASUREMENT_DECREMENT(); \
    return result; \
}
/* *INDENT-ON* */

/* *INDENT-OFF* */
#define SCOREP_MEMORY_WRAP_POSIX_MEMALIGN( FUNCTION, REGION ) \
int \
SCOREP_LIBWRAP_WRAPPER( FUNCTION )( void** ptr, \
                                    size_t alignment, \
                                    size_t size ) \
{ \
    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT(); \
    if ( !trigger || \
         !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) ) \
    { \
        SCOREP_IN_MEASUREMENT_DECREMENT(); \
        return SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr, alignment, size ); \
    } \
 \
    UTILS_DEBUG_ENTRY( "%zu, %zu", alignment, size ); \
 \
    scorep_memory_attributes_add_enter_alloc_size( size ); \
    SCOREP_EnterWrappedRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    SCOREP_ENTER_WRAPPED_REGION(); \
    int result = SCOREP_LIBWRAP_ORIGINAL( FUNCTION )( ptr, alignment, size ); \
    SCOREP_EXIT_WRAPPED_REGION(); \
 \
    if ( result == 0 && *ptr ) \
    { \
        SCOREP_AllocMetric_HandleAlloc( scorep_memory_metric, \
                                        ( uint64_t )*ptr, \
                                        size ); \
    } \
\
    scorep_memory_attributes_add_exit_return_address( ( uint64_t )*ptr ); \
    SCOREP_ExitRegion( scorep_memory_regions[ SCOREP_MEMORY_##REGION ] ); \
 \
    UTILS_DEBUG_EXIT( "%zu, %zu, %p", alignment, size, result ); \
    SCOREP_IN_MEASUREMENT_DECREMENT(); \
    return result; \
}
/* *INDENT-ON* */

#endif /* SCOREP_MEMORY_EVENT_FUNCTIONS_H */
