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


#ifndef SILC_COMPILER_INIT_H_
#define SILC_COMPILER_INIT_H_


/**
 * @brief Compiler adapter interface support to the measurement system
 */

#include <stdio.h>

#include "SILC_Adapter.h"


/**
 * @brief Hash table to map function addresses to region identifier
 * identifier is called region handle
 *
 * @param id          hash key (address of function)
 * @param name        associated function name
 * @param fname       file name
 * @param lnobegin    line number of begin of function
 * @param lnoend      line number of end of function
 * @param reghandle   region identifier
 * @param HN          pointer to next element
 */
typedef struct HashNode
{
    long              id;
    const char*       name;
    const char*       fname;
    SILC_LineNo       lnobegin;
    SILC_LineNo       lnoend;
    SILC_RegionHandle reghandle;
    struct HashNode*  next;
} HashNode;



extern void
silc_compiler_register_region
(
    HashNode* hn
);


SILC_Error_Code
silc_compiler_register
(
)
{
    printf( " register compiler adapter! \n" );
    return SILC_SUCCESS;
}

SILC_Error_Code
silc_compiler_init_adapter
    ()
{
    printf( " init compiler adapter! \n" );
    return SILC_SUCCESS;
}

SILC_Error_Code
silc_compiler_init_location
(
)
{
    printf( " compiler adapter init loacation! \n" );
    return SILC_SUCCESS;
}

void
silc_compiler_final_location
    ()
{
    printf( " compiler adapter final loacation! \n" );
}

void
silc_compiler_finalize
    ()
{
    printf( " compiler adapter finalize! \n" );
}

void
silc_compiler_deregister
    ()
{
    printf( " compiler adapter deregister! \n" );
}



/**
 * Initialize the adapter structure for compilers
 */
const SILC_Adapter SILC_Compiler_Adapter =
{
    SILC_ADAPTER_COMPILER,
    "COMPILER",
    &silc_compiler_register,
    &silc_compiler_init_adapter,
    &silc_compiler_init_location,
    &silc_compiler_final_location,
    &silc_compiler_finalize,
    &silc_compiler_deregister
};



/**
 * Compiler adapter finalizer
 */
extern void
( *silc_comp_finalize )
(
    void
);


#endif /* SILC_COMPILER_INIT_H_ */
