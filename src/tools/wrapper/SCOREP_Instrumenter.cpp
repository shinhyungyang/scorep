/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
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
 * @file       SCOREP_Instrumenter.cpp
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 */

#include <config.h>
#include <iostream>
#include <string>
#include <fstream>
#include <istream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <scorep_utility/SCOREP_Utils.h>
#include <scorep_utility/SCOREP_Error.h>

#include "SCOREP_Instrumenter.hpp"
#include "scorep_config_tool_backend.h"


/* ****************************************************************************
   Compiler specific defines
******************************************************************************/
#ifdef SCOREP_COMPILER_INTEL
#define OPARI_MANGLING_SCHEME "intel"

#elif SCOREP_COMPILER_SUN
#define OPARI_MANGLING_SCHEME "sun"

#elif SCOREP_COMPILER_IBM
#define OPARI_MANGLING_SCHEME "ibm"

#elif SCOREP_COMPILER_PGI
#define OPARI_MANGLING_SCHEME "pgi"

#elif SCOREP_COMPILER_GNU
#define OPARI_MANGLING_SCHEME "gnu"

#elif SCOREP_COMPILER_CRAY
/* Opari2 has no option for the Cray compiler, yet. However, Cray uses the same
   mangling scheme like the GNU compiler. */
#define OPARI_MANGLING_SCHEME "gnu"

#endif

/* ****************************************************************************
   Main interface
******************************************************************************/
SCOREP_Instrumenter::SCOREP_Instrumenter()
{
    /* Initialize values */
    compiler_instrumentation = enabled;
    opari_instrumentation    = detect;
    user_instrumentation     = disabled;
    mpi_instrumentation      = detect;
    pdt_instrumentation      = disabled;

    is_mpi_application    = detect;
    is_openmp_application = detect;

    is_compiling = true; // Opposite recognized if no source files in input
    is_linking   = true; // Opposite recognized on existence of -c flag

    scorep_include_path = "";
    external_libs       = "";
    pdt_bin_path        = PDT;
    pdt_config_file     = PDT_CONFIG;
    input_file_number   = 0;

    compiler_instrumentation_flags = SCOREP_CFLAGS;
    c_compiler                     = SCOREP_CC;
    openmp_cflags                  = SCOREP_OPENMP_CFLAGS;
    nm                             = "`" OPARI_CONFIG " --nm`";
    awk                            =  "`" OPARI_CONFIG " --awk_cmd`";
    opari                          = OPARI;
    opari_script                   = "`" OPARI_CONFIG " --awk_script`";
    grep                           =  "`" OPARI_CONFIG " --egrep`";
    language                       = unknown_language;
    scorep_config                  = "";

    is_dry_run    = false;
    no_final_step = false;
    lmpi_set      = false;
}

SCOREP_Instrumenter::~SCOREP_Instrumenter ()
{
}

int
SCOREP_Instrumenter::Run()
{
    if ( verbosity >= 2 )
    {
        PrintParameter();
    }

    /* First user and compiler instrumentation,  because we want to have
       the user instrumentation effects already present when we compile
       opari instrumented sources in the preparation step. The sources
       are already compiled in the preparation step if compiling and
       linking was done by the same user command. Because we must
       find the initialzation routines, compiling and linking must be splitted
       if Opari instrumentation is used.
     */
    if ( user_instrumentation == enabled )
    {
        prepare_user();
    }
    if ( compiler_instrumentation == enabled )
    {
        prepare_compiler();
    }
#ifdef HAVE_PDT
    if ( pdt_instrumentation == enabled )
    {
        prepare_pdt();
    }
#endif
    if ( opari_instrumentation == enabled )
    {
        prepare_opari();
    }
    return execute_command();
}

