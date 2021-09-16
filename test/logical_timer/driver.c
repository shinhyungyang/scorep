#include "utils.h"
#include "tests.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int
main( int argc, char* argv[] )
{
    /*
     * MPI
     */
    int provided_thread_level;

    MPI_Init_thread( &argc, &argv, mpi_thread_level, &provided_thread_level );

    if ( mpi_thread_level != provided_thread_level )
    {
        PRINTF_MASTER( " Provided(%d) and required(%d) MPI thread level do not match\n", provided_thread_level, mpi_thread_level );
        MPI_Finalize();
        return TEST_STATUS_WRONG_MPI_THREAD_SUPPORT;
    }

    MPI_Comm_size( MPI_COMM_WORLD, &world_nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &world_rank );

    if ( world_nprocs < mpi_min_ranks )
    {
        PRINTF_MASTER( "This test needs at least %d MPI ranks to run properly\n", mpi_min_ranks );
        MPI_Finalize();
        return TEST_STATUS_NOT_ENOUGH_MPI_RANKS;
    }

    /*
     * This runs the selected test.
     */
    int status = ltimer_test_func();

    MPI_Barrier( MPI_COMM_WORLD );

    /*
     * Gather the test status of each MPI rank
     * and see if all succeded
     */
    int all_statuses[ world_nprocs ];
    MPI_Gather( &status, 1, MPI_INT, all_statuses, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Finalize();

    if ( world_rank == 0 )
    {
        int final_status = TEST_STATUS_SUCCESS;
        for ( int r = 0; r < world_nprocs; ++r )
        {
            final_status = ( all_statuses[ r ] > final_status ) ? all_statuses[ r ] : final_status;
        }
        if ( final_status == TEST_STATUS_SUCCESS )
        {
            PRINTF_MASTER( "Test %s SUCCEEDED with final status %d\n", ltimer_test_name, final_status );
        }
        else
        {
            PRINTF_MASTER( "Test %s FAILED with final status %d\n", ltimer_test_name, final_status );
            printf( "Status on each process :\n" );
            for ( int r = 0; r < world_nprocs; ++r )
            {
                printf( "%d ", all_statuses[ r ] );
            }
            printf( "\n" );
        }
        return final_status;
    }

    return status;
}
