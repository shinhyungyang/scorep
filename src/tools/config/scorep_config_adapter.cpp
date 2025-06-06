/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2014, 2017, 2019-2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2014-2020, 2022, 2025,
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
#include <scorep_config_tool_llvm_plugin.h>
#include "scorep_config_adapter.hpp"
#include "scorep_config_thread.hpp"

#include <scorep_tools_utils.hpp>

#include <iostream>
#include <stdlib.h>

/* **************************************************************************************
 * class SCOREP_Config_Adapter
 * *************************************************************************************/

std::deque<SCOREP_Config_Adapter*> SCOREP_Config_Adapter::all;
void
SCOREP_Config_Adapter::init( void )
{
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION )
    all.push_back( new SCOREP_Config_CompilerAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "compiler" ) );
#endif
    all.push_back( new SCOREP_Config_UserAdapter() );
    all.push_back( new SCOREP_Config_Opari2Adapter() );
#if HAVE_BACKEND( CUDA_SUPPORT )
    all.push_back( new SCOREP_Config_CudaAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "cuda" ) );
#endif
#if HAVE_BACKEND( HIP_SUPPORT )
    all.push_back( new SCOREP_Config_HipAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "hip" ) );
#endif
#if HAVE_BACKEND( OPENACC_PROFILING_SUPPORT )
    all.push_back( new SCOREP_Config_OpenaccAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "openacc" ) );
#endif
#if HAVE_BACKEND( OPENCL_SUPPORT )
    all.push_back( new SCOREP_Config_OpenclAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "opencl" ) );
#endif
#if HAVE_BACKEND( KOKKOS_SUPPORT )
    all.push_back( new SCOREP_Config_KokkosAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "kokkos" ) );
#endif
    all.push_back( new SCOREP_Config_PreprocessAdapter() );
#if HAVE_BACKEND( MEMORY_SUPPORT )
    all.push_back( new SCOREP_Config_MemoryAdapter() );
#else
    all.push_back( new SCOREP_Config_MockupAdapter( "memory" ) );
#endif
    all.push_back( new SCOREP_Config_IoAdapter() );
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
        if ( ( *i )->m_is_enabled )
        {
            ( *i )->addLibs( libs, deps );
        }
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
        if ( ( *i )->m_is_enabled )
        {
            ( *i )->addCFlags( cflags, build_check, language, nvcc );
        }
    }
}

