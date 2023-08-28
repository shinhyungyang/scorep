/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013, 2015-2017, 2019, 2022, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#include <config.h>

#include "scorep_library_wrapper_generator.hpp"

#include <UTILS_Error.h>

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <scorep_tools_utils.hpp>

using namespace std;

SCOREP_Libwrap_Generator::SCOREP_Libwrap_Generator( const generator_configuration& config )
    : m_config( config )
    , m_toplevel_namespace( 0 )
{
    if ( m_config.include_statement.size() )
    {
        m_includes.push_back( "#include \"" + m_config.include_statement + "\"" );
    }
    else
    {
        /* Add header file to list of includes, used in the generated wrapper source file */
        m_includes.push_back( "#include \"" + m_config.header_file + "\"" );
    }
}

int
SCOREP_Libwrap_Generator::generate_source_code_files()
{
    // Parse the header file
    SCOREP_Libwrap_Parser parser( m_config.verbose,
                                  m_config.header_file,
                                  m_config.language,
                                  m_config.language_standard,
                                  m_config.variadic_is_void,
                                  m_config.ellipsis_mapping,
                                  m_config.filter );

    if ( !parser.ParseHeader() )
    {
        cerr << "ERROR: Parsing header file " << m_config.header_file
             << " failed." << endl;
        return EXIT_FAILURE;
    }

    m_toplevel_namespace  = parser.GetToplevelNamespace();
    m_all_wrapped_symbols = parser.GetAllWrappedSymbols();

    // Write file with forward declarations
    set_outputfile_name( m_config.function_list_file_name );
    output( OUTPUT_FUNCTION_LIST );

    // Write file with symbols
    set_outputfile_name( m_config.symbols_file_name );
    output( OUTPUT_SYMBOLS );

    // Write wrapper skeleton file
    set_outputfile_name( m_config.wrap_file_name );
    output( m_config.create_internal_wrapper_code_file
            ? OUTPUT_INTERNAL_ADAPTER_CODE
            : OUTPUT_EXTERNAL_ADAPTER_CODE );

    return EXIT_SUCCESS;
}

void
SCOREP_Libwrap_Generator::set_outputfile_name( const string& filename )
{
    m_output_file = filename;
}

const string&
SCOREP_Libwrap_Generator::get_outputfile_name() const
{
    return m_output_file;
}

void
SCOREP_Libwrap_Generator::clear_outputfile()
{
    m_output_file.clear();
}

bool
SCOREP_Libwrap_Generator::noSymbolsWrapped() const
{
    return m_toplevel_namespace->empty;
}

#define DEFINE_LIBWRAP_PROCESS_FUNC ( !generator.m_config.create_internal_wrapper_code_file \
                                      ? "#define SCOREP_LIBWRAP_PROCESS_FUNC( rettype, func, args, prettyname, file, line, ns ) \\\n" \
                                      : "#define SCOREP_LIBWRAP_PROCESS_FUNC( TYPE, rettype, func, args, prettyname, ns ) \\\n" )

