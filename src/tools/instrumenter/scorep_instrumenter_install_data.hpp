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
 * @file       scorep_instrumenter_install_config.hpp
 * @maintainer Daniel Lorenz  <d.lorenz@fz-juelich.de>
 *
 * @brief      Declares a class which represent the install configuration of
 *             this Score-P installation.
 */

#ifndef SCOREP_INSTRUMENTER_INSTALL_CONFIG_H_
#define SCOREP_INSTRUMENTER_INSTALL_CONFIG_H_

#include <SCOREP_ErrorCodes.h>
#include <string>

/* ****************************************************************************
   class SCOREP_Instrumenter_InstallData
******************************************************************************/

/**
 * This class represents the current installation configuration of Score-P to
 * the instrumenter. Thus, it provides installation dependent data. By default,
 * it uses values determined during configure time of the Score-P build.
 * However, it may read-in data from a config file.
 */
class SCOREP_Instrumenter_InstallData
{
    /* ****************************************************** Public methods */
public:
    /**
       Creates a new SCOREP_Instrumenter_InstallData object.
     */
    SCOREP_Instrumenter_InstallData( void );

    /**
       Destroys a SCOREP_Instrumenter_InstallData object.
     */
    virtual
    ~SCOREP_Instrumenter_InstallData();

    /**
       Returns the scorep-config tool.
     */
    std::string
    getScorepConfig( void );

    /**
       C compiler command
     */
    std::string
    getCC( void );

    /**
       C++ compiler command
     */
    std::string
    getCXX( void );

    /**
       Fortran compiler command
     */
    std::string
    getFC( void );

    /**
       Reads configuration data from a config file
     */
    SCOREP_ErrorCode
    readConfigFile( const std::string& arg0 );

    /**
       Perfroms the changes on the install data retrieval if it is a build
       check run, performed in the build directory instead of the install
       directory.
     */
    void
    setBuildCheck( void );

    /* ************************************** Compiler dependent implemented */

    /**
       Checks whether @a arg is the compiler option to build a shared library.
       @param arg the argument that is considered
     */
    static bool
    isArgForShared( const std::string& arg );

    /**
       Checks whether @a arg is the compiler option to use OpenMP.
       @param arg the argument that is considered
     */
    bool
    isArgForOpenmp( const std::string& arg );

    /**
       Checks whether @a arg is the compiler option to enable free form Fortran
       @param arg the argument that is considered
     */
    static bool
    isArgForFreeform( const std::string& arg );

    /**
       Checks whether @a arg is the compiler option to enable fixed form Fortran
       @param arg the argument that is considered
     */
    static bool
    isArgForFixedform( const std::string& arg );

    static std::string
    getCPreprocessingFlags( const std::string& input_file,
                            const std::string& output_file );

    static std::string
    getCxxPreprocessingFlags( const std::string& input_file,
                              const std::string& output_file );

    static std::string
    getFortranPreprocessingFlags( const std::string& input_file,
                                  const std::string& output_file );

    /**
       Returns envaronment variables needed by the compiler.
       If the string is non empty, it must include a final space character.
     */
    static std::string
    getCompilerEnvironmentVars( void );

    /**
       Checks whether an argument that starts with -o is an argument that does not
       specify an output file name.
     */
    static bool
    isArgWithO( std::string arg );

    /**
       Checks whether an argument instructs the compiler to only preprocess.
     */
    static bool
    isPreprocessFlag( std::string arg );

    /* ***************************************************** Private methods */
private:
    /**
       This function gives a (key, value) pair found in a configuration file
       and not processed by one of the former functions.
       @param key   The key
       @param value The value
     */
    void
    set_value( const std::string& key,
               const std::string& value );


    /**
       This function processes a setting of the PDT path from config file.
       @param pdt A string containing the binary directory of the PDT
       installation.
     */
    void
    set_pdt_path( const std::string& pdt );

    /**
       Extracts parameter from configuration file
       It expects lines of the format key=value. Furthermore it truncates line
       at the scrpit comment character '#'.
       @param line    input line from the config file
       @returns SCOREP_SUCCESS if the line was successfully parsed. Else it
                returns an error code.
     */
    SCOREP_ErrorCode
    read_parameter( std::string line );


    /* ***************************************************** Private members */
private:
    /**
       The scorep-config tool.
     */
    std::string m_scorep_config;

    /**
       C compiler command
     */
    std::string m_c_compiler;

    /**
       C++ compiler command
     */
    std::string m_cxx_compiler;

    /**
       Fortran compiler command
     */
    std::string m_fortran_compiler;

    /**
       Stores C compiler OpenMP flags
     */
    std::string m_openmp_cflags;
};

#endif
