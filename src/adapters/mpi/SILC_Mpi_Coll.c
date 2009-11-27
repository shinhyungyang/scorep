/*
 * This file is part of the SILC project (http://www.silc.de)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */


#include "SILC_Mpi.h"
#include "config.h"

/**
 * @file  SILC_Mpi_Coll.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 * @status     ALPHA
 * @ingroup    MPI_Wrapper
 *
 * @brief C interface wrappers for collective communication
 */

/**
 * @name C wrappers
 * @{
 */

#if HAVE( DECL_MPI_ALLGATHER )
/**
 * Measurement wrapper for MPI_Allgather
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Allgather( void*        sendbuf,
               int          sendcount,
               MPI_Datatype sendtype,
               void*        recvbuf,
               int          recvcount,
               MPI_Datatype recvtype,
               MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       recvsz, sendsz, N;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_ALLGATHER ] );

        return_val = PMPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );


        PMPI_Type_size( recvtype, &recvsz );
        PMPI_Type_size( sendtype, &sendsz );
        PMPI_Comm_size( comm, &N );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_ALLGATHER ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            N * sendcount * sendsz,
                            N * recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_ALLGATHER ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Allgather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_ALLGATHERV ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Allgatherv
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Allgatherv( void*        sendbuf,
                int          sendcount,
                MPI_Datatype sendtype,
                void*        recvbuf,
                int*         recvcounts,
                int*         displs,
                MPI_Datatype recvtype,
                MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       recvcount, recvsz, sendsz, i, N;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_ALLGATHERV ] );

        return_val = PMPI_Allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm );


        PMPI_Type_size( recvtype, &recvsz );
        PMPI_Type_size( sendtype, &sendsz );
        PMPI_Comm_size( comm, &N );
        recvcount = 0;
        for ( i = 0; i < N; i++ )
        {
            recvcount += recvcounts[ i ];
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_ALLGATHERV ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            N * sendcount * sendsz,
                            recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_ALLGATHERV ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Allgatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_ALLREDUCE ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Allreduce
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Allreduce( void*        sendbuf,
               void*        recvbuf,
               int          count,
               MPI_Datatype datatype,
               MPI_Op       op,
               MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sz, N;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_ALLREDUCE ] );

        return_val = PMPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );


        PMPI_Type_size( datatype, &sz );
        PMPI_Comm_size( comm, &N );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_ALLREDUCE ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            N * count * sz,
                            N * count * sz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_ALLREDUCE ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Allreduce( sendbuf, recvbuf, count, datatype, op, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_ALLTOALL ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Alltoall
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Alltoall( void*        sendbuf,
              int          sendcount,
              MPI_Datatype sendtype,
              void*        recvbuf,
              int          recvcount,
              MPI_Datatype recvtype,
              MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       recvsz, sendsz, N;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_ALLTOALL ] );

        return_val = PMPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );


        PMPI_Type_size( recvtype, &recvsz );
        PMPI_Type_size( sendtype, &sendsz );
        PMPI_Comm_size( comm, &N );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_ALLTOALL ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            N * sendcount * sendsz,
                            N * recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_ALLTOALL ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Alltoall( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_ALLTOALLV ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Alltoallv
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Alltoallv( void*        sendbuf,
               int*         sendcounts,
               int*         sdispls,
               MPI_Datatype sendtype,
               void*        recvbuf,
               int*         recvcounts,
               int*         rdispls,
               MPI_Datatype recvtype,
               MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       recvcount = 0, sendcount = 0, recvsz, sendsz, N, i;
        SILC_Mpi_Rank root_loc  = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_ALLTOALLV ] );

        return_val = PMPI_Alltoallv( sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm );


        PMPI_Type_size( recvtype, &recvsz );
        PMPI_Type_size( sendtype, &sendsz );
        PMPI_Comm_size( comm, &N );
        for ( i = 0; i < N; i++ )
        {
            recvcount += recvcounts[ i ];
            sendcount += sendcounts[ i ];
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_ALLTOALLV ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            sendcount * sendsz,
                            recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_ALLTOALLV ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Alltoallv( sendbuf, sendcounts, sdispls, sendtype, recvbuf, recvcounts, rdispls, recvtype, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_ALLTOALLW ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Alltoallw
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Alltoallw( void*        sendbuf,
               int          sendcounts[],
               int          sdispls[],
               MPI_Datatype sendtypes[],
               void*        recvbuf,
               int          recvcounts[],
               int          rdispls[],
               MPI_Datatype recvtypes[],
               MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       recvcount = 0, sendcount = 0, recvsz, sendsz, N, i;
        SILC_Mpi_Rank root_loc  = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_ALLTOALLW ] );

        return_val = PMPI_Alltoallw( sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm );


        PMPI_Comm_size( comm, &N );
        for ( i = 0; i < N; i++ )
        {
            PMPI_Type_size( recvtypes[ i ], &recvsz );
            PMPI_Type_size( sendtypes[ i ], &sendsz );
            recvcount += recvsz * recvcounts[ i ];
            sendcount += sendsz * sendcounts[ i ];
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_ALLTOALLW ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            sendcount,
                            recvcount );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_ALLTOALLW ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Alltoallw( sendbuf, sendcounts, sdispls, sendtypes, recvbuf, recvcounts, rdispls, recvtypes, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_BARRIER ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Barrier
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Barrier( MPI_Comm comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;

        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_BARRIER ] );

        return_val = PMPI_Barrier( comm );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_BARRIER ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            0,
                            0 );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_BARRIER ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Barrier( comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_BCAST ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Bcast
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Bcast( void*        buffer,
           int          count,
           MPI_Datatype datatype,
           int          root,
           MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sz, N, me;
        SILC_Mpi_Rank root_loc = SILC_MPI_RANK_TO_PE( root, comm );


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_BCAST ] );

        return_val = PMPI_Bcast( buffer, count, datatype, root, comm );


        PMPI_Type_size( datatype, &sz );
        PMPI_Comm_rank( comm, &me );
        if ( me == root )
        {
            PMPI_Comm_size( comm, &N );
        }
        else
        {
            N = 0;
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_BCAST ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            N * count * sz,
                            count * sz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_BCAST ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Bcast( buffer, count, datatype, root, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_EXSCAN ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Exscan
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Exscan( void*        sendbuf,
            void*        recvbuf,
            int          count,
            MPI_Datatype datatype,
            MPI_Op       op,
            MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sz, me, N;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_EXSCAN ] );

        return_val = PMPI_Exscan( sendbuf, recvbuf, count, datatype, op, comm );


        PMPI_Type_size( datatype, &sz );
        PMPI_Comm_rank( comm, &me );
        PMPI_Comm_size( comm, &N );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_EXSCAN ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            ( N - me - 1 ) * sz * count,
                            me * sz * count );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_EXSCAN ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Exscan( sendbuf, recvbuf, count, datatype, op, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_GATHER ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Gather
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Gather( void*        sendbuf,
            int          sendcount,
            MPI_Datatype sendtype,
            void*        recvbuf,
            int          recvcount,
            MPI_Datatype recvtype,
            int          root,
            MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sendsz, recvsz, N, me;
        SILC_Mpi_Rank root_loc = SILC_MPI_RANK_TO_PE( root, comm );


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_GATHER ] );

        return_val = PMPI_Gather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm );


        PMPI_Type_size( sendtype, &sendsz );
        PMPI_Comm_rank( comm, &me );
        if ( me == root )
        {
            PMPI_Comm_size( comm, &N );
            PMPI_Type_size( recvtype, &recvsz );
        }
        else
        {
            N = recvsz = 0;
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_GATHER ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            sendcount * sendsz,
                            N * recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_GATHER ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Gather( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_GATHERV ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Gatherv
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Gatherv( void*        sendbuf,
             int          sendcount,
             MPI_Datatype sendtype,
             void*        recvbuf,
             int*         recvcounts,
             int*         displs,
             MPI_Datatype recvtype,
             int          root,
             MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       recvsz, sendsz, recvcount, me, N, i;
        SILC_Mpi_Rank root_loc = SILC_MPI_RANK_TO_PE( root, comm );


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_GATHERV ] );

        return_val = PMPI_Gatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm );


        PMPI_Type_size( sendtype, &sendsz );
        PMPI_Comm_rank( comm, &me );
        if ( me == root )
        {
            PMPI_Comm_size( comm, &N );
            PMPI_Type_size( recvtype, &recvsz );
            for ( i = 0; i < N; i++ )
            {
                recvcount += recvcounts[ i ];
            }
        }
        else
        {
            recvcount = recvsz = 0;
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_GATHERV ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            sendcount * sendsz,
                            recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_GATHERV ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Gatherv( sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_REDUCE ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Reduce
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Reduce( void*        sendbuf,
            void*        recvbuf,
            int          count,
            MPI_Datatype datatype,
            MPI_Op       op,
            int          root,
            MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sz, me, N;
        SILC_Mpi_Rank root_loc = SILC_MPI_RANK_TO_PE( root, comm );


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_REDUCE ] );

        return_val = PMPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );


        PMPI_Type_size( datatype, &sz );
        PMPI_Comm_rank( comm, &me );
        PMPI_Comm_size( comm, &N );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_REDUCE ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            count * sz,
                            ( root == me ? N * count * sz : 0 ) );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_REDUCE ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Reduce( sendbuf, recvbuf, count, datatype, op, root, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_REDUCE_SCATTER ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Reduce_scatter
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Reduce_scatter( void*        sendbuf,
                    void*        recvbuf,
                    int*         recvcounts,
                    MPI_Datatype datatype,
                    MPI_Op       op,
                    MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       i, sz, me, N, count = 0;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_REDUCE_SCATTER ] );

        return_val = PMPI_Reduce_scatter( sendbuf, recvbuf, recvcounts, datatype, op, comm );


        PMPI_Type_size( datatype, &sz );
        PMPI_Comm_rank( comm, &me );
        PMPI_Comm_size( comm, &N );
        for ( i = 0; i < N; i++ )
        {
            count += recvcounts[ i ];
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_REDUCE_SCATTER ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            count * sz,
                            N * recvcounts[ me ] * sz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_REDUCE_SCATTER ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Reduce_scatter( sendbuf, recvbuf, recvcounts, datatype, op, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_SCAN ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Scan
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Scan( void*        sendbuf,
          void*        recvbuf,
          int          count,
          MPI_Datatype datatype,
          MPI_Op       op,
          MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sz, me, N;
        SILC_Mpi_Rank root_loc = SILC_INVALID_ROOT_RANK;


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_SCAN ] );

        return_val = PMPI_Scan( sendbuf, recvbuf, count, datatype, op, comm );


        PMPI_Type_size( datatype, &sz );
        PMPI_Comm_rank( comm, &me );
        PMPI_Comm_size( comm, &N );

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_SCAN ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            ( N - me ) * count * sz,
                            ( me + 1 ) * count * sz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_SCAN ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Scan( sendbuf, recvbuf, count, datatype, op, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_SCATTER ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Scatter
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Scatter( void*        sendbuf,
             int          sendcount,
             MPI_Datatype sendtype,
             void*        recvbuf,
             int          recvcount,
             MPI_Datatype recvtype,
             int          root,
             MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sendsz, recvsz, N, me;
        SILC_Mpi_Rank root_loc = SILC_MPI_RANK_TO_PE( root, comm );


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_SCATTER ] );

        return_val = PMPI_Scatter( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm );


        PMPI_Type_size( recvtype, &recvsz );
        PMPI_Comm_rank( comm, &me );
        if ( me == root )
        {
            PMPI_Comm_size( comm, &N );
            PMPI_Type_size( sendtype, &sendsz );
        }
        else
        {
            N = sendsz = 0;
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_SCATTER ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            N * sendcount * sendsz,
                            recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_SCATTER ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Scatter( sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm );
    }

    return return_val;
}
#endif
#if HAVE( DECL_MPI_SCATTERV ) && !defined( SILC_MPI_NO_COLL )
/**
 * Measurement wrapper for MPI_Scatterv
 * @note Auto-generated by wrapgen from template: coll.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup coll
 * It wraps the mpi call with an enter and exit event. Additionally, a collective
 * event is generated in between the enter and exit event after the PMPI call.
 */
