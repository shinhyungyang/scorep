/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014, 2016, 2022, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 *  @file
 *
 */

#include <config.h>

#include <SCOREP_Types.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Memory.h>
#include <scorep_status.h>

#include <scorep_unify_helpers.h>

#include <scorep_opencl.h>
#include <scorep_opencl_config.h>

void
scorep_opencl_define_locations( void )
{
    uint32_t offset = scorep_unify_helper_define_comm_locations(
        SCOREP_GROUP_OPENCL_LOCATIONS,
        "OPENCL", scorep_opencl_my_location_count,
        scorep_opencl_my_location_ids );

    /* Create subgroup for our locations as indices into the globally collated
     * OpenCL locations */
    for ( size_t i = 0; i < scorep_opencl_my_location_count; i++ )
    {
        scorep_opencl_my_location_ids[ i ] = i + offset;
    }

    SCOREP_GroupHandle group_handle = SCOREP_Definitions_NewGroup(
        SCOREP_GROUP_OPENCL_GROUP,
        "OPENCL_GROUP",
        scorep_opencl_my_location_count,
        scorep_opencl_my_location_ids );

    SCOREP_LOCAL_HANDLE_DEREF( scorep_opencl_interim_communicator_handle,
                               InterimCommunicator )->unified =
        SCOREP_Definitions_NewCommunicator(
            group_handle,
            SCOREP_INVALID_STRING,
            SCOREP_INVALID_COMMUNICATOR,
            0, SCOREP_COMMUNICATOR_FLAG_NONE );
}

void
scorep_opencl_define_group( void )
{
    if ( 0 == SCOREP_Status_GetRank() )
    {
        /* Count the number of OpenCL locations */
        uint32_t total_number_of_opencl_locations = 0;
        SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                             Location,
                                                             location )
        {
            if ( definition->location_type != SCOREP_LOCATION_TYPE_GPU ||
                 definition->paradigm      != SCOREP_PARADIGM_OPENCL )
            {
                continue;
            }
            total_number_of_opencl_locations++;
        }
        SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

        if ( total_number_of_opencl_locations )
        {
            /* collect the global location ids for the OpenCL locations */
            uint64_t opencl_locations[ total_number_of_opencl_locations ];
            total_number_of_opencl_locations = 0;
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_BEGIN( scorep_unified_definition_manager,
                                                                 Location,
                                                                 location )
            {
                if ( definition->location_type != SCOREP_LOCATION_TYPE_GPU ||
                     definition->paradigm      != SCOREP_PARADIGM_OPENCL )
                {
                    continue;
                }
                opencl_locations[ total_number_of_opencl_locations++ ]
                    = definition->global_location_id;
            }
            SCOREP_DEFINITIONS_MANAGER_FOREACH_DEFINITION_END();

            /* define the group of locations for all OpenCL locations */
            SCOREP_Definitions_NewUnifiedGroup( SCOREP_GROUP_LOCATIONS,
                                                "OPENCL_GROUP",
                                                total_number_of_opencl_locations,
                                                opencl_locations );
        }
    }
}
