/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023,
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

#ifndef SCOREP_OMPT_CONFVARS_H
#define SCOREP_OMPT_CONFVARS_H

/*
 * OMPT target features (to be enabled/disabled via environment variables)
 */
#define SCOREP_OMPT_TARGET_FEATURE_KERNEL          ( 1 << 0 )
#define SCOREP_OMPT_TARGET_FEATURE_MEMORY          ( 1 << 1 )
#define SCOREP_OMPT_TARGET_FEATURES_DEFAULT \
    ( SCOREP_OMPT_TARGET_FEATURE_KERNEL | SCOREP_OMPT_TARGET_FEATURE_MEMORY )

/*
 * Specifies the OMPT target tracing mode with a bit mask.
 * See SCOREP_ConfigType_SetEntry of OMPT adapter.
 */
extern uint64_t scorep_ompt_target_features;
extern bool     scorep_ompt_target_use_device_tracing;
extern size_t   scorep_ompt_target_buffer_chunk_size;

#endif // SCOREP_OMPT_CONFVARS_H
