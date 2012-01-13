/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
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

#ifndef SCOREP_PROFILE_METRIC_H
#define SCOREP_PROFILE_METRIC_H

/**
 * @file        scorep_profile_metric.h
 * @maintainer  Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief Metric handling in the profile.
 *
 */

#include <stdint.h>

#include "SCOREP_Types.h"

/* ***************************************************************************************
   Typedefs
*****************************************************************************************/

/**
   Contains the data for one dense metric. The number of dense metrics is assumed to be
   the same for each nodes. Furthermore, the order of the metrics is assumed to be in the
   same order for each node. Dense metrics are assumed to be triggered on each enter and
   exit event, thus, they are not counted for each metric separately, but once for each
   node. It also contains to metric definition data, because it can be stored globally.
   Recorded are the difference of the values between exit and enter.
 */
typedef struct
{
    uint64_t sum;
    uint64_t min;
    uint64_t max;
    uint64_t squares;
    uint64_t start_value;
} scorep_profile_dense_metric;

/**
   Contains the data for a sparse metric of integer values. The sparse metrics are assumed
   to occur only in few callpathes. Thus, they are stored as single linked list for each
   node and must contain their metric definition handle.
 */
typedef struct scorep_profile_sparse_metric_int_struct
{
    SCOREP_MetricHandle                             metric;
    uint64_t                                        count;
    uint64_t                                        sum;
    uint64_t                                        min;
    uint64_t                                        max;
    uint64_t                                        squares;
    struct scorep_profile_sparse_metric_int_struct* next_metric;
} scorep_profile_sparse_metric_int;

/**
   Contains the data for a sparse metric of double values. The sparse metrics are assumed
   to occur only in few callpathes. Thus, they are stored as single linked list for each
   node and must contain their metric definition handle.
 */
typedef struct scorep_profile_sparse_metric_double_struct
{
    SCOREP_MetricHandle                                metric;
    uint64_t                                           count;
    double                                             sum;
    double                                             min;
    double                                             max;
    double                                             squares;
    struct scorep_profile_sparse_metric_double_struct* next_metric;
} scorep_profile_sparse_metric_double;

/* ***************************************************************************************
   Functions for dense metrics
*****************************************************************************************/

/**
 *  Initializes a dense metric. The space for dense metrics is allocated as a block
 *  appended to the scorep_profile_node structure. This function initialized a already
 *  reserved piece of memory as dense metric.
 *  @param metric pointer to the dense metric instance which should be initialized.
 */
extern void
scorep_profile_init_dense_metric( scorep_profile_dense_metric* metric );

/**
 *  Updates the statistics of one dense metric on an exit event. It assumes that the start
 *  value of the counter is already stored @a metric->start_value. The difference between
 *  @a end_value and @a metric->start_value is added to the recoreded statistics.
 *  @param metric    Pointer to the dense metric instance which should be updated.
 *  @param end_value The end metrics value of the intervall which is added to the
 *                   statistics.
 */
extern void
scorep_profile_update_dense_metric( scorep_profile_dense_metric* metric,
                                    uint64_t                     end_value );

/**
 *  Copies the value of a dense metric to another dense metric. Writes the content from
 *  @a source to @a destination.
 *  @param destination Pointer to the instance to which vales are overwritten.
 *  @param source      Pointer to the instance which valies are copied to destination.
 */
extern void
scorep_profile_copy_dense_metric( scorep_profile_dense_metric* destination,
                                  scorep_profile_dense_metric* source );

/**
 *  Merges the statistics of a dense metrics to another metric. Adds the statistics from
 *  @a source to @a destination as if all events summarized in @a source would have been
 *  also occured for @destination.
 *  @param destination Pointer to the instance to which vales are updated.
 *  @param source      Pointer to the instance which contains the values which are added
 *                     to @a destination.
 */
extern void
scorep_profile_merge_dense_metric( scorep_profile_dense_metric* destination,
                                   scorep_profile_dense_metric* source );

/* ***************************************************************************************
   Functions for sparse integer metrics.
*****************************************************************************************/

/**
 * Creates a new sparse metric struct instance for integer values. The memory for the
 * new metric is allocated from the profiling pool. The new instance is initialized with
 * the given first metric sample.
 * @param metric The handle of the metric definition for the recorded values.
 * @param value  The first sample for the recorded statistics.
 */
scorep_profile_sparse_metric_int*
scorep_profile_create_sparse_int(
    SCOREP_MetricHandle metric,
    uint64_t            value );

/**
 * Copy constructor for sparse metric for integer values. It allocates memory for a new
 * instance and copies the content of @a source into the new instance.
 * @param metric Pointer to the instance from which the content is copied.
 * @return the newly created instance.
 */
scorep_profile_sparse_metric_int*
scorep_profile_copy_sparse_int( scorep_profile_sparse_metric_int* source );

/**
 * Updates a sparse metric struct instance for integer values. The given @a value is
 * added to the statistics of @a metric.
 * @param metric Pointer to the metric instance for which the new value is recorded.
 * @param value  The sample of the metric which is added to the statistics.
 */
void
scorep_profile_update_sparse_int( scorep_profile_sparse_metric_int* metric,
                                  uint64_t                          value );

/**
 *  Merges the statistics of a metric to another metric. Adds the statistics from
 *  @a source to @a destination as if all events summarized in @a source would have been
 *  also occured for @destination.
 *  @param destination Pointer to the instance to which vales are updated.
 *  @param source      Pointer to the instance which contains the values which are added
 *                     to @a destination.
 */
void
scorep_profile_merge_sparse_metric_int( scorep_profile_sparse_metric_int* destination,
                                        scorep_profile_sparse_metric_int* source );

/* ***************************************************************************************
   Functions for sparse double metrics.
*****************************************************************************************/

/**
 * Creates a new sparse metric struct instance for double values. The memory for the
 * new metric is allocated from the profiling pool. The new instance is initialized with
 * the given first metric sample.
 * @param metric The handle of the metric definition for the recorded values.
 * @param value  The first sample for the recorded statistics.
 */
scorep_profile_sparse_metric_double*
scorep_profile_create_sparse_double(
    SCOREP_MetricHandle metric,
    double              value );

/**
 * Copy constructor for sparse metric for double values. It allocates memory for a new
 * instance and copies the content of @a source into the new instance.
 * @param metric Pointer to the instance from which the content is copied.
 * @return the newly created instance.
 */
scorep_profile_sparse_metric_double*
scorep_profile_copy_sparse_double( scorep_profile_sparse_metric_double* source );

/**
 * Updates a sparse metric struct instance for double values. The given @a value is
 * added to the statistics of @a metric.
 * @param metric Pointer to the metric instance for which the new value is recorded.
 * @param value  The sample of the metric which is added to the statistics.
 */
void
scorep_profile_update_sparse_double( scorep_profile_sparse_metric_double* metric,
                                     uint64_t                             value );

/**
 *  Merges the statistics of a metric to another metric. Adds the statistics from
 *  @a source to @a destination as if all events summarized in @a source would have been
 *  also occured for @destination.
 *  @param destination Pointer to the instance to which vales are updated.
 *  @param source      Pointer to the instance which contains the values which are added
 *                     to @a destination.
 */
void
scorep_profile_merge_sparse_metric_double( scorep_profile_sparse_metric_double* destination,
                                           scorep_profile_sparse_metric_double* source );


#endif // SCOREP_PROFILE_METRIC_H
