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

    int msg = 0;
    int tag = 0;

    /* Late send test */
    if ( rank == 0 )
    {
        keep_busy( 10 );
        msg = 666;
        MPI_Send( &msg, 1, MPI_INT, 1, tag, MPI_COMM_WORLD );
    }
    if ( rank == 1 )
    {
        MPI_Recv( &msg, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

    printf( "Message buffer of rank %d of %d: %d\n", rank, nprocs, msg );
    fflush( stdout );

    /* Late recv test */
    if ( rank == 1 )
    {
        msg = 999;
        MPI_Send( &msg, 1, MPI_INT, 0, tag, MPI_COMM_WORLD );
    }
    if ( rank == 0 )
    {
        keep_busy( 10 );
        MPI_Recv( &msg, 1, MPI_INT, 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

    printf( "Message buffer of rank %d of %d: %d\n", rank, nprocs, msg );
    fflush( stdout );

    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}
