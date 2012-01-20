/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */


/**
 * @file       SCOREP_Tracing.c
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include <scorep_utility/SCOREP_Debug.h>

#include <scorep_status.h>
#include <scorep_environment.h>
#include <scorep_runtime_management.h>
#include <scorep_clock_synchronization.h>
#include <scorep_thread.h>
#include <SCOREP_Memory.h>
#include <SCOREP_Timing.h>
#include <scorep_definitions.h>
#include <scorep_definition_structs.h>
#include <scorep_mpi.h>

#include "SCOREP_Tracing.h"
#include "scorep_tracing_definitions.h"
#include "scorep_tracing_file_substrate.h"

/* *INDENT-OFF* */
/* *INDENT-ON*  */


static OTF2_Archive* scorep_otf2_archive;


/** @todo croessel in OTF2_Archive_Open we need to specify an event
    chunk-size and a definition chunk size. the chnunk size need to be
    larger than the largest item that is written. events are relatively
    small whereas some definition record grow with the number of
    processes. We nee two environment variable here. does the event chunk
    size equal the memory page size (scorep_env_page_size)? */
#define SCOREP_TRACING_CHUNK_SIZE ( 1024 * 1024 )


static OTF2_Compression
scorep_tracing_get_compression()
{
    if ( SCOREP_Env_CompressTraces() )
    {
        return OTF2_COMPRESSION_ZLIB;
    }
    else
    {
        return OTF2_COMPRESSION_NONE;
    }
}


static OTF2_FlushType
scorep_on_trace_pre_flush( void*         userData,
                           OTF2_FileType fileType,
                           uint64_t      locationId,
                           void*         callerData,
                           bool          final )
{
    if ( !SCOREP_Mpi_IsInitialized() )
    {
        // flush before MPI_Init, we are lost.
        assert( false );
    }

    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_TRACING,
                         "[%d]: %s flush on %s#%" PRIu64 "\n",
                         SCOREP_Mpi_GetRank(),
                         final ? "final" : "intermediate",
                         fileType == OTF2_FILETYPE_GLOBAL_DEFS ? "GlobalDef" :
                         fileType == OTF2_FILETYPE_LOCAL_DEFS ? "Def" : "Evt",
                         fileType == OTF2_FILETYPE_GLOBAL_DEFS ? 0 : locationId );

    if ( fileType == OTF2_FILETYPE_EVENTS && !final )
    {
        /* A buffer flush happen in an event buffer before the end of the measurement */

        /*
         * disable recording, but forever
         * - not via scorep_recording_enabled,
         *   scorep_recording_enabled can be enabled by the user again.
         * - not via SCOREP_IsTracingEnabled()
         *   because we still need this to correctly finalize
         */

        if ( SCOREP_Env_RunVerbose() )
        {
            fprintf( stderr,
                     "Score-P: Trace buffer flush on rank %d.\n",
                     SCOREP_Mpi_GetRank() );
            fprintf( stderr,
                     "Score-P: Increase SCOREP_TOTAL_MEMORY and try again.\n" );
        }
    }

    OTF2_FlushType do_flush = OTF2_FLUSH;
    if ( final )
    {
        /* Always flush if this is the final one. */
        do_flush = OTF2_FLUSH;
    }

    return do_flush;
}


static uint64_t
scorep_on_trace_post_flush( void*         userData,
                            OTF2_FileType fileType,
                            uint64_t      locationId )
{
    /* remember that we have flushed the first time
     * after this point, we can't switch into MPI mode anymore
     */
    SCOREP_Otf2_OnFlush();

    return SCOREP_GetClockTicks();
}


static OTF2_FlushCallbacks flush_callbacks =
{
    .otf2_pre_flush  = scorep_on_trace_pre_flush,
    .otf2_post_flush = scorep_on_trace_post_flush
};


static void
scorep_tracing_register_flush_callbacks( OTF2_Archive* archive )
{
    SCOREP_Error_Code status =
        OTF2_Archive_SetFlushCallbacks( archive,
                                        &flush_callbacks,
                                        NULL );
    assert( status == SCOREP_SUCCESS );
}


static void*
scorep_tracing_chunk_allocate( void*         userData,
                               OTF2_FileType fileType,
                               uint64_t      locationId,
                               void**        perBufferData,
                               uint64_t      chunkSize )
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_FUNCTION_ENTRY,
                         "%" PRIu64, chunkSize );

    if ( !*perBufferData )
    {
        /* This manager has a pre-allocated page, which is much smaller
           than the chunksize, which is wasted now */
        *perBufferData = SCOREP_Memory_CreatePageManager();
    }

    void* chunk = SCOREP_Allocator_Alloc( *perBufferData, chunkSize );

    /* ignore allocation failures, OTF2 will flush and free chunks */

    return chunk;
}


static void
scorep_tracing_chunk_free_all( void*         userData,
                               OTF2_FileType fileType,
                               uint64_t      locationId,
                               void**        perBufferData,
                               bool          final )
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_FUNCTION_ENTRY,
                         "%s", final ? "final" : "intermediate" );

    /* maybe we were called without one allocate */
    if ( !*perBufferData )
    {
        return;
    }

    /* drop all used pages */
    SCOREP_Allocator_Free( *perBufferData );

    if ( final )
    {
        /* drop also the page manager */
        SCOREP_Allocator_DeletePageManager( *perBufferData );
    }
}


static OTF2_MemoryCallbacks scorep_tracing_chunk_callbacks =
{
    .otf2_allocate = scorep_tracing_chunk_allocate,
    .otf2_free_all = scorep_tracing_chunk_free_all
};


