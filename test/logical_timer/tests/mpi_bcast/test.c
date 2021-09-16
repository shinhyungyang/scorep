#include "../../tests.h"

const char* ltimer_test_name = "mpi_bcast";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    int root = 1;

    int buf = world_rank;

    wait_logical_twice( world_rank );
    MPI_Bcast( &buf, 1, MPI_INT, root, MPI_COMM_WORLD );

    PRINTF_RANK( "Message buffer: %d \n", buf );

    int expected = root;
    if ( !( buf == expected ) )
    {
        return TEST_STATUS_ERROR;
    }
    return TEST_STATUS_SUCCESS;
}
