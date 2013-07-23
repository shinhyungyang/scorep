/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 *    RWTH Aachen University, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene, USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */


/**
 * @file       src/adapters/cuda/scorep_cuda_confvars.inc.c
 * @maintainer Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @status alpha
 *
 */



uint64_t scorep_cuda_features;
size_t   scorep_cupti_activity_buffer_size;


/*
 * Mapping of options for CUDA measurement to internal representation
 * (bit mask).
 */
static const SCOREP_ConfigType_SetEntry scorep_cuda_enable_groups[] = {
    { "runtime",        SCOREP_CUDA_FEATURE_RUNTIME_API           },
    { "driver",         SCOREP_CUDA_FEATURE_DRIVER_API            },
    { "kernel",         SCOREP_CUDA_FEATURE_KERNEL                },
    { "kernel_counter", SCOREP_CUDA_FEATURE_KERNEL_COUNTER        },
    { "concurrent",     SCOREP_CUDA_FEATURE_CONCURRENT_KERNEL     },
    { "memcpy",         SCOREP_CUDA_FEATURE_MEMCPY                },
    { "sync",           SCOREP_CUDA_FEATURE_SYNC                  },
    { "idle",           SCOREP_CUDA_FEATURE_IDLE                  },
    { "pure_idle",      SCOREP_CUDA_FEATURE_PURE_IDLE             },
    { "device_reuse",   SCOREP_CUDA_FEATURE_DEVICE_REUSE          },
    { "stream_reuse",   SCOREP_CUDA_FEATURE_STREAM_REUSE          },
    { "gpumemusage",    SCOREP_CUDA_FEATURE_GPUMEMUSAGE           },
    { "DEFAULT",        SCOREP_CUDA_FEATURES_DEFAULT              },
    { "1",              SCOREP_CUDA_FEATURES_DEFAULT              },
    { "yes",            SCOREP_CUDA_FEATURES_DEFAULT              },
    { "no",             0                                         },
    { NULL,             0                                         }
};


/*
 *  Configuration variables for the CUDA adapter.
 */
static SCOREP_ConfigVariable scorep_cuda_configs[] = {
    {
        "enable",
        SCOREP_CONFIG_TYPE_BITSET, /* type */
        &scorep_cuda_features,     /* pointer to target variable */
        ( void* )scorep_cuda_enable_groups,
        "no",                      /* default value */
        "CUDA measurement features",
        "Sets the CUDA measurement mode to capture:\n"
        "  runtime:        CUDA runtime API\n"
        "  driver:         CUDA driver API\n"
        "  kernel:         CUDA kernels\n"
        "  kernel_counter: Fixed CUDA kernel metrics.\n"
        "  concurrent:     Concurrent kernel recording.\n"
        "  idle:           GPU compute idle time\n"
        "  pure_idle:      GPU idle time (memory copies are not idle)\n"
        "  memcpy:         CUDA memory copies\n"
        "  sync:           Record implicit and explicit CUDA synchronization\n"
        "  gpumemusage:    Record CUDA memory (de)allocations as a counter\n"
        "  stream_reuse:   Reuse destroyed/closed CUDA streams\n"
        "  device_reuse:   Reuse destroyed/closed CUDA devices\n"
        "  default/yes/1:  CUDA runtime API and GPU activities\n"
        "  no:             Disable CUDA measurement\n"
    },
    {
        "buffer",
        SCOREP_CONFIG_TYPE_SIZE,
        &scorep_cupti_activity_buffer_size,
        NULL,
        "64k",
        "Total memory in bytes for the CUDA record buffer",
        ""
    },
    SCOREP_CONFIG_TERMINATOR
};
