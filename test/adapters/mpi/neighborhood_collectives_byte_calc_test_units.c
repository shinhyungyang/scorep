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
#include <scorep_mpi_coll.h>


/* -----------------------------------------------------------------------------
 * Cartesian topology
 * -------------------------------------------------------------------------- */
void
test_cart_nb_alltoall( CuTest* ct )
{
    MPI_Comm   comm     = testCommCart;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 3 * 100 * 4, 3 * 100 * 4 };

    int          sendcount = bogus32;
    int          recvcount = bogus32;
    MPI_Datatype sendtype  = BogusType;
    MPI_Datatype recvtype  = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
        case 1:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            break;
        case 2:
        case 3:
            sendcount = 25;
            sendtype  = Int32x4;
            recvcount = 50;
            recvtype  = Int32x2;
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoall( sendcount, sendtype, recvcount, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}


void
test_cart_nb_alltoallv( CuTest* ct )
{
    MPI_Comm   comm     = testCommCart;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { bogus64, bogus64 };

    int          sendcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtype        = BogusType;
    MPI_Datatype recvtype        = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
        case 2:
            sendcounts[ 0 ] = 100;
            sendcounts[ 1 ] = 100;
            sendcounts[ 2 ] = 200 + bogus32;
            sendcounts[ 3 ] = 200;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 50;
            recvcounts[ 1 ] = 50;
            recvcounts[ 2 ] = 100 + bogus32;
            recvcounts[ 3 ] = 100;
            recvtype        = Int32x2;
            expected        = ( ByteCounts ){.send = 1600, .recv = 1600 };
            break;
        case 1:
        case 3:
            sendcounts[ 0 ] = 50;
            sendcounts[ 1 ] = 50;
            sendcounts[ 2 ] = 200;
            sendcounts[ 3 ] = 200 + bogus32;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 25;
            recvcounts[ 1 ] = 25;
            recvcounts[ 2 ] = 100;
            recvcounts[ 3 ] = 100 + bogus32;
            recvtype        = Int32x2;
            expected        = ( ByteCounts ){.send = 1200, .recv = 1200 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoallv( sendcounts, sendtype, recvcounts, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}


void
test_cart_nb_alltoallw( CuTest* ct )
{
    MPI_Comm   comm     = testCommCart;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { bogus64, bogus64 };

    int sendcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    int recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };

    MPI_Datatype sendtypes[ 4 ] = { BogusType, BogusType, BogusType, BogusType };
    MPI_Datatype recvtypes[ 4 ] = { BogusType, BogusType, BogusType, BogusType };

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
        case 2:
            sendcounts[ 0 ] = 10;
            sendcounts[ 1 ] = 100;
            sendcounts[ 2 ] = 200 + bogus32;
            sendcounts[ 3 ] = 200;
            sendtypes[ 0 ]  = Int32x10;
            sendtypes[ 1 ]  = Int32x1;
            sendtypes[ 2 ]  = BogusType;
            sendtypes[ 3 ]  = Int32x1;
            recvcounts[ 0 ] = 50;
            recvcounts[ 1 ] = 25;
            recvcounts[ 2 ] = 100 + bogus32;
            recvcounts[ 3 ] = 100;
            recvtypes[ 0 ]  = Int32x2;
            recvtypes[ 1 ]  = Int32x4;
            recvtypes[ 2 ]  = BogusType;
            recvtypes[ 3 ]  = Int32x2;
            expected        = ( ByteCounts ){.send = 1600, .recv = 1600 };
            break;
        case 1:
        case 3:
            sendcounts[ 0 ] = 5;
            sendcounts[ 1 ] = 50;
            sendcounts[ 2 ] = 200;
            sendcounts[ 3 ] = 200 + bogus32;
            sendtypes[ 0 ]  = Int32x10;
            sendtypes[ 1 ]  = Int32x1;
            sendtypes[ 2 ]  = Int32x1;
            sendtypes[ 3 ]  = BogusType;
            recvcounts[ 0 ] = 25;
            recvcounts[ 1 ] = 25;
            recvcounts[ 2 ] = 100;
            recvcounts[ 3 ] = 100 + bogus32;
            recvtypes[ 0 ]  = Int32x2;
            recvtypes[ 1 ]  = Int32x2;
            recvtypes[ 2 ]  = Int32x2;
            recvtypes[ 3 ]  = BogusType;
            expected        = ( ByteCounts ){.send = 1200, .recv = 1200 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoallw( sendcounts, sendtypes, recvcounts, recvtypes, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}


void
test_cart_nb_allgather( CuTest* ct )
{
    MPI_Comm   comm     = testCommCart;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 3 * 100 * 4, 3 * 100 * 4 };

    int          sendcount = bogus32;
    int          recvcount = bogus32;
    MPI_Datatype sendtype  = BogusType;
    MPI_Datatype recvtype  = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
        case 1:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            break;
        case 2:
        case 3:
            sendcount = 25;
            sendtype  = Int32x4;
            recvcount = 50;
            recvtype  = Int32x2;
            break;
    }
    scorep_mpi_coll_bytes_neighbor_allgather( sendcount, sendtype, recvcount, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

void
test_cart_nb_allgatherv( CuTest* ct )
{
    MPI_Comm   comm     = testCommCart;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { bogus64, bogus64 };

    int          sendcount       = bogus32;
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtype        = BogusType;
    MPI_Datatype recvtype        = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
        case 2:
            sendcount       = 100;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100;
            recvcounts[ 1 ] = 100;
            recvcounts[ 2 ] = 200 + bogus32;
            recvcounts[ 3 ] = 200;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 3 * 400, .recv = 2 * 400 + 1 * 800 };
            break;
        case 1:
        case 3:
            sendcount       = 200;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 200;
            recvcounts[ 1 ] = 200;
            recvcounts[ 2 ] = 100;
            recvcounts[ 3 ] = 100 + bogus32;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 3 * 800, .recv = 2 * 800 + 1 * 400 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_allgatherv( sendcount, sendtype, recvcounts, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

/* -----------------------------------------------------------------------------
 * Graph topology
 * -------------------------------------------------------------------------- */
void
test_graph_nb_alltoall( CuTest* ct )
{
    MPI_Comm   comm     = testCommGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    int          sendcount = bogus32;
    int          recvcount = bogus32;
    MPI_Datatype sendtype  = BogusType;
    MPI_Datatype recvtype  = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 400 };
            break;
        case 1:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 20;
            recvtype  = Int32x5;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
        case 2:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
        case 3:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 20;
            recvtype  = Int32x5;
            expected  = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 400 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoall( sendcount, sendtype, recvcount, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

void
test_graph_nb_alltoallv( CuTest* ct )
{
    MPI_Comm   comm     = testCommGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { bogus64, bogus64 };

    // Arrays are larger than needed
    int          sendcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtype        = BogusType;
    MPI_Datatype recvtype        = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcounts[ 0 ] = 200;
            sendcounts[ 1 ] = 200;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100;
            recvcounts[ 1 ] = 100;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 2 * 800, .recv = 2 * 400 };
            break;
        case 1:
            sendcounts[ 0 ] = 50;
            sendtype        = Int32x2;
            recvcounts[ 0 ] = 50;
            recvtype        = Int32x4;
            expected        = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 800 };
            break;
        case 2:
            sendcounts[ 0 ] = 200;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 1 * 800, .recv = 1 * 400 };
            break;
        case 3:
            sendcounts[ 0 ] = 100;
            sendcounts[ 1 ] = 100;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 200;
            recvcounts[ 1 ] = 200;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 800 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoallv( sendcounts, sendtype, recvcounts, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

void
test_graph_nb_alltoallw( CuTest* ct )
{
    MPI_Comm   comm     = testCommGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { bogus64, bogus64 };

    // Arrays are larger than needed
    int          sendcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtypes[ 4 ]  = { BogusType, BogusType, BogusType, BogusType };
    MPI_Datatype recvtypes[ 4 ]  = { BogusType, BogusType, BogusType, BogusType };
    ;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcounts[ 0 ] = 200;
            sendcounts[ 1 ] = 200;
            sendtypes[ 0 ]  = Int32x1;
            sendtypes[ 1 ]  = Int32x1;
            recvcounts[ 0 ] = 100;
            recvcounts[ 1 ] = 100;
            recvtypes[ 0 ]  = Int32x1;
            recvtypes[ 1 ]  = Int32x1;
            expected        = ( ByteCounts ){.send = 2 * 800, .recv = 2 * 400 };
            break;
        case 1:
            sendcounts[ 0 ] = 50;
            sendtypes[ 0 ]  = Int32x2;
            recvcounts[ 0 ] = 50;
            recvtypes[ 0 ]  = Int32x4;
            expected        = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 800 };
            break;
        case 2:
            sendcounts[ 0 ] = 200;
            sendtypes[ 0 ]  = Int32x1;
            recvcounts[ 0 ] = 100;
            recvtypes[ 0 ]  = Int32x1;
            expected        = ( ByteCounts ){.send = 1 * 800, .recv = 1 * 400 };
            break;
        case 3:
            sendcounts[ 0 ] = 100;
            sendcounts[ 1 ] = 100;
            sendtypes[ 0 ]  = Int32x1;
            sendtypes[ 1 ]  = Int32x1;
            recvcounts[ 0 ] = 200;
            recvcounts[ 1 ] = 200;
            recvtypes[ 0 ]  = Int32x1;
            recvtypes[ 1 ]  = Int32x1;
            expected        = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 800 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoallw( sendcounts, sendtypes, recvcounts, recvtypes, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

void
test_graph_nb_allgather( CuTest* ct )
{
    MPI_Comm   comm     = testCommGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    int          sendcount = bogus32;
    int          recvcount = bogus32;
    MPI_Datatype sendtype  = BogusType;
    MPI_Datatype recvtype  = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 400 };
            break;
        case 1:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 20;
            recvtype  = Int32x5;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
        case 2:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
        case 3:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 20;
            recvtype  = Int32x5;
            expected  = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 400 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_allgather( sendcount, sendtype, recvcount, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

void
test_graph_nb_allgatherv( CuTest* ct )
{
    MPI_Comm   comm     = testCommGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { bogus64, bogus64 };

    // Arrays are larger than needed
    int          sendcount       = bogus32;
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtype        = BogusType;
    MPI_Datatype recvtype        = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcount       = 200;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100;
            recvcounts[ 1 ] = 100;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 2 * 800, .recv = 2 * 400 };
            break;
        case 1:
            sendcount       = 50;
            sendtype        = Int32x2;
            recvcounts[ 0 ] = 50;
            recvtype        = Int32x4;
            expected        = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 800 };
            break;
        case 2:
            sendcount       = 200;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 1 * 800, .recv = 1 * 400 };
            break;
        case 3:
            sendcount       = 100;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 200;
            recvcounts[ 1 ] = 200;
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 2 * 400, .recv = 2 * 800 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_allgatherv( sendcount, sendtype, recvcounts, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

/* -----------------------------------------------------------------------------
 * Dist graph topology
 * -------------------------------------------------------------------------- */
void
test_dist_graph_nb_alltoall( CuTest* ct )
{
    MPI_Comm   comm     = testCommDistGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    int          sendcount = bogus32;
    int          recvcount = bogus32;
    MPI_Datatype sendtype  = BogusType;
    MPI_Datatype recvtype  = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 3 * 400, .recv = 1 * 400 };
            break;
        case 1:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 25;
            recvtype  = Int32x4;
            expected  = ( ByteCounts ){.send = 0, .recv =  2 * 400 };
            break;
        case 2:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
        case 3:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 25;
            recvtype  = Int32x4;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoall( sendcount, sendtype, recvcount, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}

void
test_dist_graph_nb_alltoallv( CuTest* ct )
{
    MPI_Comm   comm     = testCommDistGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    // Arrays are larger than needed
    int          sendcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtype        = Int32x1;
    MPI_Datatype recvtype        = Int32x1;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcounts[ 0 ] = 100; // -> Rank 1
            sendcounts[ 1 ] = 200; // -> Rank 2
            sendcounts[ 2 ] = 300; // -> Rank 3
            recvcounts[ 0 ] = 500; // <- Rank 3
            expected        = ( ByteCounts ){.send = 1 * 400 + 1 * 800 + 1 * 1200, .recv = 1 * 2000 };
            break;
        case 1:
            recvcounts[ 0 ] = 100; // <- Rank 0
            recvcounts[ 1 ] = 400; // <- Rank 1
            expected        = ( ByteCounts ){.send = 0, .recv =  1 * 400 + 1 * 1600 };
            break;
        case 2:
            sendcounts[ 0 ] = 400; // -> Rank 1
            recvcounts[ 0 ] = 200; // <- Rank 0
            expected        = ( ByteCounts ){.send = 1 * 1600, .recv = 1 * 800 };
            break;
        case 3:
            sendcounts[ 0 ] = 500; // -> Rank 0
            recvcounts[ 0 ] = 300; // <- Rank 0
            expected        = ( ByteCounts ){.send = 1 * 2000, .recv = 1 * 1200 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoallv( sendcounts, sendtype, recvcounts, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}


void
test_dist_graph_nb_alltoallw( CuTest* ct )
{
    MPI_Comm   comm     = testCommDistGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    // Arrays are larger than needed
    int          sendcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtypes[ 4 ]  = { Int32x1, /*!!!*/ Int32x10, Int32x1, Int32x1 };
    MPI_Datatype recvtypes[ 4 ]  = { Int32x1, Int32x1, Int32x1, Int32x1 };

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcounts[ 0 ] = 100; // -> Rank 1
            sendcounts[ 1 ] = 20;  // -> Rank 2
            sendcounts[ 2 ] = 300; // -> Rank 3
            recvcounts[ 0 ] = 500; // <- Rank 3
            expected        = ( ByteCounts ){.send = 1 * 400 + 1 * 800 + 1 * 1200, .recv = 1 * 2000 };
            break;
        case 1:
            recvcounts[ 0 ] = 100; // <- Rank 0
            recvcounts[ 1 ] = 400; // <- Rank 2
            expected        = ( ByteCounts ){.send = 0, .recv =  1 * 400 + 1 * 1600 };
            break;
        case 2:
            sendcounts[ 0 ] = 400; // -> Rank 1
            recvcounts[ 0 ] = 200; // <- Rank 0
            expected        = ( ByteCounts ){.send = 1 * 1600, .recv = 1 * 800 };
            break;
        case 3:
            sendcounts[ 0 ] = 500; // -> Rank 0
            recvcounts[ 0 ] = 300; // <- Rank 0
            expected        = ( ByteCounts ){.send = 1 * 2000, .recv = 1 * 1200 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_alltoallw( sendcounts, sendtypes, recvcounts, recvtypes, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}


void
test_dist_graph_nb_allgather( CuTest* ct )
{
    MPI_Comm   comm     = testCommDistGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    int          sendcount = bogus32;
    int          recvcount = bogus32;
    MPI_Datatype sendtype  = BogusType;
    MPI_Datatype recvtype  = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 3 * 400, .recv = 1 * 400 };
            break;
        case 1:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 25;
            recvtype  = Int32x4;
            expected  = ( ByteCounts ){.send = 0, .recv =  2 * 400 };
            break;
        case 2:
            sendcount = 100;
            sendtype  = Int32x1;
            recvcount = 10;
            recvtype  = Int32x10;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
        case 3:
            sendcount = 50;
            sendtype  = Int32x2;
            recvcount = 25;
            recvtype  = Int32x4;
            expected  = ( ByteCounts ){.send = 1 * 400, .recv = 1 * 400 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_allgather( sendcount, sendtype, recvcount, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}


void
test_dist_graph_nb_allgatherv( CuTest* ct )
{
    MPI_Comm   comm     = testCommDistGraph;
    ByteCounts bytes    = { bogus64, bogus64 };
    ByteCounts expected = { 0, 0 };

    // Arrays are larger than needed
    int          sendcount       = bogus32;
    int          recvcounts[ 4 ] = { bogus32, bogus32, bogus32, bogus32 };
    MPI_Datatype sendtype        = BogusType;
    MPI_Datatype recvtype        = BogusType;

    int rank;
    MPI_Comm_rank( comm, &rank );
    switch ( rank )
    {
        case 0:
            sendcount       = 100;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 300; // <- Rank 3
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 3 * 400, .recv = 1 * 1200 };
            break;
        case 1:
            // Rank 1 has no outgoing neighbors. Therefore sendtype and sendcount
            // must not impact the byte calculations.
            recvcounts[ 0 ] = 100; // <- Rank 0
            recvcounts[ 1 ] = 200; // <- Rank 2
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 0, .recv =  1 * 400 + 1 * 800 };
            break;
        case 2:
            sendcount       = 200; // -> Rank 1
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100; // <- Rank 0
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 1 * 800, .recv = 1 * 400 };
            break;
        case 3:
            sendcount       = 300;
            sendtype        = Int32x1;
            recvcounts[ 0 ] = 100; // <- Rank 0
            recvtype        = Int32x1;
            expected        = ( ByteCounts ){.send = 1 * 1200, .recv = 1 * 400 };
            break;
    }
    scorep_mpi_coll_bytes_neighbor_allgatherv( sendcount, sendtype, recvcounts, recvtype, comm, &bytes.send, &bytes.recv );
    validate_byte_counts( ct, comm, bytes, expected );
}
