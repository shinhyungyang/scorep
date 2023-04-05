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

#ifndef NEIGHBORHOOD_COLLECTIVES_BYTE_CALC_TEST_UNITS_H
#define NEIGHBORHOOD_COLLECTIVES_BYTE_CALC_TEST_UNITS_H

#include <CuTest.h>

/* -----------------------------------------------------------------------------
 * Cartesian topology
 * -------------------------------------------------------------------------- */
void
test_cart_nb_alltoall( CuTest* ct );

void
test_cart_nb_alltoallv( CuTest* ct );

void
test_cart_nb_alltoallw( CuTest* ct );

void
test_cart_nb_allgather( CuTest* ct );

void
test_cart_nb_allgatherv( CuTest* ct );


/* -----------------------------------------------------------------------------
 * Graph topology
 * -------------------------------------------------------------------------- */
void
test_graph_nb_alltoall( CuTest* ct );

void
test_graph_nb_alltoallv( CuTest* ct );

void
test_graph_nb_alltoallw( CuTest* ct );

void
test_graph_nb_allgather( CuTest* ct );

void
test_graph_nb_allgatherv( CuTest* ct );


/* -----------------------------------------------------------------------------
 * Dist graph topology
 * -------------------------------------------------------------------------- */
void
test_dist_graph_nb_alltoall( CuTest* ct );

void
test_dist_graph_nb_alltoallv( CuTest* ct );

void
test_dist_graph_nb_alltoallw( CuTest* ct );

void
test_dist_graph_nb_allgather( CuTest* ct );

void
test_dist_graph_nb_allgatherv( CuTest* ct );


#endif /* NEIGHBORHOOD_COLLECTIVES_BYTE_CALC_TEST_UNITS_H */
