/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011, 2020-2022,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011, 2018-2019, 2022,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2014, 2017, 2022-2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * Copyright (c) 2022,
 * Deutsches Zentrum fuer Luft- und Raumfahrt, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 * @ingroup    MPI_Wrapper
 *
 * @brief Contains the implementation of MPI Requests management
 */

#include <config.h>

#include <scorep_mpi_request_mgmt.h>

#include <scorep_mpi_groups.h>
#include <scorep_mpi_c.h>
#include <scorep_mpi_communicator.h>
#include <SCOREP_Events.h>
#include <SCOREP_IoManagement.h>

#include <UTILS_Atomic.h>
#include <UTILS_Error.h>
#include <UTILS_Mutex.h>
#include <SCOREP_Memory.h>
#include <SCOREP_FastHashtab.h>
#include <jenkins_hash.h>

#include <stdlib.h>
#include <string.h>

/**
 * @internal
 * Request id counter
 */
static SCOREP_MpiRequestId mpi_last_request_id;

/* Type declarations for NON_MONOTONIC_HASH_TABLE */
typedef struct request_table_entry request_table_entry;
struct request_table_entry
{
    union
    {
        scorep_mpi_request*  request;
        request_table_entry* next;
    } payload;
    UTILS_Mutex request_lock;
};

typedef MPI_Request          request_table_key_t;
typedef request_table_entry* request_table_value_t;

/* ------------------------------------------------------------------------- */
/* Free-lists and other helper stuff:                                        */
static inline void
free_mpi_type( scorep_mpi_request* req )
{
    /*
     * Drop type duplicate, but only if we could have made a duplicate in the
     * first place
     */
#if HAVE( MPI_2_0_SYMBOL_PMPI_TYPE_DUP )
    if ( req->request_type == SCOREP_MPI_REQUEST_TYPE_SEND
         || req->request_type == SCOREP_MPI_REQUEST_TYPE_RECV )
    {
        PMPI_Type_free( &req->payload.p2p.datatype );
    }
    if ( req->request_type == SCOREP_MPI_REQUEST_TYPE_IO )
    {
        PMPI_Type_free( &req->payload.io.datatype );
    }
#endif
}

static request_table_value_t request_table_entry_free_list;
static UTILS_Mutex           request_table_entry_free_list_mutex;

/* Returns pointer to 0-initialized request_table_entry. */
static inline request_table_value_t
get_request_table_entry_from_pool( void )
{
    request_table_value_t ret;

    UTILS_MutexLock( &request_table_entry_free_list_mutex );
    if ( request_table_entry_free_list == NULL )
    {
        UTILS_MutexUnlock( &request_table_entry_free_list_mutex );
        ret = SCOREP_Memory_AllocForMisc( sizeof( *ret ) );
    }
    else
    {
        ret                           = request_table_entry_free_list;
        request_table_entry_free_list = request_table_entry_free_list->payload.next;

        UTILS_MutexUnlock( &request_table_entry_free_list_mutex );
    }

    memset( ret, 0, sizeof( *ret ) );
    return ret;
}

static inline void
release_request_table_entry_to_pool( request_table_value_t data )
{
    UTILS_MutexLock( &request_table_entry_free_list_mutex );

    data->payload.next            = request_table_entry_free_list;
    request_table_entry_free_list = data;

    UTILS_MutexUnlock( &request_table_entry_free_list_mutex );
}

static scorep_mpi_request* request_free_list;
static UTILS_Mutex         request_free_list_mutex;

/* Returns pointer to uninitialized request_table_entry. */
static inline scorep_mpi_request*
get_scorep_request_from_pool( void )
{
    scorep_mpi_request* ret;

    UTILS_MutexLock( &request_free_list_mutex );

    if ( request_free_list == NULL )
    {
        UTILS_MutexUnlock( &request_free_list_mutex );

        ret = SCOREP_Memory_AllocForMisc( sizeof( *ret ) );
    }
    else
    {
        ret               = request_free_list;
        request_free_list = request_free_list->next;
        UTILS_MutexUnlock( &request_free_list_mutex );
    }

    return ret;
}

static inline void
release_scorep_request_to_pool( scorep_mpi_request* req )
{
    UTILS_MutexLock( &request_free_list_mutex );

    req->next         = request_free_list;
    request_free_list = req;

    UTILS_MutexUnlock( &request_free_list_mutex );
}

