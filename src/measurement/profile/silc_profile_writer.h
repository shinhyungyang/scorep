/*
 * This file is part of the SILC project (http://www.silc.de)
 *
 * Copyright (c) 2009-2011,
 *    RWTH Aachen, Germany
 *    Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *    Technische Universitaet Dresden, Germany
 *    University of Oregon, Eugene USA
 *    Forschungszentrum Juelich GmbH, Germany
 *    Technische Universitaet Muenchen, Germany
 *
 * See the COPYING file in the package base directory for details.
 *
 */

#ifndef SILC_PROFILE_WRITER_H
#define SILC_PROFILE_WRITER_H

/**
 * @file silc_profile_writer.h
 *
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @status ALPHA
 * Declares profile writer functions. This header provides a list of top level
 * profile writers. Thus, we avoid to include a special file per format. Only
 * those formats are declared, which are available on the platform.
 */

/**
   Writes the profile in TAU snapshot format to disk. For each rank a separate
   file is created. The files are named snapshot.<rank>.0.0
 */
extern void
silc_profile_write_tau_snapshot();

#ifdef HAVE_CUBE4
/**
   Writes the profile in Cube 4 format to disk. For each metric a separate
   file is created.
 */
extern void
silc_profile_write_cube4();

#endif /* HAVE_CUBE4 */

#endif /* SILC_PROFILE_WRITER_H */
