/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2014-2015,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * Collects information about available adapters
 */

#include <config.h>
#include <scorep_config_tool_backend.h>
#include <scorep_config_tool_mpi.h>
#include "scorep_config_adapter.hpp"
#include "scorep_config_thread.hpp"
#include "scorep_config_utils.hpp"

#include <iostream>
#include <stdlib.h>

/* ****************************************************************************
   Compiler specific defines
******************************************************************************/

#if SCOREP_BACKEND_COMPILER_CRAY
#define SCOREP_COMPILER_TYPE "cray"

#elif SCOREP_BACKEND_COMPILER_GNU
#define SCOREP_COMPILER_TYPE "gnu"

#elif SCOREP_BACKEND_COMPILER_IBM
#define SCOREP_COMPILER_TYPE "ibm"

#elif SCOREP_BACKEND_COMPILER_INTEL
#define SCOREP_COMPILER_TYPE "intel"

#elif SCOREP_BACKEND_COMPILER_PGI
#define SCOREP_COMPILER_TYPE "pgi"

#elif SCOREP_BACKEND_COMPILER_STUDIO
#define SCOREP_COMPILER_TYPE "sun"

#elif SCOREP_BACKEND_COMPILER_FUJITSU
#define SCOREP_COMPILER_TYPE "fujitsu"
#endif

/* **************************************************************************************
 * class SCOREP_Config_Adapter
 * *************************************************************************************/

std::deque<SCOREP_Config_Adapter*> SCOREP_Config_Adapter::all;

void
SCOREP_Config_Adapter::init( void )
{
    all.push_back( new SCOREP_Config_CompilerAdapter() );
    all.push_back( new SCOREP_Config_UserAdapter() );
    all.push_back( new SCOREP_Config_PompAdapter() );
    all.push_back( new SCOREP_Config_CudaAdapter() );
    all.push_back( new SCOREP_Config_OpenclAdapter() );
    all.push_back( new SCOREP_Config_PreprocessAdapter() );
}

void
SCOREP_Config_Adapter::fini( void )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        delete ( *i );
    }
}

void
SCOREP_Config_Adapter::printAll( void )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        ( *i )->printHelp();
    }
}

bool
SCOREP_Config_Adapter::checkAll( const std::string& arg )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        if ( ( *i )->checkArgument( arg ) )
        {
            return true;
        }
    }
    return false;
}

void
SCOREP_Config_Adapter::addLibsAll( std::deque<std::string>&           libs,
                                   SCOREP_Config_LibraryDependencies& deps )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        ( *i )->addLibs( libs, deps );
    }
}

void
SCOREP_Config_Adapter::addCFlagsAll( std::string&           cflags,
                                     bool                   build_check,
                                     SCOREP_Config_Language language,
                                     bool                   nvcc )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        ( *i )->addCFlags( cflags, build_check, language, nvcc );
    }
}

void
SCOREP_Config_Adapter::addIncFlagsAll( std::string& incflags,
                                       bool         build_check,
                                       bool         nvcc )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        ( *i )->addIncFlags( incflags, build_check, nvcc );
    }
}

void
SCOREP_Config_Adapter::addLdFlagsAll( std::string& ldflags,
                                      bool         build_check,
                                      bool         nvcc )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        ( *i )->addLdFlags( ldflags, build_check, nvcc );
    }
}

SCOREP_Config_Adapter::SCOREP_Config_Adapter( const std::string& name,
                                              const std::string& library,
                                              bool               is_default )
    : m_is_enabled( is_default ),
      m_name( name ),
      m_library( library )
{
}

SCOREP_Config_Adapter::~SCOREP_Config_Adapter()
{
}

void
SCOREP_Config_Adapter::printHelp( void )
{
    std::cout << "   --" << m_name << "|--no" << m_name << "\n"
              << "            Specifies whether " << m_name << " instrumentation is used.\n"
              << "            On default " << m_name << " instrumentation is " \
              << ( m_is_enabled ? "enabled" : "disabled" ) << ".\n";
}