class macro_writer_adapter
    : public SCOREP_Libwrap_IterateNamespaceCb
{
public:
    macro_writer_adapter( SCOREP_Libwrap_Generator& generator,
                          ostream&                  out )
        : generator( generator )
        , out( out )
    {
        if ( generator.m_config.create_internal_wrapper_code_file )
        {
            out << "#include <config.h>\n"
                << "\n";
        }

        out << generator.iterate_includes() << endl;

        out << "#define SCOREP_LIBWRAP_ORIGINAL_TYPE( func ) \\\n"
            << "    libwrap_" << generator.m_config.wrapper_name << "_original_type__ ## func ## _t\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_WRAPPER( func ) \\\n"
            << "    libwrap_" << generator.m_config.wrapper_name << "_wrapper__ ## func\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_REGION_DESCRIPTOR( func ) \\\n"
            << "    libwrap_" << generator.m_config.wrapper_name << "_region_descr__ ## func\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ) \\\n"
            << "    SCOREP_LIBWRAP_REGION_DESCRIPTOR( func ).original_handle\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_REGION_HANDLE( func ) \\\n"
            << "    SCOREP_LIBWRAP_REGION_DESCRIPTOR( func ).region_handle\n"
            << "\n";

        if ( !generator.m_config.create_internal_wrapper_code_file )
        {
            out << "#include <scorep/SCOREP_Libwrap_Macros.h>\n"
                << "\n"
                << "#define SCOREP_LIBWRAP_API( api ) \\\n"
                << "    libwrap_plugin_api->api\n"
                << "\n"
                << "#include <scorep/SCOREP_Libwrap_Plugins.h>\n"
                << "\n"
                << "static const SCOREP_LibwrapAPI* libwrap_plugin_api;\n"
                << "\n";
        }
        else
        {
            out << "#include <SCOREP_InMeasurement.h>\n"
                << "#include <SCOREP_Events.h>\n"
                << "\n"
                << "#include <SCOREP_Libwrap_Internal.h>\n"
                << "\n";
        }

        out << "#ifdef __cplusplus\n"
            << "extern \"C\"\n"
            << "{\n"
            << "#endif\n"
            << "\n"
            << "/* wrapper declarations */\n"
            << "\n"
            << DEFINE_LIBWRAP_PROCESS_FUNC
            << "    SCOREP_LIBWRAP_DECLARE_ORIGINAL_TYPE( rettype, func, args ); \\\n"
            << "    SCOREP_LIBWRAP_DECLARE_WRAPPER( func );\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
            << "#include \"" << remove_path( generator.m_config.function_list_file_name ) << "\"\n"
            << "\n"
            << "/* region descriptors */\n"
            << "\n"
            << DEFINE_LIBWRAP_PROCESS_FUNC
            << "    static struct \\\n"
            << "    { \\\n"
            << "        SCOREP_Libwrap_OriginalHandle original_handle; \\\n"
            << "        SCOREP_RegionHandle           region_handle; \\\n"
            << "    } SCOREP_LIBWRAP_REGION_DESCRIPTOR( func );\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
            << "#include \"" << remove_path( generator.m_config.function_list_file_name ) << "\"\n"
            << "\n"
            << DEFINE_LIBWRAP_PROCESS_FUNC
            << "    static void libwrap_" << generator.m_config.wrapper_name << "_init__ ## func( SCOREP_LibwrapHandle* handle ) \\\n"
            << "    { \\\n";

        if ( !generator.m_config.create_internal_wrapper_code_file )
        {
            out << "        SCOREP_LIBWRAP_FUNC_INIT( handle, func, prettyname, file, line ); \\\n";
        }
        else
        {
            out << "        SCOREP_Libwrap_EnableWrapper( handle, \\\n"
                << "                                      prettyname, \\\n"
                << "                                      #func, \\\n"
                << "                                      " << make_string_literal( generator.m_config.display_name ) << ", \\\n"
                << "                                      SCOREP_INVALID_LINE_NO, \\\n"
                << "                                      SCOREP_PARADIGM_LIBWRAP, \\\n"
                << "                                      SCOREP_REGION_ ## TYPE, \\\n"
                << "                                      ( void* )SCOREP_LIBWRAP_WRAPPER( func ), \\\n"
                << "                                      &SCOREP_LIBWRAP_ORIGINAL_HANDLE( func ), \\\n"
                << "                                      &SCOREP_LIBWRAP_REGION_HANDLE( func ) ); \\\n";
        }

        out << "    }\n"
            << "\n"
            << "#define SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
            << "#include \"" << remove_path( generator.m_config.function_list_file_name ) << "\"\n"
            << "\n"
            << "/* region init functions */\n"
            << "\n"
            << "static void\n"
            << "libwrap_" << generator.m_config.wrapper_name << "_init( SCOREP_LibwrapHandle* handle )\n"
            << "{\n"
            << DEFINE_LIBWRAP_PROCESS_FUNC
            << "    ns libwrap_" << generator.m_config.wrapper_name << "_init__ ## func( handle );\n"
            << "\n"
            << "#include \"" << remove_path( generator.m_config.function_list_file_name ) << "\"\n"
            << "}\n"
            << "\n" << endl;

        out << "static SCOREP_LibwrapHandle*          libwrap_" << generator.m_config.wrapper_name << "_handle;\n"
            << "static const SCOREP_LibwrapAttributes libwrap_" << generator.m_config.wrapper_name << "_attributes =\n"
            << "{\n"
            << "    SCOREP_LIBWRAP_VERSION,\n"
            << "    " << make_string_literal( generator.m_config.wrapper_name ) << ",\n"
            << "    " << make_string_literal( generator.m_config.display_name ) << ",\n"
            << "    libwrap_" << generator.m_config.wrapper_name << "_init,\n"
            << "};\n"
            << "\n";

        if ( !generator.m_config.create_internal_wrapper_code_file )
        {
            out << "void\n"
                << "scorep_libwrap_plugin( const SCOREP_LibwrapAPI* const libwrapAPI,\n"
                << "                       size_t                         libwrapAPISize )\n"
                << "{\n"
                << "    if ( libwrapAPISize < sizeof( *libwrap_plugin_api ) )\n"
                << "    {\n"
                << "        return;\n"
                << "    }\n"
                << "    libwrap_plugin_api = libwrapAPI;\n"
                << "\n"
                << "    SCOREP_LIBWRAP_INIT( libwrap_" << generator.m_config.wrapper_name << "_handle,\n"
                << "                         libwrap_" << generator.m_config.wrapper_name << "_attributes );\n"
                << "}\n"
                << "\n";
        }
        else
        {
            out << "void\n"
                << "scorep_" << generator.m_config.wrapper_name << "_libwrap_init( void )\n"
                << "{\n"
                << "    SCOREP_Libwrap_Create( &libwrap_" << generator.m_config.wrapper_name << "_handle,\n"
                << "                           &libwrap_" << generator.m_config.wrapper_name << "_attributes );\n"
                << "}\n"
                << "\n";
        }
    }

    ~macro_writer_adapter()
    {
        out << "#ifdef __cplusplus\n"
            << "}\n"
            << "#endif" << endl;
    }

    SCOREP_Libwrap_Generator& generator;
    ostream&                  out;

    void
    operator()( const macro_information& decl ) override
    {
        if ( !generator.m_config.create_internal_wrapper_code_file )
        {
            generator.write_wrapper_code( decl, out );
        }
        else
        {
            generator.write_internal_wrapper_code( decl, out );
        }
    }

    void
    operator()( const string& ns,
                bool          openClose ) override
    {
        if ( ns.empty() )
        {
            return;
        }
        if ( openClose )
        {
            out << "namespace " << ns << "\n{\n" << endl;
        }
        else
        {
            out << "} // namespace " << ns << "\n" << endl;
        }
    }
};

