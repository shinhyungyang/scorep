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

#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>


#include "neighborhood_collectives_byte_calc_test.h"
#include "neighborhood_collectives_byte_calc_test_units.h"

#include <CuTest.h>

int
main( int argc, char** argv )
{
    int failCount = 0;
    MPI_Init( &argc, &argv );
    init_test();

    CuSuite* suite_cart = create_suite( "Neighborhood collective byte counts on cartesian topology",
                                        testCommCart,
                                        reduce_results_cart_comm );

    if ( suite_cart )
    {
        SUITE_ADD_TEST_NAME( suite_cart, test_cart_nb_alltoall, "Alltoall" );
        SUITE_ADD_TEST_NAME( suite_cart, test_cart_nb_alltoallv, "Alltoallv" );
        SUITE_ADD_TEST_NAME( suite_cart, test_cart_nb_alltoallw, "Alltoallw" );
        SUITE_ADD_TEST_NAME( suite_cart, test_cart_nb_allgather, "Allgather" );
        SUITE_ADD_TEST_NAME( suite_cart, test_cart_nb_allgatherv, "Allgatherv" );

        CuSuiteRun( suite_cart );
        failCount += finalize_suite( suite_cart );
    }

    MPI_Barrier( MPI_COMM_WORLD );

    CuSuite* suite_graph = create_suite( "Neighborhood collective byte counts on graph topology",
                                         testCommGraph,
                                         reduce_results_graph_comm );

    if ( suite_graph )
    {
        SUITE_ADD_TEST_NAME( suite_graph, test_graph_nb_alltoall, "Alltoall" );
        SUITE_ADD_TEST_NAME( suite_graph, test_graph_nb_alltoallv, "Alltoallv" );
        SUITE_ADD_TEST_NAME( suite_graph, test_graph_nb_alltoallw, "Alltoallw" );
        SUITE_ADD_TEST_NAME( suite_graph, test_graph_nb_allgather, "Allgather" );
        SUITE_ADD_TEST_NAME( suite_graph, test_graph_nb_allgatherv, "Allgatherv" );


        CuSuiteRun( suite_graph );
        failCount += finalize_suite( suite_graph );
    }

    MPI_Barrier( MPI_COMM_WORLD );

    CuSuite* suite_dist_graph = create_suite( "Neighborhood collective byte counts on dist graph topology",
                                              testCommDistGraph,
                                              reduce_results_dist_graph_comm );

    if ( suite_dist_graph )
    {
        SUITE_ADD_TEST_NAME( suite_dist_graph, test_dist_graph_nb_alltoall, "Alltoall" );
        SUITE_ADD_TEST_NAME( suite_dist_graph, test_dist_graph_nb_alltoallv, "Alltoallv" );
        SUITE_ADD_TEST_NAME( suite_dist_graph, test_dist_graph_nb_alltoallw, "Alltoallw" );
        SUITE_ADD_TEST_NAME( suite_dist_graph, test_dist_graph_nb_allgather, "Allgather" );
        SUITE_ADD_TEST_NAME( suite_dist_graph, test_dist_graph_nb_allgatherv, "Allgatherv" );

        CuSuiteRun(  suite_dist_graph );
        failCount += finalize_suite( suite_dist_graph );
    }

    MPI_Barrier( MPI_COMM_WORLD );

    finalize_test();
    MPI_Finalize();

    return failCount ? EXIT_FAILURE : EXIT_SUCCESS;
}
