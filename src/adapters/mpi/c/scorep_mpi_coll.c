/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#include <config.h>

#include <scorep_mpi_coll.h>

#include <stdbool.h>

static inline bool
is_intracomm( MPI_Comm comm )
{
    int is_intercomm;
    PMPI_Comm_test_inter( comm, &is_intercomm );
    return !is_intercomm;
}

static inline int
is_cart_topo( MPI_Comm comm )
{
    int topo_type = MPI_UNDEFINED;
    PMPI_Topo_test( comm, &topo_type );
    return topo_type == MPI_CART;
}


#define COUNT_T int
#define COUNT_FUN( name ) name
#define TYPE_SIZE_FUN PMPI_Type_size

#include "scorep_mpi_coll.inc.c"

#undef TYPE_SIZE_FUN
#undef COUNT_T
#undef COUNT_FUN

/*
 * Large counts
 */
#if HAVE( MPI_4_0_SYMBOL_PMPI_TYPE_SIZE_C )
#define TYPE_SIZE_FUN PMPI_Type_size_c
#elif HAVE( MPI_3_0_SYMBOL_PMPI_TYPE_SIZE_X )
#define TYPE_SIZE_FUN PMPI_Type_size_x
#endif

#if defined( TYPE_SIZE_FUN )

#define COUNT_T MPI_Count
#define COUNT_FUN( name ) name ##_c

#include "scorep_mpi_coll.inc.c"

#undef COUNT_T
#undef COUNT_FUN

#endif
