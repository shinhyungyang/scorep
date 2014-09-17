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

/**
 * @file scorep_instrumenter_cuda.cpp
 *
 * Implements the class for Pthread instrumentation.
 */

#include <config.h>
#include "scorep_instrumenter_pthread.hpp"
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
 * class SCOREP_Instrumenter_PthreadAdapter
 * *************************************************************************************/
SCOREP_Instrumenter_PthreadAdapter::SCOREP_Instrumenter_PthreadAdapter( void )
    : SCOREP_Instrumenter_Adapter( SCOREP_INSTRUMENTER_ADAPTER_PTHREAD, "pthread" )
{
}

void
SCOREP_Instrumenter_PthreadAdapter::printHelp( void )
{
    return;
}

std::string
SCOREP_Instrumenter_PthreadAdapter::getConfigToolFlag( void )
{
    return "";
}
