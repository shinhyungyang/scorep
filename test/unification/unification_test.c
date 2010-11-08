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
 * @file       unification_test.c
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


#include <config.h>

#include <CuTest.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Definitions.h>
#include <scorep_definitions.h>
#include <scorep_definition_macros.h>

/* *INDENT-OFF* */
extern SCOREP_DefinitionManager* scorep_remote_definition_manager;
extern SCOREP_DefinitionManager  scorep_local_definition_manager;
extern SCOREP_DefinitionManager* scorep_unified_definition_manager;
void init_definition_manager(CuTest* tc, SCOREP_DefinitionManager** manager, bool allocHashTables);
/* *INDENT-ON*  */

//SCOREP_DefinitionManager remote_definition_manager;


// CuTest's assert functions:
// void CuAssert(CuTest* tc, char* message, int condition);
// void CuAssertTrue(CuTest* tc, int condition);
// void CuAssertStrEquals(CuTest* tc, char* expected, char* actual);
// void CuAssertIntEquals(CuTest* tc, int expected, int actual);
// void CuAssertPtrEquals(CuTest* tc, void* expected, void* actual);
// void CuAssertPtrNotNull(CuTest* tc, void* pointer);

//extern SCOREP_Allocator_PageManager* scorep_local_movable_page_manager;
//extern SCOREP_Allocator_PageManager* scorep_remote_movable_page_manager;

void
test_1( CuTest* tc )
{
    init_definition_manager( tc, &scorep_remote_definition_manager, false );
    assert( scorep_remote_definition_manager );
    init_definition_manager( tc, &scorep_unified_definition_manager, true );
    assert( scorep_unified_definition_manager );

    // fill scorep_local_definition_manager
    int                 old_count     = scorep_local_definition_manager.string_definition_counter;
    SCOREP_StringHandle local_handle1 = scorep_string_definition_define( &scorep_local_definition_manager, "main" );
    SCOREP_StringHandle local_handle2 = scorep_string_definition_define( &scorep_local_definition_manager, "foo" );
    SCOREP_StringHandle local_handle3 = scorep_string_definition_define( &scorep_local_definition_manager, "bar" );
    CuAssert( tc, "member unified != SCOREP_MOVABLE_NULL",
              SCOREP_LOCAL_HANDLE_DEREF( local_handle1, String )->unified ==
              SCOREP_MOVABLE_NULL );
    CuAssertIntEquals( tc, 3, scorep_local_definition_manager.string_definition_counter - old_count );

    // fill scorep_remote_definition_manager
    CuAssertIntEquals( tc, 0, scorep_remote_definition_manager->string_definition_counter );
    SCOREP_StringHandle remote_handle1 = scorep_string_definition_define( scorep_remote_definition_manager, "main" );
    SCOREP_StringHandle remote_handle2 = scorep_string_definition_define( scorep_remote_definition_manager, "bar" );
    SCOREP_StringHandle remote_handle3 = scorep_string_definition_define( scorep_remote_definition_manager, "foo" );
    SCOREP_StringHandle remote_handle4 = scorep_string_definition_define( scorep_remote_definition_manager, "baz" );
    SCOREP_StringHandle remote_handle5 = scorep_string_definition_define( scorep_remote_definition_manager, "bar" );
    CuAssertIntEquals( tc, 4, scorep_remote_definition_manager->string_definition_counter );
    CuAssert( tc, "duplicate string definition", remote_handle5 == remote_handle2 );

    // copy local definitions to unified manager
    CuAssertIntEquals( tc, 0, scorep_unified_definition_manager->string_definition_counter );
    SCOREP_String_Definition* local_string1 = SCOREP_LOCAL_HANDLE_DEREF( local_handle1, String );
    SCOREP_CopyStringDefinitionToUnified( local_string1, SCOREP_Memory_GetLocalDefinitionPageManager() );
    CuAssert( tc, "unified handle equals local handle",
              SCOREP_UNIFIED_HANDLE_DEREF( local_string1->unified, String ) !=
              local_string1 );
    CuAssert( tc, "member unified != SCOREP_MOVABLE_NULL",
              SCOREP_UNIFIED_HANDLE_DEREF( local_string1->unified, String )->unified ==
              SCOREP_MOVABLE_NULL );
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( local_handle2, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( local_handle3, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    CuAssertIntEquals( tc, 3, scorep_unified_definition_manager->string_definition_counter );

    // copy remote definitions to unified manager
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( remote_handle1, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( remote_handle2, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( remote_handle3, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( remote_handle4, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    SCOREP_CopyStringDefinitionToUnified( SCOREP_LOCAL_HANDLE_DEREF( remote_handle5, String ), SCOREP_Memory_GetLocalDefinitionPageManager() );
    CuAssertIntEquals( tc, 4, scorep_unified_definition_manager->string_definition_counter );

    scorep_unified_definition_manager = 0; // memory leak
}



//SCOREP_StringHandle
//SCOREP_DefineString( const char* str );
//
//SCOREP_SourceFileHandle
//SCOREP_DefineSourceFile(const char* fileName);
//
//SCOREP_RegionHandle
//SCOREP_DefineRegion
//(
//    const char*             regionName,
//    SCOREP_SourceFileHandle fileHandle,
//    SCOREP_LineNo           beginLine,
//    SCOREP_LineNo           endLine,
//    SCOREP_AdapterType      adapter,
//    SCOREP_RegionType       regionType
//);


void
init_definition_manager( CuTest*                    tc,
                         SCOREP_DefinitionManager** manager,
                         bool                       allocHashTables )
{
    assert( manager );
    CuAssertPtrEquals( tc, *manager, 0 );
    SCOREP_InitializeDefinitionManager( manager,
                                        SCOREP_Memory_GetLocalDefinitionPageManager(),
                                        allocHashTables );
    CuAssertPtrNotNull( tc, *manager );
}


int
main()
{
    SCOREP_InitMeasurement();

    CuString* output = CuStringNew();
    CuSuite*  suite  = CuSuiteNew( "unification" );

    SUITE_ADD_TEST( suite, test_1 );

    CuSuiteRun( suite );
    CuSuiteSummary( suite, output );
    printf( "%s\n", output->buffer );

    SCOREP_FinalizeMeasurement();
    return suite->failCount;
}
