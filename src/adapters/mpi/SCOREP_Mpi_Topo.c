/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
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

/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2011                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2010-2011                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/


/**
 * @file       SCOREP_Mpi_Topo.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 * @status     alpha
 * @ingroup    MPI_Wrapper
 *
 * @brief C interface wrappers for topologies
 */

#include <config.h>
#include "SCOREP_Mpi.h"
#include <UTILS_Error.h>

/**
 * @name C wrappers
 * @{
 */

#if HAVE( DECL_PMPI_CART_CREATE )
/**
 * Measurement wrapper for MPI_Cart_create
 * @note Manually adapted wrapper
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Sequence of events:
 * @li enter region 'MPI_Cart_create'
 * @li define communicator
 * @li define topology
 * @li define coordinates
 * @li exit region 'MPI_Cart_create'
 */
int
MPI_Cart_create( MPI_Comm  comm_old,
                 int       ndims,
                 int*      dims,
                 int*      periodv,
                 int       reorder,
                 MPI_Comm* comm_cart )
{
    const int32_t event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO );
    int32_t       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_CREATE ] );
    }

    return_val = PMPI_Cart_create( comm_old, ndims, dims, periodv, reorder, comm_cart );

    if ( *comm_cart != MPI_COMM_NULL )
    {
        SCOREP_MPICartesianTopologyHandle topid = SCOREP_INVALID_CART_TOPOLOGY;
        SCOREP_LocalMPICommunicatorHandle cid;
        int32_t                           my_rank, i;
        int32_t*                          coordv;
        uint8_t*                          uperiodv;
        uint32_t*                         udimv;
        uint32_t*                         ucoordv;

        /* register the new topology communicator */
        scorep_mpi_comm_create( *comm_cart, comm_old );

        /* get the internal comminicator id for the communicator of the new topology */
        cid = scorep_mpi_comm_handle( *comm_cart );

        /* find the rank of the calling process */
        PMPI_Comm_rank( *comm_cart, &my_rank );

        /* assign the cartesian topology dimension parameters */
        udimv = calloc( ndims, sizeof( uint32_t ) );
        if ( !udimv )
        {
            UTILS_ERROR_POSIX();
        }

        uperiodv = calloc( ndims, sizeof( uint8_t ) );
        if ( !uperiodv )
        {
            UTILS_ERROR_POSIX();
        }

        for ( i = 0; i < ndims; i++ )
        {
            udimv[ i ]    =  ( uint32_t )dims[ i ];
            uperiodv[ i ] =  ( uint8_t )periodv[ i ];
        }

        /* create the cartesian topology definition record */
        topid = SCOREP_DefineMPICartesianTopology( "", cid, ndims, udimv, uperiodv );

        /* allocate space for coordv and ucoordv */
        coordv = calloc( ndims, sizeof( int ) );
        if ( !coordv )
        {
            UTILS_ERROR_POSIX();
        }

        ucoordv = calloc( ndims, sizeof( uint32_t ) );
        if ( !ucoordv )
        {
            UTILS_ERROR_POSIX();
        }

        /* get the coordinates of the calling process in coordv */
        PMPI_Cart_coords( *comm_cart, my_rank, ndims, coordv );

        /* assign the coordinates */
        for ( i = 0; i < ndims; i++ )
        {
            ucoordv[ i ] = ( uint32_t )coordv[ i ];
        }

        /* create the coordinates definition record */
        SCOREP_DefineMPICartesianCoords( topid, ndims, ucoordv );

        free( udimv );
        free( uperiodv );
        free( ucoordv );
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_CREATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CART_SUB )
/**
 * Measurement wrapper for MPI_Cart_sub
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_CommMgnt.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * It triggers an enter and an exit event. Additionally, between enter event and exit
 * event, @ref SCOREP_DefineLocalMPICommunicator is called.
 */
int
MPI_Cart_sub( MPI_Comm comm, int* remain_dims, MPI_Comm* newcomm )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_SUB ] );
    }

    return_val = PMPI_Cart_sub( comm, remain_dims, newcomm );
    if ( *newcomm != MPI_COMM_NULL )
    {
        scorep_mpi_comm_create( *newcomm, comm );
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_SUB ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif
#if HAVE( DECL_PMPI_GRAPH_CREATE )
/**
 * Measurement wrapper for MPI_Graph_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_CommMgnt.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * It triggers an enter and an exit event. Additionally, between enter event and exit
 * event, @ref SCOREP_DefineLocalMPICommunicator is called.
 */
int
MPI_Graph_create( MPI_Comm comm_old, int nnodes, int* index, int* edges, int reorder, MPI_Comm* newcomm )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_CREATE ] );
    }

    return_val = PMPI_Graph_create( comm_old, nnodes, index, edges, reorder, newcomm );
    if ( *newcomm != MPI_COMM_NULL )
    {
        scorep_mpi_comm_create( *newcomm, comm_old );
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_CREATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_DIST_GRAPH_CREATE ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Dist_graph_create )
/**
 * Measurement wrapper for MPI_Dist_graph_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_CommMgnt.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup topo
 * It triggers an enter and an exit event. Additionally, between enter event and exit
 * event, @ref SCOREP_DefineLocalMPICommunicator is called.
 */
