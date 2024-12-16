/* *INDENT-OFF* */
#include <config.h>
#include "omp_tasks.c.opari.inc"
#line 1 "omp_tasks.c"

/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2011, 2020, 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file       omp_tasks.c
 *
 *
 */



#include <stdio.h>


/* *INDENT-OFF* */
/* *INDENT-ON*  */


void
foo( int depth )
{
    printf( "in foo; depth=%d\n", depth );
    if ( depth == 1 )
    {
        return;
    }
    else
    {
        {
            int pomp2_if = 1;
            {
                POMP2_Task_handle pomp2_old_task;
                POMP2_Task_handle pomp2_new_task;
                if ( pomp2_if )
                {
                    POMP2_Task_create_begin( &opari2_region_1, &pomp2_new_task, &pomp2_old_task, pomp2_if, opari2_ctc_1  );
                }
                #line 44 "omp_tasks.c"
                #pragma omp task POMP2_DLIST_00001 if(pomp2_if) firstprivate(pomp2_new_task, pomp2_if)
                {
                    if ( pomp2_if )
                    {
                        POMP2_Task_begin( &opari2_region_1, pomp2_new_task );
                    }
                #line 45 "omp_tasks.c"
                    {
                        foo( depth - 1 );
                    }
                    if ( pomp2_if )
                    {
                        POMP2_Task_end( &opari2_region_1 );
                    }
                }
                if ( pomp2_if )
                {
                    POMP2_Task_create_end( &opari2_region_1, pomp2_old_task );
                }
            }
        }
        #line 48 "omp_tasks.c"
    }
    {
        POMP2_Task_handle pomp2_old_task;
        POMP2_Taskwait_begin( &opari2_region_2, &pomp2_old_task, opari2_ctc_2  );
        #line 49 "omp_tasks.c"
        #pragma omp taskwait
        POMP2_Taskwait_end( &opari2_region_2, pomp2_old_task );
    }
    #line 50 "omp_tasks.c"
    return;
}


int
main()
{
    printf( "in main\n" );
    {
        int               pomp2_num_threads = omp_get_max_threads();
        int               pomp2_if          = 1;
        POMP2_Task_handle pomp2_old_task;
        POMP2_Parallel_fork( &opari2_region_3, pomp2_if, pomp2_num_threads, &pomp2_old_task, opari2_ctc_3 );
        #line 58 "omp_tasks.c"
        #pragma omp parallel POMP2_DLIST_00003 firstprivate(pomp2_old_task) num_threads(pomp2_num_threads)
        {
            POMP2_Parallel_begin( &opari2_region_3 );
            #line 59 "omp_tasks.c"
            {
                {
                    POMP2_Single_enter( &opari2_region_4, opari2_ctc_4  );
                    #line 60 "omp_tasks.c"
                    #pragma omp single nowait
                    {
                        POMP2_Single_begin( &opari2_region_4 );
                        #line 61 "omp_tasks.c"
                        foo( 10 );
                        POMP2_Single_end( &opari2_region_4 );
                    }
                    {
                        POMP2_Task_handle pomp2_old_task;
                        POMP2_Implicit_barrier_enter( &opari2_region_4, &pomp2_old_task );
                            #pragma omp barrier
                        POMP2_Implicit_barrier_exit( &opari2_region_4, pomp2_old_task );
                    }
                    POMP2_Single_exit( &opari2_region_4 );
                }
                #line 62 "omp_tasks.c"
            }
            {
                POMP2_Task_handle pomp2_old_task;
                POMP2_Implicit_barrier_enter( &opari2_region_3, &pomp2_old_task );
                #pragma omp barrier
                POMP2_Implicit_barrier_exit( &opari2_region_3, pomp2_old_task );
            }
            POMP2_Parallel_end( &opari2_region_3 );
        }
        POMP2_Parallel_join( &opari2_region_3, pomp2_old_task );
    }
    #line 63 "omp_tasks.c"
}
