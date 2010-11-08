/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
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
 * @file       test/measurement/config/bool.c
 * @maintainer Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @status alpha
 *
 */


#include <config.h>


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


#include <CuTest.h>


#include <SCOREP_Config.h>


static void
test_bool_default_true( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_true",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "true",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"true\"", bool_variable == true );
}


static void
test_bool_default_yes( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_yes",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "yes",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"yes\"", bool_variable == true );
}


static void
test_bool_default_on( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_on",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "on",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"on\"", bool_variable == true );
}


static void
test_bool_default_1( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_1",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "1",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"1\"", bool_variable == true );
}


static void
test_bool_default_42( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_42",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "42",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"42\"", bool_variable == true );
}


static void
test_bool_default_false( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_false",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "false",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"false\"", bool_variable == false );
}


static void
test_bool_default_no( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_no",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "no",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"no\"", bool_variable == false );
}


static void
test_bool_default_off( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_off",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "off",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"off\"", bool_variable == false );
}


static void
test_bool_default_0( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_0",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "0",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"0\"", bool_variable == false );
}


static void
test_bool_default_garbage( CuTest* tc )
{
    bool                  bool_variable;
    SCOREP_ConfigVariable config_variables[] = {
        {
            "bool_default_garbage",
            SCOREP_CONFIG_TYPE_BOOL,
            &bool_variable,
            NULL,
            "garbage",
            "",
            ""
        },
        SCOREP_CONFIG_TERMINATOR
    };

    SCOREP_ConfigRegister( NULL, config_variables );

    CuAssert( tc, "default \"garbage\"", bool_variable == false );
}


int
main()
{
    CuString* output = CuStringNew();
    CuSuite*  suite  = CuSuiteNew( "default values for config type bool" );

    SUITE_ADD_TEST_NAME( suite, "bool with default \"true\"",
                         test_bool_default_true );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"yes\"",
                         test_bool_default_yes );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"on\"",
                         test_bool_default_on );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"1\"",
                         test_bool_default_1 );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"42\"",
                         test_bool_default_42 );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"false\"",
                         test_bool_default_false );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"no\"",
                         test_bool_default_no );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"off\"",
                         test_bool_default_off );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"0\"",
                         test_bool_default_0 );
    SUITE_ADD_TEST_NAME( suite, "bool with default \"garbage\"",
                         test_bool_default_garbage );

    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    printf( "%s", output->buffer );

    int failCount = suite->failCount;
    CuSuiteFree( suite );
    CuStringFree( output );

    return failCount;
}
