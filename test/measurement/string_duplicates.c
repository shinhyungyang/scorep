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
 * @file       test/measurement/string_duplicates.c
 *
 *
 */

#include <config.h>

#include <stdlib.h>
#include <stdio.h>

#include <CuTest.h>

#include <SCOREP_Memory.h>

#include <definitions/SCOREP_Definitions.h>

static void
test_1( CuTest* tc )
{
    SCOREP_Memory_Initialize( 4 * 4096, 4096 );
    SCOREP_Definitions_Initialize();

    SCOREP_StringHandle handle1 = SCOREP_Definitions_NewString( "foo" );
    SCOREP_StringHandle handle2 = SCOREP_Definitions_NewString( "foo" );
    SCOREP_StringHandle handle3 = SCOREP_Definitions_NewString( "foo" );

    CuAssert( tc, "1st and 2nd call should return the same handle", handle1 == handle2 );
    CuAssert( tc, "2nd and 3rd call should return the same handle", handle2 == handle3 );

    SCOREP_Definitions_Finalize();
    SCOREP_Memory_Finalize();
}

static void
test_2( CuTest* tc )
{
    SCOREP_Memory_Initialize( 4 * 4096, 4096 );
    SCOREP_Definitions_Initialize();

    SCOREP_StringHandle foo_handle1 = SCOREP_Definitions_NewString( "foo" );
    SCOREP_StringHandle bar_handle1 = SCOREP_Definitions_NewString( "bar" );
    SCOREP_StringHandle foo_handle2 = SCOREP_Definitions_NewString( "foo" );
    SCOREP_StringHandle bar_handle2 = SCOREP_Definitions_NewString( "bar" );
    SCOREP_StringHandle foo_handle3 = SCOREP_Definitions_NewString( "foo" );
    SCOREP_StringHandle bar_handle3 = SCOREP_Definitions_NewString( "bar" );

    CuAssert( tc, "1st and 2nd call should return the same handle", foo_handle1 == foo_handle2 );
    CuAssert( tc, "2nd and 3rd call should return the same handle", foo_handle2 == foo_handle3 );

    CuAssert( tc, "1st and 2nd call should return the same handle", bar_handle1 == bar_handle2 );
    CuAssert( tc, "2nd and 3rd call should return the same handle", bar_handle2 == bar_handle3 );

    SCOREP_Definitions_Finalize();
    SCOREP_Memory_Finalize();
}

int
main()
{
    CuUseColors();
    CuString* output = CuStringNew();
    CuSuite*  suite  = CuSuiteNew( "no string duplications" );

    SUITE_ADD_TEST_NAME( suite, test_1,
                         "three times \"foo\" equals one" );
    SUITE_ADD_TEST_NAME( suite, test_2,
                         "three times \"foo\" and \"bar\" equals two" );

    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );

    int failCount = suite->failCount;
    if ( failCount )
    {
        printf( "%s", output->buffer );
    }

    CuSuiteFree( suite );
    CuStringFree( output );

    return failCount ? EXIT_FAILURE : EXIT_SUCCESS;
}
