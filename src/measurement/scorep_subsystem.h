/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */


#ifndef SCOREP_INTERNAL_SUBSYSTEM_H
#define SCOREP_INTERNAL_SUBSYSTEM_H


/**
 * @file
 *
 *
 * @brief       Exports the subsystems array for the measurement system.
 */


#include <SCOREP_Subsystem.h>


size_t
scorep_subsystems_get_number( void );

void
scorep_subsystems_register( void );

void
scorep_subsystems_initialize( void );

void
scorep_subsystems_initialize_location( struct SCOREP_Location* locationData );

void
scorep_subsystems_finalize_location( struct SCOREP_Location* locationData );

void
scorep_subsystems_pre_unify( void );

void
scorep_subsystems_post_unify( void );

void
scorep_subsystems_finalize( void );

void
scorep_subsystems_deregister( void );

#endif /* SCOREP_INTERNAL_SUBSYSTEM_H */
