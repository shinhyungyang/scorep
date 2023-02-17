/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2023,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 * Tests for the monotonic FastHashtab.
 */

#include <config.h>


#if defined ( USE_HEADER_AND_DEFINITION )

#include "fasthashtab_monotonic_table.h"

#else /* ! USE_HEADER_AND_DEFINITION */

/************************** table *********************************************/

#include <SCOREP_FastHashtab.h>
#include <jenkins_hash.h>
#include <assert.h>
#include <stdlib.h>

typedef uint32_t table_key_t;
typedef uint32_t table_value_t;

#define TABLE_HASH_EXPONENT 8

static inline uint32_t
table_bucket_idx( table_key_t key )
{
#if defined( USE_JENKINS )
    return jenkins_hash( &key, sizeof( key ), 0 ) & hashmask( TABLE_HASH_EXPONENT );
#else
    /* use key directly to allow to trigger collisions */
    return key & hashmask( TABLE_HASH_EXPONENT );
#endif
}

static inline bool
table_equals( table_key_t key1,
              table_key_t key2 )
{
    return key1 == key2;
}

static inline void*
table_allocate_chunk( size_t chunkSize )
{
    void* ptr;
    assert( posix_memalign( &ptr, SCOREP_CACHELINESIZE, chunkSize ) == 0 );
    return ptr;
}

static inline void
table_free_chunk( void* chunk )
{
}

static inline table_value_t
table_value_ctor( table_key_t* key,
                  void*        ctorData )
{
    return *( table_value_t* )ctorData;
}

SCOREP_HASH_TABLE_MONOTONIC( table,
                             8,
                             hashsize( TABLE_HASH_EXPONENT ) );

#endif /* ! USE_HEADER_AND_DEFINITION */

/************************** tests *********************************************/

#include <CuTest.h>
#include <stdio.h>
#include <stdlib.h>

static void
count_cb( table_key_t   key,
          table_value_t value,
          void*         cbData )
{
    int* count = cbData;
    ( *count )++;
}

static int
count( void )
{
    int count = 0;
    table_iterate_key_value_pairs( count_cb, &count );
    return count;
}

static void
test_01( CuTest* tc )
{
    CuAssertIntEquals( tc, 0, count() );
}

static void
test_02( CuTest* tc )
{
    table_value_t value = 0;
    CuAssertTrue( tc, !table_get( value, &value ) );
    CuAssertIntEquals( tc, 0, count() );
}

static void
test_03( CuTest* tc )
{
    table_key_t   key      = 0;
    table_value_t value    = 0;
    bool          inserted = table_get_and_insert( key, &key, &value );
    CuAssertTrue( tc, inserted );
    CuAssertIntEquals( tc, key, value );
    CuAssertTrue( tc, table_get( key, &value ) );
    CuAssertIntEquals( tc, key, value );
    CuAssertIntEquals( tc, 1, count() );
}

static void
test_04( CuTest* tc )
{
    table_key_t   key      = 0;
    table_value_t value    = 0;
    bool          inserted = table_get_and_insert( key, &key, &value );
    CuAssertTrue( tc, !inserted );
    CuAssertIntEquals( tc, 1, count() );
}

static void
test_05( CuTest* tc )
{
    table_key_t   key = 0;
    table_value_t value;
    CuAssertTrue( tc, table_get( key, &value ) );
    CuAssertIntEquals( tc, key, value );
    CuAssertIntEquals( tc, 1, count() );
}

static void
test_06( CuTest* tc )
{
    table_key_t   key      = 1;
    table_value_t value    = 0;
    bool          inserted = table_get_and_insert( key, &key, &value );
    CuAssertTrue( tc, inserted );
    CuAssertIntEquals( tc, key, value );
    CuAssertTrue( tc, table_get( key, &value ) );
    CuAssertIntEquals( tc, key, value );
    CuAssertIntEquals( tc, 2, count() );
}

static void
test_07( CuTest* tc )
{
    table_key_t   key      = hashsize( TABLE_HASH_EXPONENT );
    table_value_t value    = 0;
    bool          inserted = table_get_and_insert( key, &key, &value );
    CuAssertTrue( tc, inserted );
    CuAssertIntEquals( tc, key, value );
    CuAssertTrue( tc, table_get( key, &value ) );
    CuAssertIntEquals( tc, key, value );
    CuAssertIntEquals( tc, 3, count() );
}

static void
test_08( CuTest* tc )
{
    table_key_t   key = 0;
    table_value_t value;
    CuAssertTrue( tc, table_get( key, &value ) );
    CuAssertIntEquals( tc, key, value );
    CuAssertIntEquals( tc, 3, count() );
}

static void
test_09( CuTest* tc )
{
    table_key_t   key = hashsize( TABLE_HASH_EXPONENT );
    table_value_t value;
    CuAssertTrue( tc, table_get( key, &value ) );
    CuAssertIntEquals( tc, key, value );
    CuAssertIntEquals( tc, 3, count() );
}

int
main( int argc, char** argv )
{
    CuUseColors();
    CuString* output = CuStringNew();
#if defined( USE_JENKINS )
    CuSuite* suite = CuSuiteNew( "FastHashtab: monotonic jenkins" );
#else
    CuSuite* suite = CuSuiteNew( "FastHashtab: monotonic" );
#endif

    SUITE_ADD_TEST_NAME( suite, test_01, "empty" );
    SUITE_ADD_TEST_NAME( suite, test_02, "get from empty" );
    SUITE_ADD_TEST_NAME( suite, test_03, "insert" );
    SUITE_ADD_TEST_NAME( suite, test_04, "insert same key" );
    SUITE_ADD_TEST_NAME( suite, test_05, "get from non-empty" );
    SUITE_ADD_TEST_NAME( suite, test_06, "insert new bucket" );
    SUITE_ADD_TEST_NAME( suite, test_07, "insert same bucket" );
    SUITE_ADD_TEST_NAME( suite, test_08, "get same bucket first" );
    SUITE_ADD_TEST_NAME( suite, test_09, "get same bucket second" );

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