void
SCOREP_Config_Adapter::addIncFlagsAll( std::string&           incflags,
                                       bool                   build_check,
                                       SCOREP_Config_Language language,
                                       bool                   nvcc )
{
    std::deque<SCOREP_Config_Adapter*>::iterator i;
    for ( i = all.begin(); i != all.end(); i++ )
    {
        if ( ( *i )->m_is_enabled )
        {
            ( *i )->addIncFlags( incflags, build_check, language, nvcc );
        }
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
        if ( ( *i )->m_is_enabled )
        {
            ( *i )->addLdFlags( ldflags, build_check, nvcc );
        }
    }
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

SCOREP_Config_Adapter::SCOREP_Config_Adapter( const std::string& name,
                                              const std::string& library,
                                              bool               is_default )
    : m_is_enabled( is_default ),
    m_name( name ),
    m_library( library )
{
}

void
SCOREP_Config_Adapter::printHelp( void )
{
    std::cout << "   --" << m_name << "|--no" << m_name << "\n"
              << "              Specifies whether " << m_name << " instrumentation is used.\n"
              << "              On default " << m_name << " instrumentation is " \
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
    // Event libraries need libscorep_measurement, which needs the
    // corresponding mgmt libraries.
    libs.push_back( "lib" + m_library + "_event" );
    deps.addDependency( libs.back(), "libscorep_measurement" );
    deps.addDependency( "libscorep_measurement", "lib" + m_library + "_mgmt" );
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
SCOREP_Config_Adapter::addIncFlags( std::string&           incflags,
                                    bool                   build_check,
                                    SCOREP_Config_Language language,
                                    bool                   nvcc )
{
}

void
SCOREP_Config_Adapter::appendInitStructName( std::deque<std::string>& init_structs )
{
    std::string name = m_name;
    name[ 0 ] = toupper( name[ 0 ] );
    init_structs.push_back( "SCOREP_Subsystem_" + name + "Adapter" );
}

/* **************************************************************************************
 * Mockup adapter
 * *************************************************************************************/

SCOREP_Config_MockupAdapter::SCOREP_Config_MockupAdapter( const std::string& name )
    : SCOREP_Config_Adapter( name, "", false )
{
}

void
SCOREP_Config_MockupAdapter::printHelp( void )
{
}

bool
SCOREP_Config_MockupAdapter::checkArgument( const std::string& arg )
{
    if ( arg == "--" + m_name )
    {
        std::cerr << "[Score-P] ERROR: Unsupported feature '" << m_name << "' cannot be enabled by '" << arg << "'" << std::endl;
        exit( EXIT_FAILURE );
        return false;
    }

    if ( arg == "--no" + m_name )
    {
        m_is_enabled = false;
        return true;
    }

    return false;
}

void
SCOREP_Config_MockupAdapter::addLibs( std::deque<std::string>& /* libs */,
                                      SCOREP_Config_LibraryDependencies& /* deps */ )
{
}

void
SCOREP_Config_MockupAdapter::appendInitStructName( std::deque<std::string>& init_structs )
{
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
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION )
    if ( SCOREP_Config_Adapter::checkArgument( arg ) )
    {
        return true;
    }

#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_GCC_PLUGIN ) || \
    HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_VT_INTEL ) || \
    HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_LLVM_PLUGIN )
    /* Catch any compiler plug-in args */
    if ( arg.substr( 0, 15 ) == "--compiler-arg=" )
    {
        m_cflags += arg.substr( 15 ) + " ";
        return true;
    }
#endif /* HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_{GCC_PLUGIN,VT_INTEL,LLVM_PLUGIN} ) */
#endif /* HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION )*/

    return false;
}

void
SCOREP_Config_CompilerAdapter::addCFlags( std::string&           cflags,
                                          bool                   build_check,
                                          SCOREP_Config_Language language,
                                          bool /* nvcc */ )
{
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_GCC_PLUGIN )
    bool gcc_plugin_instrumentation_available = false;
#endif
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_VT_INTEL )
    bool vt_intel_instrumentation_available = false;
#endif
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_LLVM_PLUGIN )
    bool        llvm_plugin_instrumentation_available = false;
    std::string llvm_compiler_backend_flag_arg;
#endif

    switch ( language )
    {
        case SCOREP_CONFIG_LANGUAGE_C:
            cflags += SCOREP_COMPILER_INSTRUMENTATION_CFLAGS;
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_CC_GCC_PLUGIN )
            gcc_plugin_instrumentation_available = true;
#elif HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_CC_VT_INTEL )
            vt_intel_instrumentation_available = true;
#elif HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_CC_LLVM_PLUGIN )
            llvm_plugin_instrumentation_available = true;
            llvm_compiler_backend_flag_arg        = SCOREP_LLVM_PLUGIN_COMPILER_BACKEND_FLAG_CC;
#endif  /* HAVE_BACKEND(SCOREP_COMPILER_INSTRUMENTATION_CC_[...]) */
            break;
        case SCOREP_CONFIG_LANGUAGE_CXX:
            cflags += SCOREP_COMPILER_INSTRUMENTATION_CXXFLAGS;
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_CXX_GCC_PLUGIN )
            gcc_plugin_instrumentation_available = true;
#elif HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_CXX_VT_INTEL )
            vt_intel_instrumentation_available = true;
#elif HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_CXX_LLVM_PLUGIN )
            llvm_plugin_instrumentation_available = true;
            llvm_compiler_backend_flag_arg        = SCOREP_LLVM_PLUGIN_COMPILER_BACKEND_FLAG_CXX;
