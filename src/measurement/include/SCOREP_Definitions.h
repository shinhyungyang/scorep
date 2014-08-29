/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2014,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2013,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


#ifndef SCOREP_DEFINITIONS_H
#define SCOREP_DEFINITIONS_H


/**
 * @file
 *
 * @brief Declaration of definition functions to be used by the adapter layer.
 *
 */


#include <stdlib.h>
#include "SCOREP_DefinitionHandles.h"
#include "SCOREP_Types.h"
#include <scorep/SCOREP_PublicTypes.h>
#include <scorep/SCOREP_MetricTypes.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * @defgroup SCOREP_Definitions SCOREP Definitions
 *
 * - Before calling one of the @ref SCOREP_Events event functions the adapter
     needs to define (i.e. register) entities like regions to be used in
     subsequent definition calls or event function calls.
 *
 * - The definition function calls return opaque handles. The adapter must not
     make any assumptions on the type or the operations that are allowed on
     this type.
 *
 * - A call to a definition function creates internally a process local
     definition. All these process local definitions are unified at the end of
     the measurement.
 *
 * - Definitions are stored per process (as opposed to per loccation) in the
     measurement system. This renders storing of redundant information
     unnecessary and reduces unification expense.
 *
 * - Note that calls to the definition functions need to be synchronized. See
     SCOREP_DefinitionLocking.h for the synchronization interface.
 *
 * - Note that the MPI definition functions also return handles now if
     appropriate (they previously returned void what caused some troubles
     during unification).
 *
 * - Note that the definition functions don't check for uniqueness of their
     arguments but create a new handle for each call. Checking for uniqueness
     is the responsibility of the adapter. Uniqueness of argument tuples is
     required for unification.
 *
 */
/*@{*/


SCOREP_StringHandle
SCOREP_Definitions_NewString( const char* str );


const char*
SCOREP_StringHandle_Get( SCOREP_StringHandle handle );


/**
 * Associate a file name with a process unique file handle.
 *
 * @param fileName A meaningful name for the source file.
 *
 * @return A process unique file handle to be used in calls to
 * SCOREP_Definitions_NewRegion().
 *
 */
SCOREP_SourceFileHandle
SCOREP_Definitions_NewSourceFile( const char* fileName );


const char*
SCOREP_SourceFileHandle_GetName( SCOREP_SourceFileHandle handle );


/**
 * Associate a system tree node with a process unique system tree node handle.
 *
 * @param parent Parent node in system tree.
 *
 * @param domains Bit set of domains this node spans.
 *
 * @param klass Class of the system tree node.
 *
 * @param name A meaningful name for the system tree node.
 *
 * @return A process unique system tree node handle.
 *
 */
SCOREP_SystemTreeNodeHandle
SCOREP_Definitions_NewSystemTreeNode( SCOREP_SystemTreeNodeHandle parent,
                                      SCOREP_SystemTreeDomain     domains,
                                      const char*                 klass,
                                      const char*                 name );


/**
 * Adds a property to the an already existing sytem tree node.
 */
void
SCOREP_Defininitions_AddSystemTreeNodeProperty( SCOREP_SystemTreeNodeHandle systemTreeNodeHandle,
                                                const char*                 propertyName,
                                                const char*                 propertyValue );

/**
 * Associate a code region with a process unique file handle.
 *
 * @param regionName A meaningful name for the region, e.g. a function
 * name. The string will be copied.
 *
 * @param regionCanonicalName An canonical name for the region, e.g. a
 * mangled name. The string will be copied. If this parameter is
 * @code{NULL}, content of @a regionName will be used.
 *
 * @param fileHandle A previously defined SCOREP_SourceFileHandle or
 * SCOREP_INVALID_SOURCE_FILE.
 *
 * @param beginLine The file line number where the region starts or
 * SCOREP_INVALID_LINE_NO.
 *
 * @param endLine The file line number where the region ends or
 * SCOREP_INVALID_LINE_NO.
 *
 * @param paradigm The paradigm type of adapter (SCOREP_ParadigmType)
 * that is calling.
 *
 * @param regionType The type of the region. Until now, the @a regionType was
 * not used during the measurement but during analysis. This @e may change in
 * future with e.g. dynamic regions or parameter based profiling. In the first
 * run, we can implement at least dynamic regions in the adapter.
 *
 * @note The name of e.g. Java classes, previously provided as a string to the
 * region description, should now be encoded in the region name. The region
 * description field is replaced by the adapter type as that was it's primary
 * use.
 *
 * @note During unification, we compare @a regionName, @a fileHandle, @a
 * beginLine, @a endLine and @a adapter of regions from different
 * processes. If all values are equal, we consider the regions to be equal. We
 * don't check for uniqueness of this tuple in this function, i.e. during
 * measurement, this is the adapters responsibility, but we require that every
 * call defines a unique and distinguishable region.
 *
 * @todo Change SCOREP_RegionType from enum to bitstring? When using phases,
 * parametr-based profiling and dynamic regions, combinations of these
 * features may be useful. These combinations can be encoded in a
 * bitstring. The adapter group was queried for detailed input.
 *
 * @return A process unique region handle to be used in calls to
 * SCOREP_EnterRegion() and SCOREP_ExitRegion().
 *
 */
