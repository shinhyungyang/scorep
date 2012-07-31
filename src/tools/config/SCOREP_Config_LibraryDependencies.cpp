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
 * @file SCOREP_Config_LibraryDependecies.cpp
 * @status alpha
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * Implements the representation and analysis of library dependencies
 */

#include <config.h>
#include <SCOREP_Config_LibraryDependencies.hpp>

#include <iostream>
#include <stdlib.h>

using namespace std;

/* **************************************************************************************
                                                                          local functions
****************************************************************************************/

static inline string
strip_leading_whitespace( string input )
{
    const char* pos = input.c_str();
    while ( ( *pos != '\0' ) && ( ( *pos == ' ' ) || ( *pos == '\t' ) ) )
    {
        pos++;
    }
    return pos;
}

/**
 * Strips the head and leading demiliter from a input string. Ignores leading whitespaces.
 */
static string
strip_head( string input, string head, string delimiter )
{
    string output = strip_leading_whitespace( input );
    head      = strip_leading_whitespace( head );
    delimiter = strip_leading_whitespace( delimiter );

    if ( output.compare( 0, head.length(), head ) == 0 )
    {
        output.erase( 0, head.length() );
    }

    output = strip_leading_whitespace( input );
    if ( output.compare( 0, delimiter.length(), delimiter ) == 0 )
    {
        output.erase( 0, delimiter.length() );
    }
    return output;
}

/**
 * Checks whether @a input contains @a item.
 */
static bool
has_item( const deque<string> input, string item )
{
    deque<string>::const_iterator i;
    for ( i = input.begin(); i != input.end(); i++ )
    {
        if ( *i == item )
        {
            return true;
        }
    }
    return false;
}

/**
 * Removes dublicate entries from a deque container of strings. It keeps only the
 * last occurence of each entry. This ensures that the dependencies are maintained.
 */
static deque<string>
remove_double_entries( const deque<string> input )
{
    deque<string>                         output;
    deque<string>::const_reverse_iterator i;
    for ( i = input.rbegin(); i < input.rend(); i++ )
    {
        if ( !has_item( output, *i ) )
        {
            output.push_front( *i );
        }
    }

    return output;
}

/**
 * Converts deque of strings into a string where all entries are space separated.
 */
static string
deque_to_string( const deque<string> input,
                 const string        head,
                 const string        delimiter )
{
    string                        output = head;
    deque<string>::const_iterator i;
    for ( i = input.begin(); i != input.end(); i++ )
    {
        output += delimiter + *i;
    }
    return output;
}

/* **************************************************************************************
                                                                          class la_object
****************************************************************************************/

SCOREP_Config_LibraryDependencies::la_object::la_object()
{
}

SCOREP_Config_LibraryDependencies::la_object::la_object( const la_object &source )
{
    m_lib_name       = source.m_lib_name;
    m_build_dir      = source.m_build_dir;
    m_install_dir    = source.m_install_dir;
    m_libs           = source.m_libs;
    m_ldflags        = source.m_ldflags;
    m_rpath          = source.m_rpath;
    m_dependency_las = source.m_dependency_las;
}


SCOREP_Config_LibraryDependencies::la_object::la_object( string        lib_name,
                                                         string        build_dir,
                                                         string        install_dir,
                                                         deque<string> libs,
                                                         deque<string> ldflags,
                                                         deque<string> rpath,
                                                         deque<string> dependency_las )
{
    m_lib_name       = lib_name;
    m_build_dir      = build_dir;
    m_install_dir    = install_dir;
    m_libs           = libs;
    m_ldflags        = ldflags;
    m_rpath          = rpath;
    m_dependency_las = dependency_las;
}

SCOREP_Config_LibraryDependencies::la_object::~la_object()
{
}

/* **************************************************************************************
                                                  class SCOREP_Config_LibraryDependencies
****************************************************************************************/

SCOREP_Config_LibraryDependencies::SCOREP_Config_LibraryDependencies()
{
    deque<string> libs;
    deque<string> ldflags;
    deque<string> rpaths;
    deque<string> dependency_las;

    /* scorep_library_dependencies.cpp is generated by
       vendor/common/build-config/generate-libaray-dependency.sh */
    #include <scorep_library_dependencies.cpp>
}

SCOREP_Config_LibraryDependencies::~SCOREP_Config_LibraryDependencies()
{
}

string
SCOREP_Config_LibraryDependencies::GetLibraries( const deque<string> input_libs )
{
    deque<string>           deps = get_dependencies( input_libs );
    deque<string>           libs;
    deque<string>::iterator i;
    for ( i = deps.begin(); i != deps.end(); i++ )
    {
        la_object obj = la_objects[ *i ];
        libs.push_back( "-l" + obj.m_lib_name.substr( 3 ) );
        libs.insert( libs.end(),
                     obj.m_libs.begin(),
                     obj.m_libs.end() );
    }
    libs = remove_double_entries( libs );

    return deque_to_string( libs, "", " " );
}

string
SCOREP_Config_LibraryDependencies::GetLDFlags( const deque<string> libs, bool install )
{
    deque<string>           deps = get_dependencies( libs );
    deque<string>           flags;
    deque<string>::iterator i;
    for ( i = deps.begin(); i != deps.end(); i++ )
    {
        la_object obj = la_objects[ *i ];
        if ( install )
        {
            flags.push_back( "-L" + obj.m_install_dir );
        }
        else
        {
            flags.push_back( "-L" + obj.m_build_dir + "/.libs" );
        }
        flags.insert( flags.end(),
                      obj.m_ldflags.begin(),
                      obj.m_ldflags.end() );
    }
    flags = remove_double_entries( flags );

    return deque_to_string( flags, "", " " );
}

string
SCOREP_Config_LibraryDependencies::GetRpathFlags( const deque<string> libs, bool install, const string head, const string delimiter )
{
    deque<string>           deps = get_dependencies( libs );
    deque<string>           flags;
    deque<string>::iterator i;
    deque<string>::iterator j;
    for ( i = deps.begin(); i != deps.end(); i++ )
    {
        la_object obj = la_objects[ *i ];
        if ( install )
        {
            flags.push_back( obj.m_install_dir );
        }
        else
        {
            flags.push_back( obj.m_build_dir + "/.libs" );
        }
        for ( j = obj.m_rpath.begin(); j != obj.m_rpath.end(); j++ )
        {
            flags.push_back( strip_head( *j, head, delimiter ) );
        }
    }
    flags = remove_double_entries( flags );

    return deque_to_string( flags, head, delimiter );
}

deque<string>
SCOREP_Config_LibraryDependencies::get_dependencies( const deque<string> libs )
{
    deque<string> deps = libs;
    for ( int i = 0; i < deps.size(); i++ )
    {
        if ( la_objects.find( deps[ i ] ) == la_objects.end() )
        {
            cerr << "ERROR: Can not resolve dependency \"" << deps[ i ] << "\"" << endl;
            exit( EXIT_FAILURE );
        }
        la_object obj = la_objects[ deps[ i ] ];

        deps.insert( deps.end(),
                     obj.m_dependency_las.begin(),
                     obj.m_dependency_las.end() );
    }
    return remove_double_entries( deps );
}
