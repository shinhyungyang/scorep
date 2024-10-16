/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2024,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */


#include <cstdlib>

#include "cmdline-helper.h"

#ifndef FOO
    #define FOO ""
#endif


int
main( int    argc,
      char** argv )
{
    static_assert( is_equal( FOO, "$FOO" ) );

    return EXIT_SUCCESS;
}
