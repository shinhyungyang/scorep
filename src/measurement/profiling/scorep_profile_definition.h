/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 *    RWTH Aachen University, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

#ifndef SCOREP_PROFILE_DEFINITION_H
#define SCOREP_PROFILE_DEFINITION_H

/**
 * @file        scorep_profile_definition.h
 * @maintainer  Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief The global profile definition struct and functions
 *
 */

#include <stdint.h>

#include <scorep_profile_node.h>
#include <scorep_profile_debug.h>

/* **************************************************************************************
   Defines
****************************************************************************************/

/**
   @def  SCOREP_PROFILE_ASSURE_INITIALIZED
         Check wether the profiling system is initialized.
 */
#define SCOREP_PROFILE_ASSURE_INITIALIZED if ( !scorep_profile.is_initialized ) { return; }

/**
   @def SCOREP_PROFILE_STOP
   Disables further construction of the profile.
 */
#define SCOREP_PROFILE_STOP( location )       \
    do {                                       \
        scorep_profile.is_initialized = false; \
        scorep_profile_on_error( location );   \
    } while ( 0 );

/* **************************************************************************************
   Typedefs
****************************************************************************************/

/**
   Global profile definition data
 */
typedef struct
{
    /*--------------------- Profile data */

    /**
       Points to the first root node. Further root nodes are added as siblings
       to a root node.
     */
    scorep_profile_node* first_root_node;

    /**
       True if collapse nodes occur
     */
    bool has_collapse_node;

    /**
       Maximum callpath depth actually reached during the run
     */
    uint64_t reached_depth;

    /**
       Flag wether the profile is initialized
     */
    bool is_initialized;

    /**
       Flag wether an initialize is a reinitialize
     */
    bool reinitialize;

    /*--------------------- Configuration data */

/**
   Allows to limit the depth of the calltree. If the current
   callpath becomes longer than specified by this parameter,
   no further child nodes for this callpath are created.
   This limit allows a reduction of the number of callpathes,
   especially, if the application contains recursive function
   calls.
 */
    uint64_t max_callpath_depth;

/**
   Allows to limit the number of nodes in the calltree. If the
   number of nodes in the calltree reaches its limit, no further
   callpathes are created. All new callpathes are collapsed into
   a single node. This parameter allows to limit the memory
   usage of the profile.
 */
    uint64_t max_callpath_num;
} scorep_profile_definition;

/* **************************************************************************************
   Global variables
****************************************************************************************/

/**
    Global profile definition instance
 */
extern scorep_profile_definition scorep_profile;

/**
   Initial value for scorep_profile.max_callpath_depth, which may change over
   time.
 */
extern uint64_t scorep_profile_max_callpath_depth;

/**
   Stores the configuration of the hash table size.
 */
extern uint64_t scorep_profile_task_table_size;

/**
   Contains the basename for profile files.
 */
extern char* scorep_profile_basename;

/**
   Stores the configuration setting for output format.
 */
extern uint64_t scorep_profile_output_format;

/* **************************************************************************************
   Functions
****************************************************************************************/

/**
   Initializes the profile definition struct
 */
void
scorep_profile_init_definition();

/**
   Resets the profile definition struct
 */
void
scorep_profile_delete_definition();

/**
   Returns the configuration value for SCOREP_PROFILE_BASENAME.
 */
const char*
scorep_profile_get_basename( void );

/**
   Returns the configuration value for SCORE_PROFILE_ENABLE_CORE_FILES.
 */
bool
scorep_profile_do_core_files( void );

/**
   Returns the number of locations stored in the profile.
   @return number of locations stored in the profile.
 */
uint64_t
scorep_profile_get_number_of_threads();

SCOREP_ParameterHandle scorep_profile_param_instance;

#endif // SCOREP_PROFILE_DEFINITION_H
