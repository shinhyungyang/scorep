/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2013,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * @ingroup OPARI2
 *
 * @brief Implementation of the OPARI2 OpenMP adapter initialization.
 */

#include <config.h>

#include "SCOREP_Opari2_Openmp_Lock.h"
#include "SCOREP_Opari2_Openmp_Regions.h"

#include <SCOREP_Subsystem.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Paradigms.h>
#include <SCOREP_Location.h>

#include <opari2/pomp2_lib.h>

#define SCOREP_DEBUG_MODULE_NAME OPARI2
#include <UTILS_Debug.h>

SCOREP_Opari2_Openmp_Region* scorep_opari2_openmp_regions;

/** Flag to indicate whether the adapter is initialized */
bool scorep_opari2_openmp_is_initialized = false;

/** Flag to indicate whether the adapter is finalized */
bool scorep_opari2_openmp_is_finalized = false;

/** Lock to protect on-the-fly assignments.*/
SCOREP_Mutex scorep_opari2_openmp_assign_lock;


/* ***************************************************************************
*                                                        OPARI2 lock regions *
*****************************************************************************/

/** List of registered omp function names. They must be in the same order as the
    corresponding SCOREP_Opari2_Openmp_LockRegion_Index.
 */
static char* lock_region_names[] =
{
    "omp_init_lock",
    "omp_destroy_lock",
    "omp_set_lock",
    "omp_unset_lock",
    "omp_test_lock",
    "omp_init_nest_lock",
    "omp_destroy_nest_lock",
    "omp_set_nest_lock",
    "omp_unset_nest_lock",
    "omp_test_nest_lock"
};

/** List of handles for omp regions. The handles must be stored in the same order as
    the corresponding SCOREP_Opari2_Openmp_LockRegion_Index.
 */
SCOREP_RegionHandle scorep_opari2_openmp_lock_region_handles[ SCOREP_OPARI2_OPENMP_LOCK_NUM ];


/* *****************************************************************************
 *                                              OPARI2_OPENMP OpenMP subsystem *
 ******************************************************************************/

static size_t subsystem_id;

static SCOREP_ErrorCode
opari2_openmp_subsystem_register( size_t id )
{
    UTILS_DEBUG_ENTRY();

    subsystem_id = subsystem_id;

    return SCOREP_SUCCESS;
}

static SCOREP_ErrorCode
opari2_openmp_subsystem_init( void )
{
    UTILS_DEBUG_ENTRY();

    if ( !scorep_opari2_openmp_is_initialized )
    {
        SCOREP_Paradigms_RegisterParallelParadigm(
            SCOREP_PARADIGM_OPENMP,
            SCOREP_PARADIGM_CLASS_THREAD_FORK_JOIN,
            "OpenMP",
            SCOREP_PARADIGM_FLAG_NONE );
        SCOREP_Paradigms_SetStringProperty( SCOREP_PARADIGM_OPENMP,
                                            SCOREP_PARADIGM_PROPERTY_COMMUNICATOR_TEMPLATE,
                                            "Thread team ${id}" );

        scorep_opari2_openmp_is_initialized = true;
        SCOREP_MutexCreate( &scorep_opari2_openmp_assign_lock );
        scorep_opari2_openmp_lock_initialize();

        size_t n = POMP2_Get_num_regions();

        scorep_opari2_openmp_regions = calloc( n, sizeof( SCOREP_Opari2_Openmp_Region ) );

        /* Initialize regions inserted by Opari */
        POMP2_Init_regions();

        SCOREP_SourceFileHandle scorep_opari2_openmp_file_handle
            = SCOREP_Definitions_NewSourceFile( "OMP" );

        for ( int i = 0; i < SCOREP_OPARI2_OPENMP_LOCK_NUM; i++ )
        {
            scorep_opari2_openmp_lock_region_handles[ i ] =
                SCOREP_Definitions_NewRegion( lock_region_names[ i ],
                                              NULL,
                                              scorep_opari2_openmp_file_handle,
                                              SCOREP_INVALID_LINE_NO,
                                              SCOREP_INVALID_LINE_NO,
                                              SCOREP_PARADIGM_OPENMP,
                                              SCOREP_REGION_WRAPPER );
        }
    }

    UTILS_DEBUG_EXIT();

    return SCOREP_SUCCESS;
}

static void
opari2_openmp_subsystem_finalize( void )
{
    UTILS_DEBUG_ENTRY();

    if ( !scorep_opari2_openmp_is_initialized ||
         scorep_opari2_openmp_is_finalized )
    {
        return;
    }
    scorep_opari2_openmp_is_finalized = true;

    free( scorep_opari2_openmp_regions );
    SCOREP_MutexDestroy( &scorep_opari2_openmp_assign_lock );

    UTILS_DEBUG_EXIT();
}

const SCOREP_Subsystem SCOREP_Subsystem_Opari2OpenmpAdapter =
{
    .subsystem_name              = "OPARI2 OpenMP Adapter for the POMP2 interface / Version 1.0",
    .subsystem_register          = &opari2_openmp_subsystem_register,
    .subsystem_init              = &opari2_openmp_subsystem_init,
    .subsystem_init_location     = NULL,
    .subsystem_finalize_location = NULL,
    .subsystem_pre_unify         = NULL,
    .subsystem_post_unify        = NULL,
    .subsystem_finalize          = &opari2_openmp_subsystem_finalize,
    .subsystem_deregister        = NULL,
    .subsystem_control           = NULL
};
