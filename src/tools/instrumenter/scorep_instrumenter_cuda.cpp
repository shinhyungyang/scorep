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

/**
 * @file scorep_instrumenter_cuda.cpp
 *
 * Implements the class for cuda instrumentation.
 */

#include <config.h>
#include "scorep_instrumenter_cuda.hpp"
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
 * class SCOREP_Instrumenter_CudaAdapter
 * *************************************************************************************/
SCOREP_Instrumenter_CudaAdapter::SCOREP_Instrumenter_CudaAdapter( void )
    : SCOREP_Instrumenter_Adapter( SCOREP_INSTRUMENTER_ADAPTER_CUDA, "cuda" ),
      m_nvcc_compiler( false )
{
#if !HAVE_BACKEND( CUDA_SUPPORT )
    unsupported();
#endif
}

void
SCOREP_Instrumenter_CudaAdapter::checkCompilerName( const std::string& compiler )
{
    if ( remove_path( compiler ).substr( 0, 2 ) == "nv" )
    {
        if ( m_usage == detect )
        {
            m_usage = enabled;
        }

        m_nvcc_compiler = true;
    }
}

bool
SCOREP_Instrumenter_CudaAdapter::checkCommand( const std::string& current,
                                               const std::string& next )
{
    if ( ( current[ 0 ] != '-' ) &&
         is_cuda_file( current ) &&
         ( m_usage == detect ) )
    {
        m_usage = enabled;
    }
    return false;
}

std::string
SCOREP_Instrumenter_CudaAdapter::getConfigToolFlag( void )
{
    std::string flags;

    // Per default, the scorep-config returns cuda libs.
    if ( !isEnabled() )
    {
        flags += " --no" + m_name;
    }

    // if the nvcc compiler is used, we need to notify scorep-config about this
    if ( m_nvcc_compiler )
    {
        flags += " --nvcc";
    }

    return flags;
}
