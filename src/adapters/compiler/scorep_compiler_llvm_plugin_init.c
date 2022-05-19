/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2012-2013, 2015-2016, 2020,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2015,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * @brief Support for LLVM-Compiler
 * Will be triggered by the '-Xclang load -Xclang <scorep-llvm-plugin>.so' flag of the LLVM/Clang compiler.
 */

#include <config.h>

#define SCOREP_DEBUG_MODULE_NAME COMPILER
#include <UTILS_Debug.h>

#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_Filtering.h>

#include "SCOREP_Compiler_Init.h"
#include "scorep_compiler_instrumentation_plugin.h"

/****************************************************************************************
   Adapter management
 *****************************************************************************************/

void
scorep_compiler_register_region( const scorep_compiler_region_description* regionDescr )
{
    /*
     * If unwinding is enabled, we filter out all regions.
     */
    if ( SCOREP_IsUnwindingEnabled()
         || SCOREP_Filtering_Match( regionDescr->file,
                                    regionDescr->name,
                                    regionDescr->canonical_name ) )
    {
        *regionDescr->handle = SCOREP_FILTERED_REGION;
        return;
    }

    *regionDescr->handle =
        SCOREP_Definitions_NewRegion( regionDescr->name,
                                      regionDescr->canonical_name,
                                      SCOREP_Definitions_NewSourceFile(
                                          regionDescr->file ),
                                      regionDescr->begin_lno,
                                      regionDescr->end_lno,
                                      SCOREP_PARADIGM_COMPILER,
                                      SCOREP_REGION_FUNCTION );

    UTILS_DEBUG( "registered %s:%d-%d:%s: \"%s\"",
                 regionDescr->file,
                 regionDescr->begin_lno,
                 regionDescr->end_lno,
                 regionDescr->canonical_name,
                 regionDescr->name );
}

SCOREP_ErrorCode
scorep_compiler_subsystem_init( void )
{
    UTILS_DEBUG( "initialize LLVM plugin compiler adapter" );

    /* Initialize region mutex */
    SCOREP_MutexCreate( &scorep_compiler_region_mutex );

    // NDAO: Check here again whether to add  /* Initialize plugin instrumentation */ code or not
    //       Why is it not added in the old plugin ?!

    return SCOREP_SUCCESS;
}

SCOREP_ErrorCode
scorep_compiler_subsystem_begin( void )
{
    UTILS_DEBUG( "start LLVM plugin compiler adapter" );

    return SCOREP_SUCCESS;
}

void
scorep_compiler_subsystem_end( void )
{
    UTILS_DEBUG( "stop LLVM plugin compiler adapter" );
}

/* Adapter finalization */
void
scorep_compiler_subsystem_finalize( void )
{
    UTILS_DEBUG( "finalize LLVM plugin compiler adapter" );

    /* Delete region mutex */
    SCOREP_MutexDestroy( &scorep_compiler_region_mutex );
}

SCOREP_ErrorCode
scorep_compiler_subsystem_init_location( struct SCOREP_Location* locationData,
                                         struct SCOREP_Location* parent )
{
    UTILS_DEBUG( "initialize location in LLVM plugin compiler adapter" );

    return SCOREP_SUCCESS;
}
