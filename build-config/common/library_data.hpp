#ifndef COMMON_LIBRARY_DATA_HPP
#define COMMON_LIBRARY_DATA_HPP

/*
 * This file is part of the Score-P software ecosystem (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2014,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013, 2017, 2023-2024,
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
 * @file
 */

#include <string>
#include <deque>
#include <sstream>
#include <algorithm>

/**
 * Helper struct to represent an installed library as well as its
 * dependencies and required paths to use the library. Intended to be
 * used by generated code
 * (<builddir>/src/config-external-libs-@AFS_PACKAGE_BUILD_name@.h,
 * see Makefile-rpaths.inc.am) and the packages' config tools. To make
 * usage easier, members are public.
 */
struct LibraryData
{
    /**
     * Empty constructor. Needed to allow copies of STL containers containing this
     * class
     */
    LibraryData( void )
    {
    }

    /**
     * Copy constructor.
     */
    LibraryData( const LibraryData& source )
        :
        m_lib_name( source.m_lib_name ),
        m_build_dir( source.m_build_dir ),
        m_install_dir( source.m_install_dir ),
        m_needs_libs( source.m_needs_libs ),
        m_needs_libdirs( source.m_needs_libdirs ),
        m_dependencies( source.m_dependencies )
    {
    }

    /**
     * Regular constructor for improved library dependency generation.
     * Intended to be called from generated
     * <builddir>/src/config-external-libs-@AFS_PACKAGE_BUILD_name@.h.
     *
     * @param libName Unique library name with 'lib' prefix, e.g., 'libfoo'.
     * @param buildDir Absolute directory where libfoo has been built.
     * @param installDir Absolute directory where libfoo is installed.
     * @param needsLibs Space-separated libraries needed to use
     *        libfoo, with '-l' prefix, e.g., '-lbar -lfoobar'. Empty
     *        for shared builds. Converted to a container of separate
     *        libraries.
     * @param needsLibdirs Colon-separated absolute paths needed to
     *        use libfoo's @a needsLibs. Empty for shared builds.
     *        Converted to a container of separate paths.
     */
    LibraryData( const std::string& libName,
                 const std::string& buildDir,
                 const std::string& installDir,
                 const std::string& needsLibs,
                 const std::string& needsLibdirs )
        :
        m_lib_name( libName ),
        m_build_dir( buildDir ),
        m_install_dir( installDir ),
        m_needs_libs( gen_deque( needsLibs, '\0', "" ) ),
        m_needs_libdirs( gen_deque( needsLibdirs, ':', "" ) ),
        m_dependencies()
    {
    }

    /**
     * Destructor.
     */
    virtual
    ~LibraryData()
    {
    }

    /**
     * Member.
     */
    std::string             m_lib_name;
    std::string             m_build_dir;
    std::string             m_install_dir;
    std::deque<std::string> m_needs_libs;
    std::deque<std::string> m_needs_libdirs;
    /// Container of other m_lib_name that might be populated by
    /// packages' config tools. Initially empty.
    std::deque<std::string> m_dependencies;

private:
    // Helper method to populate container<string> members from
    // strings provided to the ctor from
    // <builddir>/src/config-external-libs-@AFS_PACKAGE_BUILD_name@.h.
    static std::deque<std::string>
    gen_deque( std::string             input,
               std::string::value_type replace,
               std::string             prefix )
    {
        if ( replace != '\0' )
        {
            std::replace( input.begin(), input.end(), replace, ' ' );
        }
        std::stringstream       stream( input );
        std::deque<std::string> output;
        std::string             item;
        while ( stream >> item )
        {
            output.push_back( prefix + item );
        }
        return output;
    }
};

#endif /* COMMON_LIBRARY_DATA_HPP */
