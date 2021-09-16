#include "../../tests.h"

const char* ltimer_test_name = "mpi_comm_split";
const int   mpi_min_ranks    = 3;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    MPI_Comm comm;
    int      comm_nprocs, comm_rank;
    MPI_Comm_split( MPI_COMM_WORLD, world_rank / 2, world_rank % 2, &comm );
    MPI_Comm_size( comm, &comm_nprocs );
    MPI_Comm_rank( comm, &comm_rank );
    int tag = 0;

    int send_buf = 999;
    int recv_buf = 0;

    if ( comm_nprocs >= 2 )
    {
        if ( comm_rank == 0 )
        {
            wait_logical_twice( 5 );
            MPI_Send( &send_buf, 1, MPI_INT, 1, tag, comm );
        }
        else if ( comm_rank == 1 )
        {
            MPI_Request req;
            MPI_Irecv( &recv_buf, 1, MPI_INT, 0, tag, comm, &req );
            MPI_Wait( &req, MPI_STATUS_IGNORE );
        }
    }

    PRINTF_RANK( "Message buffers: Send %d, Recv = %d\n", send_buf, recv_buf );

    int expected = ( comm_rank == 1 ) ? 999 : 0;
    if ( !( recv_buf == expected ) )
    {
        return TEST_STATUS_ERROR;
    }
    return TEST_STATUS_SUCCESS;
}