/* Requirements for NON_MONOTONIC_HASH_TABLE:                                */
#define REQUEST_TABLE_HASH_EXPONENT 8
static inline bool
request_table_equals( request_table_key_t key1,
                      request_table_key_t key2 )
{
    return key1 == key2;
}

static inline void*
request_table_allocate_chunk( size_t chunkSize )
{
    void* chunk = SCOREP_Memory_AlignedAllocForMisc( SCOREP_CACHELINESIZE, chunkSize );

    return chunk;
}

static inline void
request_table_free_chunk( void* chunk )
{
}

static inline request_table_value_t
request_table_value_ctor( request_table_key_t* key,
                          const void*          ctorData )
{
    request_table_value_t new_request_table_entry = get_request_table_entry_from_pool();
    scorep_mpi_request*   new_scorep_mpi_request  = get_scorep_request_from_pool();

    memcpy( new_scorep_mpi_request, ctorData, sizeof( *new_scorep_mpi_request ) );
    new_request_table_entry->payload.request = new_scorep_mpi_request;

    return new_request_table_entry;
}

static inline void
request_table_value_dtor( request_table_key_t   key,
                          request_table_value_t value )
{
    scorep_mpi_request* req = value->payload.request;
    free_mpi_type( req );

    release_scorep_request_to_pool( value->payload.request );
    release_request_table_entry_to_pool( value );
}

static inline uint32_t
request_table_bucket_idx( request_table_key_t key )
{
    return jenkins_hash( &key, sizeof( key ), 0 ) & hashmask( REQUEST_TABLE_HASH_EXPONENT );
}

SCOREP_HASH_TABLE_NON_MONOTONIC( request_table, 11, hashsize( REQUEST_TABLE_HASH_EXPONENT ) );

#undef REQUEST_TABLE_HASH_EXPONENT

/*
 * This insertion-function and the corresponding get- and remove-functions are a work-around for an
 * optimization done by most MPI implementations. Most MPI Implementations may choose to immediately
 * complete small non-blocking MPI calls, eg MPI_Isend with a single double, and then return a dummy
 * value which is unequal to MPI_REQUEST_NULL. Since our hash-table does not support duplicates, our
 * work-around is to maintain a simple list of requests for these dummy values and otherwise simply
 * use the hash-table as intended.
 */
static inline void
insert_scorep_mpi_request( MPI_Request key, scorep_mpi_request* data )
{
    // Try to insert request directly.
    request_table_value_t orig_value = NULL;
    bool                  inserted   = request_table_get_and_insert( key, data, &orig_value );

    // If the MPI_Request is a duplicate, we need to append a new scorep_mpi_request to the list.
    if ( !inserted )
    {
        // Create the new request.
        scorep_mpi_request* new_request = get_scorep_request_from_pool();
        memcpy( new_request, data, sizeof( *new_request ) );

        do
        {
            // Lock the current entry, such that the new request can be inserted.
            UTILS_MutexLock( &( orig_value->request_lock ) );

            // Since another thread could have removed and possibly also reinserted
            // the entry with the same key, we need to double check.
            request_table_value_t control = NULL;
            inserted = request_table_get_and_insert( key, data, &control );

            // Another thread has removed the entry. The request was inserted directly.
            // Unlock the old entry for consistency and release the scorep_mpi_request.
            if ( inserted )
            {
                UTILS_MutexUnlock( &( orig_value->request_lock ) );

                release_scorep_request_to_pool( new_request );

                return;
            }

            // The entry has not changed. Append the scorep_mpi_request to the list.
            if ( !inserted && orig_value == control )
            {
                scorep_mpi_request* current = orig_value->payload.request;
                while ( current->next != NULL )
                {
                    current = current->next;
                }
                current->next       = new_request;
                current->next->next = NULL;

                UTILS_MutexUnlock( &( orig_value->request_lock ) );
                return;
            }

            // The entry was changed, but still remains in the hash-table. Update and retry.
            UTILS_MutexUnlock( &( orig_value->request_lock ) );
            orig_value = control;
        }
        while ( true );
    }
}
//-----------------------------------------------------------------------------

void
scorep_mpi_req_mgmt_storage_array_init( SCOREP_Location*                   location,
                                        size_t                             datatypeBytes,
                                        scorep_mpi_req_mgmt_storage_array* array )
{
    UTILS_ASSERT( array->loc == 0 );

    const size_t page_size = SCOREP_Memory_GetPageSize();
    array->loc   = SCOREP_Location_AllocForMisc( location, page_size );
    array->count = page_size / datatypeBytes;
}


