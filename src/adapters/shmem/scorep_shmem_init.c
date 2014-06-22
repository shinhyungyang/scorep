/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 * @file
 * @ingroup    SHMEM_Wrapper
 *
 * @brief Contains the initialization of the SHMEM subsystem.
 */

#include <config.h>

#include "scorep_shmem_internal.h"

#define SCOREP_DEBUG_MODULE_NAME SHMEM
#include <UTILS_Debug.h>

#include <SCOREP_Config.h>
#include <SCOREP_Subsystem.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Events.h>


/**
 * SHMEM subsystem ID
 */
static size_t subsystem_id;

/**
 * Flag to indicate whether event generation is turned on or off. If
 * it is set to TRUE, events are generated. If it is set to false, no
 * events are generated.
 */
bool scorep_shmem_generate_events = false;

/**
 * Flag to indicate whether an RmaOpCompleteRemote record should be
 * written or not. If it is set to TRUE, record will be generated.
 * If it is set to FALSE, no event record is generated.
 */
bool scorep_shmem_write_rma_op_complete_record = false;

/**
 * Flag to indicate whether the extra parallel region was entered
 * or not. For example, this extra parallel region will be entered
 * if the call to main is not recorded due to disabled compiler
 * instrumentation and the call to shmem_init()/start_pes() is the
 * first recorded event.
 */
bool scorep_shmem_parallel_entered = false;

/**
 * Implementation of the adapter_register function of the @ref
 * SCOREP_Subsystem struct for the initialization process of the SHMEM
 * adapter.
 */
static SCOREP_ErrorCode
register_adapter( size_t subsystemId )
{
    UTILS_DEBUG_ENTRY();

    subsystem_id = subsystemId;

    return SCOREP_SUCCESS;
}

static int
exit_callback( void )
{
    /* Exit the extra parallel region in case it was */
    /* entered in start_pes() */
    if ( scorep_shmem_parallel_entered )
    {
        SCOREP_ExitRegion( scorep_shmem_region__SHMEM );
    }

    /* Destroy all RmaWin in the master thread. */
    scorep_shmem_close_pe_group();

    scorep_shmem_generate_events = false;

    return 0;
}


/**
 * Implementation of the subsystem_init function of the @ref
 * SCOREP_Subsystem struct for the initialization process of the SHMEM
 * adapter.
 */
static SCOREP_ErrorCode
init_adapter( void )
{
    UTILS_DEBUG_ENTRY();

    scorep_shmem_register_regions();

    SCOREP_RegisterExitCallback( exit_callback );

    return SCOREP_SUCCESS;
}

/**
 * Implementation of the subsystem_init_location function of the @ref
 * SCOREP_Subsystem struct for the initialization process of the SHMEM
 * adapter.
 */
static SCOREP_ErrorCode
init_location( struct SCOREP_Location* locationData )
{
    UTILS_DEBUG_ENTRY();
    return SCOREP_SUCCESS;
}

/**
 * Implementation of the adapter_finalize_location function of the
 * @ref SCOREP_Subsystem struct for the initialization process of the
 * SHMEM adapter.
 */
static void
finalize_location( struct SCOREP_Location* locationData )
{
    UTILS_DEBUG_ENTRY();
}

/**
 * Define the group of all SHMEM locations.
 */
static SCOREP_ErrorCode
pre_unify( void )
{
    UTILS_DEBUG_ENTRY();

    /* Define the group of all SHMEM locations. */
    scorep_shmem_define_shmem_locations();

    return SCOREP_SUCCESS;
}

/**
 * Unify the SHMEM communicators.
 */
static SCOREP_ErrorCode
post_unify( void )
{
    UTILS_DEBUG_ENTRY();

    /* Unify the SHMEM communicators. */
    scorep_shmem_define_shmem_group();

    return SCOREP_SUCCESS;
}

/**
 * Implementation of the adapter_finalize function of the @ref
 * SCOREP_Subsystem struct for the initialization process of the SHMEM
 * adapter.
 */
static void
finalize_adapter( void )
{
    UTILS_DEBUG_ENTRY();

    /* Prevent all further events */
    SCOREP_SHMEM_EVENT_GEN_OFF();

    scorep_shmem_teardown_comm_world();

    UTILS_DEBUG_EXIT();
}

/**
 * Implementation of the adapter_deregister function of the @ref
 * SCOREP_Subsystem struct for the initialization process of the SHMEM
 * adapter.
 */
static void
deregister_adapter( void )
{
    UTILS_DEBUG_ENTRY();
    SCOREP_FinalizeMppMeasurement();
#if SHMEM_HAVE_DECL( SHMEM_FINALIZE )
    UTILS_DEBUG( "Calling shmem_finalize" );
    CALL_SHMEM( shmem_finalize ) ();
#endif
}

static void
control( SCOREP_Subsystem_Command command )
{
    switch ( command )
    {
        case SCOREP_SUBSYSTEM_COMMAND_ENABLE:
            scorep_shmem_generate_events = true;
            break;
        case SCOREP_SUBSYSTEM_COMMAND_DISABLE:
            scorep_shmem_generate_events = false;
            break;
    }
}

/** The initialization struct for the SHMEM adapter */
const SCOREP_Subsystem SCOREP_Subsystem_ShmemAdapter =
{
    .subsystem_name              = "SHMEM",
    .subsystem_register          = &register_adapter,
    .subsystem_init              = &init_adapter,
    .subsystem_init_location     = &init_location,
    .subsystem_finalize_location = &finalize_location,
    .subsystem_pre_unify         = &pre_unify,
    .subsystem_post_unify        = &post_unify,
    .subsystem_finalize          = &finalize_adapter,
    .subsystem_deregister        = &deregister_adapter,
    .subsystem_control           = &control
};
