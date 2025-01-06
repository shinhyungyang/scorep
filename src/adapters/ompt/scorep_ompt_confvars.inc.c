/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */
#include <config.h>

#include <SCOREP_Config.h>

#include "scorep_ompt_confvars.h"

/* Confvars */
uint64_t scorep_ompt_target_features = 0;
bool     scorep_ompt_target_use_device_tracing;
size_t   scorep_ompt_target_buffer_chunk_size;

/*
 * Mapping of options for OMPT measurement to internal representation
 * (bit mask).
 */
static const SCOREP_ConfigType_SetEntry ompt_target_enable_groups[] =
{
    {
        "kernel",
        SCOREP_OMPT_TARGET_FEATURE_KERNEL,
        "Enable collection of OpenMP target kernel events. On the host side, this "
        "includes events related to launching kernels, i.e. !$omp target and"
        "!$omp target submit. If the device tracing interface is available,"
        "this also includes accelerator events, i.e. kernel executions."
    },
    {
        "memory",
        SCOREP_OMPT_TARGET_FEATURE_MEMORY,
        "Enable collection of OpenMP target memory events. This includes both events "
        "related to data transfers, e.g. OpenMP directives invoking data transfers like "
        "!$omp target enter data, and allocations, e.g. done by OpenMP runtime calls like "
        "omp_target_alloc."
    },
    {
        "default/yes/1",
        SCOREP_OMPT_TARGET_FEATURES_DEFAULT,
        "Enable all OpenMP target features."
    },
    { NULL, 0, NULL }
};


/*
 *  Configuration variables for the OMPT adapter.
 */

// Host side options, not yet implemented
static SCOREP_ConfigVariable scorep_ompt_confvars[] =
{
    SCOREP_CONFIG_TERMINATOR
};


// Target side options
static SCOREP_ConfigVariable scorep_ompt_target_confvars[] =
{
    {
        "target_enable",
        SCOREP_CONFIG_TYPE_BITSET,    /* type */
        &scorep_ompt_target_features, /* pointer to target variable */
        ( void* )ompt_target_enable_groups,
        "yes",                        /* default value */
        "OpenMP target measurement features",
        "Sets the OpenMP target measurement mode to capture."
    },
    {
        "target_buffer_chunk_size",
        SCOREP_CONFIG_TYPE_SIZE,
        &scorep_ompt_target_buffer_chunk_size,
        NULL,
        "8k",
        "Chunk size in bytes for the OMPT target trace record buffer.",
        ""
    },
    {
        "target_device_tracing_enable",
        SCOREP_CONFIG_TYPE_BOOL,
        &scorep_ompt_target_use_device_tracing,
        NULL,
        "yes",
        "Usage of OMPT device tracing interface for recording accelerator events",
        "Enable/Disable the usage of the device tracing interface to collect "
        "accelerator events. If disabled, only host events will be recorded. "
        "Accelerator events can still be collected using a native accelerator adapter, "
        "such as CUDA."
    },
    SCOREP_CONFIG_TERMINATOR
};
