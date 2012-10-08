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
 * @file       scorep_instrumenter_command_line.hpp
 * @maintainer Daniel Lorenz  <d.lorenz@fz-juelich.de>
 *
 * @brief      Declares a class for parsing and evaluating a the command line
 *             and makes the results accessable.
 */

#ifndef SCOREP_INSTRUMENTER_COMMAND_LINE_H_
#define SCOREP_INSTRUMENTER_COMMAND_LINE_H_


#include <string>
#include <scorep_instrumenter_install_data.hpp>

/* ****************************************************************************
   class SCOREP_Instrumenter_CmdLine
******************************************************************************/

class SCOREP_Instrumenter_CmdLine
{
    /* ******************************************************* Private Types */
private:
    /**
       Type for the three values a adapter or paradigm configuration can have.
       Before a command can be executed, a decision must be made for all
       adapter and paradigms which are in detect state.
     */
    typedef enum
    {
        enabled,
        detect,
        disabled
    } instrumentation_usage_t;

    /**
       Type of the state of the command line parser. The parser starts in
       state scorep_parse_mode_param which means that arguments are interpreted
       as options of the wrapper tool. When the first argument is reached
       which has no leading dash it assumes that this is the compiler or
       linker command and changed to scorep_parse_mode_command. All further
       arguments are interpreted as arguments for the compiler/linker.
       The states scorep_parse_mode_output, and scorep_parse_mode_config are used
       to deal with arguments which reguire a value in a successive argument.
       Thus, if A user specifies a config file the state switches to
       scorep_parse_mode_config. The next argument is interpreted as the
       config file name. Then the state switches back to scorep_parse_mode_param.
       If the user command contains a '-o' the following argument is
       interpreted as the output file name and the state switches to
       scorep_parse_mode_output.
     */
    typedef enum
    {
        scorep_parse_mode_param,
        scorep_parse_mode_command,
        scorep_parse_mode_option_part,
        scorep_parse_mode_output,
        scorep_parse_mode_library,
        scorep_parse_mode_define,
        scorep_parse_mode_libdir,
        scorep_parse_mode_incdir
    } scorep_parse_mode_t;

    /* ****************************************************** Public methods */
public:

    /**
       Creates a new SCOREP_Instrumenter_CmdLine object.
       @param install_data Pointer to a installation configuration data
                           object.
     */
    SCOREP_Instrumenter_CmdLine( SCOREP_Instrumenter_InstallData* install_data );


    /**
       Destroys a SCOREP_Instrumenter_CmdLine object.
     */
    virtual
    ~SCOREP_Instrumenter_CmdLine();

    /**
       Parses the command line.
       @param argc The number of arguments.
       @param argv List of arguments. It assumes, that the first argument is
                   the tool name and the second argument is the action.
       @return SCOREP_SUCCESS if the parsing was successful. Else an error
               code is returned.
     */
    virtual void
    ParseCmdLine( int    argc,
                  char** argv );

    bool
    isCompilerInstrumenting();
    bool
    isOpariInstrumenting();
    bool
    isUserInstrumenting();
    bool
    isMpiInstrumenting();
    bool
    isPdtInstrumenting();
    bool
    isCobiInstrumenting();
    bool
    isMpiApplication();
    bool
    isOpenmpApplication();
    bool
    isCudaApplication();
    bool
    isCompiling();
    bool
    isLinking();
    std::string
    getCompilerName();
    std::string
    getCompilerFlags();
    std::string
    getIncludeFlags();
    std::string
    getDefineFlags();
    std::string
    getOutputName();
    std::string
    getInputFiles();
    std::string
    getLibraries();
    std::string
    getLibDirs();
    int
    getInputFileNumber();
    bool
    isLmpiSet();
    bool
    isDryRun();
    bool
    hasKeepFiles();
    int
    getVerbosity();
    bool
    isBuildCheck();

    /* ***************************************************** Private methods */
private:

    /**
       Prints the results from parsing the command line and parsing the
       configuration file to screen.
     */
    virtual void
    print_parameter();

    /**
       Checks whether command line parameter parsing provided meaningful
       information, applies remaining detection decisions.
     */
    void
    check_parameter();

