/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file       SCOREP_Instrumenter_CommandLine.cpp
 */

#include <config.h>
#include "scorep_instrumenter_cmd_line.hpp"
#include "scorep_instrumenter_adapter.hpp"
#include "scorep_instrumenter_selector.hpp"
#include "scorep_instrumenter_utils.hpp"
#include <scorep_config_tool_mpi.h>
#include <scorep_config_tool_backend.h>

#include <iostream>
#include <stdlib.h>

extern void
print_help( void );

/* ****************************************************************************
   Main interface
******************************************************************************/
SCOREP_Instrumenter_CmdLine::SCOREP_Instrumenter_CmdLine( SCOREP_Instrumenter_InstallData& install_data )
    : m_install_data( install_data )
{
    /* Execution modes */
    m_target_is_shared_lib = false;
    m_is_compiling         = true; // Opposite recognized if no source files in input
    m_is_linking           = true; // Opposite recognized on existence of -c or -E flag
    m_no_compile_link      = false;
    m_link_static          = detect;

    /* Input command elements */
    m_compiler_name     = "";
    m_flags_before_lmpi = "";
    m_flags_after_lmpi  = "";
    m_current_flags     = &m_flags_before_lmpi;
    m_include_flags     = "";
    m_define_flags      = "";
    m_output_name       = "";
    m_lmpi_set          = false;

    /* Instrumenter flags */
    m_is_dry_run     = false;
    m_keep_files     = false;
    m_verbosity      = 0;
    m_is_build_check = false;

#if defined( SCOREP_SHARED_BUILD )
    m_no_as_needed = false;
#endif
}

SCOREP_Instrumenter_CmdLine::~SCOREP_Instrumenter_CmdLine()
{
}

void
SCOREP_Instrumenter_CmdLine::ParseCmdLine( int    argc,
                                           char** argv )
{
    scorep_parse_mode_t mode = scorep_parse_mode_param;
    std::string         next = "";

    for ( int i = 1; i < argc; i++ )
    {
        next = ( i + 1 < argc ? argv[ i + 1 ] : "" );
        switch ( mode )
        {
            case scorep_parse_mode_param:
                mode = parse_parameter( argv[ i ] );
                break;
            case scorep_parse_mode_command:
                mode = parse_command( argv[ i ], next );
                break;
            case scorep_parse_mode_option_part:
                /* Skip this, it was already processed */
                mode = scorep_parse_mode_command;
                break;
        }
    }

    check_parameter();
    if ( m_verbosity >= 2 )
    {
        print_parameter();
    }
}

bool
SCOREP_Instrumenter_CmdLine::isCompiling( void )
{
    return m_is_compiling;
}

bool
SCOREP_Instrumenter_CmdLine::isLinking( void )
{
    return m_is_linking;
}

bool
SCOREP_Instrumenter_CmdLine::noCompileLink( void )
{
    return m_no_compile_link;
}

bool
SCOREP_Instrumenter_CmdLine::isNvccCompiler( void )
{
    return m_compiler_name == "nvcc";
}

std::string
SCOREP_Instrumenter_CmdLine::getCompilerName( void )
{
    return m_compiler_name;
}

std::string
SCOREP_Instrumenter_CmdLine::getFlagsBeforeLmpi( void )
{
    return m_flags_before_lmpi;
}

std::string
SCOREP_Instrumenter_CmdLine::getFlagsAfterLmpi( void )
{
    return m_flags_after_lmpi;
}

std::string
SCOREP_Instrumenter_CmdLine::getIncludeFlags( void )
{
    return m_include_flags;
}

std::string
SCOREP_Instrumenter_CmdLine::getDefineFlags( void )
{
    return m_define_flags;
}

std::string
SCOREP_Instrumenter_CmdLine::getOutputName( void )
{
    return m_output_name;
}

std::vector<std::string>*
SCOREP_Instrumenter_CmdLine::getInputFiles( void )
{
    return &m_input_files;
}

