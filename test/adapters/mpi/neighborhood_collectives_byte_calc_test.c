/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 */


#include <config.h>
#include "neighborhood_collectives_byte_calc_test.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

MPI_Datatype Int32x1;
MPI_Datatype Int32x2;
MPI_Datatype Int32x4;
MPI_Datatype Int32x5;
MPI_Datatype Int32x10;
MPI_Datatype BogusType;

const uint32_t bogus32 = 0xdeadbeef;
const uint64_t bogus64 = 0x1234567890abcedf;

MPI_Comm testCommGraph     = MPI_COMM_NULL;
MPI_Comm testCommCart      = MPI_COMM_NULL;
MPI_Comm testCommDistGraph = MPI_COMM_NULL;

void
init_test( void )
{
    create_comms();

    MPI_Type_contiguous( 1,  MPI_INT32_T, &Int32x1 );
    MPI_Type_commit( &Int32x1 );
    MPI_Type_contiguous( 2,  MPI_INT32_T, &Int32x2 );
    MPI_Type_commit( &Int32x2 );
    MPI_Type_contiguous( 4,  MPI_INT32_T, &Int32x4 );
    MPI_Type_commit( &Int32x4 );
    MPI_Type_contiguous( 5,  MPI_INT32_T, &Int32x5 );
    MPI_Type_commit( &Int32x5 );
    MPI_Type_contiguous( 10, MPI_INT32_T, &Int32x10 );
    MPI_Type_commit( &Int32x10 );

    MPI_Type_contiguous( 9876, MPI_CHAR, &BogusType );
    MPI_Type_commit( &BogusType );

    CuUseColors();
}


CuSuite*
create_suite( char* name, MPI_Comm comm, TestAllreduce reduction )
{
    if ( comm == MPI_COMM_NULL )
    {
        return NULL;
    }

    int rank;
    MPI_Comm_rank( comm, &rank );
    return CuSuiteNewParallel( name, rank, reduction );
}

int
finalize_suite( CuSuite* suite )
{
    if ( !suite )
    {
        return 0;
    }

    CuString* output = CuStringNew();
    CuSuiteSummary( suite, output );

    int failCount = suite->failCount;
    if ( failCount )
    {
        printf( "%s", output->buffer );
    }

    CuSuiteFree( suite );
    CuStringFree( output );

    return failCount;
}


void
finalize_test( void )
{
    MPI_Type_free( &Int32x1 );
    MPI_Type_free( &Int32x2 );
    MPI_Type_free( &Int32x4 );
    MPI_Type_free( &Int32x5 );
    MPI_Type_free( &Int32x10 );

    MPI_Type_free( &BogusType );

    finalize_comms();
}

void
create_comms( void )
{
    int size = 4;
    int wsize, wrank;
    MPI_Comm_size( MPI_COMM_WORLD, &wsize );
    if ( wsize < size )
    {
        printf( "Requested a communicator with %d ranks, but world size is only %d\n", size, wsize );
        MPI_Abort( MPI_COMM_WORLD, MPI_ERR_COMM );
    }
    MPI_Comm_rank( MPI_COMM_WORLD, &wrank );

    /*
     * Create a cartesian topology with 2 dimensions:
     *
     * Ranks on grid:
     *     dir 1 ->
     * dir 0
     *          | |
     * |        0 1  not periodic
     * V        2 3
     *          | |
     *        periodic
     */
    const int ndims        = 2;
    int       dims[ 2 ]    = { 2, 2 };
    int       periods[ 2 ] = { 1, 0 };
    MPI_Cart_create( MPI_COMM_WORLD, ndims, dims, periods, /*reorder*/ 0, &testCommCart );

    /*
     * Create a graph with 4 nodes and this adjacency matrix:
     *   0 1 2 3
     * 0   x   x
     * 1 x
     * 2       x
     * 3 x   x
     */
    const int nnodes     = 4;
    int       index[ 4 ] = { 2, 3, 4, 6 };
    int       edges[ 6 ] = { 1, 3, 0, 3, 0, 2 };
    MPI_Graph_create( MPI_COMM_WORLD, nnodes, index, edges, /*reorder*/ 0, &testCommGraph );


    /*
     * Create a dist graph with 4 nodes and this adjacency matrix:
     *       dest
     *       0 1 2 3
     * src 0   x x x
     *     1
     *     2   x
     *     3 x
     *
     * Directed Graph:
     * 3<===>0 ----> 2
     *       |       |
     *       |       V
     *       |-----> 1
     *
     */
    int  indegree;
    int* sources;
    int  outdegree;
    int* destinations;

    int sources_0[ 1 ] = { 3 };
    int sources_1[ 2 ] = { 0, 2 };
    int sources_2[ 1 ] = { 0 };
    int sources_3[ 1 ] = { 0 };

    int destinations_0[ 3 ] = { 1, 2, 3 };
    int destinations_1[ 0 ] = {};
    int destinations_2[ 1 ] = { 1 };
    int destinations_3[ 1 ] = { 0 };
    switch ( wrank )
    {
        case 0:
            indegree     = 1;
            sources      = sources_0;
            outdegree    = 3;
            destinations = destinations_0;
            break;
        case 1:
            indegree     = 2;
            sources      = sources_1;
            outdegree    = 0;
            destinations = destinations_1;
            break;
        case 2:
            indegree     = 1;
            sources      = sources_2;
            outdegree    = 1;
            destinations = destinations_2;
            break;
        case 3:
            indegree     = 1;
            sources      = sources_3;
            outdegree    = 1;
            destinations = destinations_3;
            break;
    }
    MPI_Dist_graph_create_adjacent( MPI_COMM_WORLD,
                                    indegree, sources, MPI_UNWEIGHTED,
                                    outdegree, destinations, MPI_UNWEIGHTED,
                                    MPI_INFO_NULL, /*reorder*/ 0,
                                    &testCommDistGraph );
}

void
free_comm( MPI_Comm* comm )
{
    if ( *comm != MPI_COMM_NULL )
    {
        MPI_Comm_free( comm );
    }
}

void
finalize_comms( void )
{
    free_comm( &testCommGraph );
    free_comm( &testCommCart );
    free_comm( &testCommDistGraph );
}

void
reduce_results( MPI_Comm comm, int* result )
{
    PMPI_Allreduce( MPI_IN_PLACE, result, 1, MPI_INT, MPI_MIN, comm );
}

void
reduce_results_cart_comm( int* result )
{
    reduce_results( testCommCart, result );
}

void
reduce_results_graph_comm( int* result )
{
    reduce_results( testCommGraph, result );
}

void
reduce_results_dist_graph_comm( int* result )
{
    reduce_results( testCommDistGraph, result );
}


void
validate_byte_counts( CuTest* ct, MPI_Comm comm, ByteCounts actual, ByteCounts expected )
{
    ByteCounts expected_total = { 0, 0 };

    MPI_Allreduce( &expected.send, &expected_total.send, 1, MPI_UINT64_T, MPI_SUM, comm );
    MPI_Allreduce( &expected.recv, &expected_total.recv, 1, MPI_UINT64_T, MPI_SUM, comm );

    char error_message[ HUGE_STRING_LEN ];
    snprintf( error_message, HUGE_STRING_LEN,
              "Expected total sent <%" PRIu64 "> differs from expected total received <%" PRIu64 "> bytes",
              expected_total.send, expected_total.recv );
    CuAssertMsg( ct, error_message, expected_total.send == expected_total.recv );

    CuAssertIntEqualsMsg( ct, "Sent bytes", expected.send, actual.send );
    CuAssertIntEqualsMsg( ct, "Received bytes", expected.recv, actual.recv );
}
