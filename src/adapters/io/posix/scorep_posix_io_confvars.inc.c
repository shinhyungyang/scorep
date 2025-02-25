/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2025,
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


static bool posix_io_enable;


/*
 *  Configuration variables for the POSIX I/O adapter.
 */
static const SCOREP_ConfigVariable scorep_posix_io_confvars[] = {
    {
        "posix",
        SCOREP_CONFIG_TYPE_BOOL,
        &posix_io_enable,
        NULL,
        "false",
        "POSIX I/O, POSIX async I/O, ISO C I/O",
        ""
    },
    SCOREP_CONFIG_TERMINATOR
};
