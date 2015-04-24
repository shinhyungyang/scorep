/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * Implements threading paradigms.
 */

#include <config.h>
#include "scorep_instrumenter_thread.hpp"
#include "scorep_instrumenter_paradigm.hpp"
#include "scorep_instrumenter_opari.hpp"
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
 * static functions
 * *************************************************************************************/

/**
 * Checks whether the current argument is indicates whether OpenMP is used.
 * @param current     The current argument.
 * @param openmpCflag The standard OpenMP C Flag for the compiler.
 * @returns true if @a current indicates OpenMP usage.
 */
static bool
check_command_for_openmp_option( std::string current, std::string openmpCflag )
{
    if ( current == openmpCflag )
    {
        return true;
    }
#if SCOREP_BACKEND_COMPILER_INTEL
    if ( current == "-openmp" || current == "-qopenmp" )
    {
        return true;
    }
#endif
#if SCOREP_BACKEND_COMPILER_IBM
    if ( ( current.length() > openmpCflag.length() ) &&
         ( current.substr( 0, 6 ) == "-qsmp=" ) )
    {
        size_t end;
        for ( size_t start = 5; start != std::string::npos; start = end )
        {
            end = current.find( ':', start + 1 );
            if ( current.substr( start + 1, end - start - 1 ) == "omp" )
            {
                return true;
            }
        }
    }
#endif
#if SCOREP_BACKEND_COMPILER_FUJITSU
    if ( ( current.length() > openmpCflag.length() ) &&
         ( current.substr( 0, 2 ) == "-K" ) )
    {
        size_t end;
        for ( size_t start = 1; start != std::string::npos; start = end )
        {
            end = current.find( ',', start + 1 );
            if ( current.substr( start + 1, end - start - 1 ) == "openmp" )
            {
                return true;
            }
        }
    }
#endif
    return false;
}


/* **************************************************************************************
 * class SCOREP_Instrumenter_SingleThreaded
 * *************************************************************************************/
SCOREP_Instrumenter_SingleThreaded::SCOREP_Instrumenter_SingleThreaded
(
    SCOREP_Instrumenter_Selector* selector
) : SCOREP_Instrumenter_Paradigm( selector, "none", "", "No thread support." )
{
}

/* **************************************************************************************
 * class SCOREP_Instrumenter_OmpTpd
 * *************************************************************************************/
SCOREP_Instrumenter_OmpTpd::SCOREP_Instrumenter_OmpTpd
(
    SCOREP_Instrumenter_Selector* selector
) : SCOREP_Instrumenter_Paradigm( selector, "omp", "pomp_tpd",
                                  "OpenMP support using OPARI2 thread tracking" )
{
    m_requires.push_back( SCOREP_INSTRUMENTER_ADAPTER_OPARI );
    m_openmp_cflag = SCOREP_OPENMP_CFLAGS;

#if !SCOREP_BACKEND_HAVE_OMP_TPD
    unsupported();
#endif
}

bool
SCOREP_Instrumenter_OmpTpd::checkCommand( const std::string& current,
                                          const std::string& next )
{
    if ( check_command_for_openmp_option( current, m_openmp_cflag ) )
    {
        m_selector->select( this, false );
    }
    return false;
}

void
SCOREP_Instrumenter_OmpTpd::setConfigValue( const std::string& key,
                                            const std::string& value )
{
    if ( key == "OPENMP_CFLAGS" && value != "" )
    {
        m_openmp_cflag = value;
    }
}

void
SCOREP_Instrumenter_OmpTpd::checkDependencies( void )
{
    SCOREP_Instrumenter_Paradigm::checkDependencies();

    SCOREP_Instrumenter_Adapter* adapter = SCOREP_Instrumenter_Adapter::getAdapter( SCOREP_INSTRUMENTER_ADAPTER_OPARI );
    if ( ( adapter != NULL ) )
    {
        ( ( SCOREP_Instrumenter_OpariAdapter* )adapter )->setTpdMode( true );
    }
}

/* **************************************************************************************
 * class SCOREP_Instrumenter_OmpAncestry
 * *************************************************************************************/
