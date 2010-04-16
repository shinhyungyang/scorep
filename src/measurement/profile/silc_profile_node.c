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

/**
 * @file silc_profile_node.c
 * Implementation for managing profile node trees.
 *
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @status ALPHA
 *
 */

#include <stddef.h>

#include "SILC_Memory.h"
#include "SILC_Utils.h"

#include "silc_profile_node.h"
#include "silc_profile_definition.h"


/* ***************************************************************************************
   Prototypes of internal comparison and copy functions for type dependent data.
*****************************************************************************************/
static silc_profile_type_data_t
silc_profile_copy_regular_region_data( silc_profile_type_data_t data );

static silc_profile_type_data_t
silc_profile_copy_parameter_string_data( silc_profile_type_data_t data );

static silc_profile_type_data_t
silc_profile_copy_parameter_integer_data( silc_profile_type_data_t data );

static silc_profile_type_data_t
silc_profile_copy_thread_root_data( silc_profile_type_data_t data );

static silc_profile_type_data_t
silc_profile_copy_thread_start_data( silc_profile_type_data_t data );

static bool
silc_profile_compare_regular_region_data( silc_profile_type_data_t data1,
                                          silc_profile_type_data_t data2 );

static bool
silc_profile_compare_parameter_string_data( silc_profile_type_data_t data1,
                                            silc_profile_type_data_t data2 );

static bool
silc_profile_compare_parameter_integer_data( silc_profile_type_data_t data1,
                                             silc_profile_type_data_t data2 );

static bool
silc_profile_compare_thread_root_data( silc_profile_type_data_t data1,
                                       silc_profile_type_data_t data2 );

static bool
silc_profile_compare_thread_start_data( silc_profile_type_data_t data1,
                                        silc_profile_type_data_t data2 );

/* ***************************************************************************************
   Type dependent data handling types and variables
*****************************************************************************************/

/**
   Type for entries in the functions table list @a silc_profile_type_data_funcs, which
   contains functions for handling type dependent data. Currently, it contains a
   pointer to a comparison function and a pointer to a copy function.
 */
typedef struct
{
    bool ( * comp_func )( silc_profile_type_data_t data1,
                          silc_profile_type_data_t data2 );
    silc_profile_type_data_t ( * copy_func )( silc_profile_type_data_t data );
} silc_profile_type_data_func_t;

/* *INDENT-OFF* */

/**
  Functions table list for handling type dependent data. All entries must occur in the
  same order like in @a silc_profile_node_type.
 */
silc_profile_type_data_func_t silc_profile_type_data_funcs[] = {
  { &silc_profile_compare_regular_region_data,    &silc_profile_copy_regular_region_data    },
  { &silc_profile_compare_parameter_string_data,  &silc_profile_copy_parameter_string_data  },
  { &silc_profile_compare_parameter_integer_data, &silc_profile_copy_parameter_integer_data },
  { &silc_profile_compare_thread_root_data,       &silc_profile_copy_thread_root_data       },
  { &silc_profile_compare_thread_start_data,      &silc_profile_copy_thread_start_data      }
};

/* *INDENT-ON* */

/* ***************************************************************************************
   Implementation of comparison ond copy functions for type dependent data.
*****************************************************************************************/

static silc_profile_type_data_t
silc_profile_copy_regular_region_data( silc_profile_type_data_t data )
{
    return data;
}

static silc_profile_type_data_t
silc_profile_copy_parameter_string_data( silc_profile_type_data_t data )
{
    silc_profile_string_node_data* old_data = SILC_PROFILE_DATA2PARAMSTR( data );
    silc_profile_string_node_data* new_data =
        SILC_Memory_AllocForProfile( sizeof( silc_profile_string_node_data ) );
    new_data->handle = old_data->handle;
    new_data->value  = old_data->value;
    return SILC_PROFILE_PARAMSTR2DATA( new_data );
}

static silc_profile_type_data_t
silc_profile_copy_parameter_integer_data( silc_profile_type_data_t data )
{
    silc_profile_integer_node_data* old_data = SILC_PROFILE_DATA2PARAMINT( data );
    silc_profile_integer_node_data* new_data =
        SILC_Memory_AllocForProfile( sizeof( silc_profile_integer_node_data ) );
    new_data->handle = old_data->handle;
    new_data->value  = old_data->value;
    return SILC_PROFILE_PARAMINT2DATA( new_data );
}

static silc_profile_type_data_t
silc_profile_copy_thread_fork_data( silc_profile_type_data_t data )
{
    return data;
}

static silc_profile_type_data_t
silc_profile_copy_thread_root_data( silc_profile_type_data_t data )
{
    return data;
}

static silc_profile_type_data_t
silc_profile_copy_thread_start_data( silc_profile_type_data_t data )
{
    return data;
}

static silc_profile_type_data_t
silc_profile_copy_collapse_data( silc_profile_type_data_t data )
{
    return data;
}

static bool
silc_profile_compare_regular_region_data( silc_profile_type_data_t data1,
                                          silc_profile_type_data_t data2 )
{
    return data1 == data2;
}