SCOREP_Error_Code
SCOREP_Instrumenter::ParseCmdLine( int    argc,
                                   char** argv )
{
    scorep_parse_mode_t mode = scorep_parse_mode_param;

    /* We must read the config file when the wrapper tool specific options
       are parsed, before the command options are parsed. Thus we must detect
       the point where the mode has changed to scorep_parse_mode_command.
     */
    bool              is_config_file_read = false;
    SCOREP_Error_Code ret_val;

    for ( int i = 2; i < argc; i++ )
    {
        switch ( mode )
        {
            case scorep_parse_mode_param:
                mode = parse_parameter( argv[ i ] );
                break;
            case scorep_parse_mode_command:
                mode = parse_command( argv[ i ] );
                break;
            case scorep_parse_mode_output:
                mode = parse_output( argv[ i ] );
                break;
            case scorep_parse_mode_config:
                mode = parse_config( argv[ i ] );
                break;
        }

        if ( mode == scorep_parse_mode_command && !is_config_file_read )
        {
            if ( config_file != "" )
            {
                ret_val = ReadConfigFile( argv[ 0 ] );
            }
            is_config_file_read = true;
        }
    }

    check_parameter();
    prepare_config_tool_calls( argv[ 0 ] );

    return ret_val;
}

void
SCOREP_Instrumenter::PrintParameter()
{
    std::cout << "\nEnabled instrumentation:";
    if ( compiler_instrumentation == enabled )
    {
        std::cout << " compiler";
    }
    if ( opari_instrumentation == enabled )
    {
        std::cout << " opari";
    }
    if ( user_instrumentation == enabled )
    {
        std::cout << " user";
    }
    if ( mpi_instrumentation == enabled )
    {
        std::cout << " mpi";
    }

    std::cout << std::endl << "Linked SCOREP library type: ";
    if ( is_openmp_application == enabled )
    {
        if ( is_mpi_application == enabled )
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
        if ( is_mpi_application == enabled )
        {
            std::cout << "MPI" << std::endl;
        }
        else
        {
            std::cout << "serial" << std::endl;
        }
    }

    std::cout << "\nCompiler name: " << compiler_name << std::endl;
    std::cout << "Compiler flags: " << compiler_flags << std::endl;
    std::cout << "Output file: " << output_name << std::endl;
    std::cout << "Input file(s): " << input_files << std::endl;

    std::cout << "\nCompiler instrumentation flags: "
              << compiler_instrumentation_flags << std::endl;
}

