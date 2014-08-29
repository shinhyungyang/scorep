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
 * Copyright (c) 2009-2014,
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


/**
 * @file
 *
 *
 * @brief       Exports the subsystems array for the measurement system.
 */


#include <config.h>
#include "scorep_subsystem.h"

#include <stdlib.h>
#include <stdio.h>

#include <UTILS_Error.h>

#include <SCOREP_Location.h>
#include "scorep_environment.h"


/* scorep_subsystems[] and scorep_number_of_subsystems will be
 * provided by instrumenter or libscorep_confvars.la. */
/* List of linked in subsystems. */
extern const SCOREP_Subsystem* scorep_subsystems[];
extern const size_t            scorep_number_of_subsystems;


size_t
scorep_subsystems_get_number( void )
{
    return scorep_number_of_subsystems;
}


void
scorep_subsystems_register( void )
{
    /* call register functions for all subsystems */
    for ( size_t i = 0; i < scorep_number_of_subsystems; i++ )
    {
        if ( !scorep_subsystems[ i ]->subsystem_register )
        {
            continue;
        }

        SCOREP_ErrorCode error = scorep_subsystems[ i ]->subsystem_register( i );
        if ( SCOREP_SUCCESS != error )
        {
            UTILS_ERROR( error, "Can't register %s subsystem",
                         scorep_subsystems[ i ]->subsystem_name );
            _Exit( EXIT_FAILURE );
        }
    }
}


void
scorep_subsystems_initialize( void )
{
    /* call initialization functions for all subsystems */
    for ( size_t i = 0; i < scorep_number_of_subsystems; i++ )
    {
        if ( !scorep_subsystems[ i ]->subsystem_init )
        {
            continue;
        }

        SCOREP_ErrorCode error = scorep_subsystems[ i ]->subsystem_init();
        if ( SCOREP_SUCCESS != error )
        {
            UTILS_ERROR( error, "Can't initialize %s subsystem",
                         scorep_subsystems[ i ]->subsystem_name );
            _Exit( EXIT_FAILURE );
        }
        else if ( SCOREP_Env_RunVerbose() )
        {
            fprintf( stderr, "[Score-P] successfully initialized %s subsystem\n",
                     scorep_subsystems[ i ]->subsystem_name );
        }
    }
}


/**
 * Initialize subsystems for existing locations.
 */
void
scorep_subsystems_initialize_location( SCOREP_Location* locationData )
{
    /* call initialization functions for all subsystems */
    for ( size_t i = 0; i < scorep_number_of_subsystems; i++ )
    {
        if ( !scorep_subsystems[ i ]->subsystem_init_location )
        {
            continue;
        }

        SCOREP_ErrorCode error = scorep_subsystems[ i ]->subsystem_init_location( locationData );
        if ( SCOREP_SUCCESS != error )
        {
            UTILS_ERROR( error, "Can't initialize location for %s subsystem",
                         scorep_subsystems[ i ]->subsystem_name );
            _Exit( EXIT_FAILURE );
        }
        else if ( SCOREP_Env_RunVerbose() )
        {
            fprintf( stderr, "[Score-P] successfully initialized location for %s subsystem\n",
                     scorep_subsystems[ i ]->subsystem_name );
        }
    }
}


void
scorep_subsystems_finalize_location( SCOREP_Location* locationData )
{
    for ( size_t i = scorep_number_of_subsystems; i-- > 0; )
    {
        if ( !scorep_subsystems[ i ]->subsystem_finalize_location )
        {
            continue;
        }

        scorep_subsystems[ i ]->subsystem_finalize_location( locationData );

        if ( SCOREP_Env_RunVerbose() )
        {
            fprintf( stderr, "[Score-P] finalized %s subsystem location\n",
                     scorep_subsystems[ i ]->subsystem_name );
        }
    }
}


/**
 * Called before the unification process starts.
 */
void
scorep_subsystems_pre_unify( void )
{
    for ( size_t i = 0; i < scorep_number_of_subsystems; i++ )
    {
        if ( !scorep_subsystems[ i ]->subsystem_pre_unify )
        {
            continue;
        }

        SCOREP_ErrorCode error = scorep_subsystems[ i ]->subsystem_pre_unify();
        if ( SCOREP_SUCCESS != error )
        {
            UTILS_ERROR( error, "pre-unify hook failed for %s subsystem",
                         scorep_subsystems[ i ]->subsystem_name );
            _Exit( EXIT_FAILURE );
        }
    }
}


/**
 * Called before the unification process starts.
 */
void
scorep_subsystems_post_unify( void )
{
    /* ?reverse order? */
    for ( size_t i = 0; i < scorep_number_of_subsystems; i++ )
    {
        if ( !scorep_subsystems[ i ]->subsystem_post_unify )
        {
            continue;
        }

        SCOREP_ErrorCode error = scorep_subsystems[ i ]->subsystem_post_unify();
        if ( SCOREP_SUCCESS != error )
        {
            UTILS_ERROR( error, "post-unify hook failed for %s subsystem",
                         scorep_subsystems[ i ]->subsystem_name );
            _Exit( EXIT_FAILURE );
        }
    }
}


void
scorep_subsystems_finalize( void )
{
    /* call finalization functions for all subsystems */
    for ( size_t i = scorep_number_of_subsystems; i-- > 0; )
    {
        if ( !scorep_subsystems[ i ]->subsystem_finalize )
        {
            continue;
        }

        scorep_subsystems[ i ]->subsystem_finalize();

        if ( SCOREP_Env_RunVerbose() )
        {
            fprintf( stderr, "[Score-P] finalized %s subsystem\n",
                     scorep_subsystems[ i ]->subsystem_name );
        }
    }
}


void
scorep_subsystems_deregister( void )
{
    /* call de-register functions for all subsystems */
    for ( size_t i = scorep_number_of_subsystems; i-- > 0; )
    {
        if ( !scorep_subsystems[ i ]->subsystem_deregister )
        {
            continue;
        }

        scorep_subsystems[ i ]->subsystem_deregister();

        if ( SCOREP_Env_RunVerbose() )
        {
            fprintf( stderr, "[Score-P] de-registered %s subsystem\n",
                     scorep_subsystems[ i ]->subsystem_name );
        }
    }
}
