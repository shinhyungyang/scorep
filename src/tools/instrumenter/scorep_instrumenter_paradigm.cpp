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
 * @file
 *
 * Implements the base class for an instrumentation method.
 */

#include <config.h>
#include "scorep_instrumenter_paradigm.hpp"
#include "scorep_instrumenter_cmd_line.hpp"
#include "scorep_instrumenter_install_data.hpp"
#include "scorep_instrumenter_selector.hpp"
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
 * class SCOREP_Instrumenter_Paradigm
 * *************************************************************************************/
SCOREP_Instrumenter_Paradigm::SCOREP_Instrumenter_Paradigm
(
    SCOREP_Instrumenter_Selector* selector,
    std::string                   name,
    std::string                   variant,
    std::string                   description
)
{
    m_selector    = selector;
    m_name        = name;
    m_variant     = variant;
    m_description = description;
    m_unsupported = false;
}

SCOREP_Instrumenter_Paradigm::~SCOREP_Instrumenter_Paradigm()
{
}

void
SCOREP_Instrumenter_Paradigm::checkDependencies( void )
{
    SCOREP_Instrumenter_DependencyList::iterator i;

    for ( i = m_conflicts.begin(); i != m_conflicts.end(); i++ )
    {
        SCOREP_Instrumenter_Adapter::conflict( m_name, *i );
    }

    for ( i = m_requires.begin(); i != m_requires.end(); i++ )
    {
        SCOREP_Instrumenter_Adapter::require( m_name, *i );
    }
}

void
SCOREP_Instrumenter_Paradigm::checkDefaults( void )
{
    SCOREP_Instrumenter_DependencyList::iterator i;

    for ( i = m_default_off.begin(); i != m_default_off.end(); i++ )
    {
        SCOREP_Instrumenter_Adapter::defaultOff( *i );
    }

    for ( i = m_default_on.begin(); i != m_default_on.end(); i++ )
    {
        SCOREP_Instrumenter_Adapter::defaultOn( *i );
    }
}


void
SCOREP_Instrumenter_Paradigm::printHelp( void )
{
    static std::string space = "                  ";

    if ( !m_unsupported )
    {
        std::cout << "              " << getConfigName() << std::endl;
        std::cout << space << m_description << "\n";

        SCOREP_Instrumenter_Adapter::printDepList( &m_requires,
                                                   space + "It requires and, thus, automatically enables " );
        SCOREP_Instrumenter_Adapter::printDepList( &m_conflicts,
                                                   space + "It conflicts and, thus, automatically disables " );
        SCOREP_Instrumenter_Adapter::printDepList( &m_default_on,
                                                   space + "By default, it enables also " );
        SCOREP_Instrumenter_Adapter::printDepList( &m_default_off,
                                                   space + "By default, it disables " );
    }
}

bool
SCOREP_Instrumenter_Paradigm::checkOption( std::string arg )
{
    if ( ( arg == getConfigName() ) || ( arg == m_name ) )
    {
        return true;
    }
    return false;
}

std::string
SCOREP_Instrumenter_Paradigm::getConfigName( void )
{
    return m_variant == "" ? m_name : m_name + ":" + m_variant;
}

bool
SCOREP_Instrumenter_Paradigm::checkCommand( const std::string& current,
                                            const std::string& next )
{
    return false;
}

void
SCOREP_Instrumenter_Paradigm::setConfigValue( const std::string& key,
                                              const std::string& value )
{
}

bool
SCOREP_Instrumenter_Paradigm::isSupported( void )
{
    return !m_unsupported;
}

void
SCOREP_Instrumenter_Paradigm::checkCompilerName( const std::string& compiler )
{
}

void
SCOREP_Instrumenter_Paradigm::unsupported( void )
{
    m_unsupported = true;
}

void
SCOREP_Instrumenter_Paradigm:: checkObjects( SCOREP_Instrumenter* instrumenter )
{
}
