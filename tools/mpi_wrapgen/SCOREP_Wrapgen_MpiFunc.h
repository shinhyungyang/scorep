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

#ifndef SCOREP_MPIFUNC_H_
#define SCOREP_MPIFUNC_H_

/**
 * @file       SCOREP_Wrapgen_MpiFunc.h
 * @ingroup    Wrapgen_module
 *
 * @brief Class representing MPI function prototypes.
 */

#include <string>
using std::string;
#include <vector>
using std::vector;

#include "SCOREP_Wrapgen_Funcparam.h"
#include "SCOREP_Wrapgen_Func.h"
#include "SCOREP_Wrapgen_Util.h"
using namespace SCOREP::Wrapgen;

/**
 * Class for representing MPI function prototype
 */
namespace SCOREP
{
namespace Wrapgen
{
class MPIFunc : public Func
{
public:
    MPIFunc( const string&      rtype,
             const string&      name,
             const string&      group,
             const string&      guard,
             const string&      version,
             const paramlist_t& params );

    /** get string representing the send count calculation rule
     * @return string representing the send count calculation rule */
    string
    get_sendcount_rule
        () const
    {
        return m_scnt;
    }
    /** get string representing the receive count calculation rule
     * @return string representing the receive count calculation rule */
    string
    get_recvcount_rule
        () const
    {
        return m_rcnt;
    }
    /** get MPI version this function call was introduced
     * @return MPI major version
     */
    int
    get_version
        () const
    {
        return m_version;
    }
    /** set string representing the send count calculation rule
     * @param rule string representing the send count calculation rule */
    void
    set_sendcount_rule
    (
        const string& rule
    );

    /** set string representing the receive count calculation rule
     * @param rule string representing the receive count calculation rule */
    void
    set_recvcount_rule
    (
        const string& rule
    );

    /** write function configuration
     * @return string holding the configurations values for the function
     *         call
     */
    virtual string
    write_conf
        () const;

private:
    /** expression to evaluate the number of bytes sent */
    string m_scnt;
    /** expression to evaluate the number of bytes received */
    string m_rcnt;
    /** additional expressions within a wrapper */
    string m_expr_block;
    /** initial declarations special to this function */
    string m_init;
    /** major version of MPI where this function was introduced */
    int m_version;
};
}   // namespace Wrapgen
}   // namespace SCOREP

#endif
