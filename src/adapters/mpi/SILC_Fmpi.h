/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
**  KOJAK       http://www.fz-juelich.de/jsc/kojak/                        **
*****************************************************************************
**  Copyright (c) 1998-2009                                                **
**  Forschungszentrum Juelich, Juelich Supercomputing Centre               **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/

#ifndef SILC_FMPI_H
#define SILC_FMPI_H

/** @file  SILC_Fmpi.h
    @brief Main file for Fortran interface measurement wrappers.

    Includes all Fortran wrappers for MPI functions. Fortran compilers decorate the
    C-calls in different ways. Thus, the macro FSUB must be set to SILC_FORTRAN_SUB1,
    SILC_FORTRAN_SUB2, SILC_FORTRAN_SUB3, or SILC_FORTRAN_SUB4 to obtain correctly
    decorated functions. The form of the decoration depends on the used compiler.

    You can compile this wrappers four times with different decorations, to have all
    possible names for the Fortran wrappers available.

    The wrapped decorated functions call the C-wrappers for MPI. Thus, the same set of
    events are generated as with the C functions.
 */

#include "SILC_Fortran_Wrapper.h"
#include "SILC_Error.h"
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#if defined( __sun ) || defined( _SX ) || defined( OPEN_MPI ) || defined( HP_MPI )
#  define NEED_F2C_CONV
#endif

#endif /* SILC_FMPI_H */
