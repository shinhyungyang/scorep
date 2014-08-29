/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012, 2014,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2012,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 *
 * The info tool of Score-P: scorep-info.
 *
 */

#include <config.h>

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

#include <UTILS_Error.h>

#include <SCOREP_Platform.h>
#include <SCOREP_Config.h>
#include <scorep_environment.h>
#include <scorep_config_tool_backend.h>

/**
   Contains the name of the tool for help output
 */
static const std::string toolname = "scorep-info";

/**
    Prints a short usage message.
 */
static void
print_short_usage( std::ostream& out )
{
    out << "Usage: " << toolname << " <info command> <command options>" << std::endl;
    out << "       " << toolname << " --help" << std::endl;
    out << "This is the " << PACKAGE_NAME << " info tool." << std::endl;
}

/**
   Prints the long help text.
 */
static void
print_help( void )
{
    print_short_usage( std::cout );
    std::cout << std::endl;
    std::cout << "Available info commands:" << std::endl;
    std::cout << std::endl;
    std::cout << "  config-vars:" << std::endl;
    std::cout << "    Shows the list of all measurement config variables with a short description." << std::endl;
    std::cout << std::endl;
    std::cout << "    Info command options:" << std::endl;
    std::cout << "      --full        Displays a detailed description for each config variable." << std::endl;
    std::cout << "      --values      Displays the current values for each config variable." << std::endl;
    std::cout << "                    Warning: These values may be wrong, please consult the\n";
    std::cout << "                             manual of the batch system how to pass the values\n";
    std::cout << "                             to the measurement job." << std::endl;
    std::cout << std::endl;
    std::cout << "  config-summary:" << std::endl;
    std::cout << "    Shows the configure summary of the Score-P package." << std::endl;
    std::cout << std::endl;
    std::cout << "  open-issues:" << std::endl;
    std::cout << "    Shows open and known issues of the Score-P package." << std::endl;
    std::cout << std::endl;
}

int
main( int   argc,
      char* argv[] )
{
    if ( argc > 1 )
    {
        std::string info_command( argv[ 1 ] );
        if ( info_command == "--help" || info_command == "-h" )
        {
            print_help();
            return EXIT_SUCCESS;
        }

        std::vector< char* > args( argv + 2,  argv + argc );

        if ( info_command == "config-vars" )
        {
            if ( args.size() > 1 )
            {
                std::cerr << "Invalid number of options for info command "
                          << info_command << std::endl;
                print_short_usage( std::cerr );
                return EXIT_FAILURE;
            }

            SCOREP_ConfigInit();

            std::string mode( args.size() == 1 ? args[ 0 ] : "" );
            bool        values = false;
            bool        full   = false;
            bool        html   = false;
            if ( mode == "--values" )
            {
                // @todo print warning again
                values = true;
            }
            else if ( mode == "--full" )
            {
                full = true;
            }
            else if ( mode == "--doxygen" )
            {
                full = true;
                html = true;
                SCOREP_ConfigForceConditionalRegister();
            }
            else if ( mode != "" )
            {
                std::cerr << "Invalid option for info command "
                          << info_command << ": " << mode << std::endl;
                print_short_usage( std::cerr );
                SCOREP_ConfigFini();
                return EXIT_FAILURE;
            }

            SCOREP_RegisterAllConfigVariables();

            if ( values )
            {
                SCOREP_ConfigApplyEnv();
                SCOREP_ConfigDump( stdout );
            }
            else
            {
                SCOREP_ConfigHelp( full, html );
            }

            SCOREP_ConfigFini();
            return EXIT_SUCCESS;
        }


        if ( info_command == "config-summary" )
        {
            if ( args.size() != 0 )
            {
                std::cerr << "Invalid number of options for info command "
                          << info_command << std::endl;
                print_short_usage( std::cerr );
                return EXIT_FAILURE;
            }

            std::string summary_command( "cat " SCOREP_DATADIR "/scorep.summary" );
            int         return_value = system( summary_command.c_str() );
            if ( return_value != 0 )
            {
                std::cerr << "Error executing: " << summary_command << std::endl;
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }


        if ( info_command == "open-issues" )
        {
            if ( args.size() != 0 )
            {
                std::cerr << "Invalid number of options for info command "
                          << info_command << std::endl;
                print_short_usage( std::cerr );
                return EXIT_FAILURE;
            }

            std::string command( "cat " SCOREP_DOCDIR "/OPEN_ISSUES" );
            int         return_value = system( command.c_str() );
            if ( return_value != 0 )
            {
                std::cerr << "Error executing: " << command << std::endl;
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }


        std::cerr << "Invalid info command: " << info_command << std::endl;
        print_short_usage( std::cerr );
        return EXIT_FAILURE;
    }
    else
    {
        print_short_usage( std::cout );
    }
    return EXIT_SUCCESS;
}