#endif  /* HAVE_BACKEND(SCOREP_COMPILER_INSTRUMENTATION_CXX_[...]) */
            break;
        case SCOREP_CONFIG_LANGUAGE_FORTRAN:
            cflags += SCOREP_COMPILER_INSTRUMENTATION_FCFLAGS;
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_FC_GCC_PLUGIN )
            gcc_plugin_instrumentation_available = true;
#elif HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_FC_VT_INTEL )
            vt_intel_instrumentation_available = true;
#elif HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_FC_LLVM_PLUGIN )
            llvm_plugin_instrumentation_available = true;
            llvm_compiler_backend_flag_arg        = SCOREP_LLVM_PLUGIN_COMPILER_BACKEND_FLAG_FC;
#endif  /* HAVE_BACKEND(SCOREP_COMPILER_INSTRUMENTATION_FC_[...]) */
            break;
        default:
            break;
    }
    cflags += " ";

#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_GCC_PLUGIN )
    if ( gcc_plugin_instrumentation_available )
    {
        if ( build_check )
        {
            extern std::string path_to_binary;
            cflags += "-fplugin=" + path_to_binary + "../build-gcc-plugin/" LT_OBJDIR "scorep_instrument_function_gcc.so ";
        }
        else
        {
            cflags += "-fplugin=" SCOREP_PKGLIBDIR "/scorep_instrument_function_gcc.so ";
        }
        cflags += m_cflags;
    }
#endif /* HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_GCC_PLUGIN ) */
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_VT_INTEL )
    if ( vt_intel_instrumentation_available )
    {
        cflags += m_cflags;
    }
#endif /* HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_VT_INTEL ) */
#if HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_LLVM_PLUGIN )
    if ( llvm_plugin_instrumentation_available )
    {
        if ( build_check )
        {
            extern std::string path_to_binary;
            cflags += "-fpass-plugin=" + path_to_binary + "../build-llvm-plugin/" LT_OBJDIR "scorep_instrument_function_llvm.so -g ";
            cflags += llvm_compiler_backend_flag_arg + " -load " + llvm_compiler_backend_flag_arg + " " + path_to_binary + "../build-llvm-plugin/" LT_OBJDIR "scorep_instrument_function_llvm.so ";
        }
        else
        {
            cflags += "-fpass-plugin=" SCOREP_PKGLIBDIR "/scorep_instrument_function_llvm.so -g ";
            cflags += llvm_compiler_backend_flag_arg + " -load " + llvm_compiler_backend_flag_arg + " " + SCOREP_PKGLIBDIR "/scorep_instrument_function_llvm.so ";
        }
        cflags += m_cflags;
    }
#endif /* HAVE_BACKEND( SCOREP_COMPILER_INSTRUMENTATION_LLVM_PLUGIN ) */
}