bool
SCOREP_Config_Adapter::checkArgument( const std::string& arg )
{
    if ( arg == "--" + m_name )
    {
        m_is_enabled = true;
        return true;
    }
    else if ( arg == "--no" + m_name )
    {
        m_is_enabled = false;
        return true;
    }
    return false;
}

void
SCOREP_Config_Adapter::addLibs( std::deque<std::string>&           libs,
                                SCOREP_Config_LibraryDependencies& deps )
{
    if ( m_is_enabled )
    {
        libs.push_back( "lib" + m_library + "_event" );
        deps.addDependency( "libscorep_measurement", "lib" + m_library + "_mgmt" );
    }
}

void
SCOREP_Config_Adapter::addCFlags( std::string&           cflags,
                                  bool                   build_check,
                                  SCOREP_Config_Language language,
                                  bool                   nvcc )
{
}

void
SCOREP_Config_Adapter::addLdFlags( std::string& /* ldflags */,
                                   bool /* build_check */,
                                   bool /* nvcc */ )
{
}

void
SCOREP_Config_Adapter::addIncFlags( std::string& incflags,
                                    bool         build_check,
                                    bool         nvcc )
{
}

void
SCOREP_Config_Adapter::appendInitStructName( std::deque<std::string>& init_structs )
{
    std::string name = m_name;
    name[ 0 ] = toupper( name[ 0 ] );
    init_structs.push_back( "SCOREP_Subsystem_" + name + "Adapter" );
}

void
SCOREP_Config_Adapter::getAllInitStructNames( std::deque<std::string>& init_structs )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        if ( ( *i )->m_is_enabled )
        {
            ( *i )->appendInitStructName( init_structs );
        }
    }
}

/* **************************************************************************************
 * Compiler adapter
 * *************************************************************************************/

SCOREP_Config_CompilerAdapter::SCOREP_Config_CompilerAdapter()
    : SCOREP_Config_Adapter( "compiler", "scorep_adapter_compiler", true )
{
}

bool
SCOREP_Config_CompilerAdapter::checkArgument( const std::string& arg )
{
    if ( SCOREP_Config_Adapter::checkArgument( arg ) )
    {
        return true;
    }

#if HAVE_BACKEND( GCC_PLUGIN_SUPPORT )
    /* Catch any compiler plug-in args */
    if ( arg.substr( 0, 22 ) == "--compiler-plugin-arg=" )
    {
        m_cflags += "-fplugin-arg-scorep_instrument_function-" + arg.substr( 22 ) + " ";
        return true;
    }
#endif

    return false;
}

void
SCOREP_Config_CompilerAdapter::addCFlags( std::string&           cflags,
                                          bool                   build_check,
                                          SCOREP_Config_Language language,
                                          bool /* nvcc */ )
{
    if ( m_is_enabled )
    {
        cflags += SCOREP_COMPILER_INSTRUMENTATION_CFLAGS " ";

#if HAVE_BACKEND( GCC_PLUGIN_SUPPORT )
        if ( build_check )
        {
            extern std::string path_to_binary;
            cflags += "-fplugin=" + path_to_binary + "../build-gcc-plugin/" LT_OBJDIR "scorep_instrument_function.so ";
        }
        else
        {
            cflags += "-fplugin=" SCOREP_PKGLIBDIR "/scorep_instrument_function.so ";
        }
        cflags += m_cflags;
#endif
    }
}

void
SCOREP_Config_CompilerAdapter::addLdFlags( std::string& ldflags,
                                           bool /* build_check */,
                                           bool         nvcc )
{
    if ( m_is_enabled )
    {
        if ( nvcc )
        {
            ldflags += " " SCOREP_COMPILER_INSTRUMENTATION_LDFLAGS;
        }
        else
        {
            /*
             * Add compiler instrumentation cflags, because we encaountered a
             * case on JUQUEEN with the XLC compiler, that performed
             * rebuilding of code during linking on high optimization
             * levels. If the link command does not contain compiler
             * instrumentation flags, the code was not instrumented.
             *
             * However, we got errors with PGI on Todi, because compiler instrumentation
             * functions were defined twice. Thus do not add the
             * compiler instrumentation for PGI.
             *
             * See also ticket #855.
             */
#if !SCOREP_BACKEND_COMPILER_PGI
            ldflags += " " SCOREP_COMPILER_INSTRUMENTATION_CFLAGS;
#endif
            ldflags += " " SCOREP_COMPILER_INSTRUMENTATION_LDFLAGS;
        }
    }
}

