#include "../../tests.h"

const char* ltimer_test_name = "mpi_wildcard_irecv";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_SINGLE;

int
ltimer_test_func()
{
    int tag = 0;

    int         send_buf = world_rank;
    int         recv_buf[ world_nprocs ];
    MPI_Request req[ world_nprocs ];

    PRINTF_RANK( "Send buf = %d\n", send_buf );

    if ( world_rank == 0 )
    {
        for ( int src = world_nprocs - 1; src >= 0; --src )
        {
            MPI_Irecv( &( recv_buf[ src ] ), 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &( req[ src ] ) );
        }
    }

    wait_logical_twice( world_rank );
    MPI_Send( &send_buf, 1, MPI_INT, 0, tag, MPI_COMM_WORLD );

    if ( world_rank == 0 )
    {
        MPI_Waitall( world_nprocs, req, MPI_STATUSES_IGNORE );

        int sum = 0;
        for ( int i = 0; i < world_nprocs; ++i )
        {
            PRINTF_RANK( "Recv buffer[%d] = %d\n", i, recv_buf[ i ] );
            sum += recv_buf[ i ];
        }

        PRINTF_RANK( "Sum: %d\n", sum );

        int expected = ( world_nprocs * ( world_nprocs - 1 ) ) / 2;
        if ( !( sum == expected ) )
        {
            return TEST_STATUS_ERROR;
        }
    }

    return TEST_STATUS_SUCCESS;
}
