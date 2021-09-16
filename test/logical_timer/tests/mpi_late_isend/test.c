#include "../../tests.h"

const char* ltimer_test_name = "mpi_late_isend";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    int tag = 0;

    int send_buf = 999;
    int recv_buf = 0;

    if ( world_rank == 0 )
    {
        wait_logical_twice( 5 );
        MPI_Request req;
        MPI_Isend( &send_buf, 1, MPI_INT, 1, tag, MPI_COMM_WORLD, &req );
        wait_logical_twice( 5 );
        MPI_Wait( &req, MPI_STATUS_IGNORE );
    }
    else if ( world_rank == 1 )
    {
        MPI_Recv( &recv_buf, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

    PRINTF_RANK( "Message buffers: Send %d, Recv = %d\n", send_buf, recv_buf );

    int expected = ( world_rank == 1 ) ? 999 : 0;
    if ( !( recv_buf == expected ) )
    {
        return TEST_STATUS_ERROR;
    }
    return TEST_STATUS_SUCCESS;
}
