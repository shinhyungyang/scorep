/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012, 2016, 2024,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2012, 2014, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#include <config.h>

#include <scorep_mpi_groups.h>

void
scorep_mpi_enable_derived_groups( void )
{
    /* See derived groups in enum scorep_mpi_groups. */
    #define ENABLE_DERIVED_GROUP( G1, G2 ) \
    if ( ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_##G1 ) && ( scorep_mpi_enabled & SCOREP_MPI_ENABLED_##G2 ) ) \
    { \
        scorep_mpi_enabled |= SCOREP_MPI_ENABLED_##G1##_##G2; \
    }

    ENABLE_DERIVED_GROUP( CG, ERR );
    ENABLE_DERIVED_GROUP( CG, EXT );
    ENABLE_DERIVED_GROUP( CG, MISC );
    ENABLE_DERIVED_GROUP( IO, ERR );
    ENABLE_DERIVED_GROUP( IO, MISC );
    ENABLE_DERIVED_GROUP( RMA, ERR );
    ENABLE_DERIVED_GROUP( RMA, EXT );
    ENABLE_DERIVED_GROUP( RMA, MISC );
    ENABLE_DERIVED_GROUP( TYPE, EXT );
    ENABLE_DERIVED_GROUP( TYPE, MISC );

    #undef ENABLE_DERIVED_GROUP

    /* Enable REQUEST group if any of its depending groups is enabled */
    uint64_t enable_request_mask = SCOREP_MPI_ENABLED_CG   |
                                   SCOREP_MPI_ENABLED_COLL |
                                   SCOREP_MPI_ENABLED_EXT  |
                                   SCOREP_MPI_ENABLED_IO   |
                                   SCOREP_MPI_ENABLED_P2P  |
                                   SCOREP_MPI_ENABLED_RMA;
    if ( scorep_mpi_enabled & enable_request_mask )
    {
        scorep_mpi_enabled |= SCOREP_MPI_ENABLED_REQUEST;
    }
}
