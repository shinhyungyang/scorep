/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2017,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 * @file
 * @ingroup    SHMEM_Wrapper
 *
 * @brief Contains the initialization of the SHMEM adapter.
 */

#include <config.h>

#include <UTILS_Error.h>
#define SCOREP_DEBUG_MODULE_NAME SHMEM
#include <UTILS_Debug.h>

#include <SCOREP_Config.h>
#include <SCOREP_Subsystem.h>
#include <SCOREP_RuntimeManagement.h>
#include "scorep_shmem_internal.h"

#include <string.h>
#include <assert.h>


/**
 *  @internal
 *  Number of SHMEM processing elements.
 */
size_t scorep_shmem_number_of_pes;

/**
 * @internal
 * Processing element ID
 */
int scorep_shmem_my_rank;

/**
 * Matching ID of consecutive RMA operations.
 */
uint64_t scorep_shmem_rma_op_matching_id;

void
scorep_shmem_rank_and_size( void )
{
    /* Determine own SHMEM rank and number of all PEs. */
    scorep_shmem_number_of_pes = pshmem_n_pes();
    scorep_shmem_my_rank       = pshmem_my_pe();
}
