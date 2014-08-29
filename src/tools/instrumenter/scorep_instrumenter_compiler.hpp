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

#ifndef SCOREP_INSTRUMENTER_COMPILER_HPP
#define SCOREP_INSTRUMENTER_COMPILER_HPP

/**
 * @file scorep_instrumenter_compiler.hpp
 *
 * Defines the class for compiler instrumentation.
 */

#include "scorep_instrumenter_adapter.hpp"

/* **************************************************************************************
 * class SCOREP_Instrumenter_CompilerAdapter
 * *************************************************************************************/

/**
 * This class represents the compiler instrumentation.
 */
class SCOREP_Instrumenter_CompilerAdapter : public SCOREP_Instrumenter_Adapter
{
public:
    SCOREP_Instrumenter_CompilerAdapter( void );
    virtual std::string
    getConfigToolFlag( void );
    virtual std::string
    precompile( SCOREP_Instrumenter&         instrumenter,
                SCOREP_Instrumenter_CmdLine& cmdLine,
                const std::string&           source_file );
};

#endif
