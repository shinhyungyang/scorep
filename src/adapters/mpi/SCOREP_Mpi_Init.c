/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
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

/**
 * @file       SCOREP_Mpi_Init.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 * @status     alpha
 * @ingroup    MPI_Wrapper
 *
 * @brief Contains the initialization function implementations for the measurement system.
 */

#include <config.h>

#include "SCOREP_Mpi_Init.h"
#include "SCOREP_Mpi_Reg.h"
#include "SCOREP_Config.h"
#include "SCOREP_Mpi.h"
#include <SCOREP_RuntimeManagement.h>

#include <stdio.h>

/**
   Stores the value of the Fortran MPI constant MPI_STATUS_SIZE. It is used for
   Fortran-C conversions.
 */
int scorep_mpi_status_size = 0;

/**
   address of fortran variable used as reference for MPI_BOTTOM
 */
void* scorep_mpi_fortran_bottom = NULL;

/**
   address of fortran variable used as reference for MPI_IN_PLACE
 */
void* scorep_mpi_fortran_in_place = NULL;

/**
   address of fortran variable used as reference for MPI_STATUS_IGNORE
 */
void* scorep_mpi_fortran_status_ignore = NULL;

/**
   address of fortran variable used as reference for MPI_STATUSES_IGNORE
 */
void* scorep_mpi_fortran_statuses_ignore = NULL;

/**
   address of fortran variable used as reference for MPI_UNWEIGHTED
 */
void* scorep_mpi_fortran_unweighted = NULL;


/**
   External fortran function to retrieve the constant value MPI_STATUS_SIZE defined
   in Fortran MPI. It is used for Fortran-C conversions.
   @param mpi_status_size Address of a variable where the value is to be stored.
 */
extern void
FORTRAN_MANGLED( scorep_fortran_get_mpi_status_size ) ( int* status_size );

/**
   External fortran function to trigger a callback which sets MPI_BOTTOM.
 */
extern void
FORTRAN_MANGLED( scorep_fortran_get_mpi_bottom ) ();

/**
   External fortran function to trigger a callback which sets MPI_IN_PLACE.
 */
extern void
FORTRAN_MANGLED( scorep_fortran_get_mpi_in_place ) ();

/**
   External fortran function to trigger a callback which sets MPI_STATUS_IGNORE.
 */
extern void
FORTRAN_MANGLED( scorep_fortran_get_mpi_status_ignore ) ();

/**
   External fortran function to trigger a callback which sets MPI_STATUSES_IGNORE.
 */
extern void
FORTRAN_MANGLED( scorep_fortran_get_mpi_statuses_ignore ) ();

/**
   External fortran function to trigger a callback which sets MPI_STATUSES_IGNORE.
 */
extern void
FORTRAN_MANGLED( scorep_fortran_get_mpi_unweighted ) ();

/**
 * Mapping of string keys to enabling group IDs
 * @note The values are sorted in decreasing order, to beautify the debug
 * output. Ie.: if all groups are enabled we get "ALL", because it matches first.
 */
static const SCOREP_ConfigType_SetEntry scorep_mpi_enable_groups[] = {
    { "ALL",       SCOREP_MPI_ENABLED_ALL       },
    { "CG",        SCOREP_MPI_ENABLED_CG        },
    { "COLL",      SCOREP_MPI_ENABLED_COLL      },
    { "DEFAULT",   SCOREP_MPI_ENABLED_DEFAULT   },
    { "ENV",       SCOREP_MPI_ENABLED_ENV       },
    { "ERR",       SCOREP_MPI_ENABLED_ERR       },
    { "EXT",       SCOREP_MPI_ENABLED_EXT       },
    { "IO",        SCOREP_MPI_ENABLED_IO        },
    { "P2P",       SCOREP_MPI_ENABLED_P2P       },
    { "MISC",      SCOREP_MPI_ENABLED_MISC      },
    { "PERF",      SCOREP_MPI_ENABLED_PERF      },
    { "RMA",       SCOREP_MPI_ENABLED_RMA       },
    { "SPAWN",     SCOREP_MPI_ENABLED_SPAWN     },
    { "TOPO",      SCOREP_MPI_ENABLED_TOPO      },
    { "TYPE",      SCOREP_MPI_ENABLED_TYPE      },
    { "XNONBLOCK", SCOREP_MPI_ENABLED_XNONBLOCK },
    { NULL,        0                            }
};


/**
   Array of configuration variables.
   They are registered to the measurement system and are filled during until the
   initialization function is called.
 */
SCOREP_ConfigVariable scorep_mpi_configs[] = {
    {
        "enable_groups",
        SCOREP_CONFIG_TYPE_BITSET,
        &scorep_mpi_enabled,
        ( void* )scorep_mpi_enable_groups,
        "DEFAULT",
        "The names of the function groups which are measured.",
        "The names of the function groups which are measured.\n"
        "Other functions are not measured.\n"
        "Possible groups are:\n"
        " All: All MPI functions\n"
        " CG: Communicator and group management\n"
        " COLL: Collective functions\n"
        " DEFAULT: Default configuration\n"
        " ENV: Environmental management\n"
        " ERR: MPI Error handling\n"
        " EXT: External interface functions\n"
        " IO: MPI file I/O\n"
        " MISC: Miscellaneous\n"
        " PERF: PControl\n"
        " P2P: Peer-to-peer communication\n"
        " RMA: One sided communication\n"
        " SPAWN: Process management\n"
        " TOPO: Topology\n"
        " TYPE: MPI datatype functions\n"
        " XNONBLOCK: Extended non-blocking events.\n",
    },
    SCOREP_CONFIG_TERMINATOR
};

