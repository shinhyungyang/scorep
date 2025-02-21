/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2014, 2017, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2014,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

#include <config.h>

#include <stdio.h>

#include "foo.h"

#define SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC_SPECIFIER static
#define SCOREP_LIBWRAP_DECLARE_REAL_FUNC_SPECIFIER    static

#include <scorep/SCOREP_Libwrap.h>
#include <scorep/SCOREP_Libwrap_Macros.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Libwrap_Internal.h>


SCOREP_LIBWRAP_DECLARE_WRAPPER_FUNC( ( void ), foo, ( void ) );
SCOREP_LIBWRAP_DECLARE_REAL_FUNC( ( void ), foo, ( void ) );

// region handles

static SCOREP_RegionHandle
SCOREP_LIBWRAP_REGION_HANDLE( foo );

static void
library_wrapper_init( SCOREP_LibwrapHandle* lw )
{
    SCOREP_LIBWRAP_FUNC_INIT( lw, foo, "foo()", "example.h", 1 );
}

/* Library wrapper object */
static SCOREP_LibwrapHandle* lw = SCOREP_LIBWRAP_NULL;

/* Library wrapper attributes */
static const SCOREP_LibwrapAttributes lw_attr =
{
    SCOREP_LIBWRAP_VERSION,
    "foo",
    "Foo",
    library_wrapper_init,
};

/*
 * Function wrapper
 */
void
SCOREP_LIBWRAP_FUNC_NAME( foo )( void )
{
    SCOREP_LIBWRAP_ENTER_MEASUREMENT();
    printf( "Wrapped function 'foo'\n" );

    SCOREP_LIBWRAP_INIT( lw, lw_attr );

    SCOREP_LIBWRAP_FUNC_ENTER( foo );

    SCOREP_LIBWRAP_ENTER_WRAPPED_REGION();
    SCOREP_LIBWRAP_FUNC_CALL( foo, ( ) );
    SCOREP_LIBWRAP_EXIT_WRAPPED_REGION();

    SCOREP_LIBWRAP_FUNC_EXIT( foo );

    SCOREP_LIBWRAP_EXIT_MEASUREMENT();
}
