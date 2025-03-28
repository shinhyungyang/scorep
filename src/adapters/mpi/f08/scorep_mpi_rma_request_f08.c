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

#include <scorep_mpi_rma_request.h>

scorep_mpi_rma_request*
scorep_mpi_rma_request_create_fromF08( SCOREP_RmaWindowHandle*       window,
                                       int*                          target,
                                       MPI_Fint*                     mpiHandle,
                                       SCOREP_Mpi_RmaCompletionType* completionType,
                                       SCOREP_MpiRequestId*          matchingId )
{
    return scorep_mpi_rma_request_create( *window,
                                          *target,
                                          PMPI_Request_f2c( *mpiHandle ),
                                          *completionType,
                                          *matchingId );
}

scorep_mpi_rma_request*
scorep_mpi_rma_request_find_fromF08( SCOREP_RmaWindowHandle*       window,
                                     int*                          target,
                                     MPI_Fint*                     mpiHandle,
                                     SCOREP_Mpi_RmaCompletionType* completionType )
{
    return scorep_mpi_rma_request_find( *window,
                                        *target,
                                        PMPI_Request_f2c( *mpiHandle ),
                                        *completionType );
}

void
scorep_mpi_rma_request_remove_fromF08( SCOREP_RmaWindowHandle*       window,
                                       int*                          target,
                                       MPI_Fint*                     mpiHandle,
                                       SCOREP_Mpi_RmaCompletionType* completionType )
{
    scorep_mpi_rma_request_remove( *window,
                                   *target,
                                   PMPI_Request_f2c( *mpiHandle ),
                                   *completionType );
}

SCOREP_MpiRequestId
scorep_mpi_rma_request_matching_id( scorep_mpi_rma_request* rmaRequest )
{
    return rmaRequest->matching_id;
}