SCOREP_RegionHandle
SCOREP_Definitions_NewRegion( const char*             regionName,
                              const char*             regionCanonicalName,
                              SCOREP_SourceFileHandle fileHandle,
                              SCOREP_LineNo           beginLine,
                              SCOREP_LineNo           endLine,
                              SCOREP_ParadigmType     paradigm,
                              SCOREP_RegionType       regionType );


uint32_t
SCOREP_RegionHandle_GetId( SCOREP_RegionHandle handle );


const char*
SCOREP_RegionHandle_GetName( SCOREP_RegionHandle handle );


const char*
SCOREP_RegionHandle_GetCanonicalName( SCOREP_RegionHandle handle );


const char*
SCOREP_RegionHandle_GetFileName( SCOREP_RegionHandle handle );


SCOREP_RegionType
SCOREP_RegionHandle_GetType( SCOREP_RegionHandle handle );


SCOREP_ParadigmType
SCOREP_RegionHandle_GetParadigmType( SCOREP_RegionHandle handle );


SCOREP_LineNo
SCOREP_RegionHandle_GetBeginLine( SCOREP_RegionHandle handle );


SCOREP_LineNo
SCOREP_RegionHandle_GetEndLine( SCOREP_RegionHandle handle );


/**
 * Add a new attribute definition.
 *
 * @param name The string naming the attribute.
 *
 * @param name The string describing the attribute.
 *
 * @param type The type of this attribute.
 *
 * @return A process unique handle to a new attribute.
 */
SCOREP_AttributeHandle
SCOREP_Definitions_NewAttribute( const char*          name,
                                 const char*          description,
                                 SCOREP_AttributeType type );


uint32_t
SCOREP_AttributeHandle_GetId( SCOREP_AttributeHandle handle );


SCOREP_AttributeType
SCOREP_AttributeHandle_GetType( SCOREP_AttributeHandle handle );


SCOREP_GroupHandle
SCOREP_Definitions_NewGroup( SCOREP_GroupType type,
                             const char*      name,
                             uint32_t         numberOfMembers,
                             const uint64_t*  members );


SCOREP_GroupHandle
SCOREP_Definitions_NewGroupFrom32( SCOREP_GroupType type,
                                   const char*      name,
                                   const uint32_t   numberOfRanks,
                                   const uint32_t*  ranks );


/**
 * Associate a MPI communicator with a process unique communicator handle.
 *
 * @param parentComm    A possible parent communicator.
 * @param paradigmType  The paradigm of the adapter which defines this
 *                      communicator.
 * @param sizeOfPayload The size of the payload which the adapter requests
 *                      for this communicator.
 * @param[out] payload  Will be set to the memory location of the payload.
 *
 * @return A process unique communicator handle to be used in calls to other
 * SCOREP_Definitions_NewMPI* functions.
 *
 */
SCOREP_InterimCommunicatorHandle
SCOREP_Definitions_NewInterimCommunicator( SCOREP_InterimCommunicatorHandle parentComm,
                                           SCOREP_ParadigmType              paradigmType,
                                           size_t                           sizeOfPayload,
                                           void**                           payload );


/**
 * Get access to the payload from a communicator definition.
 */
void*
SCOREP_InterimCommunicatorHandle_GetPayload( SCOREP_InterimCommunicatorHandle handle );


/**
 * Set the name of the communicator to @a name, but only if it wasn't done before.
 */
void
SCOREP_InterimCommunicatorHandle_SetName( SCOREP_InterimCommunicatorHandle localMPICommHandle,
                                          const char*                      name );


/**
 * Associate the parameter tuple with a process unique RMA window handle.
 *
 * @param name A meaningful name for the RMA window, e.g. 'MPI window'
 * or 'Gfx Card 1'. The string will be copied.
 *
 * @param communicatorHandle Underlying communicator
 *
 * @return A process unique RMA window handle to be used in calls to other
 * SCOREP_RMA* functions.
 */
SCOREP_InterimRmaWindowHandle
SCOREP_Definitions_NewInterimRmaWindow( const char*                      name,
                                        SCOREP_InterimCommunicatorHandle communicatorHandle );


