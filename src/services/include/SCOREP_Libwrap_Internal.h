/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2013-2015, 2017, 2020, 2025,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */


/**
 * @file
 *
 *
 */


#ifndef SCOREP_LIBWRAP_INTERNAL_H
#define SCOREP_LIBWRAP_INTERNAL_H


#include <scorep/SCOREP_Libwrap_Macros.h>
#include <scorep/SCOREP_Libwrap.h>


/**
 * Create a library wrapper object based on the attributes structure.
 *
 * @param[out] handle       Score-P library wrapper object
 * @param      attributes   Attributes of the Score-P library wrapper object
 */
void
SCOREP_Libwrap_Create( SCOREP_LibwrapHandle**          handle,
                       const SCOREP_LibwrapAttributes* attributes );


/**
 * Call this function to enable wrapping of one function.
 *
 * @param handle             Score-P library wrapper object
 * @param prettyName         Region display name (i.e., demangled)
 * @param symbolName         Symbol name (i.e., mangled)
 * @param file               Source file name
 * @param line               Line number in source file
 * @param wrapper            Function address of the wrapper
 * @param[out] funcPtr       Pointer to the function address of the actual/wrappee
 * @param[out] regionHandle  Pointer to the region handle
 *
 * @return                   @see SCOREP_LibwrapEnableErrorCode
 */
SCOREP_LibwrapEnableErrorCode
SCOREP_Libwrap_EnableWrapper( SCOREP_LibwrapHandle* handle,
                              const char*           prettyName,
                              const char*           symbolName,
                              const char*           file,
                              int                   line,
                              SCOREP_ParadigmType   paradigm,
                              SCOREP_RegionType     regionType,
                              void*                 wrapper,
                              void**                funcPtr,
                              SCOREP_RegionHandle*  regionHandle );


#endif /* SCOREP_LIBWRAP_INTERNAL_H */
