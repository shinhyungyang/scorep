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


static char* libwrap_confvar_path;
static char* libwrap_confvar_enable;
static char* libwrap_confvar_enable_sep;


/*
 *  Configuration variables for the libwrap service.
 */
static const SCOREP_ConfigVariable scorep_libwrap_confvars[] = {
    {
        "path",
        SCOREP_CONFIG_TYPE_STRING,
        &libwrap_confvar_path,
        NULL,
        "",
        "Search path for user library wrapper plug-ins.",
        "Colon-separated list of directories to search for user library wrapper "
        "plug-ins. The installation of Score-P is appended implicitly to the end."
    },
    {
        "enable",
        SCOREP_CONFIG_TYPE_STRING,
        &libwrap_confvar_enable,
        NULL,
        "",
        "Library wrapper plug-ins to load and enable.",
        "List of user library wrapper plug-ins or absolute path to a user library "
        "wrapper plug-ins. Either a full path to a plug-in or a library wrapper "
        "name to search for in Score-P's installation or in `SCOREP_LIBWRAP_PATH`. "
        "`SCOREP_LIBWRAP_ENABLE_SEP` is used as separators."
    },
    {
        "enable_sep",
        SCOREP_CONFIG_TYPE_STRING,
        &libwrap_confvar_enable_sep,
        NULL,
        ",",
        "Separators for list of library wrapper plug-ins to load and enable.",
        "Characters that delimits plug-in names in `SCOREP_LIBWRAP_ENABLE`."
    },
    SCOREP_CONFIG_TERMINATOR
};
