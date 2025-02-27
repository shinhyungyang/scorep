/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015-2018, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * Implements selector of the I/O libraries which can be wrapped.
 */

#include <config.h>
#include "scorep_instrumenter_io.hpp"
#include "scorep_instrumenter_paradigm.hpp"
#include "scorep_instrumenter_cmd_line.hpp"
#include "scorep_instrumenter_install_data.hpp"
#include "scorep_instrumenter_utils.hpp"
#include "scorep_instrumenter.hpp"
#include <scorep_config_tool_backend.h>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <sstream>

#include <UTILS_Error.h>
#include <UTILS_IO.h>

/* **************************************************************************************
 * class SCOREP_Instrumenter_IoParadigm
 * *************************************************************************************/
SCOREP_Instrumenter_IoParadigm::SCOREP_Instrumenter_IoParadigm(
    SCOREP_Instrumenter_Selector* selector,
    const std::string&            name,
    const std::string&            variant,
    const std::string&            description )
    : SCOREP_Instrumenter_Paradigm( selector, name, variant, description )
{
}

std::string
SCOREP_Instrumenter_IoParadigm::getConfigName( void )
{
    return getName();
}

/* **************************************************************************************
 * class SCOREP_Instrumenter_NoIo
 * *************************************************************************************/
SCOREP_Instrumenter_NoIo::SCOREP_Instrumenter_NoIo
(
    SCOREP_Instrumenter_Selector* selector
) : SCOREP_Instrumenter_Paradigm( selector, "none", "", "No I/O wrapping support." )
{
}

/* **************************************************************************************
 * class SCOREP_Instrumenter_Posix
 * *************************************************************************************/
SCOREP_Instrumenter_Posix::SCOREP_Instrumenter_Posix
(
    SCOREP_Instrumenter_Selector* selector
) : SCOREP_Instrumenter_IoParadigm( selector, "posix", "",
                                    "POSIX I/O support using library wrapping. "
                                    "This includes the file descriptor based POSIX API (i.e., `open`/`close`). "
                                    "The POSIX asynchronous I/O API (i.e., `aio_read`/`aio_write`), if available. "
                                    "And the ISO C `FILE` handle based API (i.e., `fopen`/`fclose`)." )
{
#if !( HAVE_BACKEND( POSIX_IO_SUPPORT ) )
    unsupported();
#endif
}

bool
SCOREP_Instrumenter_Posix::checkOption( const std::string& arg )
{
    if ( ( arg == getName() ) || ( arg == m_name ) )
    {
        std::cerr <<
            "[Score-P] WARNING: I/O paradigm '" << m_name << "' is deprecated.\n" <<
            "                   Instrumentation is always performed, but must be activated at the time of measurement." <<
            std::endl;
    }

    return SCOREP_Instrumenter_IoParadigm::checkOption( arg );
}

/* **************************************************************************************
 * class SCOREP_Instrumenter_Io
 * *************************************************************************************/
SCOREP_Instrumenter_Io::SCOREP_Instrumenter_Io()
    : SCOREP_Instrumenter_Selector( "io", true )
{
    m_paradigm_list.push_back( new SCOREP_Instrumenter_NoIo( this ) );
    m_paradigm_list.push_back( new SCOREP_Instrumenter_Posix( this ) );
    if ( m_paradigm_list.back()->isSupported() )
    {
        m_current_selection.push_back( m_paradigm_list.back() );
    }
    if ( m_current_selection.empty() )
    {
        m_current_selection.push_back( m_paradigm_list.front() );
    }
}

void
SCOREP_Instrumenter_Io::printHelp( void )
{
    std::cout << "  --" << m_name << "=<paradigm>(,<paradigm>)*";
    std::cout << "\n";
    std::cout << "                  The default is the first supported mode in the above order.\n";
    std::cout << "                  Possible paradigms are:\n";

    SCOREP_Instrumenter_ParadigmList::iterator paradigm;
    for ( paradigm = m_paradigm_list.begin();
          paradigm != m_paradigm_list.end();
          paradigm++ )
    {
        ( *paradigm )->printHelp();
    }
}
