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


/**
 * @file        scorep_types.c
 * @maintainer  Bert Wesarg <Bert.Wesarg@tu-dresden.de>
 *
 * @status      alpha
 *
 * @brief Functions to convert types from @ref SCOREP_Types.h into strings.
 */


#include <config.h>

#include "scorep_types.h"
#include "scorep_definition_structs.h"
#include <SCOREP_Memory.h>

/**
 * Generic function to convert a type with an invalid value into a string.
 *
 * @note invalid will not be copied into the @a stringBuffer.
 *
 * @note For gcc, this function should be annotated with the
 *       __attribute__ ((format_arg (3))) attribute.
 */
const char*
scorep_uint32_to_string
(
    char*       stringBuffer,
    size_t      stringBufferSize,
    const char* format,
    uint32_t    value,
    uint32_t    invalidValue
)
{
    if ( value == invalidValue )
    {
        return "invalid";
    }

    snprintf( stringBuffer, stringBufferSize, format, value );
    return stringBuffer;
}


/**
 * Generic function to convert a type with an invalid value into a string.
 *
 * @note invalid will not be copied into the @a stringBuffer.
 *
 * @note For gcc, this function should be annotated with the
 *       __attribute__ ((format_arg (3))) attribute.
 */
const char*
scorep_any_handle_to_string
(
    char*            stringBuffer,
    size_t           stringBufferSize,
    const char*      format,
    SCOREP_AnyHandle handle,
    SCOREP_AnyHandle invalidValue
)
{
    if ( handle == invalidValue )
    {
        return "invalid";
    }

    snprintf( stringBuffer, stringBufferSize,
              format,
              SCOREP_LOCAL_HANDLE_TO_ID( handle, Any ) );
    return stringBuffer;
}


/**
 * Converts a SCOREP_SourceFileHandle into a string.
 */
const char*
scorep_source_file_to_string
(
    char*                   stringBuffer,
    size_t                  stringBufferSize,
    const char*             format,
    SCOREP_SourceFileHandle handle
)
{
    return scorep_any_handle_to_string( stringBuffer,
                                        stringBufferSize,
                                        format,
                                        handle,
                                        SCOREP_INVALID_SOURCE_FILE );
}


/**
 * Converts a SCOREP_LineNo into a string.
 */
const char*
scorep_line_number_to_string
(
    char*         stringBuffer,
    size_t        stringBufferSize,
    const char*   format,
    SCOREP_LineNo lineNo
)
{
    if ( lineNo == SCOREP_INVALID_LINE_NO )
    {
        return "invalid";
    }

    snprintf( stringBuffer, stringBufferSize, format, lineNo );
    return stringBuffer;
}


/**
 * Converts a SCOREP_RegionHandle into a string.
 */
extern const char*
scorep_region_to_string
(
    char*               stringBuffer,
    size_t              stringBufferSize,
    const char*         format,
    SCOREP_RegionHandle regionHandle
)
{
    return scorep_any_handle_to_string( stringBuffer,
                                        stringBufferSize,
                                        format,
                                        regionHandle,
                                        SCOREP_INVALID_REGION );
}


/**
 * Converts a SCOREP_LocalMPICommunicatorHandle into a string.
 */
extern const char*
scorep_comm_to_string(
    char*                             stringBuffer,
    size_t                            stringBufferSize,
    const char*                       format,
    SCOREP_LocalMPICommunicatorHandle commHandle )
{
    return scorep_any_handle_to_string( stringBuffer,
                                        stringBufferSize,
                                        format,
                                        commHandle,
                                        SCOREP_INVALID_LOCAL_MPI_COMMUNICATOR );
}


/**
 * Converts a SCOREP_MPIWindowHandle into a string.
 */
extern const char*
scorep_window_to_string
(
    char*                  stringBuffer,
    size_t                 stringBufferSize,
    const char*            format,
    SCOREP_MPIWindowHandle windowHandle
)
{
    return scorep_any_handle_to_string( stringBuffer,
                                        stringBufferSize,
                                        format,
                                        windowHandle,
                                        SCOREP_INVALID_MPI_WINDOW );
}


/**
 * Converts a SCOREP_MPICartTopolHandle into a string.
 */
const char*
scorep_mpi_cart_topol_to_string
(
    char*                             stringBuffer,
    size_t                            stringBufferSize,
    const char*                       format,
    SCOREP_MPICartesianTopologyHandle cartHandle
)
{
    return scorep_any_handle_to_string( stringBuffer,
                                        stringBufferSize,
                                        format,
                                        cartHandle,
                                        SCOREP_INVALID_CART_TOPOLOGY );
}


/**
 * Converts a SCOREP_AdapterType into a string.
 */
