/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2012,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2012,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2012,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2012,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2012,
 * Technische Universitaet Muenchen, Germany
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

/**
   Contains the file name for the selective recording configuration file. This file name
   is filled out by the configuration system. In particular, it is set by the environment
   variable 'SCOREP_SELECTIVE_CONFIG_FILE'.
 */
static char* scorep_selective_file_name;

/**
   Array of configuration variables for the selective recording.
 */
SCOREP_ConfigVariable scorep_selective_configs[] = {
    {
        "config_file",
        SCOREP_CONFIG_TYPE_STRING,
        &scorep_selective_file_name,
        NULL,
        "",
        "A file name which configures selective recording",
        ""
    },
    SCOREP_CONFIG_TERMINATOR
};
