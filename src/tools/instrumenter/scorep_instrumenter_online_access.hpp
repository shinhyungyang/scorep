/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_INSTRUMENTER_ONLINE_ACCESS_HPP
#define SCOREP_INSTRUMENTER_ONLINE_ACCESS_HPP

/**
 * @file scorep_instrumenter_online_access.hpp
 *
 * Defines the class for online access.
 */

#include "scorep_instrumenter_adapter.hpp"

/* **************************************************************************************
 * class SCOREP_Instrumenter_OnlineAccess
 * *************************************************************************************/

/**
 * This class represents the compiler instrumentation.
 */
class SCOREP_Instrumenter_OnlineAccess : public SCOREP_Instrumenter_Adapter
{
public:
    SCOREP_Instrumenter_OnlineAccess( void );
    virtual void
    printHelp( void );
    virtual std::string
    getConfigToolFlag( void );
};

#endif
