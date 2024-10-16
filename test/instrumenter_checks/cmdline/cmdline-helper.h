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


#ifndef CMDLINE_HELPER_H
#define CMDLINE_HELPER_H


// Compile-time comparison of string literals
constexpr bool
is_equal( char const* lhs,
          char const* rhs )
{
    return ( *lhs == *rhs )
           && ( ( *lhs == '\0' )
                || is_equal( lhs + 1, rhs + 1 ) );
}


#endif