/**
 * Associate a MPI cartesian topology with a process unique topology handle.
 *
 * @param topologyName A meaningful name for the topology. The string will be
 * copied.
 *
 * @param communicatorHandle A handle to the associated communicator,
 * previously defined by DefineInterimCommunicator().
 *
 * @param nDimensions Number of dimensions of the cartesian topology.
 *
 * @param nProcessesPerDimension Number of processes in each dimension.
 *
 * @param periodicityPerDimension Periodicity in each dimension, true (1) or
 * false (0).
 *
 * @note The @a topologyName and the @a communicatorHandle will be used to
 * determine uniqueness during unification (only). It's the user's
 * responsibility to define unique topologies.
 *
 * @return A process unique topology handle to be used in calls to
 * SCOREP_Definitions_NewMPICartesianCoords().
 *
 */
SCOREP_MPICartesianTopologyHandle
SCOREP_Definitions_NewMPICartesianTopology( const char*                      topologyName,
                                            SCOREP_InterimCommunicatorHandle communicatorHandle,
                                            uint32_t                         nDimensions,
                                            const uint32_t                   nProcessesPerDimension[],
                                            const uint8_t                    periodicityPerDimension[] );


/**
 * Define the coordinates of the current rank in the cartesian topology
 * referenced by @a cartesianTopologyHandle.
 *
 * @param cartesianTopologyHandle Handle to the cartesian topology for which
 * the coordinates are defines.
 *
 * @param nCoords Number of dimensions of the cartesian topology.
 *
 * @param coordsOfCurrentRank Coordinates of current rank.
 *
 */
void
SCOREP_Definitions_NewMPICartesianCoords( SCOREP_MPICartesianTopologyHandle cartesianTopologyHandle,
                                          uint32_t                          nCoords,
                                          const uint32_t                    coordsOfCurrentRank[] );


/**
 * Associate a name with a process unique metric member handle.
 *
 * @param name A meaningful name of the metric member.
 *
 * @return A process unique metric member handle to be used in calls to
 * SCOREP_Definitions_NewSamplingSet().
 */
SCOREP_MetricHandle
SCOREP_Definitions_NewMetric( const char*                name,
                              const char*                description,
                              SCOREP_MetricSourceType    sourceType,
                              SCOREP_MetricMode          mode,
                              SCOREP_MetricValueType     valueType,
                              SCOREP_MetricBase          base,
                              int64_t                    exponent,
                              const char*                unit,
                              SCOREP_MetricProfilingType profilingType );


/**
 * Returns the number of unified metric definitions.
 */
uint32_t
SCOREP_Definitions_GetNumberOfUnifiedMetricDefinitions( void );


/**
 * Returns the sequence number of the unified definitions for a local metric handle from
 * the mappings.
 * @param handle handle to local metric handle.
 */
uint32_t
SCOREP_MetricHandle_GetUnifiedId( SCOREP_MetricHandle handle );


/**
 * Returns the unified handle from a local handle.
 */
SCOREP_MetricHandle
SCOREP_MetricHandle_GetUnified( SCOREP_MetricHandle handle );


/**
 * Returns the value type of a metric.
 * @param handle to local metric definition.
 */
SCOREP_MetricValueType
SCOREP_MetricHandle_GetValueType( SCOREP_MetricHandle handle );


/**
 * Returns the name of a metric.
 * @param handle to local metric definition.
 */
const char*
SCOREP_MetricHandle_GetName( SCOREP_MetricHandle handle );


/**
 * Returns the profiling type of a metric.
 * @param handle to local metric definition.
 */
SCOREP_MetricProfilingType
SCOREP_MetricHandle_GetProfilingType( SCOREP_MetricHandle handle );


/**
 * Define a new sampling set.
 *
 * @param numberOfMetrics Number of metrics contained in array @a metrics.
 * @param metrics         Array containing metric handles of all members
 *                        of this sampling set.
 * @param occurrence      Specifies whether a metric occurs at every region enter and leave
 *                        (SCOREP_METRIC_OCCURRENCE_SYNCHRONOUS_STRICT), only at a region
 *                        enter and leave but does not need to occur at every enter/leave
 *                        (SCOREP_METRIC_OCCURRENCE_SYNCHRONOUS) or at any place i.e. it is
 *                        not related to region enter or leaves (SCOREP_METRIC_OCCURRENCE_ASYNCHRONOUS).
 *
 * @return A process unique sampling set handle to be used in calls to
 * SCOREP_Definitions_NewScopedSamplingSet().
 */
SCOREP_SamplingSetHandle
SCOREP_Definitions_NewSamplingSet( uint8_t                    numberOfMetrics,
                                   const SCOREP_MetricHandle* metrics,
                                   SCOREP_MetricOccurrence    occurrence,
                                   SCOREP_SamplingSetClass    klass );


