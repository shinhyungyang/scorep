#include "../../tests.h"

const char* ltimer_test_name = "mpi_allreduce";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    int send_buf = world_rank;
    int recv_buf = 0;

    wait_logical_twice( world_rank );
    MPI_Allreduce( &send_buf, &recv_buf, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );

    PRINTF_RANK( "Message buffers: Send %d, Recv %d \n", send_buf, recv_buf );

    int expected =  ( world_nprocs * ( world_nprocs - 1 ) ) / 2;
    if ( !( recv_buf == expected ) )
    {
        return TEST_STATUS_ERROR;
    }
    return TEST_STATUS_SUCCESS;
}
