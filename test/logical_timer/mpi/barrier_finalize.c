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

    keep_busy( rank * 2 );
    MPI_Barrier( MPI_COMM_WORLD );
    keep_busy( rank * 2 );
    MPI_Finalize();
    return 0;
}
