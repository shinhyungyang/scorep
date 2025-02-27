#include <mpi.h>

#include "libwrap.h"

int
main( int   argc,
      char* argv[] )
{
    MPI_Init( &argc, &argv );

    /*
     * You need to add some calls to the target library to the main function.
     * Note, this program will never be executed, only linked against the target library.
     *
     * After finishing writing this file, run 'make'
     */

    MPI_Finalize();

    return 0;
}
