/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 *    RWTH Aachen University, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

/**
 * @status     alpha
 * @file       scorep_instrumenter_install_config.cpp
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 */

#include <config.h>
#include "scorep_instrumenter_install_data.hpp"
#include "scorep_instrumenter_utils.hpp"
#include "scorep_instrumenter_adapter.hpp"
#include <scorep_config_tool_backend.h>
#include <scorep_config_tool_mpi.h>
#include <UTILS_IO.h>
#include <UTILS_CStr.h>

#include <iostream>
#include <fstream>
#include <stdlib.h>

/* ****************************************************************************
   Main interface
******************************************************************************/

SCOREP_Instrumenter_InstallData::SCOREP_Instrumenter_InstallData( void )
{
    m_scorep_config    = SCOREP_PREFIX "/bin/scorep-config";
    m_c_compiler       = SCOREP_CC;
    m_cxx_compiler     = SCOREP_CXX;
    m_fortran_compiler = SCOREP_FC;
    m_openmp_cflags    = SCOREP_OPENMP_CFLAGS;
}

SCOREP_Instrumenter_InstallData::~SCOREP_Instrumenter_InstallData()
{
}

std::string
SCOREP_Instrumenter_InstallData::getScorepConfig( void )
{
    return m_scorep_config;
}

std::string
SCOREP_Instrumenter_InstallData::getCC( void )
{
    return m_c_compiler;
}

std::string
SCOREP_Instrumenter_InstallData::getCXX( void )
{
    return m_cxx_compiler;
}

std::string
SCOREP_Instrumenter_InstallData::getFC( void )
{
    return m_fortran_compiler;
}

SCOREP_ErrorCode
SCOREP_Instrumenter_InstallData::readConfigFile( const std::string& arg0 )
{
    std::ifstream in_file;
    in_file.open( arg0.c_str() );

    if ( in_file.good() )
    {
        while ( in_file.good() )
        {
            char line[ 512 ] = { "" };
            in_file.getline( line, 512 );
            read_parameter( line );
        }
        return SCOREP_SUCCESS;
    }
    else
    {
        return SCOREP_ERROR_FILE_CAN_NOT_OPEN;
    }
}

void
SCOREP_Instrumenter_InstallData::setBuildCheck( void )
{
    m_scorep_config = simplify_path( BUILD_DIR "/scorep-config" )
                      + " --build-check";
}

/* ****************************************************************************
   Compiler dependent implementations
******************************************************************************/

/* *************************************** CRAY */
#if SCOREP_BACKEND_COMPILER_CRAY
bool
SCOREP_Instrumenter_InstallData::isArgForOpenmp( const std::string& arg )
{
    return arg == m_openmp_cflags;
}

