/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_INSTRUMENTER_PTHREAD_HPP
#define SCOREP_INSTRUMENTER_PTHREAD_HPP

/**
 * @file scorep_instrumenter_pthread.hpp
 *
 * Defines the class for pthread instrumentation.
 */

#include "scorep_instrumenter_adapter.hpp"

/* **************************************************************************************
 * class SCOREP_Instrumenter_PthreadAdapter
 * *************************************************************************************/

/**
 * This class represents the Pthread support.
 */
class SCOREP_Instrumenter_PthreadAdapter : public SCOREP_Instrumenter_Adapter
{
public:
    SCOREP_Instrumenter_PthreadAdapter( void );

    void
    printHelp( void );

    std::string
    getConfigToolFlag( void );
};

#endif
