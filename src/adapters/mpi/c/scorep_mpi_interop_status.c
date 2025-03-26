/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
#include <config.h>
#include <scorep_mpi_interop_status.h>

/**
   @file
   @ingroup    MPI_Wrapper

   @brief Work with F08 type(MPI_Status) objects in C
 */

#if HAVE( MPI_USEMPIF08_SUPPORT )

scorep_mpi_interop_status
scorep_mpi_interop_status_create( MPI_Status* status )
{
    scorep_mpi_interop_status interop_status;

    interop_status.status          = ( void* )status;
    interop_status.origin_language = SCOREP_MPI_LANGUAGE_C;

    return interop_status;
}


void
scorep_mpi_status_source( const scorep_mpi_interop_status* interopStatus,
                          int*                             source )
{
    MPI_Status* status;
    switch ( interopStatus->origin_language )
    {
        case SCOREP_MPI_LANGUAGE_C:
            status  = ( MPI_Status* )interopStatus->status;
            *source = status->MPI_SOURCE;
            break;
        case SCOREP_MPI_LANGUAGE_F08:
            scorep_mpi_status_source_toF08( interopStatus->status, source );
            break;
    }
}

void
scorep_mpi_status_tag( const scorep_mpi_interop_status* interopStatus,
                       int*                             tag )
{
    MPI_Status* status;
    switch ( interopStatus->origin_language )
    {
        case SCOREP_MPI_LANGUAGE_C:
            status = ( MPI_Status* )interopStatus->status;
            *tag   = status->MPI_TAG;
            break;
        case SCOREP_MPI_LANGUAGE_F08:
            scorep_mpi_status_tag_toF08( interopStatus->status, tag );
            break;
    }
}


void
scorep_mpi_status_error( const scorep_mpi_interop_status* interopStatus,
                         int*                             error )
{
    MPI_Status* status;
    switch ( interopStatus->origin_language )
    {
        case SCOREP_MPI_LANGUAGE_C:
            status = ( MPI_Status* )interopStatus->status;
            *error = status->MPI_ERROR;
            break;
        case SCOREP_MPI_LANGUAGE_F08:
            scorep_mpi_status_error_toF08( interopStatus->status, error );
            break;
    }
}

void
scorep_mpi_get_count( const scorep_mpi_interop_status* interopStatus,
                      MPI_Datatype                     datatype,
                      int*                             count )
{
    MPI_Status* status;
    MPI_Fint    datatype_f08;
    switch ( interopStatus->origin_language )
    {
        case SCOREP_MPI_LANGUAGE_C:
            status = ( MPI_Status* )interopStatus->status;
            PMPI_Get_count( status, datatype, count );
            break;
        case SCOREP_MPI_LANGUAGE_F08:
            datatype_f08 = MPI_Type_c2f( datatype );
            scorep_mpi_get_count_toF08( interopStatus->status, &datatype_f08, count );
            break;
    }
}

void
scorep_mpi_test_cancelled( const scorep_mpi_interop_status* interopStatus,
                           int*                             cancelled )
{
    MPI_Status* status;
    switch ( interopStatus->origin_language )
    {
        case SCOREP_MPI_LANGUAGE_C:
            status = ( MPI_Status* )interopStatus->status;
            PMPI_Test_cancelled( status, cancelled );
            break;
        case SCOREP_MPI_LANGUAGE_F08:
            scorep_mpi_test_cancelled_toF08( interopStatus->status, cancelled );
            break;
    }
}

#else /* ! HAVE( MPI_USEMPIF08_SUPPORT ) */

scorep_mpi_interop_status
scorep_mpi_interop_status_create( MPI_Status* status )
{
    const scorep_mpi_interop_status interop_status = { .status = status };
    return interop_status;
}


void
scorep_mpi_status_source( const scorep_mpi_interop_status* interopStatus,
                          int*                             source )
{
    *source = interopStatus->status->MPI_SOURCE;
}

void
scorep_mpi_status_tag( const scorep_mpi_interop_status* interopStatus,
                       int*                             tag )
{
    *tag = interopStatus->status->MPI_TAG;
}


void
scorep_mpi_status_error( const scorep_mpi_interop_status* interopStatus,
                         int*                             error )
{
    *error = interopStatus->status->MPI_ERROR;
}

void
scorep_mpi_get_count( const scorep_mpi_interop_status* interopStatus,
                      MPI_Datatype                     datatype,
                      int*                             count )
{
    PMPI_Get_count( interopStatus->status, datatype, count );
}

void
scorep_mpi_test_cancelled( const scorep_mpi_interop_status* interopStatus,
                           int*                             cancelled )
{
    PMPI_Test_cancelled( interopStatus->status, cancelled );
}

#endif /* ! HAVE( MPI_USEMPIF08_SUPPORT ) */
