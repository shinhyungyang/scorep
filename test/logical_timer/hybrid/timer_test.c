#include <mpi.h>
#include <stdio.h>
#include <omp.h>


void
foo()
{
    printf( "Foo\n" );
}

int
main( int argc, char* argv[] )
{
    int rank, nprocs;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    foo();
    #pragma omp parallel
    {
        MPI_Comm_rank( MPI_COMM_WORLD, &rank );
        printf( "Hello world from process %d, thread %d of %d, %d\n",
                rank, omp_get_thread_num(), nprocs, omp_get_num_threads() );
    }
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
    return 0;
}