SCOREP_Instrumenter_OmpAncestry::SCOREP_Instrumenter_OmpAncestry
(
    SCOREP_Instrumenter_Selector* selector
) : SCOREP_Instrumenter_Paradigm( selector, "omp", "ancestry",
                                  "OpenMP support using thread tracking with ancestry functions in OpenMP 3.0 and later" )
{
    m_requires.push_back( SCOREP_INSTRUMENTER_ADAPTER_OPARI );
    m_openmp_cflag = SCOREP_OPENMP_CFLAGS;

#if !SCOREP_BACKEND_HAVE_OMP_ANCESTRY
    unsupported();
#endif
}

bool
SCOREP_Instrumenter_OmpAncestry::checkCommand( const std::string& current,
                                               const std::string& next )
{
    if ( check_command_for_openmp_option( current, m_openmp_cflag ) )
    {
        m_selector->select( this, false );
    }
    return false;
}

void
SCOREP_Instrumenter_OmpAncestry::setConfigValue( const std::string& key,
                                                 const std::string& value )
{
    if ( key == "OPENMP_CFLAGS" && value != "" )
    {
        m_openmp_cflag = value;
    }
}

/* ****************************************************************************
* class SCOREP_Instrumenter_Pthread
* ****************************************************************************/

SCOREP_Instrumenter_Pthread::SCOREP_Instrumenter_Pthread(
    SCOREP_Instrumenter_Selector* selector ) :
    SCOREP_Instrumenter_Paradigm( selector,
                                  "pthread",
                                  "",
                                  "Pthread support using thread tracking via "
                                  "library wrapping" ),
    m_pthread_cflag( SCOREP_BACKEND_PTHREAD_CFLAGS ),
    m_pthread_lib( SCOREP_BACKEND_PTHREAD_LIBS )
{
    m_requires.push_back( SCOREP_INSTRUMENTER_ADAPTER_PTHREAD );
    m_conflicts.push_back( SCOREP_INSTRUMENTER_ADAPTER_OPARI );
#if !SCOREP_BACKEND_HAVE_PTHREAD
    unsupported();
#endif
}

bool
SCOREP_Instrumenter_Pthread::checkCommand( const std::string& current,
                                           const std::string& next )
{
    // See ax_pthread.m4:
    // possible cflags: -Kthread -kthread -pthread -pthreads -mthreads --thread-safe -mt
    // possible libs: -lpthreads -llthread -lpthread
    if ( current == m_pthread_cflag )
    {
        m_selector->select( this, false );
    }
    else if ( current == m_pthread_lib )
    {
        m_selector->select( this, false );
    }
    else if ( ( current.substr( 0, 2 ) == "-l" ) &&
              ( is_pthread_library( current.substr( 2 ) ) ) )
    {
        m_selector->select( this, false );
    }
    else if ( ( current == "-l" ) &&
              ( is_pthread_library( next ) ) )
    {
        m_selector->select( this, false );
    }

    return false;
}

void
SCOREP_Instrumenter_Pthread::setConfigValue( const std::string& key,
                                             const std::string& value )
{
    if ( key == "PTHREAD_CFLAGS" && value != "" )
    {
        m_pthread_cflag = value;
    }
}

/* **************************************************************************************
 *  * class SCOREP_Instrumenter_PthreadAdapter
 *   * *************************************************************************************/
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
SCOREP_Instrumenter_PthreadAdapter::getConfigToolFlag( SCOREP_Instrumenter_CmdLine& /* cmdLine */ )
{
    return "";
}

/* **************************************************************************************
 * class SCOREP_Instrumenter_Thread
 * *************************************************************************************/
SCOREP_Instrumenter_Thread::SCOREP_Instrumenter_Thread()
    : SCOREP_Instrumenter_Selector( "thread" )
{
    m_paradigm_list.push_back( new SCOREP_Instrumenter_SingleThreaded( this ) );
    m_paradigm_list.push_back( new SCOREP_Instrumenter_OmpTpd( this ) );
    m_paradigm_list.push_back( new SCOREP_Instrumenter_OmpAncestry( this ) );
    m_paradigm_list.push_back( new SCOREP_Instrumenter_Pthread( this ) );
    m_current_selection = m_paradigm_list.front();
}
