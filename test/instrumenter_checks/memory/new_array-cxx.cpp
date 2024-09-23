/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2016,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2024,
 * Forschungszentrum Jülich GmbH, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 */

#include <cstring>
#include <new>

#define ARRAY_SIZE ( 1024 )

int
main( int ac, char* av[] )
{
    int  returnvalue = ARRAY_SIZE;
    int* memory      = new int[ ARRAY_SIZE ];
    std::memset( memory, 1, ARRAY_SIZE * sizeof( int ) );
    for ( int i = 0; i < ARRAY_SIZE; ++i )
    {
        returnvalue -= memory[ i ];
    }

    delete[] memory;

    return returnvalue;
}
