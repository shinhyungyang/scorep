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
#include <scorep_mpi_c.h>
#include <scorep_mpi_communicator_mgmt.h>

/** To be called from Fortran as a replacement for the macro SCOREP_MPI_COMM_HANDLE()
 * We need to translate the handle first.
 *
 * This is easier to implement in C than in Fortran.
 *
 * In Fortran, we cannot directly access the scorep_mpi_world_type struct, as this contains a C-Handle to an MPI_Group, which is not
 * interoperable.
 */
SCOREP_InterimCommunicatorHandle
scorep_mpi_comm_handle_fromF08( MPI_Fint* comm )
{
    MPI_Comm comm_c = MPI_Comm_f2c( *comm );

    /* In scorep_mpi_communicator_mgmt.h this comparison is done
     * in the macro SCOREP_MPI_COMM_HANDLE, which compares communicator
     * handles with a == operator.
     * This should not be done according to the MPI standard.
     * Therefore we use the comparison function provided by MPI.
     */
    int comparison_result;
    PMPI_Comm_compare( comm_c, MPI_COMM_WORLD, &comparison_result );
    if ( comparison_result == MPI_IDENT )
    {
        return scorep_mpi_world.handle;
    }
    return scorep_mpi_comm_handle( comm_c );
}

SCOREP_InterimCommunicatorHandle
scorep_mpi_comm_create_fromF08( MPI_Fint* comm,
                                MPI_Fint* parentComm )
{
    return scorep_mpi_comm_create( PMPI_Comm_f2c( *comm ), PMPI_Comm_f2c( *parentComm ) );
}
