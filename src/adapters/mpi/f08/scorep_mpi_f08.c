/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
#include <config.h>
#include <scorep_mpi_c.h>
#include <UTILS_Error.h>
#include <SCOREP_DefinitionHandles.h>
#include <scorep_mpi_communicator_mgmt.h>
#include <scorep_mpi_communicator.h>

#if HAVE( FORTRAN_TS_BUFFERS )
#include <ISO_Fortran_binding.h>
#endif /* HAVE( FORTRAN_TS_BUFFERS ) */


#if HAVE( MPI_USEMPIF08_SUPPORT )

void* scorep_mpi_f08_status_ignore   = NULL;
void* scorep_mpi_f08_statuses_ignore = NULL;
void* scorep_mpi_f08_in_place        = NULL;

void
scorep_mpi_f08_init_status_ignore_fromF08( void* status )
{
    scorep_mpi_f08_status_ignore = status;
}

void
scorep_mpi_f08_init_statuses_ignore_fromF08( void* statusArray )
{
    scorep_mpi_f08_statuses_ignore = statusArray;
}

void
scorep_mpi_f08_init_in_place_ext_fromF08( void* buffer )
{
    scorep_mpi_f08_in_place = buffer;
}

#if HAVE( FORTRAN_TS_BUFFERS )
void
scorep_mpi_f08_init_in_place_ts_fromF08( CFI_cdesc_t* buffer )
{
    scorep_mpi_f08_in_place = buffer->base_addr;
}
#endif /* HAVE( FORTRAN_TS_BUFFERS ) */

bool
scorep_mpi_is_status_ignore_fromF08( void* status )
{
    /* NOTE: When MPI implementations finally support F08 status conversion routines
     * this routine should become:
     * return ( status == MPI_F08_STATUS_IGNORE );
     *
     * Open MPI v4.1 does not support that.
     */
    return status == scorep_mpi_f08_status_ignore;
}

bool
scorep_mpi_is_statuses_ignore_fromF08( void* statusArray )
{
    /* When MPI implementations finally support F08 status conversion routines
     * this routine should become:
     * return ( statusArray == MPI_F08_STATUSES_IGNORE );
     *
     * Open MPI v4.1 does not support that.
     */
    return statusArray == scorep_mpi_f08_statuses_ignore;
}


bool
scorep_mpi_is_in_place_fromF08( void* buffer )
{
    return buffer == scorep_mpi_f08_in_place;
}

#if HAVE( FORTRAN_TS_BUFFERS )
bool
scorep_mpi_is_in_place_fromF08ts( CFI_cdesc_t* buffer )
{
    return buffer->base_addr == scorep_mpi_f08_in_place;
}
#endif /* HAVE( FORTRAN_TS_BUFFERS ) */


/**
 * Initialization that has to be done from Fortran
 */
extern void
scorep_mpi_f08_init_toF08( void );

void
scorep_mpi_f08_init( void )
{
    scorep_mpi_f08_init_toF08();
}


/**
 * scorep_mpi_generate_events is a TLS variable, which we cannot access directly
 * from Fortran via bind(c). We have to go through these functions instead.
 *
 * These correspond to the macros SCOREP_MPI_IS_EVENT_GEN_ON, ... in the C wrappers.
 * @{
 */
bool
scorep_mpi_is_event_gen_on_fromF08( void )
{
    return scorep_mpi_generate_events;
}

void
scorep_mpi_event_gen_on_fromF08( void )
{
    scorep_mpi_generate_events = 1;
}

void
scorep_mpi_event_gen_off_fromF08( void )
{
    scorep_mpi_generate_events = 0;
}

/**
 * @}
 */


#else /* ! HAVE( MPI_USEMPIF08_SUPPORT ) */

/**
 * Do-nothing mockup to link against when there is no support for mpi_f08.
 */
void
scorep_mpi_f08_init( void )
{
}

#endif /* ! HAVE( MPI_USEMPIF08_SUPPORT ) */