const char*
scorep_adapter_type_to_string
(
    SCOREP_AdapterType adapterType
)
{
    switch ( adapterType )
    {
        case SCOREP_ADAPTER_USER:
            return "user";
        case SCOREP_ADAPTER_COMPILER:
            return "compiler";
        case SCOREP_ADAPTER_MPI:
            return "mpi";
        case SCOREP_ADAPTER_POMP:
            return "pomp";
        case SCOREP_ADAPTER_PTHREAD:
            return "pthread";
        default:
            return "unknown";
    }
}


/**
 * Converts a SCOREP_RegionType into a string.
 */
const char*
scorep_region_type_to_string
(
    SCOREP_RegionType regionType
)
{
    switch ( regionType )
    {
        case SCOREP_REGION_FUNCTION:
            return "function";
        case SCOREP_REGION_LOOP:
            return "loop";
        case SCOREP_REGION_USER:
            return "user";
        case SCOREP_REGION_PHASE:
            return "phase";
        case SCOREP_REGION_DYNAMIC:
            return "dynamic";
        case SCOREP_REGION_DYNAMIC_PHASE:
            return "dynamic phase";
        case SCOREP_REGION_DYNAMIC_LOOP:
            return "dynamic loop";
        case SCOREP_REGION_DYNAMIC_FUNCTION:
            return "dynamix function";
        case SCOREP_REGION_DYNAMIC_LOOP_PHASE:
            return "dynamix loop phase";
        case SCOREP_REGION_MPI_COLL_BARRIER:
            return "mpi barrier";
        case SCOREP_REGION_MPI_COLL_ONE2ALL:
            return "mpi one2all";
        case SCOREP_REGION_MPI_COLL_ALL2ONE:
            return "mpi all2one";
        case SCOREP_REGION_MPI_COLL_ALL2ALL:
            return "mpi all2all";
        case SCOREP_REGION_MPI_COLL_OTHER:
            return "mpi other";
        case SCOREP_REGION_OMP_PARALLEL:
            return "omp parallel";
        case SCOREP_REGION_OMP_LOOP:
            return "omp loop";
        case SCOREP_REGION_OMP_SECTIONS:
            return "omp secions";
        case SCOREP_REGION_OMP_SECTION:
            return "omp section";
        case SCOREP_REGION_OMP_WORKSHARE:
            return "omp workshare";
        case SCOREP_REGION_OMP_SINGLE:
            return "omp single";
        case SCOREP_REGION_OMP_MASTER:
            return "omp master";
        case SCOREP_REGION_OMP_CRITICAL:
            return "omp critical";
        case SCOREP_REGION_OMP_ATOMIC:
            return "omp atomic";
        case SCOREP_REGION_OMP_BARRIER:
            return "omp barrier";
        case SCOREP_REGION_OMP_IMPLICIT_BARRIER:
            return "omp implicit barrier";
        case SCOREP_REGION_OMP_FLUSH:
            return "omp flush";
        case SCOREP_REGION_OMP_CRITICAL_SBLOCK:
            return "omp critical sblock";
        case SCOREP_REGION_OMP_SINGLE_SBLOCK:
            return "omp single sblock";
        case SCOREP_REGION_OMP_WRAPPER:
            return "omp wrapper";
        case SCOREP_REGION_OMP_TASK:
            return "omp task";
        case SCOREP_REGION_OMP_TASKWAIT:
            return "omp taskwait";
        case SCOREP_REGION_OMP_TASK_CREATE:
            return "omp task create";
        case SCOREP_REGION_OMP_ORDERED:
            return "omp ordered";
        case SCOREP_REGION_OMP_ORDERED_SBLOCK:
            return "omp ordered sblock";
        default:
            return "unknown";
    }
}


/**
 * Converts a SCOREP_ParameterType into a string.
 */
const char*
scorep_parameter_type_to_string
(
    SCOREP_ParameterType parameterType
)
{
    switch ( parameterType )
    {
        case SCOREP_PARAMETER_INT64:
            return "int64";
        case SCOREP_PARAMETER_UINT64:
            return "uint64";
        case SCOREP_PARAMETER_STRING:
            return "string";
        default:
            return "invalid";
    }
}


/**
 * Converts a SCOREP_ConfigType into a string.
 */
const char*
scorep_config_type_to_string
(
    SCOREP_ConfigType configType
)
{
    switch ( configType )
    {
        case SCOREP_CONFIG_TYPE_PATH:
            return "path";
        case SCOREP_CONFIG_TYPE_STRING:
            return "string";
        case SCOREP_CONFIG_TYPE_BOOL:
            return "boolean";
        case SCOREP_CONFIG_TYPE_NUMBER:
            return "number";
        case SCOREP_CONFIG_TYPE_SIZE:
            return "size";
        case SCOREP_CONFIG_TYPE_SET:
            return "set";
        case SCOREP_CONFIG_TYPE_BITSET:
            return "bitset";
        default:
            return "unknown";
    }
}
