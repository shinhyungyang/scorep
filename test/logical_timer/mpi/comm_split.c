#include "test_common.h"

#include <mpi.h>
#include <stdio.h>

int
main( int argc, char* argv[] )
{
    int rank_w, nprocs_w;
    int rank, nprocs;

    MPI_Init( &argc, &argv );

    MPI_Comm_size( MPI_COMM_WORLD, &nprocs_w );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank_w );

    MPI_Comm comm;
    MPI_Comm_split( MPI_COMM_WORLD, 0, rank_w, &comm );
    MPI_Comm_size( comm, &nprocs );
    MPI_Comm_rank( comm, &rank );
//     MPI_Comm_dup(MPI_COMM_WORLD, &comm);

    if ( nprocs < 2 )
    {
        printf( "This test needs at least two MPI ranks to run properly" );
        MPI_Finalize();
        return 1;
    }

    int tag = 0;

    int send_buf = 999;
    int recv_buf = 0;

    if ( rank == 0 )
    {
        keep_busy( 10 );
        MPI_Send( &send_buf, 1, MPI_INT, 1, tag, comm );
    }
    else if ( rank == 1 )
    {
        MPI_Request req;
        MPI_Irecv( &recv_buf, 1, MPI_INT, 0, tag, comm, &req );
        MPI_Wait( &req, MPI_STATUS_IGNORE );
    }

    printf( "Message buffers of rank %d of %d: send %d, recv = %d\n", rank, nprocs, send_buf, recv_buf );
    fflush( stdout );

    send_buf = 999;
    recv_buf = 0;

    if ( rank == 0 )
    {
        keep_busy( 10 );
        MPI_Request req;
        MPI_Isend( &send_buf, 1, MPI_INT, 1, tag, comm, &req );
        MPI_Wait( &req, MPI_STATUS_IGNORE );
    }
    else if ( rank == 1 )
    {
        MPI_Recv( &recv_buf, 1, MPI_INT, 0, tag, comm, MPI_STATUS_IGNORE );
    }

    printf( "Message buffers of rank %d of %d: send %d, recv = %d\n", rank, nprocs, send_buf, recv_buf );
    fflush( stdout );

    MPI_Barrier( comm );
    MPI_Finalize();
    return 0;
}
