/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2012-2014,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * @brief  The Score-P GCC plugin definition.
 *
 */

#ifndef SCOREP_PLUGIN_H
#define SCOREP_PLUGIN_H

#ifdef __cplusplus
extern "C"
{
#endif

extern int scorep_plugin_verbosity;
extern int scorep_plugin_symbol_verbosity;
extern int scorep_plugin_disable_runtime_filter;

extern unsigned int
scorep_plugin_pass_instrument_function( void );

extern struct attribute_spec scorep_no_instrument_attribute;

extern void
scorep_register_attributes( void* event_data,
                            void* data );

#ifdef __cplusplus
}
#endif

#endif /* SCOREP_PLUGIN_H */