/**
 * Define a new scoped sampling set. The scoped sampling set is recorded by
 * a location specified by @a recorderHandle. In contrast to a normal
 * <em>sampling set</em> the values of a scoped sampling set are valid for
 * another location or a group of locations.
 *
 * @param samplingSet    Handle of a previously defined sampling set.
 * @param recorderHandle Handle of the location that records this sampling set.
 * @param scopeType      Defines whether the scope of the sampling set is another
 *                       location (SCOREP_METRIC_SCOPE_LOCATION), a location group
 *                       (SCOREP_METRIC_SCOPE_LOCATION_GROUP), a system tree node
 *                       (SCOREP_METRIC_SCOPE_SYSTEM_TREE_NODE) or a generic group
 *                       of locations (SCOREP_METRIC_SCOPE_GROUP).
 * @param scopeHandle    Handle of the sampling set scope according to @a scopeType.
 */
SCOREP_SamplingSetHandle
SCOREP_Definitions_NewScopedSamplingSet( SCOREP_SamplingSetHandle samplingSet,
                                         SCOREP_LocationHandle    recorderHandle,
                                         SCOREP_MetricScope       scopeType,
                                         SCOREP_AnyHandle         scopeHandle );


/**
 * Adds a recorder to a sampling set.
 *
 * The class of the sampling set must be in {CPU, GPU} and the type of the
 * location must match this class.
 */
void
SCOREP_SamplingSet_AddRecorder( SCOREP_SamplingSetHandle samplingSetHandle,
                                SCOREP_LocationHandle    recorderHandle );

/**
 * A SCOREP_SamplingSetHandle can refer to a normal sampling set or a scoped
 * one. A scoped sampling set contains a reference to the underlying sampling
 * set, which contains all metric members needed to record their values.
 *
 * This function handles differences between normal and scoped sampling sets and
 * returns always a handle of a sampling set.
 *
 * @param samplingSet   Handle of a normal or scoped sampling set.
 *
 * @return Returns handle of a normal sampling set.
 */
SCOREP_SamplingSetHandle
SCOREP_SamplingSetHandle_GetSamplingSet( SCOREP_SamplingSetHandle samplingSet );


/**
 * Associate a name with a process unique I/O file group handle.
 *
 * @param name A meaningfule name of the I/O file group.
 *
 * @return A process unique file I/O file group handle to be used in calls to
 * SCOREP_Definitions_NewIOFile().
 *
 * @planned To be implemented in milestone 2
 */
SCOREP_IOFileGroupHandle
SCOREP_Definitions_NewIOFileGroup( const char* name );


/**
 * Associate a name and a group handle with a process unique I/O file handle.
 *
 * @param name A meaningful name for the I/O file.
 *
 * @param ioFileGroup Handle to the group the I/O file is associated to.
 *
 * @return A process unique I/O file handle to be used in calls to
 * SCOREP_TriggerIOFile().
 *
 * @planned To be implemented in milestone 2
 */
SCOREP_IOFileHandle
SCOREP_Definitions_NewIOFile( const char*              name,
                              SCOREP_IOFileGroupHandle ioFileGroup );


/**
 * Associate a name with a process unique marker group handle.
 *
 * @param name A meaningfule name of the marker group.
 *
 * @return A process unique marker group handle to be used in calls to
 * SCOREP_Definitions_NewMarker().
 *
 * @planned To be implemented in milestone 2
 */
SCOREP_MarkerGroupHandle
SCOREP_Definitions_NewMarkerGroup( const char* name );


/**
 * Associate a name and a group handle with a process unique marker handle.
 *
 * @param name A meaningful name for the marker
 *
 * @param markerGroup Handle to the group the marker is associated to.
 *
 * @return A process unique marker handle to be used in calls to
 * SCOREP_TriggerMarker().
 *
 * @planned To be implemented in milestone 2
 */
SCOREP_MarkerHandle
SCOREP_Definitions_NewMarker( const char*              name,
                              SCOREP_MarkerGroupHandle markerGroup );


/**
 * Associate a name and a type with a process unique parameter handle.
 *
 * @param name A meaningful name for the parameter.
 *
 * @param type The type of the parameter.
 *
 * @return A process unique parameter handle to be used in calls to
 * SCOREP_TriggerParameter().
 *
 * @planned To be implemented in milestone 3
 *
 */
SCOREP_ParameterHandle
SCOREP_Definitions_NewParameter( const char*          name,
                                 SCOREP_ParameterType type );


const char*
SCOREP_ParameterHandle_GetName( SCOREP_ParameterHandle handle );


SCOREP_ParameterType
SCOREP_ParameterHandle_GetType( SCOREP_ParameterHandle handle );


/**
 * Define a property with its initial value.
 */
SCOREP_PropertyHandle
SCOREP_Definitions_NewProperty( SCOREP_Property          property,
                                SCOREP_PropertyCondition condition,
                                bool                     initialValue );

/*@}*/


#endif /* SCOREP_DEFINITIONS_H */
