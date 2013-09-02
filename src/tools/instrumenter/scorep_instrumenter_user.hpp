/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

#ifndef SCOREP_INSTRUMENTER_USER_HPP
#define SCOREP_INSTRUMENTER_USER_HPP

/**
 * @file scorep_instrumenter_user.hpp
 *
 * Defines the class for the manual user instrumentation.
 */

#include "scorep_instrumenter_adapter.hpp"

/* **************************************************************************************
 * class SCOREP_Instrumenter_UserAdapter
 * *************************************************************************************/

/**
 * This class represents the manual user instrumentation.
 */
class SCOREP_Instrumenter_UserAdapter : public SCOREP_Instrumenter_Adapter
{
public:
    SCOREP_Instrumenter_UserAdapter( void );
};

#endif
