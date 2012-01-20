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
 * @maintainer Daniel Lorenz  <d.lorenz@fz-juelich.de>
 * @status alpha
 * The instrumentation tool of Score-P: scorep.
 * .
 */
#include <config.h>
#include <iostream>
#include <string>

#include <scorep_utility/SCOREP_Utils.h>
#include "SCOREP_Instrumenter.hpp"
#include "SCOREP_Measurement.hpp"

/**
   Contains the name of the tool for help output
 */
const std::string toolname = "scorep";

/**
    Prints a short usage message.
 */
void
print_short_usage()
{
    std::cout << "\nThis is the SCOREP instrumentation tool. The usage is:\n"
              << toolname << " <options> <orginal command>\n\n"
              << "To print out more detailed help information on available parameters, "
              << "type\n"
              << toolname << " --help\n"
              << std::endl;
}

/**
   Prints the long help text.
 */
void
print_help()
{
    std::cout << "\nThis is the SCOREP instrumentation tool. The usage is:\n"
              << toolname << " <options> <orginal command>\n\n"
              << "Common options are:\n"
              << "  --help, -h      Show help output. Does not execute any other command.\n"
              << "  --config=<file> Specifies file for the instrumentation configuration.\n"
              << "  --verbosity=<value> Specifies the verbosity level. The following\n"
              << "                  levels are available:\n"
              << "                  0 = No output (default)\n"
              << "                  1 = Executed commands are displayed\n"
              << "                  2 = Detailed information is displayed\n"
              << "  --dry-run       Only displays the executed commands. It does not\n"
              << "                  execute any command.\n"
              << "  --keep-files    Do not delete temporarily created files after successfull\n"
              << "                  instrumentation. By default, temporary files are deleted\n"
              << "                  if no error occures during instrumentation.\n"
              << "  --version       Prints the Score-P version and exits.\n"
              << "  --compiler      Enables compiler instrumentation. Is enabled by default.\n"
              << "  --nocompiler    Disables compiler instrumentation.\n"
              << "  --mpi           Enables mpi wrapper. They are enabled by default if it is an\n"
              << "                  mpi program.\n"
              << "  --nompi         Disables mpi wrappers. They are disabled by default if\n"
              << "                  it is no mpi program.\n"
              << "  --opari         Enables Opari instrumentation. Is enabled by default\n"
              << "                  if it is an OpenMP program.\n"
              << "  --noopari       Disables Opari instrumentation. Is disabled by default\n"
              << "                  if it is no OpenMP program.\n"
              << "  --user          Enables manual user instrumentation.\n"
              << "  --nouser        Disables manual user instrumentation. Is disabled by default.\n"
#ifdef HAVE_PDT
              << "  --pdt           Enables source code instrumentation with PDT using\n"
              << "                  the TAU instrumentor.\n"
              << "                  It will automatically enable the user instrumentation\n"
              << "                  and disable compiler instrumentation.\n"
              << "  --nopdt         Disables the source code instrumentation with PDT.\n"
              << "                  It is disabled by default.\n"
#endif
#if HAVE( COBI )
    << "  --cobi          Enables binary instrumentation with Cobi.\n"
    << "                  Disables compiler instrumentation.\n"
    << "  --nocobi        Disables the binary instrumentation with Cobi.\n"
    << "                  It is disabled by default.\n"
#endif
    << "  --openmp_support Enables OpenMP support. Needed if the instrumentation\n"
    << "                  does not correctly identify your application as OpenMP\n"
    << "                  program.\n"
    << "  --noopenmp_support Disables OpenMP support.\n"
    << std::endl;
}

/**
   Main routine of the scorep instrumentation tool.
   @param argc Number of arguments.
   @param argv List of arguments
   @returns If an error occurs, -1 is returned, if help was called, 0 is
            returned. Else it returns the return value from the user command.
 */
int
main( int   argc,
      char* argv[] )
{
    if ( argc > 1 )
    {
        SCOREP_Instrumenter app;
        app.ParseCmdLine( argc, argv );
        return app.Run();
    }
    else
    {
        print_short_usage();
    }
    return EXIT_FAILURE;
}
