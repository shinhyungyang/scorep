/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_MPI_STATUS_INTEROP
#define SCOREP_MPI_STATUS_INTEROP

/* *INDENT-OFF* */
/**
   @file
   @ingroup    MPI_Wrapper

   @brief Work with F08 type(MPI_Status) objects in C

   The MPI standard (see MPI 4.0: 19.3.5) defines conversion routines between
   the type(MPI_Status) in Fortran 08 and MPI_Status in C, as well as conversions
   between the type(MPI_Status) in the Fortran 'use mpi_f08' interface and the
   integer, dimension(MPI_STATUS_SIZE) in the older Fortran 'use mpi' interface.

   However, we cannot rely on these routines, because they are not yet supported
   by OpenMPI and MPICH.

   Because MPI_Status is an opaque object, there is no universal (across MPI
   implementations) way to construct an MPI_Status. Thus we cannot implement
   the conversion routines ourselves.

   Instead, the following workaround is used:

   - The interoperable type scorep_mpi_interop_status holds either a C or a F08
     Status and knows the origin language of that Status.
   - Pass this interop type into every routine in Score-P that needs to accept
     Status objects of both languages.
   - A Status object provides this information (in both languages):
     - source (Rank of source for recv)
     - tag (Tag of message)
     - error (Error code)
     - count (Transferred bytes)
     - cancelled (Belongs to a cancelled request?)
     The first 3 are available directly as members (e.g status.MPI_SOURCE in C, or status%MPI_SOURCE in F08),
     but the last two can only be queried by MPI functions (MPI_Get_count, MPI_Test_cancelled).

     Routines to get each piece of information from a scorep_mpi_interop_status
     are provided. These look at the language tag.
     If the language is C, the information is obtained directly from the C Status object.
     If the language is F08, then the corresponding query routine in Fortran is called.

     Example: Pass a F08 Status to C and query the source information

     F08                                                        C

     type(MPI_Status) st
        |
        |
     scorep_mpi_interop_status_create(st)
        type(scorep_mpi_interop_status)
        st_inter%status = st
        st_inter%origin_language = F08
        |
        V
      st_inter -------------------------------------------->   st_inter
                        (interoperable type)                      |
                                                                  |
                                                                  |
                                                                  |
                                                                  |
                                                          scorep_mpi_status_source(st_inter)
                                                              st_inter.origin_language == F08:
     scorep_mpi_status_source_f08(st) < --------------------- st_inter.status
       st%MPI_SOURCE ---------------------------------------> source
                                                                  |
                                                                  |
                                                                  V
                                                                result

    Example: Query the source of a C status in C:

                                                                 C

                                                            MPI_Status st
                                                                 |
                                                                 |
                                                        scorep_mpi_interop_status_create(st)
                                                            st_inter.status = &st
                                                            st_inter.origin_language = C
                                                                 |
                                                                 |
                                                                 V
                                                        scorep_mpi_status_source(st_inter)
                                                            st_inter.origin_language == C:
                                                            source = st_inter.status->MPI_SOURCE
                                                               |
                                                               |
                                                               V
                                                             result
 */
/* *INDENT-ON* */

#include <config.h>
#include <mpi.h>


typedef enum
{
    SCOREP_MPI_LANGUAGE_C   = 0,
#if HAVE( MPI_USEMPIF08_SUPPORT )
    SCOREP_MPI_LANGUAGE_F08 = 1
#endif /* HAVE( MPI_USEMPIF08_SUPPORT ) */
} scorep_mpi_language;


#if HAVE( MPI_USEMPIF08_SUPPORT )

typedef struct
{
    void*               status;
    scorep_mpi_language origin_language;
} scorep_mpi_interop_status;

#else /* ! HAVE( MPI_USEMPIF08_SUPPORT ) */

typedef struct
{
    MPI_Status* status;
} scorep_mpi_interop_status;

#endif /* ! HAVE( MPI_USEMPIF08_SUPPORT ) */


scorep_mpi_interop_status
scorep_mpi_interop_status_create( MPI_Status* status );


void
scorep_mpi_status_source( const scorep_mpi_interop_status* interopStatus,
                          int*                             source );

void
scorep_mpi_status_tag( const scorep_mpi_interop_status* interopStatus,
                       int*                             tag );

void
scorep_mpi_status_error( const scorep_mpi_interop_status* interopStatus,
                         int*                             error );

void
scorep_mpi_get_count( const scorep_mpi_interop_status* interopStatus,
                      MPI_Datatype                     datatype,
                      int*                             count );

void
scorep_mpi_test_cancelled( const scorep_mpi_interop_status* interopStatus,
                           int*                             cancelled );


#if HAVE( MPI_USEMPIF08_SUPPORT )
extern void
scorep_mpi_status_source_toF08( void* status,
                                int*  source );
extern void
scorep_mpi_status_tag_toF08( void* status,
                             int*  tag );
extern void
scorep_mpi_status_error_toF08( void* status,
                               int*  error );
extern void
scorep_mpi_get_count_toF08( void*     status,
                            MPI_Fint* datatype,
                            int*      count );
extern void
scorep_mpi_test_cancelled_toF08( void* status,
                                 int*  cancelled );

#endif /* HAVE( MPI_USEMPIF08_SUPPORT ) */

#endif
