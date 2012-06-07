/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
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

#ifndef SCOREP_MPI_H
#define SCOREP_MPI_H

/**
   @file       SCOREP_Mpi.h
   @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
   @status     ALPHA
   @ingroup    MPI_Wrapper

   @brief It includes all source files for the MPI wrappers. Macros and
          Declarations common to all MPI wrappers.
 */

#include "SCOREP_Mpi_Communicator.h"
#include "SCOREP_Mpi_Reg.h"
#include "SCOREP_Mpi_Request.h"
#include <SCOREP_Error.h>
#include <SCOREP_Debug.h>

#include <SCOREP_Definitions.h>
#include <SCOREP_Events.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_DefinitionHandles.h>

#if !defined( SCOREP_MPI_NO_HOOKS )
#include "SCOREP_MPIHooks.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/** @defgroup MPI_Wrapper SCOREP MPI wrapper library
   The MPI wrapper provides an implementation of the MPI functions which use the
   PMPI functions internally to provide the desired MPI functionality. Additionally,
   calls to the measurement system are generated for measurement. To enable the
   MPI measurement, this implementation must be linked to the instrumentated application
   before it is linked with MPI. When using a instrumenter tool, this should be handled
   by the instrumenter tool.

   All functions except MPI_Wtime and MPI_Ticks are instrumented to generate an enter
   event on function begin and an exit event on function end. Additionally, some
   functions of special interest provide additional events between the enter and exit
   event. E.g. the MPI_Send provide an send event before the data
   is send, the MPI_Recv provide a receive event after the date is received and the
   Collective functions provide a collective event. The paricular events, generated
   by each function should be defined in the function specific documentation.

   The wrapper functions are automatically generated by a tool from a set of templates.
   Thus, do not edit the generated file manually, but the templates instead. Else the
   modifications might be lost.

   The mpi functions are divided in groups. Each group can be excluded from being build,
   when compiling the mpi wrappers. Furthermore, each group can be filtered dynamically.
   Essential function wrappers, e.g. the wrapper for MPI_Init, can not be excluded from
   being build. To exclude a group of wrapper function from being build, a define must
   be set, which has the form SCOREP_MPI_NO_<GROUP_NAME>. This define should be set by the
   configure script and written to config.h.
   To specify the groups for which events are recorded, set the config variable
   "SCOREP_MPI_ENABLE_GROUPS".

   The following groups are available:
   @li ALL:       All MPI functions
   @li CG:        Communicator and group management
   @li COLL:      Collective communication
   @li ENV:       Environment management, e.g. MPI_Init
   @li ERR:       Error handling
   @li EXT:       External
   @li IO:        File IO
   @li MISC:      Miscellaneous
   @li P2P:       Peer-to-peer communication
   @li RMA:       Remote memory access, one sided communication
   @li SPAWN:     Prozess management
   @li TOPO:      Topology
   @li TYPE:      Type definition
   @li XNONBLOCK: Extended non-blocking events
   @li XREQTEST:  Test events for uncompleted requests

   @{
 */

/** internal array of statuses */
extern MPI_Status* scorep_my_status_array;

/** size of internal status array */
extern int32_t scorep_my_status_array_size;

/**
 * Get a pointer to a status array of at least 'size' statuses
 * @param  size minimal requested size
 * @return pointer to status array
 */
extern MPI_Status*
scorep_get_status_array( int32_t size );

/** Flag to indicate whether MPI Profiling hooks are turned on or off.
 */
extern bool scorep_hooks_on;

extern int scorep_mpi_status_size;

/** @def SCOREP_MPI_IS_EVENT_GEN_ON_FOR
    Check whether event generation is turned on for a specific group.
 */
#define SCOREP_MPI_IS_EVENT_GEN_ON_FOR( group ) ( ( scorep_mpi_generate_events ) && \
                                                  ( scorep_mpi_enabled & group ) )

/**
   @def SCOREP_MPI_DISABLE_GROUP( group )
   Disable the wrapper of @group
   @parameter group MPI wrapper group that is disabled.
 */
#define SCOREP_MPI_DISABLE_GROUP( group ) \
    ( scorep_mpi_enabled = scorep_mpi_enabled & ( ~( group ) ) )

/** @def SCOREP_IS_MPI_HOOKS_ON
    Check whether MPI profiling hooks are enabled globally.
 */
#define SCOREP_IS_MPI_HOOKS_ON                            scorep_hooks_on

/** @def SCOREP_MPI_HOOKS_ON
    Turn on MPI Profiling hooks inside MPI adapter.
 */
#define SCOREP_MPI_HOOKS_ON()                     scorep_hooks_on = true

/** @def SCOREP_MPI_HOOKS_OFF
    Turn off MPI Profiling hooks inside MPI adapter.
 */
#define SCOREP_MPI_HOOKS_OFF()                    scorep_hooks_on = false

#define SCOREP_MPI_HOOKS_SET( value )                     scorep_hooks_on = value

/**
 * Intializa MPI Profiling module
 */
extern void
scorep_mpiprofile_init();


/** @} */

#endif /* SCOREP_MPI_H */