/* ****************************************************************************
   Command line parsing
******************************************************************************/
SCOREP_Instrumenter::scorep_parse_mode_t
SCOREP_Instrumenter::parse_parameter( std::string arg )
{
    if ( arg[ 0 ] != '-' )
    {
        /* Assume its the compiler/linker command. Maybe we want to add a
           validity check later */
        compiler_name = arg;
        return scorep_parse_mode_command;
    }

    /* Check for execution parameters */
    else if ( arg == "--dry-run" )
    {
        is_dry_run = true;
        return scorep_parse_mode_param;
    }

    /* Check for instrumenatation settings */
    else if ( arg == "--compiler" )
    {
        compiler_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nocompiler" )
    {
        compiler_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--opari" )
    {
        opari_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--noopari" )
    {
        opari_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--user" )
    {
        user_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nouser" )
    {
        user_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--mpi" )
    {
        mpi_instrumentation = enabled;
        is_mpi_application  = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nompi" )
    {
        is_mpi_application  = disabled;
        mpi_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
#ifdef HAVE_PDT
    else if ( arg == "--pdt" )
    {
        pdt_instrumentation = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--nopdt" )
    {
        pdt_instrumentation = disabled;
        return scorep_parse_mode_param;
    }
#endif

    /* Check for application type settings */
    else if ( arg == "--openmp_support" )
    {
        is_openmp_application = enabled;
        return scorep_parse_mode_param;
    }
    else if ( arg == "--noopenmp_support" )
    {
        is_openmp_application = disabled;
        return scorep_parse_mode_param;
    }
    /* Configuration file */
    else if ( CheckForCommonArg( arg ) )
    {
        return scorep_parse_mode_param;
    }
    else
    {
        std::cerr << "ERROR: Unknown parameter: " << arg << std::endl;
        abort();
    }

    /* Never executed but removes a warning with xl-compilers. */
    return scorep_parse_mode_param;
}

SCOREP_Instrumenter::scorep_parse_mode_t
SCOREP_Instrumenter::parse_command( std::string arg )
{
    if ( arg[ 0 ] != '-' )
    {
        /* Assume it is a input file */
        input_files += " " + arg;
        input_file_number++;
        if ( is_c_file( arg ) )
        {
            language = c_language;
        }
        else if ( is_cpp_file( arg ) )
        {
            language = cpp_language;
        }
        else if ( is_fortran_file( arg ) )
        {
            language = fortran_language;
        }
        return scorep_parse_mode_command;
    }
    else if ( arg == "-lmpi" )
    {
        lmpi_set = true;
        /* is_mpi_application can only be disabled, if --nompi was specified. In this case
           do not enable mpi wrappers.
         */
        if ( is_mpi_application != disabled )
        {
            is_mpi_application = enabled;
        }
        /* We must append the -lmpi after our flags, else our mpi wrappers are not
           used. Thus, do not store this flag in the flag list, but return.
         */
        return scorep_parse_mode_command;
    }
    else if ( arg == "-c" )
    {
        is_linking = false;
    }
    else if ( arg == "-o" )
    {
        return scorep_parse_mode_output;
    }
    else if ( arg == openmp_cflags )
    {
        if ( is_openmp_application == detect )
        {
            is_openmp_application = enabled;
        }
        if ( opari_instrumentation == detect )
        {
            opari_instrumentation = enabled;
        }
    }

    /* In any case that not yet returned, save the flag */
    compiler_flags += " " + arg;
    return scorep_parse_mode_command;
}

SCOREP_Instrumenter::scorep_parse_mode_t
SCOREP_Instrumenter::parse_output( std::string arg )
{
    output_name = arg;
    return scorep_parse_mode_command;
}

void
SCOREP_Instrumenter::check_parameter()
{
    /* If is_mpi_application not manually specified, try a guess from the
       compiler name */
    if ( is_mpi_application == detect )
    {
        if ( compiler_name.substr( 0, 3 ) == "mpi" )
        {
            is_mpi_application = enabled;
        }
        else
        {
            is_mpi_application = disabled;
        }
    }

    /* If openmp is not manuelly specified and no openmp flags found, it is
       probably not an openmp application. */
    if ( is_openmp_application == detect )
    {
        is_openmp_application = disabled;
    }

    /* Set mpi and opari instrumenatation if not done manually by the user */
    if ( opari_instrumentation == detect )
    {
        opari_instrumentation = is_openmp_application;
    }

    if ( mpi_instrumentation == detect )
    {
        mpi_instrumentation = is_mpi_application;
    }

    /* Check pdt dependencies */
    if ( pdt_instrumentation == enabled )
    {
        user_instrumentation     = enabled;  // Needed to activate the inserted macros.
        compiler_instrumentation = disabled; // Avoid double instrumentation.
    }

    /* If this is a dry run, enable printing out commands, if it is not already */
    if ( is_dry_run && verbosity < 1 )
    {
        verbosity = 1;
    }

    if ( compiler_name == "" )
    {
        std::cout << "ERROR: Could not identify compiler name." << std::endl;
        abort();
    }

    if ( output_name != "" && !is_linking && input_file_number > 1 )
    {
        std::cerr << "ERROR: Can not specify -o with multiple files if only"
                  << " compiling or preprocessing." << std::endl;
    }

    if ( input_files == "" || input_file_number < 1 )
    {
        std::cout << "WARNING: Found no input files." << std::endl;
    }
}

SCOREP_Instrumenter::scorep_parse_mode_t
SCOREP_Instrumenter::parse_config( std::string arg )
{
    config_file = arg;
    return scorep_parse_mode_param;
}

/* ****************************************************************************
   Config file parsing
******************************************************************************/

void
SCOREP_Instrumenter::AddIncDir( std::string dir )
{
}

void
SCOREP_Instrumenter::AddLibDir( std::string dir )
{
}

void
SCOREP_Instrumenter::AddLib( std::string lib )
{
}

void
SCOREP_Instrumenter::set_pdt_path( std::string pdt )
{
    if ( pdt == "yes" )
    {
        char* path = SCOREP_GetExecutablePath( "tau_instrumentor" );
        if ( path != NULL )
        {
            pdt_bin_path = path;
            free( path );
        }
        else
        {
            std::cout << "ERROR: Unable to find PDT binaries.\n";
            abort();
        }
    }
    else if ( pdt == "no" )
    {
        return;
    }
    else
    {
        pdt_bin_path = pdt;
    }
}

void
SCOREP_Instrumenter::SetValue( std::string key,
                               std::string value )
{
    if ( key == "EGREP" && value != "" )
    {
        grep = value;
    }

    else if ( key == "OPARI_CONFIG" && value != "" )
    {
        nm           = "`" + value + " --nm`";
        awk          = "`" + value + " --awk_cmd`";
        opari_script = "`" + value + " --awk_script`";
        grep         = "`" + value + " --egrep`";
    }
    else if ( key == "PREFIX" && value != "" )
    {
        AddIncDir( value + "/include" );
        AddIncDir( value + "/include/scorep" );
        AddLibDir( value + "/lib" );
    }
    else if ( key == "PDT" && value != "" )
    {
        set_pdt_path( value );
    }
    else if ( key == "OPENMP_CFLAGS" && value != "" )
    {
        openmp_cflags = value;
    }
    else if ( key == "OPARI" && value != "" )
    {
        opari = value;
    }
    else if ( key == "CC"  && value != "" )
    {
        c_compiler = value;
    }
    else if ( key == "COMPILER_INSTRUMENTATION_CPPFLAGS" && value != "" )
    {
        compiler_instrumentation_flags = value;
    }
    else if ( key == "PDT_CONFIG" && value != "" )
    {
        pdt_config_file = value;
    }
    else if ( key == "SCOREP_CONFIG" && value != "" )
    {
        scorep_config = value;
    }
}

/* ****************************************************************************
   Preparation
******************************************************************************/
void
SCOREP_Instrumenter::prepare_config_tool_calls( std::string arg )
{
    std::string mode = " --seq";
    if ( scorep_config == "" )
    {
        char* path = SCOREP_GetExecutablePath( arg.c_str() );

        // Determine config tool path
        if ( path != NULL )
        {
            scorep_config = path;
            free( path );
        }
        else
        {
            std::cout << "ERROR: Unable to find Score-P config tool.\n";
            abort();
        }
        scorep_config += "/scorep_config";
    }

    // Determine mode parameter
    if ( is_mpi_application == enabled )
    {
        mode = ( is_openmp_application == enabled ? " --hyb" : " --mpi" );
    }
    else if ( is_openmp_application == enabled )
    {
        mode = " --omp";
    }

    // Generate calls
    scorep_include_path = "`" + scorep_config + mode + " --inc` ";
    external_libs       = "`" + scorep_config + mode + " --libs` ";

    // Handle manual -lmpi flag
    if ( lmpi_set )
    {
        external_libs += "-lmpi ";
    }
}

void
SCOREP_Instrumenter::prepare_compiler()
{
    compiler_flags += " -g " + compiler_instrumentation_flags;

    /* The sun compiler can only instrument Fortran files. Thus, any C/C++ files
       are not instrumented. To avoid user confusion, the instrumneter aborts in
       case a C/C++ file should be compiler instumented.
     */
#ifdef SCOREP_COMPILER_SUN
    if ( is_compiling )
    {
        std::string current_file = "";
        std::string extension    = "";
        size_t      old_pos      = 0;
        size_t      cur_pos      = 0;
        while ( cur_pos != std::string::npos )
        {
            cur_pos = input_files.find( " ", old_pos );
            if ( old_pos < cur_pos ) // Discard a blank
            {
                current_file = input_files.substr( old_pos, cur_pos - old_pos );
                if ( !is_fortran_file( current_file ) )
                {
                    std::cerr << "Compiler instrumentation with the Sun compiler is "
                              << "only possible for Fortran files. If you want to "
                              << "switch off compiler instrumentation, please use the "
                              << "--nocompiler option."
                              << std::endl;
                    abort();
                }
            }
            // Setup for next file
            old_pos = cur_pos + 1;
        }
    }
#endif // SCOREP_COMPILER_SUN
}

void
SCOREP_Instrumenter::prepare_user()
{
    #ifdef SCOREP_COMPILER_IBM
    if ( language == fortran_language )
    {
        compiler_flags = " -WF,-DSCOREP_USER_ENABLE=1" + compiler_flags;
        return;
    }
    #endif // SCOREP_COMPILER_IBM

    compiler_flags = " -DSCOREP_USER_ENABLE=1" + compiler_flags;
}

void
SCOREP_Instrumenter::invoke_opari( std::string input_file,
                                   std::string output_file )
{
    std::string command = opari + " --tpd "
#ifdef SCOREP_COMPILER_CRAY
                          "--nosrc "
#endif
#ifdef OPARI_MANGLING_SCHEME
                          "--tpd-mangling " OPARI_MANGLING_SCHEME " "
#endif
                          + input_file
                          + " " + output_file;
    if ( verbosity >= 1 )
    {
        std::cout << command << std::endl;
    }
    if ( !is_dry_run )
    {
        if ( system( command.c_str() ) != 0 )
        {
            if ( verbosity < 1 )
            {
                std::cout << "Error executing: " << command << std::endl;
            }
            abort();
        }
    }
}

void
SCOREP_Instrumenter::invoke_awk_script( std::string object_files,
                                        std::string output_file )
{
    std::string command = nm + " " +  object_files
                          + " | grep -E -i \"T \\.{0,1}_{0,2}pomp2_init_regions\" | "
                          + awk + " -f " + opari_script
                          + " > " + output_file;
    if ( verbosity >= 1 )
    {
        std::cout << command << std::endl;
    }
    if ( !is_dry_run )
    {
        if ( system( command.c_str() ) != 0 )
        {
            if ( verbosity < 1 )
            {
                std::cout << "Error executing: " << command << std::endl;
            }
            abort();
        }
    }
}

void
SCOREP_Instrumenter::compile_init_file( std::string input_file,
                                        std::string output_file )
{
    std::string command = c_compiler
                          + " -c " + input_file
                          + " -o " + output_file;
    if ( verbosity >= 1 )
    {
        std::cout << command << std::endl;
    }
    if ( !is_dry_run )
    {
        if ( system( command.c_str() ) != 0 )
        {
            if ( verbosity < 1 )
            {
                std::cout << "Error executing: " << command << std::endl;
            }
            abort();
        }
    }
}

void
SCOREP_Instrumenter::compile_source_file( std::string input_file,
                                          std::string output_file )
{
    /* If the we have only one source file and do not link, then the user could
       specify object file name.
     */
    if ( ( input_file_number == 1 ) && !is_linking && ( output_name != "" ) )
    {
        output_file = output_name;
    }

    /* Construct command */
    std::string command = compiler_name + " "
                          + scorep_include_path
                          + " -c " + input_file
                          + compiler_flags
                          + " -o " + output_file;

    if ( verbosity >= 1 )
    {
        std::cout << command << std::endl;
    }

    /* Execute compilation */
    if ( !is_dry_run )
    {
        if ( system( command.c_str() ) != 0 )
        {
            if ( verbosity < 1 )
            {
                std::cout << "Error executing: " << command << std::endl;
            }
            abort();
        }
    }
}

std::string
SCOREP_Instrumenter::get_extension( std::string filename )
{
    int pos = filename.rfind( "." );
    if ( pos == std::string::npos )
    {
        return "";
    }
    return filename.substr( pos );
}

std::string
SCOREP_Instrumenter::get_basename( std::string filename )
{
    int pos = filename.rfind( "." );
    if ( pos == std::string::npos )
    {
        return filename;
    }
    return filename.substr( 0, pos );
}

bool
SCOREP_Instrumenter::is_fortran_file( std::string filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".f" );
    SCOREP_CHECK_EXT( ".F" );
    SCOREP_CHECK_EXT( ".f90" );
    SCOREP_CHECK_EXT( ".F90" );
    SCOREP_CHECK_EXT( ".fpp" );
    SCOREP_CHECK_EXT( ".FPP" );
    SCOREP_CHECK_EXT( ".FOR" );
    SCOREP_CHECK_EXT( ".FTN" );
    SCOREP_CHECK_EXT( ".F95" );
    SCOREP_CHECK_EXT( ".F03" );
    SCOREP_CHECK_EXT( ".F08" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
SCOREP_Instrumenter::is_c_file( std::string filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".c" );
    SCOREP_CHECK_EXT( ".C" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
SCOREP_Instrumenter::is_cpp_file( std::string filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".cpp" );
    SCOREP_CHECK_EXT( ".CPP" );
    SCOREP_CHECK_EXT( ".cxx" );
    SCOREP_CHECK_EXT( ".CXX" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
SCOREP_Instrumenter::is_source_file( std::string filename )
{
    return is_c_file( filename ) ||
           is_cpp_file( filename ) ||
           is_fortran_file( filename );
}

bool
SCOREP_Instrumenter::is_object_file( std::string filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    if ( extension == ".o" )
    {
        return true;
    }
    return false;
}

bool
SCOREP_Instrumenter::is_library( std::string filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    if ( extension == ".a" )
    {
        return true;
    }
    if ( extension == ".so" )
    {
        return true;
    }
    if ( extension.find( ".a." ) != std::string::npos )
    {
        return true;
    }
    if ( extension.find( ".so." ) != std::string::npos )
    {
        return true;
    }
    return false;
}

void
SCOREP_Instrumenter::prepare_opari()
{
    std::string new_input_files = "";
    std::string current_file    = "";

    // Perform source code transformation on all source files
    if ( is_compiling )
    {
        std::string modified_file = "";
        std::string object_file   = "";
        size_t      old_pos       = 0;
        size_t      cur_pos       = 0;
        while ( cur_pos != std::string::npos )
        {
            cur_pos = input_files.find( " ", old_pos );
            if ( old_pos < cur_pos ) // Discard a blank
            {
                current_file = input_files.substr( old_pos, cur_pos - old_pos );
                if ( is_source_file( current_file ) )
                {
                    modified_file = get_basename( current_file )
                                    + ".opari"
                                    + get_extension( current_file );
                    invoke_opari( current_file, modified_file );

                    /* If the original command compile and link in one step,
                       we need to split compilation and linking, because
                       we need to run the script on the object files.
                       Thus, we do already compile the source and add the
                       object files.
                       Furthermore, if the user specifies multiple source files
                       in the compile step, we need to compile them separately, because
                       the sources are modified and, thus, the object file ends up with
                       a different name. In this case we do not execute the last step.
                     */
                    object_file = get_basename( current_file ) + ".o";
                    compile_source_file( modified_file, object_file );
                    new_input_files += " " + object_file;
                }
                // If it is no source file, leave the file in the input list
                else
                {
                    new_input_files += " " + current_file;
                }
            }
            // Setup for next file
            old_pos = cur_pos + 1;
        }

        // Replace sources by modified sources in compile command
        input_files = new_input_files;
    }

    // if linking: Generate POMP_Region_init() and add it
    if ( is_linking )
    {
        std::string object_files = "";
        std::string init_source  = output_name + ".pomp_init.c";
        std::string init_object  = output_name + ".pomp_init.o";
        size_t      old_pos      = 0;
        size_t      cur_pos      = 0;
        while ( cur_pos != std::string::npos )
        {
            cur_pos = input_files.find( " ", old_pos );
            if ( old_pos < cur_pos ) // Discard a blank
            {
                current_file = input_files.substr( old_pos, cur_pos - old_pos );
                if ( is_object_file( current_file ) || is_library( current_file ) )
                {
                    object_files += " " + current_file;
                }
            }
            // Setup for next file
            old_pos = cur_pos + 1;
        }
        invoke_awk_script( object_files, init_source );
        compile_init_file( init_source, init_object );
        input_files += " " + init_object;
    }
    /* In OpenMP case all compilation is done here. Thus, if we do not link, no final
       step is performed
     */
    else
    {
        no_final_step = true;
    }
}

std::string
SCOREP_Instrumenter::remove_path( std::string full_path )
{
    size_t pos = full_path.rfind( "/" );
    if ( pos == std::string::npos )
    {
        return full_path;
    }
    else
    {
        return full_path.substr( pos + 1, std::string::npos );
    }
}

void
SCOREP_Instrumenter::prepare_pdt()
{
    std::string new_input_files = "";
    std::string current_file    = "";
    std::string extension       = "";
    std::string modified_file   = "";
    std::string pdb_file        = "";
    std::string command         = "";
    size_t      old_pos         = 0;
    size_t      cur_pos         = 0;

    // Instrument sources and substitute the sources in the source list.
    while ( cur_pos != std::string::npos )
    {
        int return_value = 0;
        cur_pos = input_files.find( " ", old_pos );
        if ( old_pos < cur_pos ) // Discard a blank
        {
            current_file  = input_files.substr( old_pos, cur_pos - old_pos );
            extension     = get_extension( current_file );
            modified_file = get_basename( current_file ) + "_pdt" + extension;
            pdb_file      = remove_path( get_basename( current_file ) + ".pdb" );
            if ( is_source_file( current_file ) )
            {
                // Create database file
                if ( extension == ".c" || extension == ".C" )
                {
                    command = pdt_bin_path + "/cparse " + current_file;
                }
                else if ( extension == ".f" || extension == ".F" ||
                          extension == ".f90" || extension == ".F90" )
                {
                    command = pdt_bin_path + "/f90parse " + current_file;
                }
                else
                {
                    command = pdt_bin_path + "/cxxparse " + current_file;
                }
                if ( verbosity >= 1 )
                {
                    std::cout << command << std::endl;
                }
                return_value = system( command.c_str() );
                if ( return_value != 0 )
                {
                    std::cerr << "Failed to create PDT database file." << std::endl;
                    exit( EXIT_FAILURE );
                }

                // instrument source
                command = pdt_bin_path + "/tau_instrumentor "
                          + pdb_file + " "
                          + compiler_flags + " "
                          + current_file
                          + " -o " + modified_file
                          + " -spec " + pdt_config_file;

                if ( is_openmp_application == enabled )
                {
                    command += " -D_OPENMP";
                }

                if ( verbosity >= 1 )
                {
                    std::cout << command << std::endl;
                }
                return_value = system( command.c_str() );
                if ( return_value != 0 )
                {
                    std::cerr << "PDT instrumentation failed." << std::endl;
                    exit( EXIT_FAILURE );
                }


                // Add modified source to new sourse list
                new_input_files += " " + modified_file;
            }
            else
            {
                new_input_files += " " + current_file;
            }
        }
        // Setup for next file
        old_pos = cur_pos + 1;
    }

    // Replace sources by modified sources in compile command
    input_files = new_input_files;
}

/* ****************************************************************************
   Command execution
******************************************************************************/
int
SCOREP_Instrumenter::execute_command()
{
    if ( no_final_step )
    {
        return 0;
    }

    std::string scorep_lib;
    if ( is_linking )
    {
        if ( is_openmp_application == enabled &&
             opari_instrumentation == disabled )
        {
            scorep_lib += " -lscorep_pomp_dummy";
        }
    }
    std::string command = compiler_name + " "
                          + scorep_include_path
                          + input_files
                          + scorep_lib
                          + compiler_flags
                          + " " + external_libs;
    if ( output_name != "" )
    {
        command += " -o " + output_name;
    }

    if ( verbosity >= 1 )
    {
        std::cout << command << std::endl;
    }

    if ( !is_dry_run )
    {
        return system( command.c_str() );
    }
    return 0;
}
