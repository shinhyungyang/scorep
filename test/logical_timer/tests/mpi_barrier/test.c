#include "../../tests.h"

const char* ltimer_test_name = "mpi_barrier";
const int   mpi_min_ranks    = 1;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    wait_logical_twice( 3 * ( world_nprocs - world_rank ) );
    MPI_Barrier( MPI_COMM_WORLD );

    return TEST_STATUS_SUCCESS;
}