class macro_writer_function_list
    : public SCOREP_Libwrap_IterateNamespaceCb
{
public:
    macro_writer_function_list( SCOREP_Libwrap_Generator& generator,
                                ostream&                  out )
        : generator( generator )
        , out( out )
    {
        out << "#ifndef SCOREP_LIBWRAP_PROCESS_FUNC\n"
            << "#error SCOREP_LIBWRAP_PROCESS_FUNC not defined\n"
            << "#endif\n" << endl;
    }

    ~macro_writer_function_list()
    {
        out << "#undef SCOREP_LIBWRAP_PROCESS_FUNC\n"
            << "#ifdef SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
            << "#undef SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
            << "#endif" << endl;
    }

    SCOREP_Libwrap_Generator& generator;
    ostream&                  out;
    string                    current_namespace;

    void
    operator()( const macro_information& decl ) override
    {
        generator.write_function_process_macro( decl, current_namespace, out );
    }

    void
    operator()( const string& ns,
                bool          openClose ) override
    {
        if ( ns.empty() )
        {
            return;
        }
        if ( openClose )
        {
            out << "#ifdef SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
                << "namespace " << ns << "\n{\n"
                << "#endif\n" << endl;
            current_namespace += ns + "::";
        }
        else
        {
            out << "#ifdef SCOREP_LIBWRAP_PROCESS_FUNC_WITH_NAMESPACE\n"
                << "} // namespace " << ns << "\n"
                << "#endif\n" << endl;
            current_namespace.resize( current_namespace.size() - ns.size() - 2 );
        }
    }
};

class macro_writer_symbols
    : public SCOREP_Libwrap_IterateNamespaceCb
{
public:
    macro_writer_symbols( SCOREP_Libwrap_Generator& generator,
                          ostream&                  out )
        : generator( generator )
        , out( out )
    {
    }

    SCOREP_Libwrap_Generator& generator;
    ostream&                  out;

    void
    operator()( const macro_information& decl ) override
    {
        out << decl.symbolname << "\n";
    }
};

