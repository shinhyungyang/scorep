/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2017, 2024-2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013, 2024-2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2015,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * Copyright (c) 2017,
 * Technische Universitaet Darmstadt, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 */

#include <config.h>
#include "scorep_instrumenter_utils.hpp"
#include <scorep_config_tool_backend.h>
#include <scorep_config_tool_shmem.h>

#include <scorep_tools_utils.hpp>

#include <UTILS_IO.h>
#include <UTILS_CStr.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <sys/time.h>

bool
is_fortran_file( const std::string& filename )
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
    SCOREP_CHECK_EXT( ".for" );
    SCOREP_CHECK_EXT( ".For" );
    SCOREP_CHECK_EXT( ".FOR" );
    SCOREP_CHECK_EXT( ".Ftn" );
    SCOREP_CHECK_EXT( ".FTN" );
    SCOREP_CHECK_EXT( ".f95" );
    SCOREP_CHECK_EXT( ".F95" );
    SCOREP_CHECK_EXT( ".f03" );
    SCOREP_CHECK_EXT( ".F03" );
    SCOREP_CHECK_EXT( ".f08" );
    SCOREP_CHECK_EXT( ".F08" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
is_c_file( const std::string& filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".c" );
    SCOREP_CHECK_EXT( ".C" );
    SCOREP_CHECK_EXT( ".i" );
    SCOREP_CHECK_EXT( ".I" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
is_cpp_file( const std::string& filename )
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
    SCOREP_CHECK_EXT( ".cc" );
    SCOREP_CHECK_EXT( ".CC" );
    SCOREP_CHECK_EXT( ".ii" );
    SCOREP_CHECK_EXT( ".II" );
    #undef SCOREP_CHECK_EXT
    return false;
}

/**
    Checks whether a file is a CUDA source file.
    @param filename A file name.
    @returns true if the file extension indicates CUDA source file.
 */
bool
is_cuda_file( const std::string& filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".cuf" );
    SCOREP_CHECK_EXT( ".CUF" );
    SCOREP_CHECK_EXT( ".cu" );
    SCOREP_CHECK_EXT( ".CU" );
    SCOREP_CHECK_EXT( ".cpp.ii" );
    SCOREP_CHECK_EXT( ".CPP.II" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
is_header_file( const std::string& filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".h" );
    SCOREP_CHECK_EXT( ".hpp" );
    SCOREP_CHECK_EXT( ".inc" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
is_assembler_file( const std::string& filename )
{
    std::string extension = get_extension( filename );
    if ( extension == "" )
    {
        return false;
    }
    #define SCOREP_CHECK_EXT( ext ) if ( extension == ext ) return true
    SCOREP_CHECK_EXT( ".s" );
    SCOREP_CHECK_EXT( ".S" );
    SCOREP_CHECK_EXT( ".asm" );
    #undef SCOREP_CHECK_EXT
    return false;
}

bool
is_source_file( const std::string& filename )
{
    return is_c_file( filename ) ||
           is_cpp_file( filename ) ||
           is_cuda_file( filename ) ||
           is_fortran_file( filename ) ||
           is_assembler_file( filename );
}

bool
is_object_file( const std::string& filename )
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
is_library( const std::string& filename,
            bool               allowDynamic,
            bool               allowStatic )
{
    std::string basename  = remove_path( filename );
    std::string extension = get_extension( basename );

    /* library archives do not need to start with `lib` and are not searched in the library path */
    if ( allowStatic && extension == ".a" )
    {
        return true;
    }

    if ( basename.compare( 0, 3, "lib" ) != 0 || extension == "" )
    {
        return false;
    }

    if ( allowDynamic )
    {
        if ( extension == ".so" )
        {
            return true;
        }

        /* Check for libtool library version extension */
        std::string::size_type pos = basename.rfind( ".so." );
        if ( pos == std::string::npos )
        {
            return false;
        }
        if ( basename.find_first_not_of( ".0123456789", pos + 3 ) == std::string::npos
             && extension != "." )
        {
            /* everything after ".so" consists only of "." or digits, and does not end in "." */
            return true;
        }
    }

    return false;
}

std::string
get_library_name( const std::string& filename )
{
    std::string            basename = remove_path( filename );
    std::string::size_type dot      = basename.rfind( "." );
    if ( dot == std::string::npos )
    {
        return "";
    }

    if ( basename.compare( dot, 2, ".a" ) != 0
         && basename.compare( dot, 3, ".so" ) != 0 )
    {
        dot = basename.rfind( ".so." );
        /* is_library already ensured, that this is a libtool library version extension */
    }

    /* Drop "lib" prefix and entension */
    return basename.substr( 3, dot - 3 );
}


std::string
get_preprocessed_extension( const std::string& filename )
{
    if ( is_c_file( filename ) )
    {
        return ".i";
    }
    else if ( is_cpp_file( filename ) )
    {
        /* NVHPC fails to recognize .ii as a pre-processed C++ file.
         * Fall back to using the C extension, as it is recognized just fine. */
        #if SCOREP_BACKEND_COMPILER_CXX_PGI
        return ".i";
        #else
        return ".ii";
        #endif
    }

    else if ( is_fortran_file( filename ) )
    {
        return scorep_tolower( get_extension( filename ) );
    }
    else if ( is_cuda_file( filename ) )
    {
        return ".cpp.ii";
    }
    else
    {
        return get_extension( filename );
    }
}


bool
check_lib_name( const std::string& libraryName,
                const std::string& value )
{
    std::string value_with_dot        = value + ".";
    std::string value_with_underscore = value + "_";
    return ( ( libraryName.length() == value.length() ) && ( libraryName.substr( 0, value.length() ) == value ) ) ||
           ( ( libraryName.length() > value.length() ) && ( libraryName.substr( 0, value.length() + 1 ) == value_with_dot ) ) ||
           ( ( libraryName.length() > value.length() ) && ( libraryName.substr( 0, value.length() + 1 ) == value_with_underscore ) );
}

std::string
create_random_string( void )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    std::stringstream random_string;
    random_string << "_" << tv.tv_sec
                  << "_" << tv.tv_usec;
    return random_string.str();
}

void
append_list_to_env( std::vector<std::string> newEnvItems,
                    const std::string&       environmentVariable,
                    const std::string&       delimiter )
{
    if ( newEnvItems.empty() )
    {
        return;
    }

    const char* env_variable_value = std::getenv( environmentVariable.c_str() );
    if ( env_variable_value )
    {
        newEnvItems.insert( newEnvItems.begin(), std::string( env_variable_value ) );
    }
    const auto new_value = vector_to_string( newEnvItems, "", "", delimiter );

    setenv( environmentVariable.c_str(), new_value.c_str(), /* Overwrite */ 1 );
}
