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
 * @brief      Defines types for the score tool.
 */

#ifndef SCOREP_SCORE_TYPES_H
#define SCOREP_SCORE_TYPES_H

#include <string>
#include <stdint.h>

#define SCOREP_SCORE_TYPES \
    SCOREP_SCORE_TYPE( ALL, POSSIBLE ) \
    SCOREP_SCORE_TYPE( FLT, YES )      \
    SCOREP_SCORE_TYPE( USR, POSSIBLE ) \
    SCOREP_SCORE_TYPE( COM, POSSIBLE ) \
    SCOREP_SCORE_TYPE( MPI, NO )       \
    SCOREP_SCORE_TYPE( OMP, NO )       \
    SCOREP_SCORE_TYPE( SHMEM, NO )     \
    SCOREP_SCORE_TYPE( PTHREAD, NO )

#define SCOREP_SCORE_FILTER_STATES \
    SCOREP_SCORE_FILTER_STATE( UNSPECIFIED, ' ' ) \
    SCOREP_SCORE_FILTER_STATE( YES,         '+' ) \
    SCOREP_SCORE_FILTER_STATE( NO,          '-' ) \
    SCOREP_SCORE_FILTER_STATE( POSSIBLE,    '*' ) \

/**
 * Defines an enumeration of the group types.
 */
typedef enum
{
    #define SCOREP_SCORE_TYPE( type, flt ) SCOREP_SCORE_TYPE_ ## type,
    SCOREP_SCORE_TYPES
    #undef SCOREP_SCORE_TYPE
} SCOREP_Score_Type;

extern const uint64_t SCOREP_SCORE_TYPE_NUM;

/**
 * Defines an enumaration of the available filter states.
 */
typedef enum
{
    #define SCOREP_SCORE_FILTER_STATE( state, symbol ) SCOREP_SCORE_FILTER_ ## state,
    SCOREP_SCORE_FILTER_STATES
    #undef SCOREP_SCORE_FILTER_STATE
} SCOREP_Score_FilterState;

/**
 * Returns the name associated with a group type.
 * @param type  The group type.
 */
std::string
SCOREP_Score_getTypeName( uint64_t type );

/**
 * Returns the filter possibilities for a group type.
 * @param type  The group type for which the filter state is requested.
 */
SCOREP_Score_FilterState
SCOREP_Score_getFilterState( uint64_t type );

/**
 * Returns the symbol (*,-,+) associated with a particular filter state.
 * @param state  The filter state.
 */
char
SCOREP_Score_getFilterSymbol( SCOREP_Score_FilterState state );

#endif // SCOREP_SCORE_TYPES_H