void
scorep_mpi_req_mgmt_storage_array_grow( SCOREP_Location*                   location,
                                        size_t                             datatypeBytes,
                                        scorep_mpi_req_mgmt_storage_array* array,
                                        size_t                             newCount )
{
    if ( newCount > array->count )
    {
        const size_t num_bytes = newCount * datatypeBytes;
        const size_t page_size = SCOREP_Memory_GetPageSize();
        const size_t num_pages = ( num_bytes + page_size - 1 ) / page_size;
        /*
         * NOTE: We deliberately leak memory here since we do not have the option
         * to free individual allocations.
         *
         * From src/measurement/include/SCOREP_Memory.h:
         *     These functions are the replacement of malloc and free. Note that there is
         *     no possibility to free a single allocation but only to free the entire
         *     allocated memory of a specific type. Due to the usual memory access
         *     patterns during measurement this design is hopefully justified.
         *
         * We do however try and reduce the memory leak by increasing the array
         * in multiples of SCOREP_PAGE_SIZE.
         */
        array->loc   = SCOREP_Location_AllocForMisc( location, num_pages * page_size );
        array->count = ( num_pages * page_size ) / datatypeBytes;
    }
}

SCOREP_MpiRequestId
scorep_mpi_get_request_id( void )
{
    return UTILS_Atomic_AddFetch_uint64( &mpi_last_request_id, 1, UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
}

void
scorep_mpi_request_p2p_create( MPI_Request             request,
                               scorep_mpi_request_type type,
                               scorep_mpi_request_flag flags,
                               int                     tag,
                               int                     dest,
                               uint64_t                bytes,
                               MPI_Datatype            datatype,
                               MPI_Comm                comm,
                               SCOREP_MpiRequestId     id )
{
    scorep_mpi_request data = { .request      = request,
                                .request_type = type,
                                .payload.p2p  = {
                                    .tag         = tag,
                                    .dest        = dest,
                                    .bytes       = bytes,
                                    .comm_handle = SCOREP_MPI_COMM_HANDLE( comm )
                                },
                                .flags  = flags,
                                .id     = id,
                                .next   = NULL,
                                .marker = false };

#if HAVE( MPI_2_0_SYMBOL_PMPI_TYPE_DUP )
    PMPI_Type_dup( datatype, &data.payload.p2p.datatype );
#else
    data.payload.p2p.datatype = datatype;
#endif

    insert_scorep_mpi_request( request, &data );
}

void
scorep_mpi_request_icoll_create( MPI_Request             request,
                                 scorep_mpi_request_flag flags,
                                 SCOREP_CollectiveType   collectiveType,
                                 int                     rootLoc,
                                 uint64_t                bytesSent,
                                 uint64_t                bytesRecv,
                                 MPI_Comm                comm,
                                 SCOREP_MpiRequestId     id )
{
    scorep_mpi_request data = { .request       = request,
                                .request_type  = SCOREP_MPI_REQUEST_TYPE_ICOLL,
                                .payload.icoll = {
                                    .coll_type   = collectiveType,
                                    .root_loc    = rootLoc,
                                    .bytes_sent  = bytesSent,
                                    .bytes_recv  = bytesRecv,
                                    .comm_handle = SCOREP_MPI_COMM_HANDLE( comm ),
                                },
                                .flags  = flags,
                                .id     = id,
                                .next   = NULL,
                                .marker = false };

    insert_scorep_mpi_request( request, &data );
}

void
scorep_mpi_request_io_create( MPI_Request            request,
                              SCOREP_IoOperationMode mode,
                              uint64_t               bytes,
                              MPI_Datatype           datatype,
                              MPI_File               fh,
                              SCOREP_MpiRequestId    id )
{
    scorep_mpi_request data = { .request      = request,
                                .request_type = SCOREP_MPI_REQUEST_TYPE_IO,
                                .payload.io   = {
                                    .mode  = mode,
                                    .bytes = bytes,
                                    .fh    = fh
                                },
                                .flags  = SCOREP_MPI_REQUEST_FLAG_NONE,
                                .id     = id,
                                .next   = NULL,
                                .marker = false };

#if HAVE( MPI_2_0_SYMBOL_PMPI_TYPE_DUP )
    PMPI_Type_dup( datatype, &data.payload.io.datatype );
#else
    data.payload.io.datatype = datatype;
#endif

    insert_scorep_mpi_request( request, &data );
}

void
scorep_mpi_request_comm_idup_create( MPI_Request         request,
                                     MPI_Comm            parentComm,
                                     MPI_Comm*           newComm,
                                     SCOREP_MpiRequestId id )
{
    scorep_mpi_request_comm_idup_create_interop( request, parentComm, SCOREP_MPI_LANGUAGE_C, ( void* )newComm, id );
}


void
scorep_mpi_request_comm_idup_create_interop( MPI_Request         request,
                                             MPI_Comm            parentComm,
                                             scorep_mpi_language originLanguage,
                                             void*               newComm,
                                             SCOREP_MpiRequestId id )
{
    scorep_mpi_request data = { .request           = request,
                                .request_type      = SCOREP_MPI_REQUEST_TYPE_COMM_IDUP,
                                .payload.comm_idup = {
                                    .origin_language    = originLanguage,
                                    .new_comm           = newComm,
                                    .parent_comm_handle = SCOREP_INVALID_INTERIM_COMMUNICATOR
                                },
                                .flags  = SCOREP_MPI_REQUEST_FLAG_NONE,
                                .id     = id,
                                .next   = NULL,
                                .marker = false };

    if ( parentComm != MPI_COMM_NULL )
    {
        data.payload.comm_idup.parent_comm_handle = SCOREP_MPI_COMM_HANDLE( parentComm );
    }

    insert_scorep_mpi_request( request, &data );
}

void
scorep_mpi_request_win_create( MPI_Request             mpiRequest,
                               scorep_mpi_rma_request* rmaRequest )
{
    scorep_mpi_request data = { .request      = mpiRequest,
                                .request_type = SCOREP_MPI_REQUEST_TYPE_RMA,
                                .payload.rma  = { .request_ptr = rmaRequest },
                                .flags        = SCOREP_MPI_REQUEST_FLAG_NONE,
                                .id           = rmaRequest->matching_id,
                                .next         = NULL,
                                .marker       = false };

    insert_scorep_mpi_request( mpiRequest, &data );
}

scorep_mpi_request*
scorep_mpi_request_get( MPI_Request request )
{
    request_table_value_t value, control;
    if ( !request_table_get( request, &value ) )
    {
        return NULL;
    }

    do
    {
        UTILS_MutexLock( &( value->request_lock ) );

        if ( !request_table_get( request, &control ) )
        {
            UTILS_MutexUnlock( &( value->request_lock ) );
            return NULL;
        }

        if ( control == value )
        {
            scorep_mpi_request* current = value->payload.request;
            while ( current != NULL )
            {
                if ( !UTILS_Atomic_LoadN_bool( &( current->marker ), UTILS_ATOMIC_SEQUENTIAL_CONSISTENT ) )
                {
                    break;
                }
                else
                {
                    current = current->next;
                }
            }

            if ( current == NULL )
            {
                UTILS_MutexUnlock( &( value->request_lock ) );
                return NULL;
            }

            UTILS_Atomic_StoreN_bool( &( current->marker ), true, UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
            UTILS_MutexUnlock( &( value->request_lock ) );
            return current;
        }

        UTILS_MutexUnlock( &( value->request_lock ) );
        value = control;
    }
    while ( true );

    return NULL;
}

void
scorep_mpi_unmark_request( scorep_mpi_request* req )
{
    if ( req )
    {
        UTILS_Atomic_StoreN_bool( &( req->marker ), false, UTILS_ATOMIC_SEQUENTIAL_CONSISTENT );
    }
}

void
scorep_mpi_request_free( scorep_mpi_request* req )
{
    request_table_value_t value, control;
    if ( !request_table_get( req->request, &value ) )
    {
        UTILS_ERROR( SCOREP_ERROR_MPI_REQUEST_NOT_REMOVED,
                     "Request to be freed, not found in hashtable." );

        return;
    }

    do
    {
        UTILS_MutexLock( &( value->request_lock ) );

        if ( !request_table_get( req->request, &control ) )
        {
            UTILS_ERROR( SCOREP_ERROR_MPI_REQUEST_NOT_REMOVED,
                         "Request to be freed, not found in hashtable on control." );

            UTILS_MutexUnlock( &( value->request_lock ) );
            return;
        }

        if ( control == value )
        {
            scorep_mpi_request* current = value->payload.request;

            if ( value->payload.request == NULL )
            {
                UTILS_ERROR( SCOREP_ERROR_MPI_REQUEST_NOT_REMOVED,
                             "Linked list empty, should be impossible." );

                UTILS_MutexUnlock( &( value->request_lock ) );
                return;
            }

            if ( value->payload.request->next == NULL && value->payload.request == req )
            {
                if ( !request_table_remove( req->request ) )
                {
                    UTILS_ERROR( SCOREP_ERROR_MPI_REQUEST_NOT_REMOVED,
                                 "Removing hashtable entry failed." );
                }

                UTILS_MutexUnlock( &( value->request_lock ) );
                return;
            }

            scorep_mpi_request* previous = NULL;

            while ( current != NULL && current != req )
            {
                previous = current;
                current  = current->next;
            }

            if ( current == NULL )
            {
                UTILS_ERROR( SCOREP_ERROR_MPI_REQUEST_NOT_REMOVED,
                             "Request to be freed, not found in list of hashtable entry." );

                UTILS_MutexUnlock( &( value->request_lock ) );
                return;
            }

            if ( previous == NULL )
            {
                value->payload.request = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            free_mpi_type( current );
            UTILS_MutexUnlock( &( value->request_lock ) );

            release_scorep_request_to_pool( current );
            return;
        }

        UTILS_MutexUnlock( &( value->request_lock ) );
        value = control;
    }
    while ( true );
}


void
scorep_mpi_check_all_or_test_all( int         count,
                                  int         flag,
                                  MPI_Status* array_of_statuses )
{
    if ( flag )
    {
        for ( int i = 0; i < count; i++ )
        {
            scorep_mpi_request* scorep_req = scorep_mpi_saved_request_get( i );
            scorep_mpi_check_request( scorep_req, &( array_of_statuses[ i ] ) );
            scorep_mpi_cleanup_request( scorep_req );
            scorep_mpi_unmark_request( scorep_req );
        }
    }
    else
    {
        for ( int i = 0; i < count; i++ )
        {
            scorep_mpi_request* scorep_req = scorep_mpi_saved_request_get( i );
            scorep_mpi_request_tested( scorep_req );
            scorep_mpi_unmark_request( scorep_req );
        }
    }
}

void
scorep_mpi_check_all_or_none( int         count,
                              int         flag,
                              MPI_Status* array_of_statuses )
{
    if ( flag )
    {
        for ( int i = 0; i < count; i++ )
        {
            scorep_mpi_request* scorep_req = scorep_mpi_saved_request_get( i );
            scorep_mpi_check_request( scorep_req, &( array_of_statuses[ i ] ) );
            scorep_mpi_cleanup_request( scorep_req );
            scorep_mpi_unmark_request( scorep_req );
        }
    }
}

void
scorep_mpi_check_some_test_some( int         incount,
                                 int         outcount,
                                 int*        array_of_indices,
                                 MPI_Status* array_of_statuses )
{
    /* For all requests in the input array_of_requests, do one of two things:
     * 1. Process the request if it has been been completed by the MPI_Waitsome
     * 2. or record an MpiRequestTested event if it has not been completed.
     *
     * Because of 2. we iterate over the input array and search for the matching index
     * in array_of_indices (rather than iterating over array_of_indices directly).
     *
     * The search in array_of_indices is optimized such that it only needs to look at
     * the entries that have not been processed already.
     * This is achieved by reordering (with a swap) array_of_indices and array_of_statuses such that
     * the entries corresponding to processed requests are a the start.
     */

    /* Position of first entry in array_of_indices (and array_of_statuses) that belongs
     * to a not yet processed request.
     */
    int cur = 0;

    for ( int req_idx = 0; req_idx < incount; ++req_idx )
    {
        scorep_mpi_request* scorep_req = scorep_mpi_saved_request_get( req_idx );
        if ( scorep_req )
        {
            /* Search for j such that array_of_indices[j] == req_idx */
            int j = cur;
            while ( j < outcount && req_idx != array_of_indices[ j ] )
            {
                ++j;
            }

            /* Found j:
             * The request array_of_requests[req_idx] has been completed by this MPI_Waitsome
             * and the corresponding status has been returned in array_of_statuses[j]
             */
            if ( j < outcount )
            {
                /* Swap j <-> cur in array_of_indices */
                int tmp = array_of_indices[ cur ];
                array_of_indices[ cur ] = array_of_indices[ j ];
                array_of_indices[ j ]   = tmp;

                /* Swap j <-> cur in array_of_statuses */
                MPI_Status tmpstat = array_of_statuses[ cur ];
                array_of_statuses[ cur ] = array_of_statuses[ j ];
                array_of_statuses[ j ]   = tmpstat;

                /* Process the completed request */
                scorep_mpi_check_request( scorep_req, &( array_of_statuses[ cur ] ) );
                scorep_mpi_cleanup_request( scorep_req );

                ++cur;
            }
            /* Did not find j:
             * The request has not been completed by this MPI_Waitsome. */
            else
            {
                scorep_mpi_request_tested( scorep_req );
            }
            scorep_mpi_unmark_request( scorep_req );
        }
    }
}

void
scorep_mpi_check_some( int         incount,
                       int         outcount,
                       int*        array_of_indices,
                       MPI_Status* array_of_statuses )
{
    for ( int j  = 0; j < outcount; ++j )
    {
        scorep_mpi_request* scorep_req = scorep_mpi_saved_request_get( array_of_indices[ j ] );
        scorep_mpi_check_request( scorep_req, &( array_of_statuses[ j ] ) );
        scorep_mpi_cleanup_request( scorep_req );
        scorep_mpi_unmark_request( scorep_req );
    }
}

void
scorep_mpi_test_all( int count )
{
    for ( int i = 0; i < count; i++ )
    {
        scorep_mpi_request* scorep_req = scorep_mpi_saved_request_get( i );
        scorep_mpi_request_tested( scorep_req );
        scorep_mpi_unmark_request( scorep_req );
    }
}

void
scorep_mpi_request_start( MPI_Request request )
{
    scorep_mpi_request* scorep_req = scorep_mpi_request_get( request );

    if ( scorep_req && ( scorep_req->flags & SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT ) )
    {
        scorep_req->flags |= SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE;
        if ( ( scorep_req->request_type == SCOREP_MPI_REQUEST_TYPE_SEND ) && ( scorep_req->payload.p2p.dest != MPI_PROC_NULL ) )
        {
            SCOREP_MpiIsend( scorep_req->payload.p2p.dest, scorep_req->payload.p2p.comm_handle,
                             scorep_req->payload.p2p.tag, scorep_req->payload.p2p.bytes, scorep_req->id );
        }
        else if ( scorep_req->request_type == SCOREP_MPI_REQUEST_TYPE_RECV )
        {
            SCOREP_MpiIrecvRequest( scorep_req->id );
        }
    }
    scorep_mpi_unmark_request( scorep_req );
}


void
scorep_mpi_request_set_completed( scorep_mpi_request* req )
{
    if ( !req )
    {
        return;
    }
    req->flags |= SCOREP_MPI_REQUEST_FLAG_IS_COMPLETED;
}

void
scorep_mpi_request_set_cancel( scorep_mpi_request* req )
{
    if ( !req )
    {
        return;
    }
    req->flags |= SCOREP_MPI_REQUEST_FLAG_CAN_CANCEL;
}

void
scorep_mpi_request_free_wrapper( MPI_Request* request )
{
    const int           event_gen_active_for_group = scorep_mpi_enabled & SCOREP_MPI_ENABLED_REQUEST;
    scorep_mpi_request* scorep_req                 = scorep_mpi_request_get( *request );

    if ( scorep_req )
    {
        if ( scorep_req->flags & SCOREP_MPI_REQUEST_FLAG_CAN_CANCEL && event_gen_active_for_group )
        {
            MPI_Status* status = scorep_mpi_get_status_array( 1 );
            int         cancelled;
            /* -- Must check if request was cancelled and write the
             *    cancel event. Not doing so will confuse the trace
             *    analysis.
             */
            PMPI_Wait( request, status );
            PMPI_Test_cancelled( status, &cancelled );

            if ( cancelled )
            {
                SCOREP_MpiRequestCancelled( scorep_req->id );
            }
        }

        if ( ( scorep_req->flags & SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT ) && ( scorep_req->flags & SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE ) )
        {
            /* mark active requests for deallocation */
            scorep_req->flags |= SCOREP_MPI_REQUEST_FLAG_DEALLOCATE;
        }
        else
        {
            /* deallocate inactive requests -*/
            scorep_mpi_request_free( scorep_req );
        }
    }
    scorep_mpi_unmark_request( scorep_req );
}

void
scorep_mpi_request_tested( scorep_mpi_request* req )
{
    if ( !req ||
         ( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT &&
           !( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE ) )
         )
    {
        return;
    }
    if ( req->request_type == SCOREP_MPI_REQUEST_TYPE_IO )
    {
        SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_MPI,
                                                                     &( req->payload.io.fh ) );
        if ( io_handle != SCOREP_INVALID_IO_HANDLE )
        {
            SCOREP_IoOperationTest( io_handle, req->id );
        }
    }
    else
    {
        SCOREP_MpiRequestTested( req->id );
    }
}


void
scorep_mpi_check_request( scorep_mpi_request* req,
                          MPI_Status*         status )
{
    scorep_mpi_interop_status interop_status = scorep_mpi_interop_status_create( status );
    scorep_mpi_check_request_interop( req, &interop_status );
}

void
scorep_mpi_check_request_interop( scorep_mpi_request*        req,
                                  scorep_mpi_interop_status* interopStatus )
{
    const int p2p_events_active  = ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_P2P );
    const int io_events_active   = ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_IO );
    const int coll_events_active = ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_COLL );

    if ( !req ||
         ( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_COMPLETED ) ||
         ( ( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT ) &&
           !( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE ) ) )
    {
        return;
    }

    int cancelled = 0;
    if ( req->flags & SCOREP_MPI_REQUEST_FLAG_CAN_CANCEL )
    {
        scorep_mpi_test_cancelled( interopStatus, &cancelled );
    }
    if ( cancelled )
    {
        if ( req->id != UINT64_MAX )
        {
            SCOREP_MpiRequestCancelled( req->id );
        }
    }
    else
    {
        int count, sz;

        int status_source, status_tag;
        scorep_mpi_status_source( interopStatus, &status_source );
        scorep_mpi_status_tag( interopStatus, &status_tag );

        MPI_Comm comm_f08 = MPI_COMM_NULL;
        switch ( req->request_type )
        {
            case SCOREP_MPI_REQUEST_TYPE_RECV:
                if ( p2p_events_active && status_source != MPI_PROC_NULL )
                {
                    /* if receive request, write receive trace record */

                    PMPI_Type_size( req->payload.p2p.datatype, &sz );
                    scorep_mpi_get_count( interopStatus, req->payload.p2p.datatype, &count );

                    SCOREP_MpiIrecv( status_source, req->payload.p2p.comm_handle,
                                     status_tag, ( uint64_t )count * sz, req->id );
                }
                break;

            case SCOREP_MPI_REQUEST_TYPE_SEND:
                if ( p2p_events_active )
                {
                    SCOREP_MpiIsendComplete( req->id );
                }
                break;

            case SCOREP_MPI_REQUEST_TYPE_COMM_IDUP:
                switch ( req->payload.comm_idup.origin_language )
                {
                    case SCOREP_MPI_LANGUAGE_C:
                        scorep_mpi_comm_create_finalize( *( MPI_Comm* )req->payload.comm_idup.new_comm,
                                                         req->payload.comm_idup.parent_comm_handle );
                        SCOREP_CommCreate( SCOREP_MPI_COMM_HANDLE( *( MPI_Comm* )req->payload.comm_idup.new_comm ) );
                        SCOREP_MpiNonBlockingCollectiveComplete( req->payload.comm_idup.parent_comm_handle,
                                                                 SCOREP_INVALID_ROOT_RANK,
                                                                 SCOREP_COLLECTIVE_CREATE_HANDLE,
                                                                 0,
                                                                 0,
                                                                 req->id );
                        break;
#if HAVE( MPI_USEMPIF08_SUPPORT )
                    case SCOREP_MPI_LANGUAGE_F08:
                        comm_f08 = PMPI_Comm_f2c( *( MPI_Fint* )req->payload.comm_idup.new_comm );
                        scorep_mpi_comm_create_finalize( comm_f08,
                                                         req->payload.comm_idup.parent_comm_handle );
                        /* TODO: SCOREP_CommCreate and SCOREP_MpiNonBlockingCollectiveComplete */
                        break;
#endif /* HAVE( MPI_USEMPIF08_SUPPORT ) */
                }
                break;

            case SCOREP_MPI_REQUEST_TYPE_RMA:
                // sanity check for rma_request handle
                UTILS_BUG_ON( req->payload.rma.request_ptr == NULL,
                              "No request information associated with MPI request." );
                UTILS_BUG_ON( req->payload.rma.request_ptr->mpi_handle != req->request,
                              "Request information inconsistent with associated MPI request" );

                // if request has not yet been completed locally
                // write an RmaOpCompleteNonBlocking event
                if ( !req->payload.rma.request_ptr->completed_locally )
                {
                    SCOREP_RmaOpCompleteNonBlocking( req->payload.rma.request_ptr->window, req->payload.rma.request_ptr->matching_id );
                    req->payload.rma.request_ptr->completed_locally = true;

                    if ( !req->payload.rma.request_ptr->schedule_removal
                         && req->payload.rma.request_ptr->completion_type == SCOREP_MPI_RMA_REQUEST_COMBINED_COMPLETION )
                    {
                        SCOREP_RmaOpCompleteRemote( req->payload.rma.request_ptr->window, req->payload.rma.request_ptr->matching_id );
                        req->payload.rma.request_ptr->schedule_removal = true;
                    }
                }
                if ( req->payload.rma.request_ptr->schedule_removal )
                {
                    scorep_mpi_rma_request_remove_by_ptr( req->payload.rma.request_ptr );
                }
                break;

            case SCOREP_MPI_REQUEST_TYPE_IO:
                if ( io_events_active )
                {
                    PMPI_Type_size( req->payload.io.datatype, &sz );
                    scorep_mpi_get_count( interopStatus, req->payload.io.datatype, &count );

                    SCOREP_IoHandleHandle io_handle = SCOREP_IoMgmt_GetIoHandle( SCOREP_IO_PARADIGM_MPI,
                                                                                 &( req->payload.io.fh ) );
                    if ( io_handle != SCOREP_INVALID_IO_HANDLE )
                    {
                        SCOREP_IoOperationComplete( io_handle,
                                                    req->payload.io.mode,
                                                    ( uint64_t )sz * count,
                                                    req->id /* matching id */ );
                    }
                }
                break;

            case SCOREP_MPI_REQUEST_TYPE_ICOLL:
                if ( coll_events_active )
                {
                    SCOREP_MpiNonBlockingCollectiveComplete(
                        req->payload.icoll.comm_handle,
                        req->payload.icoll.root_loc,
                        req->payload.icoll.coll_type,
                        req->payload.icoll.bytes_sent,
                        req->payload.icoll.bytes_recv,
                        req->id );
                }
                break;

            default:
                break;
        }
    }
}

