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

#ifndef SCOREP_MPI_IO_H
#define SCOREP_MPI_IO_H

#include <scorep/SCOREP_PublicTypes.h>

SCOREP_IoAccessMode
scorep_mpi_io_get_access_mode( int amode );

SCOREP_IoCreationFlag
scorep_mpi_io_get_creation_flags( int amode );

SCOREP_IoStatusFlag
scorep_mpi_io_get_status_flags( int amode );

SCOREP_IoSeekOption
scorep_mpi_io_get_seek_option( int whence );

#endif
