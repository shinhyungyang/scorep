/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2022-2023,
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

#include <config.h>

#include "scorep_ompt.h"

#include <SCOREP_Definitions.h>

#include "scorep_ompt_debug.inc.c"

#include "scorep_ompt_regions.inc.c"
#include "scorep_ompt_mutex.inc.c"
#include "scorep_ompt_events_host.inc.c"
#include "scorep_ompt_events_device.inc.c"
#include "scorep_ompt_events_device_id_mapping.inc.c"
#include "scorep_ompt_events_device_tracing.inc.c"
#include "scorep_ompt_events_device_target.inc.c"
#include "scorep_ompt_events_device_data_op.inc.c"
#include "scorep_ompt_events_device_submit.inc.c"
#include "scorep_ompt_events_device_callbacks.inc.c"
