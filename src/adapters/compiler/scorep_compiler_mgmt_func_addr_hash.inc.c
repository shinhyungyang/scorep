/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022, 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 */

#include <SCOREP_Addr2line.h>
#include <SCOREP_RuntimeManagement.h>
#include <UTILS_Error.h>

/* To prevent undefined reference linker errors during make check in
   the pure static case, this dlcose callback needs to live in
   compiler_mgmt, even if, in a future version, it is to use functions
   from scorep_compiler_event_func_addr_hash.inc.c. */
static void
func_addr_hash_dlclose_cb( void*       soHandle,
                           const char* soFileName,
                           uintptr_t   soBaseAddr,
                           uint16_t    soToken )
{
    UTILS_WARNING( "Shared object %s was dlclosed. It's addresses are not "
                   "removed from the compiler address hash table. If another "
                   "shared object is dlopened and addressees are reused, the "
                   "compiler adapter might enter/exit regions referring to "
                   "wrong source code locations", soFileName );
}

static void
func_addr_hash_register_obj_close_cb( void )
{
    if ( !SCOREP_IsUnwindingEnabled() )
    {
        SCOREP_Addr2line_RegisterObjcloseCb( func_addr_hash_dlclose_cb );
    }
}
