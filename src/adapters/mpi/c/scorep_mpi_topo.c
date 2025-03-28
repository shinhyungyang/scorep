/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2017,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2016, 2022-2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * Copyright (c) 2016,
 * Technische Universitaet Darmstadt, Germany
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
 * @brief Topologies
 */

#include <config.h>
#include <SCOREP_Definitions.h>
#include "scorep_mpi_topo.h"
#include "scorep_mpi_communicator_mgmt.h"
#include "scorep_mpi_c.h"

#include <stdio.h>
#include <string.h>

void
scorep_mpi_topo_create_cart_definition( char*     namePrefix,
                                        MPI_Comm* cartComm )
{
    SCOREP_InterimCommunicatorHandle comm_id = scorep_mpi_comm_handle( *cartComm );

    int ndims;
    PMPI_Cartdim_get( *cartComm, &ndims );

    int dims[ ndims ], periods[ ndims ], coords[ ndims ];
    PMPI_Cart_get( *cartComm, ndims, dims, periods, coords );

    /* create topology name*/
    /* length = prefix + ndims separators + 11 bytes per dimension */
    char topo_name[ strlen( namePrefix ) + ndims + 11 * ndims ];
    strcpy( topo_name, namePrefix );
    for ( int i = 0; i < ndims; ++i )
    {
        sprintf( topo_name + strlen( topo_name ), "%c%d", ( i > 0 ) ? 'x' : '_', dims[ i ] );
    }

    /* create the cartesian topology definition record */
    SCOREP_CartesianTopologyHandle topo_id = SCOREP_Definitions_NewCartesianTopology( topo_name,
                                                                                      comm_id,
                                                                                      ndims,
                                                                                      dims,
                                                                                      periods,
                                                                                      NULL,
                                                                                      SCOREP_TOPOLOGIES_MPI );
    int my_rank;
    PMPI_Comm_rank( *cartComm, &my_rank );

    /* create the coordinates definition record */
    SCOREP_Definitions_NewCartesianCoords( topo_id, my_rank, 0, ndims, coords );
}
