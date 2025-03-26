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

#include <scorep_mpi_io_mgmt.h>

void
scorep_mpi_io_split_begin_fromF08( SCOREP_IoHandleHandle* ioHandle,
                                   uint64_t*              matchingId,
                                   MPI_Fint*              datatype )
{
    scorep_mpi_io_split_begin( *ioHandle, *matchingId, PMPI_Type_f2c( *datatype ) );
}

void
scorep_mpi_io_split_end_fromF08( SCOREP_IoHandleHandle* ioHandle,
                                 uint64_t*              matchingId,
                                 MPI_Fint*              datatype )
{
    MPI_Datatype datatype_c;
    scorep_mpi_io_split_end( *ioHandle, matchingId, &datatype_c );
    *datatype = PMPI_Type_c2f( datatype_c );
}
