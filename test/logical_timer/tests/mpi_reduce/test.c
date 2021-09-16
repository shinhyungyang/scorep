#include "../../tests.h"

const char* ltimer_test_name = "mpi_reduce";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    int root = 1;

    int send_buf = world_rank;
    int recv_buf = 0;

    wait_logical_twice( world_rank );
    MPI_Reduce( &send_buf, &recv_buf, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );

    PRINTF_RANK( "Message buffers: Send %d, Recv %d \n", send_buf, recv_buf );

    int expected = ( world_rank == root ) ? ( world_nprocs * ( world_nprocs - 1 ) ) / 2 : 0;
    if ( !( recv_buf == expected ) )
    {
        return TEST_STATUS_ERROR;
    }
    return TEST_STATUS_SUCCESS;
}
