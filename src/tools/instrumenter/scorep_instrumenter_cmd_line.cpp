/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
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
 * @file       SCOREP_Instrumenter_CommandLine.cpp
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 */

#include <config.h>
#include <scorep_instrumenter_cmd_line.hpp>
#include <scorep_instrumenter_utils.hpp>
#include <scorep_config_tool_mpi.h>

#include <iostream>
#include <stdlib.h>

extern void
print_help();

/* ****************************************************************************
   Main interface
******************************************************************************/
SCOREP_Instrumenter_CmdLine::SCOREP_Instrumenter_CmdLine( SCOREP_Instrumenter_InstallData* install_data )
{
    m_install_data = install_data;

    /* Instrumentation methods */
    m_compiler_instrumentation = enabled;
    m_opari_instrumentation    = detect;
    m_user_instrumentation     = disabled;
    m_mpi_instrumentation      = detect;
    m_pdt_instrumentation      = disabled;
    m_cobi_instrumentation     = disabled;

    /* Appplication types */
    m_is_mpi_application    = detect;
    m_is_openmp_application = detect;
    m_is_cuda_application   = detect;

    /* Execution modes */
    m_is_compiling = true; // Opposite recognized if no source files in input
    m_is_linking   = true; // Opposite recognized on existence of -c flag

    /* Input command elements */
    m_compiler_name     = "";
    m_compiler_flags    = "";
    m_include_flags     = "";
    m_define_flags      = "";
    m_output_name       = "";
    m_input_file_number = 0;
    m_input_files       = "";
    m_libraries         = "";
    m_libdirs           = "";
    m_lmpi_set          = false;

    /* Instrumenter flags */
    m_is_dry_run     = false;
    m_keep_files     = false;
    m_verbosity      = 0;
    m_is_build_check = false;
}

SCOREP_Instrumenter_CmdLine::~SCOREP_Instrumenter_CmdLine()
{
}

