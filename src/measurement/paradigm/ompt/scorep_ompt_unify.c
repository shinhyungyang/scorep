/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 *  Implementation of OMPT unification routines.
 */

#include <config.h>

#include <UTILS_Error.h>
#define SCOREP_DEBUG_MODULE_NAME OMPT
#include <UTILS_Debug.h>

#include <SCOREP_Definitions.h>


#include "scorep_ompt.h"
#include "scorep_unify_helpers.h"
#include "scorep_status.h"


void
scorep_ompt_unify_pre( void )
{
    UTILS_DEBUG_ENTRY();
    uint32_t offset = scorep_unify_helper_define_comm_locations(
        SCOREP_GROUP_OPENMP_TARGET_LOCATIONS,
        "OpenMP Target",
        ( uint64_t )scorep_ompt_my_location_count,
        scorep_ompt_my_location_ids );

    /* Create subgroup for our locations as indices into the globally collated
     * OpenMP locations */
    for ( uint32_t i = 0; i < scorep_ompt_my_location_count; ++i )
    {
        scorep_ompt_my_location_ids[ i ] = i + offset;
    }

    SCOREP_GroupHandle group_handle = SCOREP_Definitions_NewGroup(
        SCOREP_GROUP_OPENMP_TARGET_GROUP,
        "OPENMP_TARGET_GROUP",
        scorep_ompt_my_location_count,
        scorep_ompt_my_location_ids );

    SCOREP_LOCAL_HANDLE_DEREF( scorep_ompt_interim_communicator_handle,
                               InterimCommunicator )->unified =
        SCOREP_Definitions_NewCommunicator(
            group_handle,
            SCOREP_INVALID_STRING,
            SCOREP_INVALID_COMMUNICATOR,
            0, SCOREP_COMMUNICATOR_FLAG_NONE );
    UTILS_DEBUG_EXIT();
}

void
scorep_ompt_unify_post( void )
{
    if ( 0 == SCOREP_Status_GetRank() )
    {
        UTILS_DEBUG_ENTRY();
        /* Count the number of OMPT target locations, i.e. OpenMP virtual streams */
        uint32_t total_number_of_ompt_locations = 0;
        SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                             Location,
                                                             location )
        {
            if ( definition->location_type != SCOREP_LOCATION_TYPE_GPU ||
                 definition->paradigm      != SCOREP_PARADIGM_OPENMP_TARGET )
            {
                continue;
            }

            total_number_of_ompt_locations++;
        }
        SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

        if ( total_number_of_ompt_locations )
        {
            /* collect the global location ids for the OMPT target locations,
             * i.e. OpenMP virtual streams */
            uint64_t ompt_locations[ total_number_of_ompt_locations ];
            total_number_of_ompt_locations = 0;
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                                 Location,
                                                                 location )
            {
                if ( definition->location_type != SCOREP_LOCATION_TYPE_GPU ||
                     definition->paradigm      != SCOREP_PARADIGM_OPENMP_TARGET )
                {
                    continue;
                }
                ompt_locations[ total_number_of_ompt_locations++ ]
                    = definition->global_location_id;
            }
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

            UTILS_DEBUG( "Created OMPT group with %u locations", total_number_of_ompt_locations );
            /* define the group of locations for all OMPT locations */
            SCOREP_Definitions_NewUnifiedGroup( SCOREP_GROUP_LOCATIONS,
                                                "OPENMP_TARGET_GROUP",
                                                total_number_of_ompt_locations,
                                                ompt_locations );
        }
        UTILS_DEBUG_EXIT();
    }
}
