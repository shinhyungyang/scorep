#include "test_common.h"

#include <mpi.h>
#include <stdio.h>

int
main( int argc, char* argv[] )
{
    int rank, nprocs;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if ( nprocs < 2 )
    {
        printf( "This test needs at least two MPI ranks to run properly" );
        MPI_Finalize();
        return 1;
    }

    int next = ( rank < nprocs - 1 ) ? rank + 1 : 0;
    int prev = ( rank > 0 ) ? rank - 1 : nprocs - 1;

    int tag = 0;

    int send_buf = rank * rank;
    int recv_buf = 0;

    MPI_Request req[ 2 ];
    MPI_Irecv( &recv_buf, 1, MPI_INT, prev, tag, MPI_COMM_WORLD, &( req[ 0 ] ) );

    keep_busy( rank * 2 );
    MPI_Isend( &send_buf, 1, MPI_INT, next, tag, MPI_COMM_WORLD, &( req[ 1 ] ) );

    MPI_Waitall( 2, req, MPI_STATUSES_IGNORE );

    printf( "Message buffers of rank %d of %d: send %d, recv = %d\n", rank, nprocs, send_buf, recv_buf );
    fflush( stdout );


    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}
