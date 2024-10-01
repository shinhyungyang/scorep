/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2013, 2017,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013, 2017, 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#ifndef SCOREP_CONFIG_LIBRARY_DEPENDENCIES_HPP
#define SCOREP_CONFIG_LIBRARY_DEPENDENCIES_HPP

/**
 * @file
 *
 * Declares classes for the representation and analysis of library dependencies
 */

#include <map>
#include <deque>
#include <string>
#include <library_data.hpp>


/**
 * A class to store and analyze the dependencies from libraries. To generate
 * the dependecies, Makefile variables are parsed and header files are generated,
 * see build-config/common/Makefile-rpaths.inc.am. Note that is is taken care of
 * that no libdirs from SYS_LIB_DLSEARCH_PATH_SPEC will make it to m_library_objects.
 */
class SCOREP_Config_LibraryDependencies final
{
    // ------------------------------------- Public functions
public:
    /**
     * Constructs the library dependencies
     */
    SCOREP_Config_LibraryDependencies( void );

    /**
     * Destructor
     */
    ~SCOREP_Config_LibraryDependencies() = default;

    /**
     * Inserts a new @a libName that is installed in @a installDir into the
     * @a m_la_objects database. Intended to be used by the library wrapper
     * only. Here, libs are known only at instrumentation time and need to be
     * introduced by a mechanism different from add_library_dependencies_*.
     * See SCOREP_Config_LibwrapAdapter
     */
    void
    insert( const std::string& libName,
            const std::string& libInstallDir );

    /**
     * Returns a container of -l prefixed libraries containing the @p
     * inputLibs (if @p honorLibs == true) and their needed libraries
     * and its dependencies (if @p HonorDeps == true) and their needed
     * libraries.
     * @param inputLibs   A list of library names.
     * @param honorLibs   Takes libraries listed in @p libs into account.
     * @param honorDeps   Takes dependencies from libraries listed in @p
     *                    libs into account.
     * @see addDependency()
     */
    std::deque<std::string>
    getLibraries( const std::deque<std::string>& inputLibs,
                  bool                           honorLibs = true,
                  bool                           honorDeps = true );

    /**
     * Returns a duplicate-free list containing the -L library
     * path flags for the @p input_libs and its dependencies.
     * @param libs    A list of library names.
     * @param install If true, the install paths are used. If false, the
     *                build path are prepended (to allow the config tool
     *                to generate valid link lines at make check time).
     */
    std::string
    getLDFlags( const std::deque<std::string>& libs,
                bool                           install );

    /**
     * Returns a duplicate-free container of paths an executable should search
     * to find NEEDED libraries. It can be used to construct the rpath flags for
     * the @a input and its dependencies.
     * @param libs      A list of library names.
     * @param install   If true, the install paths are used. If false, the
     *                  build path are prepended (to allow the config tool
     *                  to generate valid link lines at make check time).
     * @param honorLibs Takes libraries listed in @p libs into account.
     * @param honorDeps Takes dependencies from libraries listed in @p
     *                  libs into account.
     */
    std::deque<std::string>
    getLibdirs( const std::deque<std::string>& libs,
                bool                           install,
                bool                           honorLibs = true,
                bool                           honorDeps = true );

    /**
     * This function adds a @p dependency to an @p dependentLib. Both
     * must be available in @a m_library_objects. Intended to add a @a
     * libscorep_measurenment depenency to events libs and
     * corresponding mgmt dependencies as well as threading and MPP
     * dependencies to @a libscorep_measurenment.
     * @param dependentLib  The library which depends on @a dependency.
     * @param dependency    The library which is added to the dependencies of
     *                      @a dependentLib.
     */
    void
    addDependency( const std::string& dependentLib,
                   const std::string& dependency );

    // ------------------------------------- Protected functions
protected:
    /**
     * Calculates the dependencies for a given set of libraries.
     * @param libs  A list of library names.
     */
    std::deque<std::string>
    get_dependencies( const std::deque<std::string>& libs,
                      bool                           honorLibs = true,
                      bool                           honorDeps = true );

    // ------------------------------------- Public members
private:
    std::map< std::string, LibraryData> m_library_objects;
};

#endif // SCOREP_CONFIG_LIBRARY_DEPENDENCIES_HPP
