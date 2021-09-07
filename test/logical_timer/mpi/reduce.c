#include "test_common.h"

#include <mpi.h>
#include <stdio.h>
#include <assert.h>

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

    int root = 1;

    int send_buf = rank;
    int recv_buf = 0;

    keep_busy( rank * 2 );
    MPI_Reduce( &send_buf, &recv_buf, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
    assert(
        ( rank == root && recv_buf == nprocs * ( nprocs - 1 ) / 2 )
        || ( rank != root && recv_buf == 0 )
        );

    printf( "Message buffers of rank %d of %d: Send %d, Recv %d \n", rank, nprocs, send_buf, recv_buf );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );

    send_buf = rank;
    recv_buf = 0;

    keep_busy( rank * 2 );
    MPI_Allreduce( &send_buf, &recv_buf, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
    assert( recv_buf == nprocs * ( nprocs - 1 ) / 2 );

    printf( "Message buffers of rank %d of %d: Send %d, Recv %d \n", rank, nprocs, send_buf, recv_buf );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}