static bool
silc_profile_compare_parameter_string_data( silc_profile_type_data_t data1,
                                            silc_profile_type_data_t data2 )
{
    silc_profile_string_node_data* string_data1 = SILC_PROFILE_DATA2PARAMSTR( data1 );
    silc_profile_string_node_data* string_data2 = SILC_PROFILE_DATA2PARAMSTR( data2 );
    return ( string_data1->handle == string_data2->handle ) &&
           ( string_data1->value  == string_data2->value );
}

static bool
silc_profile_compare_parameter_integer_data( silc_profile_type_data_t data1,
                                             silc_profile_type_data_t data2 )
{
    silc_profile_integer_node_data* string_data1 = SILC_PROFILE_DATA2PARAMINT( data1 );
    silc_profile_integer_node_data* string_data2 = SILC_PROFILE_DATA2PARAMINT( data2 );
    return ( string_data1->handle == string_data2->handle ) &&
           ( string_data1->value  == string_data2->value );
}

static bool
silc_profile_compare_thread_fork_data( silc_profile_type_data_t data1,
                                       silc_profile_type_data_t data2 )
{
    return true;
}

static bool
silc_profile_compare_thread_root_data( silc_profile_type_data_t data1,
                                       silc_profile_type_data_t data2 )
{
    return data1 == data2;
}

static bool
silc_profile_compare_thread_start_data( silc_profile_type_data_t data1,
                                        silc_profile_type_data_t data2 )
{
    return data1 == data2;
}

static bool
silc_profile_compare_collapse_data( silc_profile_type_data_t data1,
                                    silc_profile_type_data_t data2 )
{
    return true;
}

/* ***************************************************************************************
   Type dependent functions
*****************************************************************************************/

/* Compares to data sets */
bool
silc_profile_compare_type_data( silc_profile_type_data_t data1,
                                silc_profile_type_data_t data2,
                                silc_profile_node_type   type )
{
    return ( *silc_profile_type_data_funcs[ type ].comp_func )( data1, data2 );
}

/* Copies a data set */
silc_profile_type_data_t
silc_profile_copy_type_data( silc_profile_type_data_t data,
                             silc_profile_node_type   type )
{
    return ( *silc_profile_type_data_funcs[ type ].copy_func )( data );
}



/* ***************************************************************************************
   Creation / Destruction
*****************************************************************************************/

/** Creates a new child node of given type and data */
silc_profile_node*
silc_profile_create_node( silc_profile_node*       parent,
                          silc_profile_node_type   type,
                          silc_profile_type_data_t data,
                          uint64_t                 timestamp )
{
    int i;

    /* Reserve space for the node record and dense metrics */
    silc_profile_node* node
        = ( silc_profile_node* )
          SILC_Memory_AllocForProfile( sizeof( silc_profile_node )
                                       + silc_profile.num_of_dense_metrics
                                       * sizeof( silc_profile_dense_metric ) );
    if ( !node )
    {
        SILC_ERROR_POSIX();
        return NULL;
    }

    /* Space for dense metrics are reserved after the node struct */
    node->dense_metrics =
        ( silc_profile_dense_metric* )( ( void* )node + sizeof( silc_profile_node ) );

    /* Initialize values */
    node->callpath_handle     = SILC_INVALID_CALLPATH;
    node->parent              = parent;
    node->first_child         = NULL;
    node->next_sibling        = NULL;
    node->first_double_sparse = NULL;
    node->first_int_sparse    = NULL;
    node->count               = 0; /* Is increased to one during SILC_Profile_Enter() */
    node->first_enter_time    = timestamp;
    node->last_exit_time      = timestamp;
    node->node_type           = type;
    node->type_specific_data  = data;

    /* Initialize dense metric values */
    silc_profile_init_dense_metric( &node->implicit_time );
    for ( i = 0; i < silc_profile.num_of_dense_metrics; i++ )
    {
        silc_profile_init_dense_metric( &node->dense_metrics[ i ] );
    }

    return node;
}

