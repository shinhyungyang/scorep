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

    int buf[ nprocs ];

    int root = 1;

    if ( rank == root )
    {
        init_const( buf, nprocs, 999 );
    }
    else
    {
        init_const( buf, nprocs, 0 );
    }

    keep_busy( rank * 2 );
    MPI_Bcast( buf, nprocs, MPI_INT, root, MPI_COMM_WORLD );

    printf( "Message buffers of rank %d of %d: ", rank, nprocs );
    print_buf( buf, nprocs );
    printf( "\n" );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}
