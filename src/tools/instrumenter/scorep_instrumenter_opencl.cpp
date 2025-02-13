/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014-2015, 2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2023,
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
 * Implements the class for OpenCL instrumentation.
 */

#include <config.h>
#include "scorep_instrumenter_opencl.hpp"
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

#include <UTILS_Error.h>
#include <UTILS_IO.h>

/* **************************************************************************************
 * class SCOREP_Instrumenter_OpenCLAdapter
 * *************************************************************************************/
SCOREP_Instrumenter_OpenCLAdapter::SCOREP_Instrumenter_OpenCLAdapter( void )
    : SCOREP_Instrumenter_Adapter( SCOREP_INSTRUMENTER_ADAPTER_OPENCL, "opencl" )
{
#if HAVE_BACKEND( OPENCL_SUPPORT )
    m_usage = enabled;
#else
    unsupported();
#endif
}

bool
SCOREP_Instrumenter_OpenCLAdapter::checkOption( const std::string& arg )
{
    if ( arg == "--" + m_name || arg == "--no" + m_name )
    {
        std::cerr <<
            "[Score-P] WARNING: Option '" << arg << "' is deprecated.\n" <<
            "                   Instrumentation is always performed, but must be activated at the time of measurement." <<
            std::endl;
    }

    return SCOREP_Instrumenter_Adapter::checkOption( arg );
}

std::string
SCOREP_Instrumenter_OpenCLAdapter::getConfigToolFlag( SCOREP_Instrumenter_CmdLine& /* cmdLine */,
                                                      const std::string& /* inputFile */ )
{
    if ( isEnabled() )
    {
        return " --" + m_name;
    }
    else
    {
        return " --no" + m_name;
    }
}

void
SCOREP_Instrumenter_OpenCLAdapter::printHelp( void )
{
    if ( m_unsupported )
    {
        return;
    }

    std::cout << "  --opencl\n"
              << "                  Enables OpenCL instrumentation.\n"
              << "  --noopencl      Disables OpenCL instrumentation."
              << std::endl;
}

bool
SCOREP_Instrumenter_OpenCLAdapter::checkCommand( const std::string& current,
                                                 const std::string& next )
{
    bool skip_next = false;

    if ( current.substr( 0, 2 ) == "-l" )
    {
        std::string lib = current.substr( 2 );
        if ( lib.length() == 0 )
        {
            lib       = next;
            skip_next = true;
        }
        if ( ( m_usage == detect )
             && is_opencl_library( lib ) )
        {
            m_usage = enabled;
        }
    }

    return skip_next;
}

bool
SCOREP_Instrumenter_OpenCLAdapter::is_opencl_library( const std::string& libraryName )
{
    return check_lib_name( libraryName, "OpenCL" );
}