/* Copies a node */
silc_profile_node*
silc_profile_copy_node( silc_profile_node* source )
{
    silc_profile_sparse_metric_int*    dest_sparse_int      = NULL;
    silc_profile_sparse_metric_int*    source_sparse_int    = source->first_int_sparse;
    silc_profile_sparse_metric_double* dest_sparse_double   = NULL;
    silc_profile_sparse_metric_double* source_sparse_double = source->first_double_sparse;

    /* Reserve space for the node record and dense metrics */
    silc_profile_node* node =
        ( silc_profile_node* )
        SILC_Memory_AllocForProfile( sizeof( silc_profile_node )
                                     + silc_profile.num_of_dense_metrics
                                     * sizeof( silc_profile_dense_metric ) );
    if ( !node )
    {
        SILC_ERROR_POSIX();
        return NULL;
    }

    /* Space for dense metrics are reserved after the node struct */
    node->dense_metrics =
        ( silc_profile_dense_metric* )( ( void* )node + sizeof( silc_profile_node ) );

    /* Initialize values */
    node->callpath_handle     = SILC_INVALID_CALLPATH;
    node->parent              = NULL;
    node->first_child         = NULL;
    node->next_sibling        = NULL;
    node->first_double_sparse = NULL;
    node->first_int_sparse    = NULL;
    node->node_type           = source->node_type;

    /* Copy dense metric values */
    silc_profile_copy_all_dense_metrics( node, source );

    /* Copy sparse integer metrics */
    while ( source_sparse_int != NULL )
    {
        dest_sparse_int              = silc_profile_copy_sparse_int( source_sparse_int );
        dest_sparse_int->next_metric = node->first_int_sparse;
        node->first_int_sparse       = dest_sparse_int;

        source_sparse_int = source_sparse_int->next_metric;
    }

    /* Copy sparse double metrics */
    while ( source_sparse_double != NULL )
    {
        dest_sparse_double              = silc_profile_copy_sparse_double( source_sparse_double );
        dest_sparse_double->next_metric = node->first_double_sparse;
        node->first_double_sparse       = dest_sparse_double;

        source_sparse_double = source_sparse_double->next_metric;
    }

    return node;
}

/* ***************************************************************************************
   Node operation
*****************************************************************************************/

bool
silc_profile_compare_nodes( silc_profile_node* node1,
                            silc_profile_node* node2 )
{
    if ( node1->node_type != node2->node_type )
    {
        return false;
    }
    return silc_profile_compare_type_data( node1->type_specific_data,
                                           node2->type_specific_data,
                                           node1->node_type );
}

/* Copies all dense metrics from source to destination */
void
silc_profile_copy_all_dense_metrics( silc_profile_node* destination,
                                     silc_profile_node* source )
{
    int i;

    destination->count              = source->count;
    destination->first_enter_time   = source->first_enter_time;
    destination->last_exit_time     = source->last_exit_time;
    destination->type_specific_data =
        silc_profile_copy_type_data( source->type_specific_data, source->node_type );

    /* Copy dense metric values */
    silc_profile_copy_dense_metric( &destination->implicit_time, &source->implicit_time );
    for ( i = 0; i < silc_profile.num_of_dense_metrics; i++ )
    {
        silc_profile_copy_dense_metric( &destination->dense_metrics[ i ],
                                        &source->dense_metrics[ i ] );
    }
}

/* Moves the children of a node to another node */
void
silc_profile_move_children(  silc_profile_node* destination,
                             silc_profile_node* source )
{
    silc_profile_node* child = NULL;

    SILC_ASSERT( source != NULL );

    /* If source has no child -> nothing to do */
    child = source->first_child;
    if ( child == NULL )
    {
        return;
    }

    /* Set new parent of all children of source. */
    while ( child != NULL )
    {
        child->parent = destination;
        child         = child->next_sibling;
    }

    /* If destination is NULL -> append all children as root nodes */
    if ( destination == NULL )
    {
        child = silc_profile.first_root_node;

        /* If profile is empty */
        if ( child == NULL )
        {
            silc_profile.first_root_node = source->first_child;
            source->first_child          = NULL;
            return;
        }

        /* Append at end of root nodes, see below */
    }
    else
    {
        /* If destination has no childs */
        child = destination->first_child;
        if ( child == NULL )
        {
            destination->first_child = source->first_child;
            source->first_child      = NULL;
            return;
        }
        /* Else append list of source to end of children list of destination, see below */
    }

    /* Search end of sibling list and append children */
    while ( child->next_sibling != NULL )
    {
        child = child->next_sibling;
    }
    child->next_sibling = source->first_child;
    source->first_child = NULL;
}

/* Removes a node with it subtree from its parents children */
void
silc_profile_remove_node( silc_profile_node* node )
{
    SILC_ASSERT( node != NULL );

    silc_profile_node* parent = node->parent;
    silc_profile_node* before = NULL;

    /* Obtain start of the siblings list of node */
    if ( parent == NULL )
    {
        before = silc_profile.first_root_node;
    }
    else
    {
        before = parent->first_child;
    }

    /* If node is the first entry */
    if ( before == node )
    {
        if ( parent == NULL )
        {
            silc_profile.first_root_node = node->next_sibling;
        }
        else
        {
            parent->first_child = node->next_sibling;
        }
        node->parent       = NULL;
        node->next_sibling = NULL;
        return;
    }

    /* Else search for the sibling before node. */
    while ( ( before != NULL ) && ( before->next_sibling != node ) )
    {
        before = before->next_sibling;
    }

    /* Node is already removed */
    if ( before == NULL )
    {
        SILC_DEBUG_PRINTF( SILC_DEBUG_PROFILE,
                           "Trying to remove a node which is not contained in the siblings "
                           "list.\nMaybe an inconsistent profile." );
        node->parent       = NULL;
        node->next_sibling = NULL;
        return;
    }

    /* Remove node from list */
    before->next_sibling = node->next_sibling;
    node->parent         = NULL;
    node->next_sibling   = NULL;
}
