/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
#include <config.h>
#include <SCOREP_Definitions.h>


SCOREP_IoFileHandle
SCOREP_IoHandleHandle_GetIoFile_fromF08( SCOREP_IoHandleHandle handle )
{
    return SCOREP_IoHandleHandle_GetIoFile( handle );
}
