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
#include <scorep_mpi_topo.h>
#include <scorep_mpi_c.h>


void
scorep_mpi_topo_create_cart_definition_fromF08( char*     namePrefix,
                                                MPI_Fint* cartComm )
{
    MPI_Comm cart_comm_c = PMPI_Comm_f2c( *cartComm );
    scorep_mpi_topo_create_cart_definition( namePrefix, &cart_comm_c );
}
