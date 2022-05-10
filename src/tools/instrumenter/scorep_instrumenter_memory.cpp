/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016-2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2017,
 * Technische Universitaet Darmstadt, Germany
 *
 * Copyright (c) 2017, 2023,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * Implements the class for memory tracking.
 */

#include <config.h>
#include "scorep_instrumenter_memory.hpp"
#include "scorep_instrumenter_cmd_line.hpp"
#include "scorep_instrumenter_install_data.hpp"
#include "scorep_instrumenter_utils.hpp"
#include "scorep_instrumenter.hpp"
#include <scorep_config_tool_backend.h>
#include <scorep_config_tool_mpi.h>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <sstream>
#include <deque>

#include <UTILS_Error.h>
#include <UTILS_IO.h>

/* **************************************************************************************
 * class SCOREP_Instrumenter_MemoryAdapter
 * *************************************************************************************/
SCOREP_Instrumenter_MemoryAdapter::SCOREP_Instrumenter_MemoryAdapter( void )
    : SCOREP_Instrumenter_Adapter( SCOREP_INSTRUMENTER_ADAPTER_MEMORY, "memory" )
{
#if !HAVE_BACKEND( MEMORY_SUPPORT )
    unsupported();
#endif
}


void
SCOREP_Instrumenter_MemoryAdapter::printHelp( void )
{
    if ( m_unsupported )
    {
        return;
    }

    std::cout << "  --memory        Enables memory usage instrumentation. It is enabled by default.\n";
    std::cout << "  --nomemory      Disables memory usage instrumentation.\n";
}

std::string
SCOREP_Instrumenter_MemoryAdapter::getConfigToolFlag( SCOREP_Instrumenter_CmdLine& /* cmdLine */,
                                                      const std::string& /* inputFile */ )
{
    if ( isEnabled() )
    {
        return " --" + m_name;
    }
    return " --no" + m_name;
}

bool
SCOREP_Instrumenter_MemoryAdapter::isInterpositionLibrary( const std::string& libraryName )
{
    return check_lib_name( libraryName, "memkind" );
}
