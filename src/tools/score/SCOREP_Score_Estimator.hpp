/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2014,
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

/**
 * @file
 *
 * @brief      Defines a class which performs calculations for trace
 *             size estimation.
 */

#ifndef SCOREP_SCORE_ESTIMATOR_H
#define SCOREP_SCORE_ESTIMATOR_H

#include "SCOREP_Score_Profile.hpp"
#include "SCOREP_Score_Group.hpp"
#include "SCOREP_Score_Event.hpp"
#include <deque>

/**
 * This class implements the estimation logic.
 */
class SCOREP_Score_Estimator
{
public:
    /**
     * Creates an instance of SCOREP_Score_Estimator.
     * @param profile   A pointer to the profile.
     * @param denseNum  Number of dense metrics that should be recorded in the trace.
     */
    SCOREP_Score_Estimator( SCOREP_Score_Profile* profile,
                            uint64_t              denseBum );

    /**
     * Destructor.
     */
    virtual
    ~SCOREP_Score_Estimator();

    /**
     * Claculates the group an region data.
     * @param showRegions  Pass true if the user wants to see per region data
     *                     in addition to the groups.
     */
    void
    calculate( bool showRegions );

    /**
     * Prints the group information to the screen.
     */
    void
    printGroups( void );

    /**
     * Prints the per region information to the screen.
     */
    void
    printRegions( void );

    /**
     * Reads and evaluates a filter file.
     * @param filterFile  The name of the filter file.
     */
    void
    initializeFilter( std::string filterFile );

    /**
     * Dumps the event sizes to the screen, Used for debug purposes.
     */
    void
    dumpEventSizes( void );

private:
    /**
     * Checks whether @a region is filtered.
     * @param regionId  Specifies the region by its ID.
     */
    bool
    match_filter( uint64_t regionId );

    /**
     * Initialize per region data.
     */
    void
    initialize_regions( void );

    /**
     * Delete list of groups.
     * @param groups Pointer to an array of pointer to SCOREP_Score_Group instances.
     * @param num    Number of entries in @a group.
     */
    void
    delete_groups( SCOREP_Score_Group** groups,
                   uint64_t             num );

    /**
     * Initializes the event sizes.
     */
    void
    calculate_event_sizes( void );

private:
    /**
     * True, if a filter is used.
     */
    bool m_has_filter;

    /**
     * Stores the pointer to the profile.
     */
    SCOREP_Score_Profile* m_profile;

    /**
     * Array of pointers to the main groups (ALL, USR, MPI, COM, OMP).
     */
    SCOREP_Score_Group** m_groups;

    /**
     * Array of pointers to the regions. NULL if the user does not
     * want to see per region data.
     */
    SCOREP_Score_Group** m_regions;

    /**
     * Array of pointers to the groups that represent the filtered amount
     * of events.
     */
    SCOREP_Score_Group** m_filtered;

    /**
     * Stores the minimal required field widths of the score output.
     */
    SCOREP_Score_FieldWidths m_widths;

    /**
     * Stores the number of region definitons.
     */
    uint64_t m_region_num;

    /**
     * Stores the number of processes.
     */
    uint64_t m_process_num;

    /**
     * Stores the number of dense metrics that should be taken into account.
     */
    uint64_t m_dense_num;
};


#endif // SCOREP_SCORE_ESTIMATOR_H