static void
scorep_tracing_register_memory_callbacks( OTF2_Archive* archive )
{
    SCOREP_Error_Code status =
        OTF2_Archive_SetMemoryCallbacks( archive,
                                         &scorep_tracing_chunk_callbacks,
                                         NULL );
    assert( status == SCOREP_SUCCESS );
}


void
SCOREP_Tracing_Initialize()
{
    assert( !scorep_otf2_archive );

    /* @todo croessel step1: remove the "4 *" intoduced on Michael's request
     * when overflow checking for definitions is implemented.
     * step2: provide environment variables to adjust the chunck sizes.
     * For the scorep_tracing_get_file_substrate() see paradigm/scorep_sion_*.c
     */
    scorep_otf2_archive = OTF2_Archive_Open( SCOREP_GetExperimentDirName(),
                                             "traces",
                                             OTF2_FILEMODE_WRITE,
                                             SCOREP_TRACING_CHUNK_SIZE,
                                             4 * SCOREP_TRACING_CHUNK_SIZE,
                                             scorep_tracing_get_file_substrate(),
                                             scorep_tracing_get_compression() );
    assert( scorep_otf2_archive );

    scorep_tracing_register_flush_callbacks( scorep_otf2_archive );
    scorep_tracing_register_sion_callbacks( scorep_otf2_archive );
    scorep_tracing_register_memory_callbacks( scorep_otf2_archive );

    OTF2_Archive_SetCreator( scorep_otf2_archive, "Score-P " PACKAGE_VERSION );
}


void
SCOREP_Tracing_Finalize()
{
    assert( scorep_otf2_archive );
    /// @todo? set archive to "unified"/"not unified"
    OTF2_Archive_Close( scorep_otf2_archive );
    scorep_otf2_archive = 0;
}


SCOREP_Error_Code
SCOREP_Tracing_SetIsMaster( bool isMaster )
{
    // call this function only once
    static int master_mode_set;
    assert( !master_mode_set );
    master_mode_set = 1;

    return OTF2_Archive_SetMasterSlaveMode( scorep_otf2_archive,
                                            isMaster ? OTF2_MASTER : OTF2_SLAVE );
}


OTF2_EvtWriter*
SCOREP_Tracing_GetEventWriter()
{
    OTF2_EvtWriter* evt_writer =
        OTF2_Archive_GetEvtWriter( scorep_otf2_archive, OTF2_UNDEFINED_UINT64 );
    if ( !evt_writer )
    {
        /* aborts */
        SCOREP_Memory_HandleOutOfMemory();
    }

    return evt_writer;
}


static void
scorep_trace_finalize_event_writer_cb( SCOREP_Thread_LocationData* locationData,
                                       void*                       userData )
{
    SCOREP_LocationHandle       location_handle =
        SCOREP_Thread_GetLocationHandle( locationData );
    SCOREP_Location_Definition* location_definition =
        SCOREP_LOCAL_HANDLE_DEREF( location_handle, Location );

    SCOREP_Trace_LocationData* trace_data =
        SCOREP_Thread_GetTraceLocationData( locationData );

    assert( trace_data->otf_writer );

    uint64_t number_of_events;
    OTF2_EvtWriter_GetNumberOfEvents( trace_data->otf_writer,
                                      &number_of_events );

    /* update number of events */
    location_definition->number_of_events = number_of_events;

    /* close the event writer */
    OTF2_Archive_CloseEvtWriter( scorep_otf2_archive,
                                 trace_data->otf_writer );
    trace_data->otf_writer = NULL;
}


void
SCOREP_Tracing_FinalizeEventWriters( void )
{
    if ( !SCOREP_IsTracingEnabled() )
    {
        return;
    }

    assert( scorep_otf2_archive );

    SCOREP_Thread_ForAllLocations( scorep_trace_finalize_event_writer_cb,
                                   NULL );
}


void
SCOREP_Tracing_WriteDefinitions()
{
    assert( scorep_otf2_archive );

    extern SCOREP_DefinitionManager scorep_local_definition_manager;

    /* Write for all local locations the same local definition file */
    SCOREP_CreateExperimentDir();
    SCOREP_DEFINITION_FOREACH_DO( &scorep_local_definition_manager, Location, location )
    {
        OTF2_DefWriter* local_definition_writer = OTF2_Archive_GetDefWriter(
            scorep_otf2_archive,
            definition->global_location_id );
        if ( !local_definition_writer )
        {
            /* aborts */
            SCOREP_Memory_HandleOutOfMemory();
        }

        scorep_tracing_write_mappings( local_definition_writer );
        scorep_tracing_write_clock_offsets( local_definition_writer );
        scorep_tracing_write_local_definitions( local_definition_writer );

        OTF2_Archive_CloseDefWriter( scorep_otf2_archive,
                                     local_definition_writer );
    }
    SCOREP_DEFINITION_FOREACH_WHILE();


    uint64_t epoch_begin;
    uint64_t epoch_end;
    SCOREP_GetGlobalEpoch( &epoch_begin, &epoch_end );
    if ( SCOREP_Mpi_GetRank() == 0 )
    {
        OTF2_GlobalDefWriter* global_definition_writer =
            OTF2_Archive_GetGlobalDefWriter( scorep_otf2_archive );
        if ( !global_definition_writer )
        {
            /* aborts */
            SCOREP_Memory_HandleOutOfMemory();
        }

        OTF2_GlobalDefWriter_WriteTimeRange( global_definition_writer, epoch_begin, epoch_end - epoch_begin );
        scorep_tracing_write_global_definitions( global_definition_writer );

        /* There is no OTF2_Archive_CloseGlobalDefWriter in OTF2 */
    }
}