int
MPI_Scatterv( void*        sendbuf,
              int*         sendcounts,
              int*         displs,
              MPI_Datatype sendtype,
              void*        recvbuf,
              int          recvcount,
              MPI_Datatype recvtype,
              int          root,
              MPI_Comm     comm )
{
    int return_val;

    if ( SILC_MPI_IS_EVENT_GEN_ON_FOR( SILC_MPI_ENABLED_COLL ) )
    {
        int32_t       sendcount, sendsz, recvsz, me, N, i;
        SILC_Mpi_Rank root_loc = SILC_MPI_RANK_TO_PE( root, comm );


        SILC_MPI_EVENT_GEN_OFF();
        SILC_EnterRegion( silc_mpi_regid[ SILC__MPI_SCATTERV ] );

        return_val = PMPI_Scatterv( sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm );


        sendcount = sendsz = 0;
        PMPI_Type_size( recvtype, &recvsz );
        PMPI_Comm_rank( comm, &me );
        if ( me == root )
        {
            PMPI_Comm_size( comm, &N );
            PMPI_Type_size( sendtype, &sendsz );
            for ( i = 0; i < N; i++ )
            {
                sendcount += sendcounts[ i ];
            }
        }

        SILC_MpiCollective( silc_mpi_regid[ SILC__MPI_SCATTERV ],
                            SILC_MPI_COMM_ID( comm ),
                            root_loc,
                            sendcount * sendsz,
                            recvcount * recvsz );
        SILC_ExitRegion( silc_mpi_regid[ SILC__MPI_SCATTERV ] );
        SILC_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Scatterv( sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm );
    }

    return return_val;
}
#endif

/**
 * @}
 */
