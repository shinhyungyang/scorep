/*
 * This file is part of the SILC project (http://www.silc.de)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

#ifndef SILC_WRAPGEN_HANDLERS_MPI_H_
#define SILC_WRAPGEN_HANDLERS_MPI_H_

/**
 * @file SILC_Wrapgen_Handlers_mpi.h
 *
 * @ brief Output handlers for template variables in MPI wrappers.
 *
 */

#include <string>
#include <map>
#include <vector>
#include "SILC_Wrapgen_Func.h"
#include "SILC_Wrapgen_Handlers.h"

namespace SILC
{
namespace Wrapgen
{
namespace handler
{
namespace mpi
{
/**
 * @name Administrative function calls
 * @{
 */
void
_initialize
    ();

/**
 * @}
 * @name Function call handlers
 * @{
 */

/** Function handler to create a PMPI call.
 */
std::string
call_pmpi
(
    const SILC::Wrapgen::Func& func
);

std::string
call_f2c_c2f
(
    const SILC::Wrapgen::Func& func
);

/** Creates a call of the corresponding C function from a Fortran wrapper.
 */
std::string
call_fortran
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 * @name Cleanup handlers
 * @{
 */
/** Insert the cleanup block from the prototypes
 */
std::string
cleanup
(
    const SILC::Wrapgen::Func& func
);

std::string
cleanup_f2c_c2f
(
    const SILC::Wrapgen::Func& func
);

/** Frees allocated memory for created c_strings.
    For every char pointer in the parameter list, a "free(c_name)" is inserted.
 */
std::string
cleanup_fortran
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 * @name Additional-variable-declaration handlers
 * @{
 */
/** Inserts a declaration block from the prototypes
 */
std::string
decl
(
    const SILC::Wrapgen::Func& func
);

std::string
decl_f2c_c2f
(
    const SILC::Wrapgen::Func& func
);

/** Declares c-strings for Fortran wrappers.
    For every char pointer in the parameter list, a char pointer with prepended 'c_'
    is declared.
 */
std::string
decl_fortran
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 * @name Additional-variable-initialization handlers
 * @{
 */
/** Inserts the init block from the prototypes.
 */
std::string
init
(
    const SILC::Wrapgen::Func& func
);

std::string
init_f2c_c2f
(
    const SILC::Wrapgen::Func& func
);

/** Converts Fortran strings to C-Strings.
    For every char pointer in the parameter list it assumes that a char pointer with
    a prepended 'c_', and an integer with the name of the char pointer but a appended
    '_len' exists which contains the length of the Fortran string. It allocates memory
    for the c-String, copies the content of the Fortran string and terminates the
    C-string with a '\0'.
 */
std::string
init_fortran
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 * @name Prototype handlers
 * @{
 */
/** Creates a prototype of the function for C-wrappers.
 */
std::string
proto_c
(
    const SILC::Wrapgen::Func& func
);

/** Creates a prototype of the function for Fortran-wrappers.
 */
std::string
proto_fortran
(
    const SILC::Wrapgen::Func& func
);

std::string
proto_f2c_c2f
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 * @name Miscelaneous handlers
 * @{
 */
/** Returns the group of the function.
 */
std::string
group
(
    const SILC::Wrapgen::Func& func
);
std::string
id
(
    const SILC::Wrapgen::Func& func
);

/** Returns the name of the function
 */
std::string
name
(
    const SILC::Wrapgen::Func& func
);

/** Returns the opening statement of the guard.
    The guard prevents the building of wrappers for functions for which no corresponding
    PMPI function exists or which belong to a group which should not be build.
    it inserts an '#if HAVE(DECL_PMPI_name)' and appends ' && ! defined SILC_MPI_NO_GUARD'
    for every entry listed in the prototypes in guard.
 */
std::string
guard_start
(
    const SILC::Wrapgen::Func& func
);

/** Closes the guard.
    Inserts '#endif'.
 */
std::string
guard_end
(
    const SILC::Wrapgen::Func& func
);

/** Returns the return type of the function
 */
std::string
rtype
(
    const SILC::Wrapgen::Func& func
);

/** Inserts the expr block from the prototypes.
 */
std::string
xblock
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 * @name MPI related functions
 * @{
 */
/** Returns the MPI version, which introduced this function
 */
std::string
version
(
    const SILC::Wrapgen::Func& func
);

/** Returns the expression from the prototypes, how to calculate the number of
    sent bytes.
 */
std::string
send_rule
(
    const SILC::Wrapgen::Func& func
);

/** Returns the expression from the prototypes, how to calculate the number of
    received bytes.
 */
std::string
recv_rule
(
    const SILC::Wrapgen::Func& func
);

/** Returns the kind of collective operation in capital letters.
 */
std::string
multiplicity
(
    const SILC::Wrapgen::Func& func
);

/**
 * @}
 */
}           // namespace mpi
}           // namespace handler
}           // namespace wrapgen
}           // namespace SILC
#endif
