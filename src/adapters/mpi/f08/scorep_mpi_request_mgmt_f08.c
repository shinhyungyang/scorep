/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
#include <config.h>

#include <SCOREP_Types.h>
#include <UTILS_Error.h>

#include <scorep_mpi_c.h>
#include <scorep_mpi_rma_request.h>
#include <scorep_mpi_request_mgmt.h>


int
scorep_mpi_request_is_null_fromF08( scorep_mpi_request* scorepRequest )
{
    return scorepRequest == NULL;
}


void
scorep_mpi_request_p2p_create_fromF08( MPI_Fint*                request,
                                       scorep_mpi_request_type* type,
                                       scorep_mpi_request_flag* flags,
                                       int*                     tag,
                                       int*                     dest,
                                       uint64_t*                bytes,
                                       MPI_Fint*                datatype,
                                       MPI_Fint*                comm,
                                       SCOREP_MpiRequestId*     id )
{
    scorep_mpi_request_p2p_create(
        MPI_Request_f2c( *request ),
        *type,
        *flags,
        *tag,
        *dest,
        *bytes,
        MPI_Type_f2c( *datatype ),
        MPI_Comm_f2c( *comm ),
        *id );
}


void
scorep_mpi_request_comm_idup_create_fromF08( MPI_Fint*            request,
                                             MPI_Fint*            parentComm,
                                             void*                newComm,
                                             SCOREP_MpiRequestId* id )
{
    scorep_mpi_request_comm_idup_create_interop(
        PMPI_Request_f2c( *request ),
        PMPI_Comm_f2c( *parentComm ),
        SCOREP_MPI_LANGUAGE_F08,
        newComm,
        *id );
}

void
scorep_mpi_request_icoll_create_fromF08( MPI_Fint*                request,
                                         scorep_mpi_request_flag* flags,
                                         SCOREP_CollectiveType*   collectiveType,
                                         MPI_Fint*                rootLoc,
                                         uint64_t*                bytesSent,
                                         uint64_t*                bytesRecv,
                                         MPI_Fint*                comm,
                                         SCOREP_MpiRequestId*     id )
{
    scorep_mpi_request_icoll_create(
        PMPI_Request_f2c( *request ),
        *flags,
        *collectiveType,
        *rootLoc,
        *bytesSent,
        *bytesRecv,
        PMPI_Comm_f2c( *comm ),
        *id
        );
}

void
scorep_mpi_request_win_create_fromF08( MPI_Fint*               mpiRequest,
                                       scorep_mpi_rma_request* rmaRequest )
{
    scorep_mpi_request_win_create( PMPI_Request_f2c( *mpiRequest ), rmaRequest );
}

void
scorep_mpi_request_io_create_fromF08( MPI_Fint*               request,
                                      SCOREP_IoOperationMode* mode,
                                      uint64_t*               bytes,
                                      MPI_Fint*               datatype,
                                      MPI_Fint*               fh,
                                      SCOREP_MpiRequestId*    id )
{
    scorep_mpi_request_io_create( PMPI_Request_f2c( *request ),
                                  *mode,
                                  *bytes,
                                  PMPI_Type_f2c( *datatype ),
                                  PMPI_File_f2c( *fh ),
                                  *id );
}

scorep_mpi_request*
scorep_mpi_request_get_fromF08( MPI_Fint* request )
{
    return scorep_mpi_request_get(  MPI_Request_f2c( *request ) );
}


void
scorep_mpi_save_f08_request_array_fromF08( MPI_Fint* requestArray,
                                           size_t    count )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    scorep_mpi_req_mgmt_storage_array_grow( location,
                                            sizeof( MPI_Request ),
                                            &( storage->f08_request_array ),
                                            count );

    for ( size_t i = 0; i < count; ++i )
    {
        ( ( MPI_Request* )storage->f08_request_array.loc )[ i ] = PMPI_Request_f2c( requestArray[ i ] );
    }
}

scorep_mpi_request*
scorep_mpi_saved_f08_request_get_fromF08( size_t arrayIndex )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    UTILS_ASSERT( arrayIndex >= 0 );
    UTILS_ASSERT( arrayIndex < storage->f08_request_array.count );
    return scorep_mpi_request_get( ( ( MPI_Request* )storage->f08_request_array.loc )[ arrayIndex ] );
}

void*
scorep_mpi_get_f08_status_array_fromF08( size_t count )
{
    SCOREP_Location*                         location = SCOREP_Location_GetCurrentCPULocation();
    scorep_mpi_req_mgmt_location_data* const storage  = SCOREP_Location_GetSubsystemData( location,
                                                                                          scorep_mpi_subsystem_id );

    scorep_mpi_req_mgmt_storage_array_grow( location,
                                            scorep_mpi_sizeof_f08_status_toF08(),
                                            &( storage->f08_status_array ),
                                            count );


    return storage->f08_status_array.loc;
}
