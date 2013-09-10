/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2013,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 */

/**
 * @file
 * @ingroup    POMP2
 *
 * Dummy implementation of two functions which are generated by the awk script. In case
 * the Opari instrumentation is not used, the instrumenter should link these dummy
 * functions to the binary, else the initialization of the pomp adapter contains
 * unresolved links.
 *
 */

#include <config.h>
#include <stddef.h>

void
POMP2_Init_regions( void )
{
}

size_t
POMP2_Get_num_regions( void )
{
    return 0;
}