void
SCOREP_Libwrap_Generator::output( output_mode mode )
{
    streambuf* buf;
    ofstream   fout;

    if ( !fout.is_open() && m_output_file.empty() )
    {
        buf = cout.rdbuf();
    }
    else if ( !fout.is_open() )
    {
        fout.open( m_output_file.c_str(), ios::out );
        buf = fout.rdbuf();
    }

    ostream out( buf );

    SCOREP_Libwrap_IterateNamespaceCb* cb;
    switch ( mode )
    {
        case OUTPUT_INTERNAL_ADAPTER_CODE:
        case OUTPUT_EXTERNAL_ADAPTER_CODE:
            cb = new macro_writer_adapter( *this, out );
            break;
        case OUTPUT_FUNCTION_LIST:
            cb = new macro_writer_function_list( *this, out );
            break;
        case OUTPUT_SYMBOLS:
            cb = new macro_writer_symbols( *this, out );
            break;
    }

    SCOREP_Libwrap_IterateNamespace( *m_toplevel_namespace, *cb );
    delete cb;

    if ( fout.is_open() )
    {
        fout.close();
    }
}

void
SCOREP_Libwrap_Generator::add_include( const string& includeFile,
                                       include_mode  includeMode )
{
    string include = "#include ";
    ( includeMode == INCLUDE_GLOBAL_FILE ) ? include.append( "<" ) : include.append( "\"" );
    include.append( includeFile );
    ( includeMode == INCLUDE_GLOBAL_FILE ) ? include.append( ">" ) : include.append( "\"" );

    m_includes.push_back( include );
}

const string
SCOREP_Libwrap_Generator::argdecls_iterate( const macro_information& data ) const
{
    if ( data.argdecls.size() == 0 )
    {
        return "void";
    }

    string s;
    string sep = "";
    for ( int i = 0; i < data.argdecls.size(); i++ )
    {
        s.append( sep );
        sep = ", ";

        s.append( data.argdecls.at( i ) );
    }
    return s;
}

const string
SCOREP_Libwrap_Generator::argnames_iterate( const macro_information& data ) const
{
    string s;
    string sep = "";
    for ( int i = 0; i < data.argnames.size(); i++ )
    {
        s.append( sep );
        sep = ", ";

        s.append( data.argnames.at( i ) );
    }
    return s;
}

const string
SCOREP_Libwrap_Generator::iterate_includes() const
{
    string                         s;
    vector<string>::const_iterator iter;
    iter = m_includes.begin();
    while ( iter != m_includes.end() )
    {
        s.append( *iter );
        s.append( "\n" );
        ++iter;
    }
    return s;
}

void
SCOREP_Libwrap_Generator::write_wrapper_code( const macro_information& data,
                                              ostream&                 out ) const
{
    out << data.returntype << "\n"
        << "SCOREP_LIBWRAP_WRAPPER( " << data.symbolname << " )( " << argdecls_iterate( data ) << " )\n"
        << "{\n";

    if ( m_config.language == "c" && data.hasreturn )
    {
        out << "    " << data.returntype << " return_value;\n";
    }
    if ( data.va_list_symbol.size() )
    {
        out << "    va_list scorep_libwrap_var_vl;\n";
    }

    /* SCOREP_LIBWRAP_ENTER_MEASUREMENT declares variables */
    out << "    SCOREP_LIBWRAP_ENTER_MEASUREMENT();\n";

    out << "    SCOREP_LIBWRAP_FUNC_ENTER( " << data.symbolname << " );\n";

    string call_symbol = data.symbolname;
    if ( data.va_list_symbol.size() )
    {
        call_symbol = data.va_list_symbol;
        /*
         * va_start needs the argument prior to the elipsis, as we already added the 'va_list' argument
         * this is the second to last argument now in 'argnames'.
         */
        out << "    va_start( scorep_libwrap_var_vl, " << data.argnames[ data.argnames.size() - 2 ] << " );\n";

        // We need a decl for the va_list function, if we are not wrapping it
        if ( m_all_wrapped_symbols.count( call_symbol ) == 0 )
        {
            // dont use argdecls_iterate, as it is the ellipsis one
            out << "    " << data.returntype << " " << call_symbol << "( ";
            string sep = "";
            for ( int i = 0; i < data.argdecls.size() - 1; i++ )
            {
                out << sep << data.argdecls.at( i );
                sep = ", ";
            }
            out << sep << "va_list );\n";
        }
    }

    out << "    SCOREP_LIBWRAP_ENTER_WRAPPED_REGION();\n"
        << "    ";
    if ( m_config.language == "c++" && data.hasreturn )
    {
        out << data.returntype << " ";
    }
    if ( data.hasreturn )
    {
        out << "return_value = ";
    }
    if ( !data.va_list_symbol.size() || m_all_wrapped_symbols.count( call_symbol ) != 0 )
    {
        out << "SCOREP_LIBWRAP_ORIGINAL( " << call_symbol << " )( " << argnames_iterate( data ) << " );\n";
    }
    else
    {
        // the target va_list will not be wrapped, thus directly call the symbol
        out << call_symbol << "( " << argnames_iterate( data ) << " );\n";
    }
    out << "    SCOREP_LIBWRAP_EXIT_WRAPPED_REGION();\n";

    if ( data.va_list_symbol.size() )
    {
        out << "    va_end( scorep_libwrap_var_vl );\n";
    }

    out << "    SCOREP_LIBWRAP_FUNC_EXIT( " << data.symbolname << " );\n";

    out << "    SCOREP_LIBWRAP_EXIT_MEASUREMENT();\n";

    if ( data.hasreturn )
    {
        out << "    return return_value;\n";
    }
    out << "}\n" << endl;
}

