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

#ifndef SCOREP_OMPT_CALLBACKS_TARGET_H
#define SCOREP_OMPT_CALLBACKS_TARGET_H


void
scorep_ompt_cb_device_initialize( int                    device_num,
                                  const char*            type,
                                  ompt_device_t*         device,
                                  ompt_function_lookup_t lookup,
                                  const char*            documentation );

void
scorep_ompt_cb_device_finalize( int device_num );

void
scorep_ompt_cb_target_emi( ompt_target_t         kind,
                           ompt_scope_endpoint_t endpoint,
                           int                   device_num,
                           ompt_data_t*          task_data,
                           ompt_data_t*          target_task_data,
                           ompt_data_t*          target_data,
                           const void*           codeptr_ra );

void
scorep_ompt_cb_target_data_op_emi( ompt_scope_endpoint_t endpoint,
                                   ompt_data_t*          target_task_data,
                                   ompt_data_t*          target_data,
                                   ompt_id_t*            host_op_id,
                                   ompt_target_data_op_t optype,
                                   void*                 src_addr,
                                   int                   src_device_num,
                                   void*                 dest_addr,
                                   int                   dest_device_num,
                                   size_t                bytes,
                                   const void*           codeptr_ra );

void
scorep_ompt_cb_target_submit_emi( ompt_scope_endpoint_t endpoint,
                                  ompt_data_t*          target_data,
                                  ompt_id_t*            host_op_id,
                                  unsigned int          requested_num_teams );

#endif /* SCOREP_OMPT_CALLBACKS_TARGET_H */
