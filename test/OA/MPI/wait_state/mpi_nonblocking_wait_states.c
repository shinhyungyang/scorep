/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen University, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

/* *INDENT-OFF* */
#include <config.h>

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SCOREP_User.h"

int
main( int    argc,
      char** argv )
{
    int	retVal = 0; /* return value */
    int k, myrank=0, np=1;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if(np%2!=0)
    {
        printf("Wrong number of processes, has to be even\n",myrank);
        return 1;
    }

    for(k=0;k<1;k++)
    {
        SCOREP_USER_REGION_DEFINE( mainRegion );
        SCOREP_USER_OA_PHASE_BEGIN( mainRegion, "mainRegion", SCOREP_USER_REGION_TYPE_COMMON);

        int comm_partner;
        int tag=0;
        int buffer1=-1;
        int buffer2=-1;
        int buffer3=-1;
        int buffer4=-1;
        void* mpi_buffer=malloc(MPI_BSEND_OVERHEAD+sizeof(int));
        MPI_Buffer_attach(mpi_buffer,MPI_BSEND_OVERHEAD+sizeof(int));

        MPI_Status status;
        MPI_Status statuses[4];
        MPI_Request requests[4];
        int indices[4];
        int index;
        if(myrank%2==0)
        {
                comm_partner=myrank+1;
                buffer1=0;
                buffer2=1;
                buffer3=2;
                buffer4=3;
                MPI_Issend(&buffer1,1,MPI_INT,comm_partner,100,MPI_COMM_WORLD,&(requests[0]));
                MPI_Ibsend(&buffer2,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&(requests[1]));
                MPI_Irsend(&buffer3,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&(requests[2]));
                MPI_Isend(&buffer4,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&(requests[3]));

                //MPI_Cancel(request1);
                MPI_Wait((&requests[0]),(&statuses[0]));
                MPI_Waitall(4,requests,statuses);
                MPI_Waitany(4,requests,&index,statuses);
                MPI_Waitsome(4,requests,&index,indices,statuses);


        }
        else
        {
                comm_partner=myrank-1;
                sleep(5);
//              MPI_Recv(&buffer1,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&status);

//              MPI_Recv(&buffer1,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&status);

//              MPI_Recv(&buffer1,1,MPI_INT,comm_partner,100,MPI_COMM_WORLD,&status);

//              MPI_Recv(&buffer1,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&status);

                MPI_Irecv(&buffer1,1,MPI_INT,comm_partner,100,MPI_COMM_WORLD,&(requests[0]));
                MPI_Irecv(&buffer2,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&(requests[1]));
                MPI_Irecv(&buffer3,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&(requests[2]));
                MPI_Irecv(&buffer4,1,MPI_INT,comm_partner,tag,MPI_COMM_WORLD,&(requests[3]));
                sleep(3);
                int flag=0;
                while(!flag)
                {
				MPI_Test(&(requests[0]),&flag,&status);
				MPI_Testany(4,requests,&index,&flag,&status);
				MPI_Testsome(4,requests,&index,indices,statuses);
				MPI_Testall(4,requests,&flag,statuses);

                }
                //MPI_Waitall(4,requests,statuses);

        }


        tag++;

        SCOREP_USER_OA_PHASE_END( mainRegion );
        int buf_size;
        MPI_Buffer_detach(mpi_buffer,&buf_size);
        free(mpi_buffer);
    }

    MPI_Finalize();
    return retVal;
}