void
SCOREP_Libwrap_Generator::write_internal_wrapper_code( const macro_information& data,
                                                       ostream&                 out ) const
{
    string return_statement;
    string prep;

    out << endl
        << data.returntype << "\n"
        << "SCOREP_LIBWRAP_WRAPPER( " << data.symbolname << " )( " << argdecls_iterate( data ) << " )\n"
        << "{\n"
        << "    bool trigger = SCOREP_IN_MEASUREMENT_TEST_AND_INCREMENT();\n"
        << "    if ( !trigger || !SCOREP_IS_MEASUREMENT_PHASE( WITHIN ) )\n"
        << "    {\n"
        << "        SCOREP_IN_MEASUREMENT_DECREMENT();\n"
        << "        return " << "SCOREP_LIBWRAP_ORIGINAL( " << data.symbolname << " )( " << argnames_iterate( data ) << " );\n"
        << "    }\n"
        << "\n"
        << "    SCOREP_EnterWrappedRegion( SCOREP_LIBWRAP_REGION_HANDLE( " << data.symbolname << " ) );\n"
        << "\n"
        << "    SCOREP_ENTER_WRAPPED_REGION();\n";

    out << "    ";

    if ( data.hasreturn )
    {
        out << data.returntype << " return_value = ";
    }

    out << "SCOREP_LIBWRAP_ORIGINAL( " << data.symbolname << " )( " << argnames_iterate( data ) << " );\n"
        << "    SCOREP_EXIT_WRAPPED_REGION();\n"
        << "\n"
        << "    SCOREP_ExitRegion( SCOREP_LIBWRAP_REGION_HANDLE( " << data.symbolname << " ) );\n"
        << "\n"
        << "    SCOREP_IN_MEASUREMENT_DECREMENT();\n";

    if ( data.hasreturn )
    {
        out << "    return return_value;\n";
    }
    out << "}\n" << endl;
}

void
SCOREP_Libwrap_Generator::write_function_process_macro( const macro_information& data,
                                                        const string&            enclosingNamespace,
                                                        ostream&                 out ) const
{
    out << "#ifdef " << data.symbolname << "\n"
        << "#undef " << data.symbolname << "\n"
        << "#endif // " << data.symbolname << "\n";
    if ( !m_config.create_internal_wrapper_code_file )
    {
        out << "SCOREP_LIBWRAP_PROCESS_FUNC( ( " << data.returntype << " ),\n"
            << "                             " << data.symbolname << ",\n"
            << "                             ( " << argdecls_iterate( data ) << " ),\n"
            << "                             \"" << data.functionname << "\",\n"
            << "                             \"" << canonicalize_path( data.filename ) << "\",\n"
            << "                             " << data.linenr << ",\n"
            << "                             " << enclosingNamespace << " )\n" << endl;
    }
    else
    {
        out << "SCOREP_LIBWRAP_PROCESS_FUNC( WRAPPER,\n"
            << "                             ( " << data.returntype << " ),\n"
            << "                             " << data.symbolname << ",\n"
            << "                             ( " << argdecls_iterate( data ) << " ),\n"
            << "                             \"" << data.functionname << "\",\n"
            << "                             " << enclosingNamespace << " )\n" << endl;
    }
}
