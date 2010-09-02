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

#ifndef SILC_INTERNAL_DEFINITION_LOCKING_H
#define SILC_INTERNAL_DEFINITION_LOCKING_H



/**
 * @file       silc_definition_locking.h
 * @maintainer Christian R&ouml;ssel <c.roessel@fz-juelich.de>
 *
 * @status alpha
 *
 */


void
SILC_DefinitionLocks_Initialize();


void
SILC_DefinitionLocks_Finalize();


/**
 * Lock the SILC_DefineLocation() and related operations (in the measurement core).
 */
void
SILC_LockLocationDefinition();


/**
 * Unlock the SILC_DefineLocation() and related operations (in the measurement core).
 */
void
SILC_UnlockLocationDefinition();


#endif /* SILC_INTERNAL_DEFINITION_LOCKING_H */
