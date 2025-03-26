/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2013,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2018, 2020, 2022-2023,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2013,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2019, 2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2014,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2013,
 * Technische Universitaet Muenchen, Germany
 *
 * Copyright (c) 2016,
 * Technische Universitaet Darmstadt, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

#include <config.h>

#include "scorep_mpi_io.h"

#include <mpi.h>

SCOREP_IoAccessMode
scorep_mpi_io_get_access_mode( int amode )
{
    SCOREP_IoAccessMode scorep_mode = SCOREP_IO_ACCESS_MODE_NONE;

    if ( amode & MPI_MODE_RDONLY )
    {
        scorep_mode = SCOREP_IO_ACCESS_MODE_READ_ONLY;
    }
    else if ( amode & MPI_MODE_WRONLY )
    {
        scorep_mode = SCOREP_IO_ACCESS_MODE_WRITE_ONLY;
    }
    else if ( amode & MPI_MODE_RDWR )
    {
        scorep_mode = SCOREP_IO_ACCESS_MODE_READ_WRITE;
    }

    return scorep_mode;
}

SCOREP_IoCreationFlag
scorep_mpi_io_get_creation_flags( int amode )
{
    SCOREP_IoCreationFlag creation_flags = SCOREP_IO_CREATION_FLAG_NONE;

    /* Handle creation flags */
    if ( amode & MPI_MODE_CREATE )
    {
        creation_flags |= SCOREP_IO_CREATION_FLAG_CREATE;
    }
    if ( amode & MPI_MODE_EXCL )
    {
        creation_flags |= SCOREP_IO_CREATION_FLAG_EXCLUSIVE;
    }

    return creation_flags;
}

SCOREP_IoStatusFlag
scorep_mpi_io_get_status_flags( int amode )
{
    SCOREP_IoStatusFlag status_flags = SCOREP_IO_STATUS_FLAG_NONE;

    /* Handle status flags */
    if ( amode & MPI_MODE_APPEND )
    {
        status_flags |= SCOREP_IO_STATUS_FLAG_APPEND;
    }
    if ( amode & MPI_MODE_DELETE_ON_CLOSE )
    {
        status_flags |= SCOREP_IO_STATUS_FLAG_CLOSE_ON_EXEC;
    }

    return status_flags;
}

SCOREP_IoSeekOption
scorep_mpi_io_get_seek_option( int whence )
{
    switch ( whence )
    {
        case MPI_SEEK_SET:
            return SCOREP_IO_SEEK_FROM_START;
        case MPI_SEEK_CUR:
            return SCOREP_IO_SEEK_FROM_CURRENT;
        case MPI_SEEK_END:
            return SCOREP_IO_SEEK_FROM_END;
    }

    return SCOREP_IO_SEEK_INVALID;
}
