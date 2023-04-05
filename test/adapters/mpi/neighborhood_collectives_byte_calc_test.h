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

#ifndef NEIGHBORHOOD_COLLECTIVES_BYTE_CALC_TEST_H
#define NEIGHBORHOOD_COLLECTIVES_BYTE_CALC_TEST_H

#include <mpi.h>

#include <stdint.h>
#include <stdbool.h>


#include <CuTest.h>

extern MPI_Datatype Int32x1;
extern MPI_Datatype Int32x2;
extern MPI_Datatype Int32x4;
extern MPI_Datatype Int32x5;
extern MPI_Datatype Int32x10;
extern MPI_Datatype BogusType;

extern const uint32_t bogus32;
extern const uint64_t bogus64;

typedef struct ByteCounts
{
    uint64_t send;
    uint64_t recv;
} ByteCounts;

extern MPI_Comm testCommGraph;
extern MPI_Comm testCommCart;
extern MPI_Comm testCommDistGraph;

void
init_test( void );

void
finalize_test( void );

CuSuite*
create_suite( char*         name,
              MPI_Comm      comm,
              TestAllreduce reduction );

int
finalize_suite( CuSuite* suite );

void
create_comms( void );

void
finalize_comms( void );

void
reduce_results( MPI_Comm comm,
                int*     result );

void
reduce_results_cart_comm( int* result );

void
reduce_results_graph_comm( int* result );

void
reduce_results_dist_graph_comm( int* result );

void
validate_byte_counts( CuTest*    ct,
                      MPI_Comm   comm,
                      ByteCounts actual,
                      ByteCounts expected );


#endif /* NEIGHBORHOOD_COLLECTIVES_BYTE_CALC_TEST_H */
