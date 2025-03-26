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
#include <stddef.h>
#include <SCOREP_IoManagement.h>
#include <mpi.h>

SCOREP_IoHandleHandle
SCOREP_IoMgmt_CompleteHandleCreation_fromF08( SCOREP_IoParadigmType* paradigm,
                                              SCOREP_IoFileHandle*   file,
                                              uint32_t*              unifyKey,
                                              MPI_Fint*              ioHandle )
{
    MPI_File ioHandle_c = PMPI_File_f2c( *ioHandle );
    return SCOREP_IoMgmt_CompleteHandleCreation( *paradigm, *file, *unifyKey, &ioHandle_c );
}

SCOREP_IoHandleHandle
SCOREP_IoMgmt_RemoveHandle_fromF08( SCOREP_IoParadigmType* paradigm,
                                    MPI_Fint*              ioHandle )
{
    MPI_File ioHandle_c = PMPI_File_f2c( *ioHandle );
    return SCOREP_IoMgmt_RemoveHandle( *paradigm, &ioHandle_c );
}

SCOREP_IoHandleHandle
SCOREP_IoMgmt_GetAndPushHandle_fromF08( SCOREP_IoParadigmType* paradigm,
                                        MPI_Fint*              ioHandle )
{
    MPI_File ioHandle_c = PMPI_File_f2c( *ioHandle );
    return SCOREP_IoMgmt_GetAndPushHandle( *paradigm, &ioHandle_c );
}