int
SCOREP_Instrumenter_CmdLine::getInputFileNumber( void )
{
    return m_input_files.size();
}

bool
SCOREP_Instrumenter_CmdLine::isLmpiSet( void )
{
    return m_lmpi_set;
}

bool
SCOREP_Instrumenter_CmdLine::isDryRun( void )
{
    return m_is_dry_run;
}

bool
SCOREP_Instrumenter_CmdLine::hasKeepFiles( void )
{
    return m_keep_files;
}

int
SCOREP_Instrumenter_CmdLine::getVerbosity( void )
{
    return m_verbosity;
}

bool
SCOREP_Instrumenter_CmdLine::isBuildCheck( void )
{
    return m_is_build_check;
}

bool
SCOREP_Instrumenter_CmdLine::enforceStaticLinking( void )
{
    return m_link_static == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::enforceDynamicLinking( void )
{
    return m_link_static == disabled;
}

bool
SCOREP_Instrumenter_CmdLine::isTargetSharedLib( void )
{
    return m_target_is_shared_lib;
}

std::string
SCOREP_Instrumenter_CmdLine::getLibraryFiles( void )
{
    std::string lib_files = "";

    for ( std::vector<std::string>::iterator current_lib = m_libraries.begin();
          current_lib != m_libraries.end();
          current_lib++ )
    {
        lib_files += " " + backslash_special_chars( find_library( *current_lib, m_libdirs ) );
    }
    return lib_files;
}

#if defined( SCOREP_SHARED_BUILD )
bool
SCOREP_Instrumenter_CmdLine::getNoAsNeeded( void )
{
    return m_no_as_needed;
}
#endif

SCOREP_Instrumenter_InstallData*
SCOREP_Instrumenter_CmdLine::getInstallData()
{
    return &m_install_data;
}

/* ****************************************************************************
   private methods
******************************************************************************/

void
SCOREP_Instrumenter_CmdLine::print_parameter( void )
{
    std::cout << "\nEnabled instrumentation:";
    SCOREP_Instrumenter_Adapter::printEnabledAdapterList();
    std::cout << std::endl;

    std::cout << "Selected paradigms:\n";
    SCOREP_Instrumenter_Selector::printSelectedParadigms();
    std::cout << std::endl;

    std::cout << "\nCompiler name: " << m_compiler_name << std::endl;
    std::cout << "Flags before -lmpi: " << m_flags_before_lmpi << std::endl;
    std::cout << "Flags after -lmpi: " << m_flags_after_lmpi << std::endl;
    std::cout << "Output file: " << m_output_name << std::endl;
    std::cout << "Input file(s): "
              << scorep_vector_to_string( m_input_files, "", "", ", " )
              << std::endl;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_parameter( const std::string& arg )
{
    if ( arg[ 0 ] != '-' )
    {
        /* Assume its the compiler/linker command. Maybe we want to add a
           validity check later */
        m_compiler_name = arg;
        SCOREP_Instrumenter_Adapter::checkAllCompilerName( arg );
        SCOREP_Instrumenter_Selector::checkAllCompilerName( arg );
        return scorep_parse_mode_command;
    }

    /* Check for execution parameters */
    else if ( arg == "--dry-run" )
    {
        m_is_dry_run = true;
        return scorep_parse_mode_param;
    }

    else if ( arg == "--build-check" )
    {
        m_is_build_check = true;
        SCOREP_Instrumenter_Adapter::setAllBuildCheck();
        m_install_data.setBuildCheck();
        return scorep_parse_mode_param;
    }

    else if ( arg == "--keep-files" )
    {
        m_keep_files = true;
        return scorep_parse_mode_param;
    }

    else if ( arg == "--help" || arg == "-h" )
    {
        print_help();
        exit( EXIT_SUCCESS );
    }

    /* Check for instrumentation and paradigms */
    else if ( SCOREP_Instrumenter_Adapter::checkAllOption( arg ) )
    {
        return scorep_parse_mode_param;
    }

    else if ( SCOREP_Instrumenter_Selector::checkAllOption( arg ) )
    {
        return scorep_parse_mode_param;
    }

    /* Link options */
#if defined( SCOREP_STATIC_BUILD ) && defined( SCOREP_SHARED_BUILD )
    else if ( arg == "--static" )
    {
#if HAVE_LINK_FLAG_BSTATIC
        m_link_static = enabled;
        return scorep_parse_mode_param;
#else   // HAVE_LINK_FLAG_BSTATIC
        std::cerr << "The --static option is not supported with the used compiler."
                  << std::endl;
        exit( EXIT_FAILURE );
#endif  // HAVE_LINK_FLAG_BSTATIC
    }
    else if ( arg == "--dynamic" )
    {
#if HAVE_LINK_FLAG_BDYNAMIC
        m_link_static = disabled;
        return scorep_parse_mode_param;
#else   // HAVE_LINK_FLAG_BDYNAMIC
        std::cerr << "The --dynamic option is not supported with the used compiler."
                  << std::endl;
        exit( EXIT_FAILURE );
#endif  // HAVE_LINK_FLAG_BDYNAMIC
    }
#elif defined( SCOREP_STATIC_BUILD )
    else if ( arg == "--static" )
    {
        return scorep_parse_mode_param;
    }
    else if ( arg == "--dynamic" )
    {
        std::cerr << "Dynamic linking is not possible. This installation contains "
                  << "no shared Score-P libraries."
                  << std::endl;
        exit( EXIT_FAILURE );
    }
#elif defined( SCOREP_SHARED_BUILD )
    else if ( arg == "--static" )
    {
        std::cerr << "Static linking is not possible. This installation contains "
                  << "no static Score-P libraries."
                  << std::endl;
        exit( EXIT_FAILURE );
    }
    else if ( arg == "--dynamic" )
    {
        return scorep_parse_mode_param;
    }
#endif

    /* Misc parameters */
    else if ( arg == "--version" )
    {
        std::cout << PACKAGE_STRING << std::endl;
        exit( EXIT_SUCCESS );
    }
    else if ( ( arg.length() >= 8 ) && ( arg.substr( 0, 8 ) == "--config" ) )
    {
        if ( arg.length() < 11 )
        {
            std::cerr << "ERROR: No config file specified." << std::endl;
            exit( EXIT_FAILURE );
        }
        std::string config_file = arg.substr( 9, arg.length() - 9 );
        if ( m_install_data.readConfigFile( config_file ) != SCOREP_SUCCESS )
        {
            std::cerr << "ERROR: Failed to read config file." << std::endl;
            exit( EXIT_FAILURE );
        }
        return scorep_parse_mode_param;
    }
    else if ( ( arg.length() > 9 ) && ( arg.substr( 0, 9 ) == "--verbose" ) )
    {
        if ( arg.length() > 10 )
        {
            m_verbosity = atol( arg.substr( 10, arg.length() - 10 ).c_str() );
        }
        else
        {
            m_verbosity = 1;
        }
        return scorep_parse_mode_param;
    }
    else if ( ( arg.length() >= 2 ) && ( arg.substr( 0, 2 ) == "-v" ) )
    {
        m_verbosity = 1;
        return scorep_parse_mode_param;
    }
#if defined( SCOREP_SHARED_BUILD )
    else if ( arg == "--no-as-needed" )
    {
        m_no_as_needed = true;
    }
#endif
    else
    {
        std::cerr << "ERROR: Unknown parameter: "
                  << arg
                  << ". Abort."
                  << std::endl;
        exit( EXIT_FAILURE );
    }

    /* Never executed but removes a warning with xl-compilers. */
    return scorep_parse_mode_param;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_command( const std::string& current,
                                            const std::string& next )
{
    scorep_parse_mode_t ret_val = scorep_parse_mode_command;
    if ( SCOREP_Instrumenter_Adapter::checkAllCommand( current, next ) )
    {
        ret_val = scorep_parse_mode_option_part;
    }
    if ( SCOREP_Instrumenter_Selector::checkAllCommand( current, next ) )
    {
        ret_val = scorep_parse_mode_option_part;
    }

    /* Detect input files */
    if ( ( current[ 0 ] != '-' ) && is_library( current ) )
    {
        add_library( current );
    }
    else if ( ( current[ 0 ] != '-' ) &&
              ( is_source_file( current ) || is_object_file( current ) ) )
    {
        add_input_file( current );
        return scorep_parse_mode_command;
    }

    else if ( ( current.length() >= 2 ) && ( is_mpi_library( current.substr( 2 ) ) ) )
    {
        m_lmpi_set      = true;
        m_current_flags = &m_flags_after_lmpi;
        add_library( current );
    }
    else if ( current == "-c" )
    {
        m_is_linking = false;
        /* Do not add -c to the compiler options, because the instrumenter
           will add a -c during the compile step, anyway. */
        return scorep_parse_mode_command;
    }
    else if ( m_install_data.isPreprocessFlag( current ) )
    {
        m_no_compile_link = true;
        m_is_linking      = false;
        m_is_compiling    = false;
    }
    else if ( current == "-M" ) /* Generate dependencies */
    {
        m_no_compile_link = true;
        m_is_linking      = false;
        m_is_compiling    = false;
    }
    else if ( current == "-l" )
    {
        if ( is_mpi_library( next ) )
        {
            m_lmpi_set      = true;
            m_current_flags = &m_flags_after_lmpi;
        }
        add_library( "-l" + next );
        ret_val = scorep_parse_mode_option_part;
    }
    else if ( current == "-L" )
    {
        add_library_path( next );
        ret_val = scorep_parse_mode_option_part;
    }
    else if ( current == "-D" )
    {
        /* The add_define function add the parameter to the parameter list,
           because, the value may need to be quoted and some characters
           baskslashed. Thus, we cannot add the value as it is. */
        add_define( current + next );
        return scorep_parse_mode_option_part;
    }
    else if ( current == "-I" )
    {
        add_include_path( next );
        ret_val = scorep_parse_mode_option_part;
    }
    else if ( current == "-o" )
    {
        /* Do not add the output name to parameter list, because the intermediate
           files may have a different name and having then an -o paramter in
           the parameter list makes trouble. */
        set_output_file( next );
        return scorep_parse_mode_option_part;
    }
    else if ( current == "-MF" )
    {
        ret_val = scorep_parse_mode_option_part;
    }
    else if ( current == "-MT" )
    {
        ret_val = scorep_parse_mode_option_part;
    }
    else if (  m_install_data.isArgForShared( current ) )
    {
#ifndef SCOREP_SHARED_BUILD
        std::cerr << "[Score-P] It is not possible to build instrumented shared "
                  << "libraries with a statically build Score-P. "
                  << "You need an Score-P installation that has "
                  << "shared libraries." << std::endl;
        exit( EXIT_FAILURE );
#endif
        m_target_is_shared_lib = true;
    }

    /* Some stupid compilers have options starting with -o that do not
       specify an output filename */
    else if ( m_install_data.isArgWithO( current ) )
    {
        *m_current_flags += " " + current;
        return scorep_parse_mode_command;
    }

    else if ( current[ 0 ] == '-' )
    {
        /* Check standard parameters */
        if ( current[ 1 ] == 'o' )
        {
            /* Do not add the output name to parameter list, because the intermediate
               files may have a different name and having then an -o paramter in
               the parameter list makes trouble. */
            set_output_file( current.substr( 2, std::string::npos ) );
            return scorep_parse_mode_command;
        }
        else if ( current[ 1 ] == 'I' )
        {
            add_include_path( current.substr( 2, std::string::npos ) );
        }
        else if ( current[ 1 ] == 'D' )
        {
            /* The add_define function add the parameter to the parameter list,
               because, the value may need to be quoted and some characters
               baskslashed. Thus, we cannot add the value as it is. */
            add_define( current );
            return scorep_parse_mode_command;
        }
        else if ( current[ 1 ] == 'L' )
        {
            add_library_path( current.substr( 2, std::string::npos ) );
        }
        else if ( current[ 1 ] == 'l' )
        {
            if ( is_mpi_library( current.substr( 2 ) ) )
            {
                m_lmpi_set      = true;
                m_current_flags = &m_flags_after_lmpi;
            }
            add_library( current );
        }
    }

    if ( m_install_data.isCompositeArg( current, next ) )
    {
        ret_val = scorep_parse_mode_option_part;
    }

    /* In any case that not yet returned, save the flag */
    *m_current_flags += " " + backslash_special_chars( current );

    /* If we already processed both, save the second, too */
    if ( ret_val == scorep_parse_mode_option_part )
    {
        *m_current_flags += " " + backslash_special_chars( next );
    }

    return ret_val;
}

void
SCOREP_Instrumenter_CmdLine::add_define( std::string arg )
{
    /* we need to escape quotes since they get lost otherwise when calling
       system() */
    size_t pos = 0;
    while ( ( pos = arg.find( '"', pos ) ) != std::string::npos )
    {
        arg.insert( pos, 1, '\\' );
        pos += 2;
    }

    /* Because enclosing quotes may have disappeared, we must always enclose the
       argument with quotes */
    pos =  arg.find( '=', 0 );
    if ( pos !=  std::string::npos )
    {
        arg.insert( pos + 1, 1, '\"' );
        arg.append( 1, '\"' );
    }

    *m_current_flags += " " + arg;
    m_define_flags   += " " + arg;
}

void
SCOREP_Instrumenter_CmdLine::check_parameter( void )
{
    /* Check dependencies */
    SCOREP_Instrumenter_Selector::checkDependencies();
    SCOREP_Instrumenter_Adapter::checkAllDependencies();

    /* Check default relations */
    SCOREP_Instrumenter_Selector::checkDefaults();
    SCOREP_Instrumenter_Adapter::defaultOn( SCOREP_INSTRUMENTER_ADAPTER_COMPILER );
    SCOREP_Instrumenter_Adapter::checkAllDefaults();

    /* Check whether the selected paradigms are supported */
    SCOREP_Instrumenter_Selector::checkAllSupported();

    /* If this is a dry run, enable printing out commands, if it is not already */
    if ( m_is_dry_run && m_verbosity < 1 )
    {
        m_verbosity = 1;
    }

    if ( m_compiler_name == "" )
    {
        std::cerr << "ERROR: Could not identify compiler name." << std::endl;
        exit( EXIT_FAILURE );
    }

    if ( getInputFileNumber() < 1 )
    {
        std::cerr << "WARNING: Found no input files." << std::endl;
    }
}

void
SCOREP_Instrumenter_CmdLine::add_input_file( std::string input_file )
{
    m_input_files.push_back( backslash_special_chars( input_file ) );
}

void
SCOREP_Instrumenter_CmdLine::set_output_file( std::string output_file )
{
    m_output_name = backslash_special_chars( output_file );
}

void
SCOREP_Instrumenter_CmdLine::add_include_path( std::string include_path )
{
    m_include_flags += " -I" + backslash_special_chars( include_path );
}

void
SCOREP_Instrumenter_CmdLine::add_library_path( std::string library_path )
{
    m_libdirs.push_back( library_path );
}

void
SCOREP_Instrumenter_CmdLine::add_library( std::string library )
{
    m_libraries.push_back( library );
}
