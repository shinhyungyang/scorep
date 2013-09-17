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
 * Copyright (c) 2009-2012, 2013
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


#ifndef SCOREP_INTERNAL_TYPES_H
#define SCOREP_INTERNAL_TYPES_H

#include <stdlib.h>
#include <stdio.h>


/**
 * @file
 *
 *
 * @brief Functions to convert types from @ref SCOREP_Types.h into strings.
 */


#include <SCOREP_Types.h>


/**
 * Converts a SCOREP_ParadigmType into a string.
 *
 * @param paradigmType   The paradigm type which should be expressed as a string.
 *
 * @return              A string representation of @a paradigmType.
 */
extern const char*
scorep_paradigm_type_to_string( SCOREP_ParadigmType paradigmType );


/**
 * Converts a SCOREP_RegionType into a string.
 *
 * @param regionType    The region type which should be expressed as a string.
 *
 * @return              A string representation of @a regionType.
 */
extern const char*
scorep_region_type_to_string( SCOREP_RegionType regionType );


/**
 * Converts a SCOREP_ParameterType into a string.
 *
 * @param parameterType The parameter type which should be expressed as a
 *                      string.
 *
 * @return              A string representation of @a parameterType.
 */
extern const char*
scorep_parameter_type_to_string( SCOREP_ParameterType parameterType );


/**
 * Converts a SCOREP_ConfigType into a string.
 *
 * @param configType    The config type which should be expressed as a string.
 *
 * @return              A string representation of @a configType.
 */
extern const char*
scorep_config_type_to_string( SCOREP_ConfigType configType );


/**
 * Converts a SCOREP_LocationType into a string.
 *
 * @param locationType    The config type which should be expressed as a string.
 *
 * @return                A string representation of @a locationType.
 */
extern const char*
scorep_location_type_to_string( SCOREP_LocationType locationType );


#endif /* SCOREP_INTERNAL_TYPES_H */