bool
SCOREP_Instrumenter_InstallData::isArgForShared( const std::string& arg )
{
    return arg == "-dynamic";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFreeform( const std::string& arg )
{
    return arg == "-ffree";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFixedform( const std::string& arg )
{
    return arg == "-ffixed";
}

std::string
SCOREP_Instrumenter_InstallData::getCPreprocessingFlags( const std::string& input_file,
                                                         const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCxxPreprocessingFlags( const std::string& input_file,
                                                           const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getFortranPreprocessingFlags( const std::string& input_file,
                                                               const std::string& output_file )
{
    return "-eP && mv "
           + remove_extension( remove_path( input_file ) ) + ".i "
           + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars( void )
{
    return "";
}

bool
SCOREP_Instrumenter_InstallData::isArgWithO( std::string arg )
{
    return false;
}

bool
SCOREP_Instrumenter_InstallData::isPreprocessFlag( std::string arg )
{
    return ( arg == "-E" ) || ( arg == "-eP" );
}

/* *************************************** GNU */
#elif SCOREP_BACKEND_COMPILER_GNU
bool
SCOREP_Instrumenter_InstallData::isArgForOpenmp( const std::string& arg )
{
    return arg == m_openmp_cflags;
}

bool
SCOREP_Instrumenter_InstallData::isArgForShared( const std::string& arg )
{
    return arg == "-shared";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFreeform( const std::string& arg )
{
    return arg == "-ffree-form";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFixedform( const std::string& arg )
{
    return arg == "-ffixed-form";
}

std::string
SCOREP_Instrumenter_InstallData::getCPreprocessingFlags( const std::string& input_file,
                                                         const std::string& output_file )
{
    return "-E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCxxPreprocessingFlags( const std::string& input_file,
                                                           const std::string& output_file )
{
    return "-E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getFortranPreprocessingFlags( const std::string& input_file,
                                                               const std::string& output_file )
{
    return "-cpp -E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars( void )
{
    return "";
}

bool
SCOREP_Instrumenter_InstallData::isArgWithO( std::string arg )
{
    return false;
}

bool
SCOREP_Instrumenter_InstallData::isPreprocessFlag( std::string arg )
{
    return arg == "-E";
}

/* *************************************** IBM */
#elif SCOREP_BACKEND_COMPILER_IBM
bool
SCOREP_Instrumenter_InstallData::isArgForOpenmp( const std::string& arg )
{
    return arg.substr( 0, m_openmp_cflags.length() ) == m_openmp_cflags;
}

bool
SCOREP_Instrumenter_InstallData::isArgForShared( const std::string& arg )
{
    return arg == "-qmkshrobj";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFreeform( const std::string& arg )
{
    return arg == "-qfree";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFixedform( const std::string& arg )
{
    return arg == "-qfixed";
}

std::string
SCOREP_Instrumenter_InstallData::getCPreprocessingFlags( const std::string& input_file,
                                                         const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCxxPreprocessingFlags( const std::string& input_file,
                                                           const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getFortranPreprocessingFlags( const std::string& input_file,
                                                               const std::string& output_file )
{
    std::string basename      = remove_extension( remove_path( input_file ) );
    std::string prep_file_v13 = "F" + basename + ".f";
    std::string prep_file_v14 = "F" + basename + scorep_tolower( get_extension( input_file ) );

    return "-d -qnoobject && if [ -e " + prep_file_v14 + " ]; then mv " + prep_file_v14 + " " + output_file + "; else mv " + prep_file_v13 + " " + output_file + "; fi";
}

std::string
SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars( void )
{
    return "";
}

bool
SCOREP_Instrumenter_InstallData::isArgWithO( std::string arg )
{
    return false;
}

bool
SCOREP_Instrumenter_InstallData::isPreprocessFlag( std::string arg )
{
    return ( arg == "-E" ) || ( arg == "-qnoobject" );
}

/* *************************************** INTEL */
#elif SCOREP_BACKEND_COMPILER_INTEL
bool
SCOREP_Instrumenter_InstallData::isArgForOpenmp( const std::string& arg )
{
    return ( arg == m_openmp_cflags ) || ( arg == "-openmp" );
}

bool
SCOREP_Instrumenter_InstallData::isArgForShared( const std::string& arg )
{
    return ( arg == "-shared" ) || ( arg == "-dynamiclib" );
}

bool
SCOREP_Instrumenter_InstallData::isArgForFreeform( const std::string& arg )
{
    return arg == "-free";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFixedform( const std::string& arg )
{
    return arg == "-nofree";
}

std::string
SCOREP_Instrumenter_InstallData::getCPreprocessingFlags( const std::string& input_file,
                                                         const std::string& output_file )
{
    return "-E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCxxPreprocessingFlags( const std::string& input_file,
                                                           const std::string& output_file )
{
    return "-E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getFortranPreprocessingFlags( const std::string& input_file,
                                                               const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars( void )
{
    /* We are responsible for the final space charachter. */
    return "VT_LIB_DIR=. VT_ROOT=. VT_ADD_LIBS=\"\" ";
}

bool
SCOREP_Instrumenter_InstallData::isArgWithO( std::string arg )
{
    return ( arg.substr( 0, 16 ) == "-offload-option," ) ||
           ( arg.substr( 0, 26 ) == "-offload-attribute-target=" );
}

bool
SCOREP_Instrumenter_InstallData::isPreprocessFlag( std::string arg )
{
    return arg == "-E";
}

/* *************************************** PGI */
#elif SCOREP_BACKEND_COMPILER_PGI
bool
SCOREP_Instrumenter_InstallData::isArgForOpenmp( const std::string& arg )
{
    return arg == m_openmp_cflags;
}

bool
SCOREP_Instrumenter_InstallData::isArgForShared( const std::string& arg )
{
    return arg == "-shared";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFreeform( const std::string& arg )
{
    return arg == "-Mfree" || arg == "-Mfreeform";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFixedform( const std::string& arg )
{
    return arg == "-Mnofree" || arg == "-Mnofreeform";
}

std::string
SCOREP_Instrumenter_InstallData::getCPreprocessingFlags( const std::string& input_file,
                                                         const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCxxPreprocessingFlags( const std::string& input_file,
                                                           const std::string& output_file )
{
    return "-E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getFortranPreprocessingFlags( const std::string& input_file,
                                                               const std::string& output_file )
{
    return "-E > " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars( void )
{
    return "";
}

bool
SCOREP_Instrumenter_InstallData::isArgWithO( std::string arg )
{
    return false;
}

bool
SCOREP_Instrumenter_InstallData::isPreprocessFlag( std::string arg )
{
    return arg == "-E";
}

/* *************************************** STUDIO */
#elif SCOREP_BACKEND_COMPILER_STUDIO
bool
SCOREP_Instrumenter_InstallData::isArgForOpenmp( const std::string& arg )
{
    return arg == m_openmp_cflags;
}

bool
SCOREP_Instrumenter_InstallData::isArgForShared( const std::string& arg )
{
    return arg == "-G";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFreeform( const std::string& arg )
{
    return arg == "-free";
}

bool
SCOREP_Instrumenter_InstallData::isArgForFixedform( const std::string& arg )
{
    return arg == "-fixed";
}

std::string
SCOREP_Instrumenter_InstallData::getCPreprocessingFlags( const std::string& input_file,
                                                         const std::string& output_file )
{
    return "-E -o " + output_file;
}

std::string
SCOREP_Instrumenter_InstallData::getCxxPreprocessingFlags( const std::string& input_file,
                                                           const std::string& output_file )
{
    /* The sed statements remove every line directive if the following line
       starts with a closing bracket */
    if ( get_extension( output_file ) == ".i" )
    {
        return "-E | sed 'N;s/\\n)/)/;P;D;' | sed 's/#[0-9]*)/)/g' > "
               + output_file;
    }
    else
    {
        return "-E | sed 'N;s/\\n)/)/;P;D;' | sed 's/#[0-9]*)/)/g' > "
               + remove_extension( remove_path( output_file ) ) + ".i && mv "
               + remove_extension( remove_path( output_file ) ) + ".i "
               + output_file;
    }
}

std::string
SCOREP_Instrumenter_InstallData::getFortranPreprocessingFlags( const std::string& input_file,
                                                               const std::string& output_file )
{
    std::string basename = remove_extension( remove_path( output_file ) );

    if ( get_extension( output_file ) == ".f90" )
    {
        return "-fpp -F -o " + basename
               + ".i && grep -v \\# "
               + basename + ".i > "
               + output_file;
    }
    else
    {
        return "-fpp -F -o " + basename
               + ".f90 && grep -v \\# "
               + basename + ".f90 > "
               + output_file
               + " && rm -f "
               + basename + ".f90";
    }
}

std::string
SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars( void )
{
    return "";
}

bool
SCOREP_Instrumenter_InstallData::isArgWithO( std::string arg )
{
    return false;
}

bool
SCOREP_Instrumenter_InstallData::isPreprocessFlag( std::string arg )
{
    return ( arg == "-E" ) || ( arg == "-F" );
}

#endif

/* ****************************************************************************
   Private methods
******************************************************************************/

void
SCOREP_Instrumenter_InstallData::set_value( const std::string& key,
                                            const std::string& value )
{
    if ( key == "OPENMP_CFLAGS" && value != "" )
    {
        m_openmp_cflags = value;
    }
    else if ( key == "CC"  && value != "" )
    {
        m_c_compiler = value;
    }
    else if ( key == "SCOREP_CONFIG" && value != "" )
    {
        m_scorep_config = value;
    }
    else
    {
        SCOREP_Instrumenter_Adapter::setAllConfigValue( key, value );
    }
}

SCOREP_ErrorCode
SCOREP_Instrumenter_InstallData::read_parameter( std::string line )
{
    /* check for comments */
    int pos = line.find( "#" );
    if ( pos == 0 )
    {
        return SCOREP_SUCCESS;                      // Whole line cemmented out
    }
    if ( pos != std::string::npos )
    {
        // Truncate line at comment
        line = line.substr( pos, line.length() - pos - 1 );
    }

    /* separate value and key */
    pos = line.find( "=" );
    if ( pos == std::string::npos )
    {
        return SCOREP_ERROR_PARSE_NO_SEPARATOR;
    }
    std::string key   = line.substr( 0, pos );
    std::string value = line.substr( pos + 2, line.length() - pos - 3 );

    set_value( key, value );

    return SCOREP_SUCCESS;
}