void
SCOREP_Instrumenter_CmdLine::ParseCmdLine( int    argc,
                                           char** argv )
{
    scorep_parse_mode_t mode = scorep_parse_mode_param;

    /* We must read the config file when the wrapper tool specific options
       are parsed, before the command options are parsed. Thus we must detect
       the point where the mode has changed to scorep_parse_mode_command.
     */
    bool is_config_file_read = false;

    for ( int i = 1; i < argc; i++ )
    {
        switch ( mode )
        {
            case scorep_parse_mode_param:
                mode = parse_parameter( argv[ i ] );
                break;
            case scorep_parse_mode_command:
                mode = parse_command( argv[ i ] );
                break;
            case scorep_parse_mode_option_part:
                mode = parse_option_part( argv[ i ] );
                break;
            case scorep_parse_mode_output:
                mode = parse_output( argv[ i ] );
                break;
            case scorep_parse_mode_library:
                mode = parse_library( argv[ i ] );
                break;
            case scorep_parse_mode_define:
                mode = parse_define( argv[ i ] );
                break;
            case scorep_parse_mode_incdir:
                mode = parse_incdir( argv[ i ] );
                break;
            case scorep_parse_mode_libdir:
                mode = parse_libdir( argv[ i ] );
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
SCOREP_Instrumenter_CmdLine::isCompilerInstrumenting()
{
    return m_compiler_instrumentation == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isOpariInstrumenting()
{
    return m_opari_instrumentation == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isUserInstrumenting()
{
    return m_user_instrumentation == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isMpiInstrumenting()
{
    return m_mpi_instrumentation == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isPdtInstrumenting()
{
    return m_pdt_instrumentation == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isCobiInstrumenting()
{
    return m_cobi_instrumentation == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isMpiApplication()
{
    return m_is_mpi_application == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isOpenmpApplication()
{
    return m_is_openmp_application == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isCudaApplication()
{
    return m_is_cuda_application == enabled;
}

bool
SCOREP_Instrumenter_CmdLine::isCompiling()
{
    return m_is_compiling;
}

bool
SCOREP_Instrumenter_CmdLine::isLinking()
{
    return m_is_linking;
}

std::string
SCOREP_Instrumenter_CmdLine::getCompilerName()
{
    return m_compiler_name;
}

std::string
SCOREP_Instrumenter_CmdLine::getCompilerFlags()
{
    return m_compiler_flags;
}

std::string
SCOREP_Instrumenter_CmdLine::getIncludeFlags()
{
    return m_include_flags;
}

std::string
SCOREP_Instrumenter_CmdLine::getDefineFlags()
{
    return m_define_flags;
}

std::string
SCOREP_Instrumenter_CmdLine::getOutputName()
{
    return m_output_name;
}

std::string
SCOREP_Instrumenter_CmdLine::getLibraries()
{
    return m_libraries;
}

std::string
SCOREP_Instrumenter_CmdLine::getLibDirs()
{
    return m_libdirs;
}

std::string
SCOREP_Instrumenter_CmdLine::getInputFiles()
{
    return m_input_files;
}

int
SCOREP_Instrumenter_CmdLine::getInputFileNumber()
{
    return m_input_file_number;
}

bool
SCOREP_Instrumenter_CmdLine::isLmpiSet()
{
    return m_lmpi_set;
}

bool
SCOREP_Instrumenter_CmdLine::isDryRun()
{
    return m_is_dry_run;
}

bool
SCOREP_Instrumenter_CmdLine::hasKeepFiles()
{
    return m_keep_files;
}

int
SCOREP_Instrumenter_CmdLine::getVerbosity()
{
    return m_verbosity;
}

bool
SCOREP_Instrumenter_CmdLine::isBuildCheck()
{
    return m_is_build_check;
}

/* ****************************************************************************
   private methods
******************************************************************************/

void
SCOREP_Instrumenter_CmdLine::print_parameter()
{
    std::cout << "\nEnabled instrumentation:";
    if ( m_compiler_instrumentation == enabled )
    {
        std::cout << " compiler";
    }
    if ( m_opari_instrumentation == enabled )
    {
        std::cout << " opari";
    }
    if ( m_user_instrumentation == enabled )
    {
        std::cout << " user";
    }
    if ( m_mpi_instrumentation == enabled )
    {
        std::cout << " mpi";
    }
    if ( m_pdt_instrumentation == enabled )
    {
        std::cout << " pdt";
    }
    if ( m_cobi_instrumentation == enabled )
    {
        std::cout << " cobi";
    }

    std::cout << std::endl << "Linked SCOREP library type: ";
    if ( m_is_openmp_application == enabled )
    {
        if ( m_is_mpi_application == enabled )
        {
            std::cout << "hybrid" << std::endl;
        }
        else
        {
            std::cout << "OpenMP" << std::endl;
        }
    }
    else
    {
        if ( m_is_mpi_application == enabled )
        {
            std::cout << "MPI" << std::endl;
        }
        else
        {
            std::cout << "serial" << std::endl;
        }
    }

    std::cout << "\nCompiler name: " << m_compiler_name << std::endl;
    std::cout << "Compiler flags: " << m_compiler_flags << std::endl;
    std::cout << "Output file: " << m_output_name << std::endl;
    std::cout << "Input file(s): " << m_input_files << std::endl;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_parameter( std::string arg )
{
    if ( arg[ 0 ] != '-' )
    {
        /* Assume its the compiler/linker command. Maybe we want to add a
           validity check later */
        m_compiler_name = arg;
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
        m_install_data->setBuildCheck();
        return scorep_parse_mode_param;
    }

    else if ( arg == "--keep-files" )
    {
        m_keep_files = true;
        return scorep_parse_mode_param;
    }

    /* Check for instrumenatation settings */
    else if ( arg == "--compiler" )
    {
        m_compiler_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nocompiler" )
    {
        m_compiler_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--opari" )
    {
        m_opari_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--noopari" )
    {
        m_opari_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--user" )
    {
        m_user_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nouser" )
    {
        m_user_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--mpi" )
    {
        m_mpi_instrumentation = enabled;
        m_is_mpi_application  = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nompi" )
    {
        m_is_mpi_application  = disabled;
        m_mpi_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
#ifdef HAVE_PDT
    else if ( arg == "--pdt" )
    {
        m_pdt_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nopdt" )
    {
        m_pdt_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
#endif
#if HAVE( COBI )
    else if ( arg == "--cobi" )
    {
        m_cobi_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nocobi" )
    {
        m_cobi_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
#endif

    /* Check for application type settings */
    else if ( arg == "--openmp" )
    {
        m_is_openmp_application = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--noopenmp" )
    {
        m_is_openmp_application = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--help" || arg == "-h" )
    {
        print_help();
        exit( EXIT_SUCCESS );
    }
    /* Misc parameters */
    else if ( arg == "--version" )
    {
        std::cout << PACKAGE_STRING << std::endl;
        exit( EXIT_SUCCESS );
    }
    else if ( arg.substr( 0, 8 ) == "--config" )
    {
        std::string config_file = arg.substr( 9, arg.length() - 9 );
        if ( config_file == "" )
        {
            std::cerr << "ERROR: No config file specified." << std::endl;
            exit( EXIT_FAILURE );
        }
        if ( m_install_data->readConfigFile( config_file ) != SCOREP_SUCCESS )
        {
            std::cerr << "ERROR: Failed to read config file." << std::endl;
            exit( EXIT_FAILURE );
        }
        return scorep_parse_mode_param;
    }
    else if ( arg.substr( 0, 9 ) == "--verbose" )
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
    else if ( arg.substr( 0, 2 ) == "-v" )
    {
        m_verbosity = 1;
        return scorep_parse_mode_param;
    }
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
SCOREP_Instrumenter_CmdLine::parse_command( std::string arg )
{
    if ( ( arg[ 0 ] != '-' ) &&
         ( is_source_file( arg ) || is_object_file( arg ) || is_library( arg ) ) )
    {
        /* Assume it is a input file */
        m_input_files += " " + arg;
        m_input_file_number++;
        if ( is_cuda_file( arg ) )
        {
            m_is_cuda_application = enabled;
        }
        return scorep_parse_mode_command;
    }
    else if ( arg == "-lmpi" )
    {
        m_lmpi_set = true;
        /* is_mpi_application can only be disabled, if --nompi was specified.
           In this case do not enable mpi wrappers.
         */
        if ( m_is_mpi_application != disabled )
        {
            m_is_mpi_application = enabled;
        }
        /* We must append the -lmpi after our flags, else our mpi wrappers are
           not used. Thus, do not store this flag in the flag list, but return.
         */
        return scorep_parse_mode_command;
    }
    else if ( arg == "-c" )
    {
        m_is_linking = false;
        /* Do not add -c to the compiler options, because the instrumenter
           will add a -c during the compile step, anyway. */
        return scorep_parse_mode_command;
    }
    else if ( arg == "-l" )
    {
        return scorep_parse_mode_library;
    }
    else if ( arg == "-L" )
    {
        return scorep_parse_mode_libdir;
    }
    else if ( arg == "-D" )
    {
        return scorep_parse_mode_define;
    }
    else if ( arg == "-I" )
    {
        return scorep_parse_mode_incdir;
    }
    else if ( arg == "-o" )
    {
        return scorep_parse_mode_output;
    }
    else if ( arg == "-MF" )
    {
        m_compiler_flags += " " + arg;
        return scorep_parse_mode_option_part;
    }
    else if ( arg == "-MT" )
    {
        m_compiler_flags += " " + arg;
        return scorep_parse_mode_option_part;
    }

    /* Check for OpenMP flags. The compiler's OpenMP flag is detected during
       configure time. Unfortunately, newer intel compiler versions support
       the gnu-like -fopenmp in addition. In this case the configure test
       detects -fopenmp as the OpenMP flag. Thus, we hardcode support for the
       standard -openmp flag for intel compilers.
     */
#ifdef SCOREP_COMPILER_INTEL
    else if ( ( arg == "-openmp" ) ||
              ( arg == m_install_data->getOpenmpFlags() ) )
#else
    else if ( arg == m_install_data->getOpenmpFlags() )
#endif
    {
        if ( m_is_openmp_application == detect )
        {
            m_is_openmp_application = enabled;
        }
        if ( m_opari_instrumentation == detect )
        {
            m_opari_instrumentation = enabled;
        }
    }
    else if ( arg[ 1 ] == 'o' )
    {
        m_output_name = arg.substr( 2, std::string::npos );
    }
    else if ( arg[ 1 ] == 'I' )
    {
        m_include_flags += " " + arg;
    }
    else if ( arg[ 1 ] == 'D' )
    {
        add_define( arg );
        return scorep_parse_mode_command;
    }
    else if ( arg[ 1 ] == 'L' )
    {
        m_libdirs += " " + arg.substr( 2, std::string::npos );
    }
    else if ( arg[ 1 ] == 'l' )
    {
        m_libraries += " " + arg;
    }

    /* In any case that not yet returned, save the flag */
    m_compiler_flags += " " + arg;
    return scorep_parse_mode_command;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_option_part( std::string arg )
{
    m_compiler_flags += " " + arg;
    return scorep_parse_mode_command;
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

    /* Because enclosing quotes may disappear, we must always enclose the
       argument of with quotes */
    pos =  arg.find( '=', 0 );
    if ( pos !=  std::string::npos )
    {
        arg.insert( pos + 1, 1, '\"' );
        arg.append( 1, '\"' );
    }

    m_define_flags += " " + arg;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_output( std::string arg )
{
    m_output_name = arg;
    return scorep_parse_mode_command;
}

void
SCOREP_Instrumenter_CmdLine::check_parameter()
{
    /* If is_mpi_application not manually specified, try a guess from the
       compiler name */
    if ( m_is_mpi_application == detect )
    {
        if ( m_compiler_name.substr( 0, 2 ) == "mp" )
        {
            m_is_mpi_application = enabled;
        }
        else
        {
            m_is_mpi_application = disabled;
        }
    }

    /* If openmp is not manuelly specified and no openmp flags found, it is
       probably not an openmp application. */
    if ( m_is_openmp_application == detect )
    {
        m_is_openmp_application = disabled;
    }

    /* Set mpi and opari instrumenatation if not done manually by the user */
    if ( m_opari_instrumentation == detect )
    {
        m_opari_instrumentation = m_is_openmp_application;
    }

    if ( m_mpi_instrumentation == detect )
    {
        m_mpi_instrumentation = m_is_mpi_application;
    }

    /* Check pdt dependencies */
    if ( m_pdt_instrumentation == enabled )
    {
        m_user_instrumentation     = enabled;  // Needed to activate the inserted macros.
        m_compiler_instrumentation = disabled; // Avoid double instrumentation.
    }

    /* Check pdt dependencies */
    if ( m_cobi_instrumentation == enabled )
    {
        m_compiler_instrumentation = disabled; // Avoid double instrumentation.
    }

    /* If this is a dry run, enable printing out commands, if it is not already */
    if ( m_is_dry_run && m_verbosity < 1 )
    {
        m_verbosity = 1;
    }

    if ( m_compiler_name == "" )
    {
        std::cerr << "ERROR: Could not identify compiler name." << std::endl;
        abort();
    }
    /*
       if ( output_name != "" && !is_linking && input_file_number > 1 )
       {
        std::cerr << "ERROR: Can not specify -o with multiple files if only"
                  << " compiling or preprocessing." << std::endl;
       }
     */
    if ( m_input_files == "" || m_input_file_number < 1 )
    {
        std::cerr << "WARNING: Found no input files." << std::endl;
    }

    /* If we want to instrument mpi applications with PDT we need to pass the
       include path to mpi.h to PDT. Thus, we can onyl support this compbination
       if we have this information. */
    if ( ( m_pdt_instrumentation == enabled ) &&
         ( m_is_mpi_application == enabled ) &&
         !SCOREP_HAVE_PDT_MPI_INSTRUMENTATION )
    {
        std::cerr << "Error: Your installation does not support PDT instrumentation for "
                  << "MPI applications." << std::endl;
        abort();
    }

    /* To avoid remarks from the compiler about VT_ROOT' environment variable not set
       we set those variables to harmless values when using intel compilers. */
    #ifdef SCOREP_COMPILER_INTEL
    m_compiler_name = "VT_LIB_DIR=. VT_ROOT=. VT_ADD_LIBS=\"\" " + m_compiler_name;
    #endif
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_library( std::string arg )
{
    m_libraries += " -l" + arg;
    if ( arg == "mpi" )
    {
        m_lmpi_set = true;
        /* is_mpi_application can only be disabled, if --nompi was specified.
           In this case, do not enable mpi wrappers.
         */
        if ( m_is_mpi_application != disabled )
        {
            m_is_mpi_application = enabled;
        }
        /* We must append the -lmpi after our flags, else our mpi wrappers are
           not used. Thus, do not store this flag in the flag list, but return.
         */
    }
    else
    {
        m_compiler_flags += " -l" + arg;
    }
    return scorep_parse_mode_command;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_define( std::string arg )
{
    add_define( "-D" + arg );
    return scorep_parse_mode_command;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_incdir( std::string arg )
{
    m_include_flags  += " -I" + arg;
    m_compiler_flags += " -I" + arg;
    return scorep_parse_mode_command;
}

SCOREP_Instrumenter_CmdLine::scorep_parse_mode_t
SCOREP_Instrumenter_CmdLine::parse_libdir( std::string arg )
{
    m_compiler_flags += " -L" + arg;
    m_libdirs        += " " + arg;
    return scorep_parse_mode_command;
}
