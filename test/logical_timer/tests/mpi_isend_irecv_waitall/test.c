#include "../../tests.h"

const char* ltimer_test_name = "mpi_isend_irecv_waitall";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    int next = ( world_rank < world_nprocs - 1 ) ? world_rank + 1 : 0;
    int prev = ( world_rank > 0 ) ? world_rank - 1 : world_nprocs - 1;

    int tag = 0;

    int send_buf = 3 * world_rank;
    int recv_buf = 0;

    MPI_Request req[ 2 ];
    MPI_Irecv( &recv_buf, 1, MPI_INT, prev, tag, MPI_COMM_WORLD, &( req[ 0 ] ) );

    wait_logical_twice( world_rank );
    MPI_Isend( &send_buf, 1, MPI_INT, next, tag, MPI_COMM_WORLD, &( req[ 1 ] ) );

    MPI_Waitall( 2, req, MPI_STATUSES_IGNORE );

    PRINTF_RANK( "Message buffers: Send %d, Recv %d \n", send_buf, recv_buf );

    int expected = ( 3 * prev );
    if ( !( recv_buf == expected ) )
    {
        return TEST_STATUS_ERROR;
    }
    return TEST_STATUS_SUCCESS;
}