void
scorep_mpi_cleanup_request( scorep_mpi_request* req )
{
    if ( !req ||
         ( ( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT ) &&
           !( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE ) ) )
    {
        return;
    }

    if ( req->flags & SCOREP_MPI_REQUEST_FLAG_IS_PERSISTENT )
    {
        /* if persistent request, set to inactive and incomplete,
           and, if requested delete request */
        req->flags &= ~SCOREP_MPI_REQUEST_FLAG_IS_ACTIVE;
        req->flags &= ~SCOREP_MPI_REQUEST_FLAG_IS_COMPLETED;
        if ( req->flags & SCOREP_MPI_REQUEST_FLAG_DEALLOCATE )
        {
            scorep_mpi_request_free( req );
        }
    }
    else
    {
        /* if non-persistent request, delete always request */
        scorep_mpi_request_free( req );
    }
}

void*
scorep_mpi_get_request_f2c_array( size_t count )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    scorep_mpi_req_mgmt_storage_array_grow( location,
                                            sizeof( MPI_Request ),
                                            &( storage->f2c_request_array ),
                                            count );

    return storage->f2c_request_array.loc;
}

void
scorep_mpi_save_request_array( MPI_Request* requestArray, size_t count )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    scorep_mpi_req_mgmt_storage_array_grow( location,
                                            sizeof( MPI_Request ),
                                            &( storage->request_array ),
                                            count );

    memcpy( storage->request_array.loc, requestArray, count * sizeof( MPI_Request ) );
}

scorep_mpi_request*
scorep_mpi_saved_request_get( size_t i )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    UTILS_ASSERT( i < storage->request_array.count );
    return scorep_mpi_request_get( ( ( MPI_Request* )storage->request_array.loc )[ i ] );
}

/**
 * Get a pointer to a status array of at least 'count' statuses
 * @param  count minimal requested array size
 * @return pointer to status array
 */
MPI_Status*
scorep_mpi_get_status_array( size_t count )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    scorep_mpi_req_mgmt_storage_array_grow( location,
                                            sizeof( MPI_Status ),
                                            &( storage->status_array ),
                                            count );

    return storage->status_array.loc;
}
