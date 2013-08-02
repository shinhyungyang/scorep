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
 * @file       scorep_instrumenter.cpp
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 */

#include <config.h>
#include <iostream>
#include <string>
#include <fstream>
#include <istream>
#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <UTILS_Error.h>
#include <UTILS_IO.h>

#include "scorep_instrumenter.hpp"
#include "scorep_instrumenter_cobi.hpp"
#include "scorep_instrumenter_compiler.hpp"
#include "scorep_instrumenter_cuda.hpp"
#include "scorep_instrumenter_opari.hpp"
#include "scorep_instrumenter_preprocess.hpp"
#include "scorep_instrumenter_pdt.hpp"
#include "scorep_instrumenter_user.hpp"
#include "scorep_instrumenter_utils.hpp"
#include <scorep_config_tool_backend.h>
#include <scorep_config_tool_mpi.h>

void
print_help();

/* ****************************************************************************
                                                                 public methods
 * ***************************************************************************/
SCOREP_Instrumenter::SCOREP_Instrumenter( SCOREP_Instrumenter_InstallData& install_data,
                                          SCOREP_Instrumenter_CmdLine&     command_line )
    : m_install_data( install_data ),
      m_command_line( command_line )
{
    /* Create adapters */
    m_cobi_adapter       = new SCOREP_Instrumenter_CobiAdapter();
    m_compiler_adapter   = new SCOREP_Instrumenter_CompilerAdapter();
    m_cuda_adapter       = new SCOREP_Instrumenter_CudaAdapter();
    m_opari_adapter      = new SCOREP_Instrumenter_OpariAdapter();
    m_preprocess_adapter = new SCOREP_Instrumenter_PreprocessAdapter();
    m_pdt_adapter        = new SCOREP_Instrumenter_PdtAdapter();
    m_user_adapter       = new SCOREP_Instrumenter_UserAdapter();

    /* pre-compile adapter order */
    m_precompile_adapters.push_back( m_compiler_adapter );
    m_precompile_adapters.push_back( m_preprocess_adapter );
    m_precompile_adapters.push_back( m_opari_adapter );
    m_precompile_adapters.push_back( m_pdt_adapter );

    /* pre-link adapter order */
    m_prelink_adapters.push_back( m_opari_adapter );

    /* post-link adapter order */
    m_postlink_adapters.push_back( m_cobi_adapter );
}

SCOREP_Instrumenter::~SCOREP_Instrumenter ()
{
    SCOREP_Instrumenter_Adapter::destroyAll();
}