/* **************************************************************************************
 * User adapter
 * *************************************************************************************/
SCOREP_Config_UserAdapter::SCOREP_Config_UserAdapter()
    : SCOREP_Config_Adapter( "user", "scorep_adapter_user", false )
{
}

void
SCOREP_Config_UserAdapter::addCFlags( std::string&           cflags,
                                      bool                   build_check,
                                      SCOREP_Config_Language language,
                                      bool /* nvcc */ )
{
    if ( m_is_enabled )
    {
        if ( language == SCOREP_CONFIG_LANGUAGE_FORTRAN )
        {
                #if SCOREP_BACKEND_COMPILER_IBM
            cflags += "-WF,-DSCOREP_USER_ENABLE ";
                #else
            cflags += "-DSCOREP_USER_ENABLE ";
                #endif // SCOREP_BACKEND_COMPILER_IBM
        }
        else
        {
            cflags += "-DSCOREP_USER_ENABLE ";
        }
    }
}

/* **************************************************************************************
 * Cuda adapter
 * *************************************************************************************/
SCOREP_Config_CudaAdapter::SCOREP_Config_CudaAdapter()
    : SCOREP_Config_Adapter( "cuda", "scorep_adapter_cuda", true )
{
}

bool
SCOREP_Config_CudaAdapter::checkArgument( const std::string& arg )
{
#if HAVE_BACKEND( CUDA_SUPPORT )
    if ( arg == "--" + m_name )
    {
        m_is_enabled = true;
        return true;
    }
#endif
    if ( arg == "--no" + m_name )
    {
        m_is_enabled = false;
        return true;
    }
    return false;
}

void
SCOREP_Config_CudaAdapter::addLibs( std::deque<std::string>&           libs,
                                    SCOREP_Config_LibraryDependencies& deps )
{
    if ( HAVE_BACKEND_CUDA_SUPPORT && m_is_enabled )
    {
        libs.push_back( "lib" + m_library + "_event" );
        deps.addDependency( "libscorep_measurement", "lib" + m_library + "_mgmt" );
    }
}

/* **************************************************************************************
 * OpenCL adapter
 * *************************************************************************************/
SCOREP_Config_OpenclAdapter::SCOREP_Config_OpenclAdapter()
    : SCOREP_Config_Adapter( "opencl", "scorep_adapter_opencl", true )
{
}

bool
SCOREP_Config_OpenclAdapter::checkArgument( const std::string& arg )
{
#if HAVE_BACKEND( OPENCL_SUPPORT )
    if ( arg == "--" + m_name )
    {
        m_is_enabled = true;
        return true;
    }
#endif
    if ( arg == "--no" + m_name )
    {
        m_is_enabled = false;
        return true;
    }
    return false;
}

void
SCOREP_Config_OpenclAdapter::addLibs( std::deque<std::string>&           libs,
                                      SCOREP_Config_LibraryDependencies& deps )
{
    if ( HAVE_BACKEND_OPENCL_SUPPORT && m_is_enabled )
    {
        libs.push_back( "lib" + m_library + "_event_static" );
        deps.addDependency( "libscorep_measurement", "lib" + m_library + "_mgmt_static" );
    }
}

void
SCOREP_Config_OpenclAdapter::addLdFlags( std::string& ldflags,
                                         bool         build_check,
                                         bool         nvcc )
{
    if ( m_is_enabled )
    {
        if ( build_check )
        {
            extern std::string path_to_binary;
            if ( nvcc )
            {
                ldflags += " -Wl,@" + path_to_binary + "../share/opencl.nvcc.wrap";
            }
            else
            {
                ldflags += " -Wl,@" + path_to_binary + "../share/opencl.wrap";
            }
        }
        else
        {
            if ( nvcc )
            {
                ldflags += " -Wl,@" SCOREP_DATADIR "/opencl.nvcc.wrap";
            }
            else
            {
                ldflags += " -Wl,@" SCOREP_DATADIR "/opencl.wrap";
            }
        }
    }
}

