#include "../../tests.h"

const char* ltimer_test_name = "hybrid_reduction";
const int   mpi_min_ranks    = 2;
const int   mpi_thread_level = MPI_THREAD_FUNNELED;

int
ltimer_test_func()
{
    int root = 1;

    int local_sum = 0;

    #pragma omp parallel reduction(+: local_sum)
    {
        wait_logical_twice( world_rank + omp_get_thread_num() );
        local_sum = world_rank * omp_get_num_threads() + omp_get_thread_num();
    }

    int global_sum = 0;
    MPI_Reduce( &local_sum, &global_sum, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD );
    PRINTF_RANK( "Message buffers: local sum %d, global sum %d \n", local_sum, global_sum );

    int N            = world_nprocs * omp_get_max_threads();
    int expected_sum = ( world_rank == root ) ? ( N * ( N - 1 ) ) / 2 : 0;
    PRINTF_RANK( "Expected global sum %d \n", expected_sum );

    if ( !( global_sum == expected_sum ) )
    {
        return TEST_STATUS_ERROR;
    }

    return TEST_STATUS_SUCCESS;
}