int
SCOREP_Instrumenter::Run( void )
{
    std::string object_files = "";
    m_input_files = m_command_line.getInputFiles();

    /* If no compiling or linking happens, e.g., because the command does only
       perform preprocessing or dependency generation, execute the unmodified command */
    if ( m_command_line.noCompileLink() )
    {
        /* Construct command */
        std::string command = m_command_line.getCompilerName()
                              + " " + m_command_line.getFlagsBeforeLmpi()
                              + " " + m_command_line.getFlagsAfterLmpi()
                              + " " + m_command_line.getInputFiles();

        std::string output_name =  m_command_line.getOutputName();
        if ( output_name != "" )
        {
            command += " -o " + output_name;
        }
        executeCommand( command );

        return EXIT_SUCCESS;
    }

    if ( m_command_line.isCompiling() )
    {
        /* Because Opari and PDT perform source code modifications, and store
           the modified source file with a different name, we get object files
           with a different name. To avoid this, we need to compile every
           source file separately and explicitly specify the output file name.
         */
        std::string current_file = "";
        std::string object_file  = "";
        size_t      old_pos      = 0;
        size_t      cur_pos      = 0;
        char*       cwd          = UTILS_IO_GetCwd( NULL, 0 );
        char*       cwd_to_free  = cwd;
        if ( !cwd )
        {
            /* ensure that cwd is non-NULL */
            cwd = ( char* )"";
        }

        /* If the original command compile and link in one step,
           we need to split compilation and linking, because for Opari
           we need to run the script on the object files.
           Furthermore, if the user specifies multiple source files
           in the compile step, we need to compile them separately, because
           the sources are modified and, thus, the object file ends up with
           a different name. In this case we do not execute the last step.
         */
        while ( cur_pos != std::string::npos )
        {
            cur_pos = m_input_files.find( " ", old_pos );
            if ( old_pos < cur_pos ) // Discard a blank
            {
                current_file = m_input_files.substr( old_pos, cur_pos - old_pos );
                if ( is_source_file( current_file ) )
                {
                    /* Make sure, it has full path => Some compilers and
                       user instrumentation use the file name given to the compiler.
                       Thus, if we make all file names have full paths, we get a
                       consistent input.
                       However, temporary files are without paths. Thus, if a source
                       code instrumenter does not insert line directives, the result
                       may not contain path information anymore.
                     */
                    char* simplified = UTILS_IO_JoinPath( 2, cwd, current_file.c_str() );
                    if ( simplified )
                    {
                        UTILS_IO_SimplifyPath( simplified );
                        current_file = simplified;
                        free( simplified );
                    }

                    // Determine object file name
                    if ( ( !m_command_line.isLinking() ) &&
                         ( m_command_line.getOutputName() != "" ) &&
                         ( m_command_line.getInputFileNumber() == 1 ) )
                    {
                        object_file = m_command_line.getOutputName();
                    }
                    else
                    {
                        object_file = remove_extension(
                            remove_path( current_file ) ) + ".o";
                    }

                    /* Setup the config tool calls for the new input file. This
                       will already setup the compiler and user instrumentation
                       if desired
                     */
                    prepare_config_tool_calls( current_file );

                    /* If we create modified source, we must add the original
                       source directory to the include dirs, because local
                       files may be included
                     */
                    m_compiler_flags += " -I" + extract_path( current_file );

                    /* If compiling and linking is performed in one step.
                       The compiler leave no object file.
                       Thus, we delete the object file, too.
                     */
                    if ( m_command_line.isLinking() )
                    {
                        m_temp_files += " " + object_file;
                    }

                    // Perform instrumentation
                    current_file = precompile( current_file );

                    #if SCOREP_BACKEND_COMPILER_CRAY
                    if ( m_opari_adapter->isEnabled() &&
                         m_command_line.getCompilerName().find( "ftn" ) != std::string::npos )
                    {
                        m_compiler_flags += " -I.";
                    }
                    #endif

                    // Compile instrumented file
                    compile_source_file( current_file, object_file );

                    // Add object file to the input file list for the link command
                    object_files += " " + object_file;
                }
                // If it is no source file, leave the file in the input list
                else
                {
                    object_files += " " + current_file;
                }
            }
            // Setup for next file
            old_pos = cur_pos + 1;
        }
        free( cwd_to_free );

        // Replace sources by compiled by their object file names for the link command
        m_input_files = object_files;
    }

    if ( m_command_line.isLinking() )
    {
        // Create the config tool calls for linking
        prepare_config_tool_calls( "" );

        // Perform pre-link instrumentation actions
        prelink();

        // Perform linking
        link_step();

        // Perform post-link instrumentation actions
        postlink();
    }

    clean_temp_files();

    return EXIT_SUCCESS;
}

std::string
SCOREP_Instrumenter::getCompilerFlags( void )
{
    return m_compiler_flags;
}

std::string
SCOREP_Instrumenter::getInputFiles( void )
{
    return m_input_files;
}

void
SCOREP_Instrumenter::prependInputFile( std::string filename )
{
    m_input_files = filename + " " + m_input_files;
}

std::string
SCOREP_Instrumenter::getConfigBaseCall( void )
{
    return m_config_base;
}

/* ****************************************************************************
 *                                                              private methods
 * ***************************************************************************/
void
SCOREP_Instrumenter::addTempFile( const std::string& filename )
{
    m_temp_files += " " + filename;
}

void
SCOREP_Instrumenter::clean_temp_files( void )
{
    if ( ( !m_command_line.hasKeepFiles() ) && ( m_temp_files != "" ) )
    {
        m_temp_files = "rm" + m_temp_files;
        executeCommand( m_temp_files );
    }
}

