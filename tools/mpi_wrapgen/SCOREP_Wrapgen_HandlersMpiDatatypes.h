/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011,
 *    Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 *    University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2011,
 *    Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 *    Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2011                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2010-2011                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/

#ifndef SCOREP_WRAPGEN_HANDLERS_MPI_DATATYPES_H_
#define SCOREP_WRAPGEN_HANDLERS_MPI_DATATYPES_H_

/**
 * @file       SCOREP_Wrapgen_Handlers_MpiDatatypes.h
 * @ingroup    Wrapgen_module
 *
 * @brief Helper functions for MPI handlers.
 */


#include "SCOREP_Wrapgen_Funcparam.h"

namespace SCOREP
{
namespace Wrapgen
{
namespace handler
{
namespace mpi
{
namespace datatype
{
/**
 * Check for pointers to transfer buffers
 */
bool
is_pointer_to_buffer
(
    const Funcparam& param
)
{
    return param.get_type() == "void*";
}

bool
is_char_pointer
(
    const Funcparam& param
)
{
    return param.get_type() == "char*";
}

bool
is_handler_function
(
    const Funcparam& param
)
{
    return ( param.get_type().find( "_function" ) != string::npos ) ||
           ( param.get_type().find( "handler" )   != string::npos );
}

bool
is_pointer
(
    const Funcparam& param
)
{
    return ( param.get_type().find( "*" )     != string::npos ) ||
           ( param.get_name().find( "array" ) != string::npos ) ||
           ( param.get_suffix().find( "[]" )  != string::npos );
}

bool
is_special_int
(
    const Funcparam& param
)
{
    return ( param.get_type().find( "MPI_Aint" )   != string::npos ) ||
           ( param.get_type().find( "MPI_Offset" ) != string::npos );
}

bool
is_input_param
(
    const Funcparam& param
)
{
    return param.get_atype() == 'i';
}

bool
is_output_param
(
    const Funcparam& param
)
{
    return param.get_atype() == 'o';
}

bool
is_input_output_param
(
    const Funcparam& param
)
{
    return param.get_atype() == 'b';
}

bool
is_range_triplet
(
    const Funcparam& param
)
{
    return param.get_name() == "ranges";
}

bool
is_array
(
    const Funcparam& param
)
{
    return param.get_name().find( "array" ) != string::npos;
}

bool
needs_input_handling( const Funcparam& param )
{
    return is_input_param( param ) ||
           is_input_output_param( param );
}

bool
needs_output_handling
(
    const Funcparam& param
)
{
    return is_output_param( param ) ||
           is_input_output_param( param );
}

bool
needs_cast
(
    const Funcparam& param
)
{
    return is_handler_function( param ) ||
           is_range_triplet( param ) ||
           param.get_name().find( "Request" ) != string::npos;
}
}               // namespace datatypes
}               // namespace mpi
}               // namespace handler
}               // namespace Wrapgen
}               // namespace SCOREP

#endif