void
SCOREP_Config_CompilerAdapter::addLdFlags( std::string& ldflags,
                                           bool /* build_check */,
                                           bool         nvcc )
{
    if ( nvcc )
    {
        ldflags += " " SCOREP_COMPILER_INSTRUMENTATION_LDFLAGS;
    }
    else
    {
        /* We used to add compiler instrumentation cflags here because the
         * xlc compiler on BG/Q (deprecated) rebuild code at link time if ipo
         * optimization levels were used. For PGI, adding flags failed as it then
         * linked its own profiling libraries.
         * Given that compiler instrumentation flags are now language dependent
         * and calls are inserted at compile time, cflags should
         * not be needed at link time.
         *
         * See https://gitlab.jsc.fz-juelich.de/perftools-svntogit/scorep-trac/-/blob/master/ticket/855.tracwiki
         */
        ldflags += " " SCOREP_COMPILER_INSTRUMENTATION_LDFLAGS;
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
SCOREP_Config_UserAdapter::addIncFlags( std::string&           cflags,
                                        bool                   build_check,
                                        SCOREP_Config_Language language,
                                        bool /* nvcc */ )
{
    if ( language == SCOREP_CONFIG_LANGUAGE_FORTRAN )
    {
#if SCOREP_BACKEND_COMPILER_FC_IBM
        cflags += "-WF,";
#endif  // SCOREP_BACKEND_COMPILER_FC_IBM
        cflags += "-DSCOREP_USER_ENABLE ";
    }
    else
    {
        cflags += "-DSCOREP_USER_ENABLE ";
    }
}

/* **************************************************************************************
 * Cuda adapter
 * *************************************************************************************/
SCOREP_Config_CudaAdapter::SCOREP_Config_CudaAdapter()
    : SCOREP_Config_Adapter( "cuda", "scorep_adapter_cuda", false )
{
}

void
SCOREP_Config_CudaAdapter::addCFlags( std::string& cflags,
                                      bool /* build_check */,
                                      SCOREP_Config_Language /* language */,
                                      bool /* nvcc */ )
{
}

void
SCOREP_Config_CudaAdapter::addLdFlags( std::string& ldflags,
                                       bool         build_check,
                                       bool         nvcc )
{
#if !HAVE_BACKEND( CUDA_TESTS ) && HAVE_BACKEND( BUILD_SHARED_LT_LIBRARIES )
    ldflags += " " SCOREP_BACKEND_CUDA_STUBS_LDFLAGS;
#endif /* !HAVE_BACKEND( CUDA_TESTS ) && HAVE_BACKEND( BUILD_SHARED_LT_LIBRARIES ) */
#if SCOREP_BACKEND_COMPILER_CC_PGI && \
    SCOREP_BACKEND_COMPILER_CXX_PGI && \
    SCOREP_BACKEND_COMPILER_FC_PGI
    if ( !nvcc )
    {
        ldflags += " -cuda";
    }
#endif /* SCOREP_BACKEND_COMPILER_{CC|CXX|FC}_PGI */
}

void
SCOREP_Config_CudaAdapter::appendInitStructName( std::deque<std::string>& initStructs )
{
    initStructs.push_back( "SCOREP_Subsystem_AcceleratorManagement" );
    SCOREP_Config_Adapter::appendInitStructName( initStructs );
}

/* **************************************************************************************
 * HIP adapter
 * *************************************************************************************/
SCOREP_Config_HipAdapter::SCOREP_Config_HipAdapter()
    : SCOREP_Config_Adapter( "hip", "scorep_adapter_hip", false )
{
}

void
SCOREP_Config_HipAdapter::addCFlags( std::string&           cflags,
                                     bool                   build_check,
                                     SCOREP_Config_Language language,
                                     bool                   nvcc )
{
    /* The HIP adapter should only be invoked on hipcc normally.
     * In this case we want to ensure that `-finstrument-functions*`
     * is restricted to host code only.
     */
    const std::string pattern     = "-finstrument-functions";
    const std::string replacement = "-Xarch_host -finstrument-functions";
    if ( cflags.find( replacement ) == std::string::npos )
    {
        cflags = replace_all( pattern, replacement, cflags );
    }
}

void
SCOREP_Config_HipAdapter::addLibs( std::deque<std::string>&           libs,
                                   SCOREP_Config_LibraryDependencies& deps )
{
    /* Add implicit dependency to libscorep_measurement as there is no
       event adapter who usually takes care of this. */
    deps.addImplicitDependency( "libscorep_measurement" );
    deps.addDependency( "libscorep_measurement", "lib" + m_library + "_mgmt" );
}

void
SCOREP_Config_HipAdapter::appendInitStructName( std::deque<std::string>& initStructs )
{
    initStructs.push_back( "SCOREP_Subsystem_AcceleratorManagement" );
    SCOREP_Config_Adapter::appendInitStructName( initStructs );
}

/* **************************************************************************************
 * OpenACC adapter
 * *************************************************************************************/
SCOREP_Config_OpenaccAdapter::SCOREP_Config_OpenaccAdapter()
    : SCOREP_Config_Adapter( "openacc", "scorep_adapter_openacc", false )
{
}

/* **************************************************************************************
 * OpenCL adapter
 * *************************************************************************************/
SCOREP_Config_OpenclAdapter::SCOREP_Config_OpenclAdapter()
    : SCOREP_Config_Adapter( "opencl", "scorep_adapter_opencl", true )
{
}

void
SCOREP_Config_OpenclAdapter::printHelp( void )
{
    std::cout << "   --opencl|--noopencl\n"
              << "              Specifies whether OpenCL instrumentation is used.\n"
              << "              On default OpenCL instrumentation is enabled.\n"
              << "              Recording must be explicitly enabled.\n";
}

void
SCOREP_Config_OpenclAdapter::appendInitStructName( std::deque<std::string>& initStructs )
{
    initStructs.push_back( "SCOREP_Subsystem_AcceleratorManagement" );
    SCOREP_Config_Adapter::appendInitStructName( initStructs );
}

/* **************************************************************************************
 * Kokkos adapter
 * *************************************************************************************/
SCOREP_Config_KokkosAdapter::SCOREP_Config_KokkosAdapter()
    : SCOREP_Config_Adapter( "kokkos", "scorep_adapter_kokkos", true )
{
}

void
SCOREP_Config_KokkosAdapter::addLibs( std::deque<std::string>&           libs,
                                      SCOREP_Config_LibraryDependencies& deps )
{
    /* Add implicit dependency to libscorep_measurement as there is no event
       adapter (to be linked at link time) who usually takes care of this. */
    deps.addImplicitDependency( "libscorep_measurement" );
    /* the Kokkos event library is loaded by the Kokkos runtime via
       KOKKOS_PROFILE_LIBRARY */
    deps.addDependency( "libscorep_measurement", "lib" + m_library + "_mgmt" );
}

void
SCOREP_Config_KokkosAdapter::appendInitStructName( std::deque<std::string>& initStructs )
{
    initStructs.push_back( "SCOREP_Subsystem_AcceleratorManagement" );
    SCOREP_Config_Adapter::appendInitStructName( initStructs );
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
    if ( language == SCOREP_CONFIG_LANGUAGE_C )
    {
        cflags += SCOREP_C_NO_PREINCLUDE_FLAG " ";
    }
    if ( language == SCOREP_CONFIG_LANGUAGE_CXX )
    {
        cflags += SCOREP_CXX_NO_PREINCLUDE_FLAG " ";
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
 * Opari2 adapter
 * *************************************************************************************/
SCOREP_Config_Opari2Adapter::SCOREP_Config_Opari2Adapter()
    : SCOREP_Config_Adapter( "pomp", "scorep_adapter_opari2_user", false )
{
}

void
SCOREP_Config_Opari2Adapter::addIncFlags( std::string&           incflags,
                                          bool                   build_check,
                                          SCOREP_Config_Language language,
                                          bool                   nvcc )
{
    printOpariCFlags( build_check,
                      false,
                      language,
                      nvcc );
}

void
SCOREP_Config_Opari2Adapter::addCFlags( std::string&           cflags,
                                        bool                   build_check,
                                        SCOREP_Config_Language language,
                                        bool                   nvcc )
{
    printOpariCFlags( build_check,
                      true,
                      language,
                      nvcc );
}


void
SCOREP_Config_Opari2Adapter::appendInitStructName( std::deque<std::string>& init_structs )
{
    init_structs.push_back( "SCOREP_Subsystem_Opari2UserAdapter" );
}


void
SCOREP_Config_Opari2Adapter::printOpariCFlags( bool                   build_check,
                                               bool                   with_cflags,
                                               SCOREP_Config_Language language,
                                               bool                   nvcc )
{
    std::string compiler_type;
    switch ( language )
    {
        case SCOREP_CONFIG_LANGUAGE_C:
        {
#if SCOREP_BACKEND_COMPILER_CC_CRAY
            compiler_type = "cray";
#elif SCOREP_BACKEND_COMPILER_CC_GNU
            compiler_type = "gnu";
#elif SCOREP_BACKEND_COMPILER_CC_IBM
            compiler_type = "ibm";
#elif SCOREP_BACKEND_COMPILER_CC_INTEL
            compiler_type = "intel";
#elif SCOREP_BACKEND_COMPILER_CC_INTEL_ONEAPI
            compiler_type = "gnu";
#elif SCOREP_BACKEND_COMPILER_CC_PGI
            compiler_type = "pgi";
#elif SCOREP_BACKEND_COMPILER_CC_FUJITSU
            compiler_type = "fujitsu";
#elif SCOREP_BACKEND_COMPILER_CC_CLANG
            compiler_type = "clang";
#else
#error "Missing compiler specific handling, extension required."
#endif
            break;
        }
        case SCOREP_CONFIG_LANGUAGE_CXX:
        {
#if SCOREP_BACKEND_COMPILER_CXX_CRAY
            compiler_type = "cray";
#elif SCOREP_BACKEND_COMPILER_CXX_GNU
            compiler_type = "gnu";
#elif SCOREP_BACKEND_COMPILER_CXX_IBM
            compiler_type = "ibm";
#elif SCOREP_BACKEND_COMPILER_CXX_INTEL
            compiler_type = "intel";
#elif SCOREP_BACKEND_COMPILER_CXX_INTEL_ONEAPI
            compiler_type = "gnu";
#elif SCOREP_BACKEND_COMPILER_CXX_PGI
            compiler_type = "pgi";
#elif SCOREP_BACKEND_COMPILER_CXX_FUJITSU
            compiler_type = "fujitsu";
#elif SCOREP_BACKEND_COMPILER_CXX_CLANG
            compiler_type = "clang";
#else
#error "Missing compiler specific handling, extension required."
#endif
            break;
        }
        case SCOREP_CONFIG_LANGUAGE_FORTRAN:
        {
#if SCOREP_BACKEND_HAVE_FC_COMPILER
#if SCOREP_BACKEND_COMPILER_FC_CRAY
            compiler_type = "cray";
#elif SCOREP_BACKEND_COMPILER_FC_GNU
            compiler_type = "gnu";
#elif SCOREP_BACKEND_COMPILER_FC_IBM
            compiler_type = "ibm";
#elif SCOREP_BACKEND_COMPILER_FC_INTEL || SCOREP_BACKEND_COMPILER_FC_INTEL_ONEAPI
            compiler_type = "intel";
#elif SCOREP_BACKEND_COMPILER_FC_PGI
            compiler_type = "pgi";
#elif SCOREP_BACKEND_COMPILER_FC_FUJITSU
            compiler_type = "fujitsu";
#elif SCOREP_BACKEND_COMPILER_FC_CLANG
            compiler_type = "clang";
#else
#error "Missing compiler specific handling, extension required."
#endif
#endif      /* SCOREP_BACKEND_HAVE_FC_COMPILER */
            break;
        }
    }

    static bool printed_once = false;
    if ( !printed_once )
    {
        printed_once = true;

#if !HAVE( EXTERNAL_OPARI2 )
        if ( build_check )
        {
            extern std::string path_to_src;
            std::cout << "-I" + path_to_src + "vendor/opari2/include ";
            std::cout.flush();
            return;
        }
#endif

        std::string opari_config = "`" OPARI_CONFIG " --cflags";
        if ( with_cflags )
        {
            opari_config += "=" + compiler_type;
            if ( language == SCOREP_CONFIG_LANGUAGE_FORTRAN )
            {
                opari_config += " --fortran";
            }
        }
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
            std::cerr << "[Score-P] ERROR: Execution failed: " << opari_config.c_str() << std::endl;
            exit( EXIT_FAILURE );
        }
        std::cout << " ";
        std::cout.flush();
    }
}

/* **************************************************************************************
 * Memory adapter
 * *************************************************************************************/
SCOREP_Config_MemoryAdapter::SCOREP_Config_MemoryAdapter()
    : SCOREP_Config_Adapter( "memory", "scorep_adapter_memory", true )
{
}

void
SCOREP_Config_MemoryAdapter::printHelp( void )
{
    std::cout << "   --" << m_name << "|--no" << m_name << "\n"
              << "              Specifies whether memory usage recording is used.\n"
              << "              On default memory instrumentation is enabled.\n"
              << "              The following memory interfaces may be recorded, if present:\n"
              << "               ISO C:\n"
              << "                malloc,realloc,calloc,free,memalign,posix_memalign,valloc,\n"
              << "                aligned_alloc\n"
              << "               ISO C++:\n"
              << "                new,new[],delete,delete[]\n"
              << "               Intel KNL MCDRAM API:\n"
              << "                hbw_malloc,hbw_realloc,hbw_calloc,hbw_free,hbw_posix_memalign,\n"
              << "                hbw_posix_memalign_psize\n";
}

bool
SCOREP_Config_MemoryAdapter::checkArgument( const std::string& arg )
{
    if ( arg == "--" + m_name )
    {
        m_is_enabled = true;
        return true;
    }

    if ( arg == "--no" + m_name )
    {
        m_is_enabled = false;
        return true;
    }
    return false;
}

/* **************************************************************************************
 * I/O wrapping adapter
 * *************************************************************************************/
SCOREP_Config_IoAdapter::SCOREP_Config_IoAdapter()
    : SCOREP_Config_Adapter( "io", "", true )
{
#if HAVE_BACKEND( POSIX_IO_SUPPORT )
    m_supported_ios.emplace( "posix",
                             SCOREP_Config_SupportedIo( "Posix", "posix_io" ) );
#endif
}

void
SCOREP_Config_IoAdapter::printHelp( void )
{
    std::cout << "   --io=<paradigm,...>|--noio\n"
              << "              Specifies whether I/O instrumentation is used.\n"
              << "              On default I/O instrumentation is enabled.\n"
              << "              The following I/O paradigms may be recorded, if present:\n"
              << "               none\n";
    for ( const auto& pair : m_supported_ios )
    {
        std::cout << "               " << pair.first << "\n";
    }
}

bool
SCOREP_Config_IoAdapter::checkArgument( const std::string& arg )
{
    if ( arg.substr( 0, 5 ) == ( "--io=" ) )
    {
        for ( const auto& ioParadigm : string_to_deque( arg.substr( 5 ), "," ) )
        {
            if ( ioParadigm == "none" )
            {
                m_selected_ios.clear();
                continue;
            }

            if ( !m_supported_ios.count( ioParadigm ) )
            {
                std::cerr << "ERROR: I/O paradigm " << ioParadigm << " not supported by this Score-P installation." << std::endl;
                exit( EXIT_FAILURE );
            }
            m_selected_ios.insert( ioParadigm );
        }

        m_is_enabled = m_selected_ios.size() != 0;

        return true;
    }
    if ( arg == "--noio" )
    {
        m_is_enabled = false;
        return true;
    }
    return false;
}

void
SCOREP_Config_IoAdapter::addLibs( std::deque<std::string>&           libs,
                                  SCOREP_Config_LibraryDependencies& deps )
{
    if ( !m_is_enabled )
    {
        return;
    }

    for ( const auto& ioParadigm : m_selected_ios )
    {
        const auto& io = m_supported_ios[ ioParadigm ];
        libs.push_back( "libscorep_adapter_" + io.m_lib_name + "_event" );
        deps.addDependency( libs.back(), "libscorep_measurement" );
        deps.addDependency( "libscorep_measurement", "libscorep_adapter_" + io.m_lib_name + "_mgmt" );
    }
}

void
SCOREP_Config_IoAdapter::addLdFlags( std::string& ldflags,
                                     bool         buildCheck,
                                     bool         nvcc )
{
    if ( !m_is_enabled )
    {
        return;
    }
}

void
SCOREP_Config_IoAdapter::appendInitStructName( std::deque<std::string>& initStructs )
{
    if ( !m_is_enabled )
    {
        return;
    }

    initStructs.push_back( "SCOREP_Subsystem_IoManagement" );

    for ( const auto& ioParadigm : m_selected_ios )
    {
        const auto& io = m_supported_ios[ ioParadigm ];
        initStructs.push_back( "SCOREP_Subsystem_" + io.m_subsystem_name + "IoAdapter" );
    }
}
