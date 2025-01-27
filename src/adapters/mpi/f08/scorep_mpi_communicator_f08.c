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
#include <scorep_mpi_communicator.h>

void
scorep_mpi_comm_free_fromF08( MPI_Fint* comm )
{
    scorep_mpi_comm_free( PMPI_Comm_f2c( *comm ) );
}

void
scorep_mpi_group_create_fromF08( MPI_Fint* group )
{
    scorep_mpi_group_create( PMPI_Group_f2c( *group ) );
}

void
scorep_mpi_group_free_fromF08( MPI_Fint* group )
{
    scorep_mpi_group_free( PMPI_Group_f2c( *group ) );
}

SCOREP_Mpi_GroupHandle
scorep_mpi_group_handle_fromF08( MPI_Fint* group )
{
    return scorep_mpi_group_handle( PMPI_Group_f2c( *group ) );
}

void
scorep_mpi_comm_set_name_fromF08( MPI_Fint*   comm,
                                  const char* name )
{
    scorep_mpi_comm_set_name( PMPI_Comm_f2c( *comm ), name );
}


SCOREP_RmaWindowHandle
scorep_mpi_win_create_fromF08( const char* name,
                               MPI_Fint*   win,
                               MPI_Fint*   comm )
{
    return scorep_mpi_win_create( name, PMPI_Win_f2c( *win ), PMPI_Comm_f2c( *comm ) );
}

void
scorep_mpi_win_free_fromF08( MPI_Fint* win )
{
    scorep_mpi_win_free( PMPI_Win_f2c( *win ) );
}

SCOREP_RmaWindowHandle
scorep_mpi_win_handle_fromF08( MPI_Fint* win )
{
    return scorep_mpi_win_handle( PMPI_Win_f2c( *win ) );
}

SCOREP_CollectiveType
scorep_mpi_win_collective_type_fromF08( MPI_Fint* win )
{
    return scorep_mpi_win_collective_type( PMPI_Win_f2c( *win ) );
}

void
scorep_mpi_win_set_name_fromF08( MPI_Fint*   win,
                                 const char* name )
{
    scorep_mpi_win_set_name( PMPI_Win_f2c( *win ), name );
}

void
scorep_mpi_epoch_start_fromF08( MPI_Fint*             win,
                                MPI_Fint*             group,
                                SCOREP_Mpi_EpochType* epochType )
{
    scorep_mpi_epoch_start( PMPI_Win_f2c( *win ), PMPI_Group_f2c( *group ), *epochType );
}

void
scorep_mpi_epoch_end_fromF08( MPI_Fint*             win,
                              SCOREP_Mpi_EpochType* epochType )
{
    scorep_mpi_epoch_end( PMPI_Win_f2c( *win ), *epochType );
}

SCOREP_Mpi_GroupHandle
scorep_mpi_epoch_get_group_handle_fromF08( MPI_Fint*             win,
                                           SCOREP_Mpi_EpochType* epochType )
{
    return scorep_mpi_epoch_get_group_handle( PMPI_Win_f2c( *win ), *epochType );
}