void
SCOREP_Instrumenter::prepare_config_tool_calls( const std::string& input_file )
{
    std::string mode          = "";
    std::string scorep_config = m_install_data.getScorepConfig();

    // Determine mode parameter
    if ( !m_command_line.isMpiApplication() )
    {
        mode = " --mpp=none";
    }

    if ( m_command_line.isOpenmpApplication() )
    {
        mode += " --thread=omp";
    }

    mode += SCOREP_Instrumenter_Adapter::getAllConfigToolFlags();

    if ( is_fortran_file( input_file ) )
    {
        mode += " --fortran";
    }

    if ( m_command_line.isNvccCompiler() )
    {
        mode += " --nvcc";
    }

    // Generate calls
    m_config_base  = scorep_config + mode;
    m_linker_flags = "`" + scorep_config + mode + " --ldflags` " +
                     "`" + scorep_config + mode + " --libs` ";
#if defined( SCOREP_SHARED_BUILD )
    /* temporary, see ticket:385 */
    if ( m_command_line.getNoAsNeeded() )
    {
        m_linker_flags = "`" + scorep_config + mode + " --ldflags` " +
                         LIBDIR_FLAG_WL "--no-as-needed " +
                         "`" + scorep_config + mode + " --libs` "
                         LIBDIR_FLAG_WL "--as-needed ";
    }
#endif
}

void
SCOREP_Instrumenter::compile_source_file( const std::string& input_file,
                                          const std::string& output_file )
{
    /* Construct command */
    std::stringstream command;
    command << SCOREP_Instrumenter_InstallData::getCompilerEnvironmentVars();
    command << m_command_line.getCompilerName();
    command << " `" << m_config_base << " --cflags` " << m_compiler_flags;
    command << " " << m_command_line.getFlagsBeforeLmpi();
    command << " " << m_command_line.getFlagsAfterLmpi();
    command << " -c " << input_file;
    command << " -o " << output_file;
    executeCommand( command.str() );
}

std::string
SCOREP_Instrumenter::precompile( std::string current_file )
{
    std::deque<SCOREP_Instrumenter_Adapter*>::iterator adapter;
    for ( adapter = m_precompile_adapters.begin();
          adapter != m_precompile_adapters.end();
          adapter++ )
    {
        if ( ( *adapter )->isEnabled() )
        {
            current_file = ( *adapter )->precompile( *this,
                                                     m_command_line,
                                                     current_file );
        }
    }
    return current_file;
}

void
SCOREP_Instrumenter::prelink( void )
{
    std::deque<SCOREP_Instrumenter_Adapter*>::iterator adapter;
    for ( adapter = m_prelink_adapters.begin();
          adapter != m_prelink_adapters.end();
          adapter++ )
    {
        if ( ( *adapter )->isEnabled() )
        {
            ( *adapter )->prelink( *this, m_command_line );
        }
    }
}

void
SCOREP_Instrumenter::postlink( void )
{
    std::deque<SCOREP_Instrumenter_Adapter*>::iterator adapter;
    for ( adapter = m_postlink_adapters.begin();
          adapter != m_postlink_adapters.end();
          adapter++ )
    {
        if ( ( *adapter )->isEnabled() )
        {
            ( *adapter )->postlink( *this, m_command_line );
        }
    }
}

void
SCOREP_Instrumenter::link_step( void )
{
    if ( m_command_line.enforceStaticLinking() )
    {
        m_linker_flags = "-Bstatic " + m_linker_flags;
    }
    else if ( m_command_line.enforceDynamicLinking() )
    {
        m_linker_flags = "-Bdynamic " + m_linker_flags;
    }

    std::stringstream command;
    command << m_command_line.getCompilerName();
    command << " " << m_input_files;
    command << " " << m_command_line.getFlagsBeforeLmpi();
    command << " " << m_linker_flags;
    command << " " << m_command_line.getFlagsAfterLmpi();

    if ( m_command_line.getOutputName() != "" )
    {
        /* nvcc requires a space between -o and the output name. */
        command << " -o " << m_command_line.getOutputName();
    }

    executeCommand( command.str() );
}

void
SCOREP_Instrumenter::executeCommand( const std::string& orig_command )
{
    std::string command( orig_command );
    if ( m_command_line.getVerbosity() >= 1 )
    {
        std::cout << orig_command << std::endl;

        /* --dry-run implies --verbose */
        if ( m_command_line.isDryRun() )
        {
            return;
        }

        /* Let the shell do its job and show us all executed commands. */
        command = "PS4=' Executing: '; set -x; " + command;
    }

    int return_value = system( command.c_str() );
    if ( return_value != 0 )
    {
        std::cerr << "Error executing: " << orig_command << std::endl;
        exit( EXIT_FAILURE );
    }
}
