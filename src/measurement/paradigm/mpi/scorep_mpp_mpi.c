/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014, 2022,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2016,
 * Technische Universitaet Darmstadt, Germany
 *
 * Copyright (c) 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file
 *
 *
 *
 */

#include <config.h>
#include "scorep_mpp.h"
#include "scorep_ipc.h"
#include <scorep_status.h>

#include <stdio.h>

#define MPI_LOCATION_GROUP_NAME_LENGTH 32

static char name[ MPI_LOCATION_GROUP_NAME_LENGTH ];


char*
SCOREP_Mpp_GetLocationGroupName( void )
{
    int rank = SCOREP_Ipc_GetRank();
    snprintf( name, MPI_LOCATION_GROUP_NAME_LENGTH, "MPI Rank %d", rank );

    return name;
}

#undef MPI_LOCATION_GROUP_NAME_LENGTH
