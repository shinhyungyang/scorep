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

#define SCOREP_LIBWRAP_DECLARE_WRAPPER_SPECIFIER         static
#define SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE_SPECIFIER static

#include <scorep/SCOREP_Libwrap.h>

#define SCOREP_LIBWRAP_API( api ) scorep_libwrap_plugin_api.api

#include <scorep/SCOREP_Libwrap_Plugins.h>
#include <scorep/SCOREP_Libwrap_Macros.h>

#include <SCOREP_Definitions.h>
#include <SCOREP_RuntimeManagement.h>


extern const SCOREP_LibwrapAPI scorep_libwrap_plugin_api;


SCOREP_LIBWRAP_DECLARE_ORIGINAL_TYPE( ( void ), foo, ( void ) );
SCOREP_LIBWRAP_DECLARE_WRAPPER( foo );
SCOREP_LIBWRAP_DECLARE_ORIGINAL_HANDLE( foo );


// region handles

static SCOREP_RegionHandle
SCOREP_LIBWRAP_REGION_HANDLE( foo );

static void
library_wrapper_init( SCOREP_LibwrapHandle* lw )
{
    SCOREP_LIBWRAP_FUNC_REGISTER( lw, foo, "foo()", "example.h", 1 );
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
SCOREP_LIBWRAP_WRAPPER( foo )( void )
{
    SCOREP_LIBWRAP_ENTER_MEASUREMENT();
    printf( "Wrapped function 'foo'\n" );

    SCOREP_LIBWRAP_INIT( lw, lw_attr );

    SCOREP_LIBWRAP_FUNC_ENTER( foo );

    SCOREP_LIBWRAP_ENTER_WRAPPED_REGION();
    SCOREP_LIBWRAP_ORIGINAL( foo )();
    SCOREP_LIBWRAP_EXIT_WRAPPED_REGION();

    SCOREP_LIBWRAP_FUNC_EXIT( foo );

    SCOREP_LIBWRAP_EXIT_MEASUREMENT();
}