int
MPI_Dist_graph_create( MPI_Comm comm_old, int n, int sources[], int degrees[], int destinations[], int weights[], MPI_Info info, int reorder, MPI_Comm* newcomm )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_CREATE ] );
    }

    return_val = PMPI_Dist_graph_create( comm_old, n, sources, degrees, destinations, weights, info, reorder, newcomm );
    if ( *newcomm != MPI_COMM_NULL )
    {
        scorep_mpi_comm_create( *newcomm, comm_old );
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_CREATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif
#if HAVE( DECL_PMPI_DIST_GRAPH_CREATE_ADJACENT ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Dist_graph_create_adjacent )
/**
 * Measurement wrapper for MPI_Dist_graph_create_adjacent
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_CommMgnt.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup topo
 * It triggers an enter and an exit event. Additionally, between enter event and exit
 * event, @ref SCOREP_DefineLocalMPICommunicator is called.
 */
int
MPI_Dist_graph_create_adjacent( MPI_Comm comm_old, int indegree, int sources[], int sourceweights[], int outdegree, int destinations[], int destweights[], MPI_Info info, int reorder, MPI_Comm* newcomm )
{
    const int event_gen_active = SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO );
    int       return_val;

    if ( event_gen_active )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_CREATE_ADJACENT ] );
    }

    return_val = PMPI_Dist_graph_create_adjacent( comm_old, indegree, sources, sourceweights, outdegree, destinations, destweights, info, reorder, newcomm );
    if ( *newcomm != MPI_COMM_NULL )
    {
        scorep_mpi_comm_create( *newcomm, comm_old );
    }

    if ( event_gen_active )
    {
        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_CREATE_ADJACENT ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CART_COORDS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Cart_coords )
/**
 * Measurement wrapper for MPI_Cart_coords
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Cart_coords call with enter and exit events.
 */
int
MPI_Cart_coords( MPI_Comm comm, int rank, int maxdims, int* coords )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_COORDS ] );

        return_val = PMPI_Cart_coords( comm, rank, maxdims, coords );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_COORDS ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Cart_coords( comm, rank, maxdims, coords );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CART_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Cart_get )
/**
 * Measurement wrapper for MPI_Cart_get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Cart_get call with enter and exit events.
 */
int
MPI_Cart_get( MPI_Comm comm, int maxdims, int* dims, int* periods, int* coords )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_GET ] );

        return_val = PMPI_Cart_get( comm, maxdims, dims, periods, coords );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_GET ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Cart_get( comm, maxdims, dims, periods, coords );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CART_MAP ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Cart_map )
/**
 * Measurement wrapper for MPI_Cart_map
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Cart_map call with enter and exit events.
 */
int
MPI_Cart_map( MPI_Comm comm, int ndims, int* dims, int* periods, int* newrank )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_MAP ] );

        return_val = PMPI_Cart_map( comm, ndims, dims, periods, newrank );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_MAP ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Cart_map( comm, ndims, dims, periods, newrank );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CART_RANK ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Cart_rank )
/**
 * Measurement wrapper for MPI_Cart_rank
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Cart_rank call with enter and exit events.
 */
int
MPI_Cart_rank( MPI_Comm comm, int* coords, int* rank )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_RANK ] );

        return_val = PMPI_Cart_rank( comm, coords, rank );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_RANK ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Cart_rank( comm, coords, rank );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CART_SHIFT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Cart_shift )
/**
 * Measurement wrapper for MPI_Cart_shift
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Cart_shift call with enter and exit events.
 */
int
MPI_Cart_shift( MPI_Comm comm, int direction, int disp, int* rank_source, int* rank_dest )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CART_SHIFT ] );

        return_val = PMPI_Cart_shift( comm, direction, disp, rank_source, rank_dest );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CART_SHIFT ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Cart_shift( comm, direction, disp, rank_source, rank_dest );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_CARTDIM_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Cartdim_get )
/**
 * Measurement wrapper for MPI_Cartdim_get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Cartdim_get call with enter and exit events.
 */
int
MPI_Cartdim_get( MPI_Comm comm, int* ndims )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_CARTDIM_GET ] );

        return_val = PMPI_Cartdim_get( comm, ndims );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_CARTDIM_GET ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Cartdim_get( comm, ndims );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_DIST_GRAPH_NEIGHBORS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Dist_graph_neighbors )
/**
 * Measurement wrapper for MPI_Dist_graph_neighbors
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Dist_graph_neighbors call with enter and exit events.
 */