/* **************************************************************************************
 * Preprocess adapter
 * *************************************************************************************/
SCOREP_Config_PreprocessAdapter::SCOREP_Config_PreprocessAdapter()
    : SCOREP_Config_Adapter( "preprocess", "", false )
{
}

void
SCOREP_Config_PreprocessAdapter::addCFlags( std::string&           cflags,
                                            bool                   build_check,
                                            SCOREP_Config_Language language,
                                            bool                   nvcc )
{
    if ( m_is_enabled && ( language == SCOREP_CONFIG_LANGUAGE_CXX ) )
    {
        cflags += SCOREP_NO_PREINCLUDE_FLAG " ";
    }
}

void
SCOREP_Config_PreprocessAdapter::addLibs( std::deque<std::string>&           libs,
                                          SCOREP_Config_LibraryDependencies& deps )
{
}

void
SCOREP_Config_PreprocessAdapter::appendInitStructName( std::deque<std::string>& init_structs )
{
    // Do nothing here as PreprocessAdapter is no SCOREP_Subsystem.
}


/* **************************************************************************************
 * Pomp adapter
 * *************************************************************************************/
SCOREP_Config_PompAdapter::SCOREP_Config_PompAdapter()
    : SCOREP_Config_Adapter( "pomp", "scorep_adapter_pomp_user", false )
{
}

void
SCOREP_Config_PompAdapter::addIncFlags( std::string& incflags,
                                        bool         build_check,
                                        bool         nvcc )
{
    if ( m_is_enabled )
    {
        printOpariCFlags( build_check,
                          false,
                          SCOREP_CONFIG_LANGUAGE_C,
                          nvcc );
    }
}

void
SCOREP_Config_PompAdapter::addCFlags( std::string&           cflags,
                                      bool                   build_check,
                                      SCOREP_Config_Language language,
                                      bool                   nvcc )
{
    if ( m_is_enabled )
    {
        printOpariCFlags( build_check,
                          true,
                          language,
                          nvcc );
    }
}


void
SCOREP_Config_PompAdapter::appendInitStructName( std::deque<std::string>& init_structs )
{
    init_structs.push_back( "SCOREP_Subsystem_PompUserAdapter" );
}


void
SCOREP_Config_PompAdapter::printOpariCFlags( bool                   build_check,
                                             bool                   with_cflags,
                                             SCOREP_Config_Language language,
                                             bool                   nvcc )
{
    static bool printed_once = false;
    if ( !printed_once )
    {
        printed_once = true;

#if !HAVE( EXTERNAL_OPARI2 )
        if ( build_check )
        {
            extern std::string path_to_binary;
            std::cout << "-I" + path_to_binary + AFS_PACKAGE_SRCDIR "vendor/opari2/include ";
            std::cout.flush();
            return;
        }
#endif

        std::string opari_config = "`" OPARI_CONFIG " --cflags";
#if ( !defined HAVE_BACKEND_OPARI2_REVISION ) || ( HAVE_BACKEND_OPARI2_REVISION < 1068 )
        if ( with_cflags )
        {
            opari_config += "=" SCOREP_COMPILER_TYPE;

            if ( language == SCOREP_CONFIG_LANGUAGE_FORTRAN )
            {
                opari_config += " --fortran";
            }
        }
#endif
        opari_config += "` ";

        if ( nvcc )
        {
            opari_config = "printf -- \"-Xcompiler %s \" " + opari_config;
        }
        else
        {
            opari_config = "printf -- \"%s \" " + opari_config;
        }

        int return_value = system( opari_config.c_str() );
        if ( return_value != 0 )
        {
            std::cerr << "Error executing: " << opari_config.c_str() << std::endl;
            exit( EXIT_FAILURE );
        }
        std::cout << " ";
        std::cout.flush();
    }
}