    /**
       Evaluates one parameter when in output mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_output( std::string arg );

    /**
       Evaluates one parameter when in command mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_command( std::string arg );

    /**
       Evaluates one parameter when in option_part mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_option_part( std::string arg );

    /**
       Evaluates one parameter when in parameter mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_parameter( std::string arg );

    /**
       Evaluates one parameter when in libray mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_library( std::string arg );

    /**
       Evaluates one parameter when in define mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_define( std::string arg );

    /**
       Evaluates one parameter when in incdir mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_incdir( std::string arg );

    /**
       Evaluates one parameter when in libdir mode.
       @param arg The current argument
       @returns the parsing mode for the next parameter.
     */
    scorep_parse_mode_t
    parse_libdir( std::string arg );

    /**
       Processes a define parameter.
       @param arg The define argument.
     */
    void
    add_define( std::string arg );


    /* ***************************************************** Private members */
private:
    /**
       Pointer to the associated installation configration data
     */
    SCOREP_Instrumenter_InstallData* m_install_data;


    /* --------------------------------------------
       Flags for used adapters
       ------------------------------------------*/
    /**
       Specifies if compiler instrumentation is enabled. Default is enabled
     */
    instrumentation_usage_t m_compiler_instrumentation;

    /**
       Specifies if OPARI instrumentation is enabled. Default detect.
     */
    instrumentation_usage_t m_opari_instrumentation;

    /**
       Specifies if user instrumentation is enabled. Default is disabled.
     */
    instrumentation_usage_t m_user_instrumentation;

    /**
       Specifies if mpi wrappers are enabled. Default detect.
     */
    instrumentation_usage_t m_mpi_instrumentation;

    /**
       Specifies if pdt instrumentation enabled. Default disabled. If it is
       enabled, it will automatically enable the user adapter and disable
       the compiler instrumentation.
     */
    instrumentation_usage_t m_pdt_instrumentation;

    /**
       Specifies if binary instrumentation with Cobi is enabled.
     */
    instrumentation_usage_t m_cobi_instrumentation;

    /* --------------------------------------------
       Flags for application type
       ------------------------------------------*/
    /**
       Specifies whether it is a MPI application.
     */
    instrumentation_usage_t m_is_mpi_application;

    /**
       Specifies whether it is an OpenMP application.
     */
    instrumentation_usage_t m_is_openmp_application;

    /**
       Specifies whether it is an CUDA application.
     */
    instrumentation_usage_t m_is_cuda_application;

    /* --------------------------------------------
       Work mode information
       ------------------------------------------*/
    /**
       True if compiling
     */
    bool m_is_compiling;

    /**
       True if linking
     */
    bool m_is_linking;

    /* --------------------------------------------
       Input command elements
       ------------------------------------------*/
    /**
        compiler/linker name
     */
    std::string m_compiler_name;

    /**
       all compiler/linker flags, except defines.
     */
    std::string m_compiler_flags;

    /**
       include flags
     */
    std::string m_include_flags;

    /**
       define flags
     */
    std::string m_define_flags;

    /**
       file name of the compiler/linker output
     */
    std::string m_output_name;

    /**
       library names
     */
    std::string m_libraries;

    /**
       libdirs
     */
    std::string m_libdirs;

    /**
       input file names. Need to be separated because OPARI may
       perform source code modifications which take these as input and
       the original command needs the result from the OPRI output. Thus,
       they are then substituted by the OPRI output.
     */
    std::string m_input_files;

    /**
       number of input file names.
     */
    int m_input_file_number;

    /**
       True, if -lmpi was specified.
     */
    bool m_lmpi_set;

    /* --------------------------------------------
       Instrumenter flags
       ------------------------------------------*/
    /**
       True, if the instrumentation is a dry run. It means that the commands
       are only printed to stdout but are not executed. The default is false.
     */
    bool m_is_dry_run;

    /**
       True, if temporary files should be kept. By default temprary files are
       deleted after successful instrumentation, but kept when the instrumenter
       aborts with an error.
     */
    bool m_keep_files;

    /**
       The level of verbosity. Currently, we know the levels:
       <ul>
       <li> 0 = No output
       <li> 1 = Print executed commands
       <li> 2 = Print executed commands plus further diagnostic output
       </ul>
     */
    int m_verbosity;

    /**
       True, if this is a build check is run. Does not assume to use data
       from an instrumented run, but from the build location.
     */
    bool m_is_build_check;
};

#endif