int
MPI_Dist_graph_neighbors( MPI_Comm comm, int maxindegree, int sources[], int sourceweights[], int maxoutdegree, int destinations[], int destweights[] )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_NEIGHBORS ] );

        return_val = PMPI_Dist_graph_neighbors( comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_NEIGHBORS ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Dist_graph_neighbors( comm, maxindegree, sources, sourceweights, maxoutdegree, destinations, destweights );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_DIST_GRAPH_NEIGHBORS_COUNT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Dist_graph_neighbors_count )
/**
 * Measurement wrapper for MPI_Dist_graph_neighbors_count
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-2
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Dist_graph_neighbors_count call with enter and exit events.
 */
int
MPI_Dist_graph_neighbors_count( MPI_Comm comm, int* indegree, int* outdegree, int* weighted )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_NEIGHBORS_COUNT ] );

        return_val = PMPI_Dist_graph_neighbors_count( comm, indegree, outdegree, weighted );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_DIST_GRAPH_NEIGHBORS_COUNT ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Dist_graph_neighbors_count( comm, indegree, outdegree, weighted );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GRAPH_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Graph_get )
/**
 * Measurement wrapper for MPI_Graph_get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Graph_get call with enter and exit events.
 */
int
MPI_Graph_get( MPI_Comm comm, int maxindex, int maxedges, int* index, int* edges )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_GET ] );

        return_val = PMPI_Graph_get( comm, maxindex, maxedges, index, edges );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_GET ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Graph_get( comm, maxindex, maxedges, index, edges );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GRAPH_MAP ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Graph_map )
/**
 * Measurement wrapper for MPI_Graph_map
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Graph_map call with enter and exit events.
 */
int
MPI_Graph_map( MPI_Comm comm, int nnodes, int* index, int* edges, int* newrank )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_MAP ] );

        return_val = PMPI_Graph_map( comm, nnodes, index, edges, newrank );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_MAP ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Graph_map( comm, nnodes, index, edges, newrank );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GRAPH_NEIGHBORS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Graph_neighbors )
/**
 * Measurement wrapper for MPI_Graph_neighbors
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Graph_neighbors call with enter and exit events.
 */
int
MPI_Graph_neighbors( MPI_Comm comm, int rank, int maxneighbors, int* neighbors )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_NEIGHBORS ] );

        return_val = PMPI_Graph_neighbors( comm, rank, maxneighbors, neighbors );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_NEIGHBORS ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Graph_neighbors( comm, rank, maxneighbors, neighbors );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GRAPH_NEIGHBORS_COUNT ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Graph_neighbors_count )
/**
 * Measurement wrapper for MPI_Graph_neighbors_count
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Graph_neighbors_count call with enter and exit events.
 */
int
MPI_Graph_neighbors_count( MPI_Comm comm, int rank, int* nneighbors )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_NEIGHBORS_COUNT ] );

        return_val = PMPI_Graph_neighbors_count( comm, rank, nneighbors );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPH_NEIGHBORS_COUNT ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Graph_neighbors_count( comm, rank, nneighbors );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_GRAPHDIMS_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Graphdims_get )
/**
 * Measurement wrapper for MPI_Graphdims_get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Graphdims_get call with enter and exit events.
 */
int
MPI_Graphdims_get( MPI_Comm comm, int* nnodes, int* nedges )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPHDIMS_GET ] );

        return_val = PMPI_Graphdims_get( comm, nnodes, nedges );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_GRAPHDIMS_GET ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Graphdims_get( comm, nnodes, nedges );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_TOPO_TEST ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Topo_test )
/**
 * Measurement wrapper for MPI_Topo_test
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Topo_test call with enter and exit events.
 */
int
MPI_Topo_test( MPI_Comm comm, int* status )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_TOPO_TEST ] );

        return_val = PMPI_Topo_test( comm, status );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_TOPO_TEST ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Topo_test( comm, status );
    }

    return return_val;
}
#endif

#if HAVE( DECL_PMPI_DIMS_CREATE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_TOPO ) && !defined( MPI_Dims_create )
/**
 * Measurement wrapper for MPI_Dims_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_Std.w
 * @note C interface
 * @note Introduced with MPI-1
 * @ingroup topo
 * Triggers an enter and exit event.
 * It wraps the MPI_Dims_create call with enter and exit events.
 */
int
MPI_Dims_create( int nnodes, int ndims, int* dims )
{
    int return_val;

    if ( SCOREP_MPI_IS_EVENT_GEN_ON_FOR( SCOREP_MPI_ENABLED_TOPO ) )
    {
        SCOREP_MPI_EVENT_GEN_OFF();
        SCOREP_EnterRegion( scorep_mpi_regid[ SCOREP__MPI_DIMS_CREATE ] );

        return_val = PMPI_Dims_create( nnodes, ndims, dims );

        SCOREP_ExitRegion( scorep_mpi_regid[ SCOREP__MPI_DIMS_CREATE ] );
        SCOREP_MPI_EVENT_GEN_ON();
    }
    else
    {
        return_val = PMPI_Dims_create( nnodes, ndims, dims );
    }

    return return_val;
}
#endif


/**
 * @}
 */