/**
   Implementation of the adapter_register function of the @ref SCOREP_Adapter struct
   for the initialization process of the MPI adapter.
 */
SCOREP_Error_Code
scorep_mpi_register
    ()
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "In scorep_mpi_register\n" );
    SCOREP_ConfigRegister( "mpi", scorep_mpi_configs );
    return SCOREP_SUCCESS;
}

/**
   Implementation of the adapter_init function of the @ref SCOREP_Adapter struct
   for the initialization process of the MPI adapter.
 */
SCOREP_Error_Code
scorep_mpi_init_adapter
    ()
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "In scorep_mpi_init_adapter\n" );

    /* Set Fortran constants */
    FORTRAN_MANGLED( scorep_fortran_get_mpi_status_size ) ( &scorep_mpi_status_size );
#if HAVE( MPI_BOTTOM )
    FORTRAN_MANGLED( scorep_fortran_get_mpi_bottom ) ();
#endif
#if HAVE( MPI_IN_PLACE )
    FORTRAN_MANGLED( scorep_fortran_get_mpi_in_place ) ();
#endif
#if HAVE( MPI_STATUS_IGNORE )
    FORTRAN_MANGLED( scorep_fortran_get_mpi_status_ignore ) ();
#endif
#if HAVE( MPI_STATUSES_IGNORE )
    FORTRAN_MANGLED( scorep_fortran_get_mpi_statuses_ignore ) ();
#endif
#if HAVE( MPI_UNWEIGHTED )
    FORTRAN_MANGLED( scorep_fortran_get_mpi_unweighted ) ();
#endif

    scorep_mpi_win_init();
    scorep_mpi_register_regions();
    return SCOREP_SUCCESS;
}

/**
   Implementation of the adapter_init_location function of the @ref SCOREP_Adapter struct
   for the initialization process of the MPI adapter.
 */
SCOREP_Error_Code
scorep_mpi_init_location
    ()
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "In scorep_mpi_init_location\n" );
    return SCOREP_SUCCESS;
}

/**
   Implementation of the adapter_finalize_location function of the @ref SCOREP_Adapter
   struct for the initialization process of the MPI adapter.
 */
void
scorep_mpi_final_location
    ( void* location )
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "In scorep_mpi_final_location\n" );
}

/**
   Implementation of the adapter_finalize function of the @ref SCOREP_Adapter struct
   for the initialization process of the MPI adapter.
 */
void
scorep_mpi_finalize
    ()
{
    int res;

    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "In scorep_mpi_finalize\n" );
    SCOREP_MPI_EVENT_GEN_OFF();

    /* MPICH somehow creates some extra processes/threads. If PMPI_Finalize is called
       from the exit handler of SCOREP, these processes also try to execute MPI_Finalize.
       This causes errors, thus, we test if the call to PMPI_Finalize is save.
     */
    PMPI_Initialized( &res );
    if ( res )
    {
        PMPI_Finalized( &res );
        if ( !res )
        {
            PMPI_Finalize();
            SCOREP_OnPMPI_Finalize();
        }
    }
    SCOREP_MPI_EVENT_GEN_ON();

    scorep_mpi_win_final();
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "End of scorep_mpi_finalize\n" );
}

/**
   Implementation of the adapter_deregister function of the @ref SCOREP_Adapter struct
   for the initialization process of the MPI adapter.
 */
void
scorep_mpi_deregister
    ()
{
    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_MPI | SCOREP_DEBUG_FUNCTION_ENTRY,
                         "In scorep_mpi_deregister\n" );
}

/* The initialization struct for the MPI adapter */
const SCOREP_Adapter SCOREP_Mpi_Adapter =
{
    SCOREP_ADAPTER_MPI,
    "MPI",
    &scorep_mpi_register,
    &scorep_mpi_init_adapter,
    &scorep_mpi_init_location,
    &scorep_mpi_final_location,
    &scorep_mpi_finalize,
    &scorep_mpi_deregister
};

/**
   Flag to indicate whether event generation is turned on or off. If it is set to 0,
   events are generated. If it is set to non-zero, no events are generated.
 */
int32_t scorep_mpi_nogen = 0;

/**
   Array of MPI status. It it used to track
   the open requests between MPI calls.
 */
MPI_Status* scorep_my_status_array = 0;

/**
   Size of the status array
 */
int32_t scorep_my_status_array_size = 0;

/**
   Allocates or reallocates the status array of a size @a size. It it used to track
   the open requests between MPI calls.
   @param size Size of the status Array.
   @returns the status array
 */
MPI_Status*
scorep_get_status_array( int32_t size )
{
    if ( scorep_my_status_array_size == 0 )
    {
        /* -- never used: initialize -- */
        scorep_my_status_array = malloc( size * sizeof( MPI_Status ) );
        if ( scorep_my_status_array == NULL )
        {
            SCOREP_ERROR_POSIX();
        }
        scorep_my_status_array_size = size;
    }
    else if ( size > scorep_my_status_array_size )
    {
        /* -- not enough room: expand -- */
        scorep_my_status_array = realloc( scorep_my_status_array,
                                          size * sizeof( MPI_Status ) );
        if ( scorep_my_status_array == NULL )
        {
            SCOREP_ERROR_POSIX();
        }
        scorep_my_status_array_size = size;
    }
    return scorep_my_status_array;
}
