/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
 * RWTH Aachen University, Germany
 *
 * Copyright (c) 2009-2011,
 * Gesellschaft fuer numerische Simulation mbH Braunschweig, Germany
 *
 * Copyright (c) 2009-2014, 2017,
 * Technische Universitaet Dresden, Germany
 *
 * Copyright (c) 2009-2011,
 * University of Oregon, Eugene, USA
 *
 * Copyright (c) 2009-2011, 2023, 2025,
 * Forschungszentrum Juelich GmbH, Germany
 *
 * Copyright (c) 2009-2011,
 * German Research School for Simulation Sciences GmbH, Juelich/Aachen, Germany
 *
 * Copyright (c) 2009-2011,
 * Technische Universitaet Muenchen, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 */

/* This file is also included from Fortran. The Fortran preprocessor does not
 * support all features of the C preprocessor, in particular:
 * - preprocessor directives have to start at column 1,
 * - the HAVE_ macros defined here cannot be used in Fortran.
 *
 * Note: C-comments are stripped by both preprocessors.
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file
 *
 * @brief      Provide a single config.h that hides the frontend/backend
 *             issues from the developer
 *
 */

#if defined CROSS_BUILD
#if defined FRONTEND_BUILD
#include <config-frontend.h>
#include <config-backend-for-frontend.h>
#define HAVE_BACKEND( H ) ( defined( HAVE_BACKEND_ ## H ) && HAVE_BACKEND_ ## H )
#elif defined BACKEND_BUILD_NOMPI
#include <config-backend.h>
#define HAVE_BACKEND( H ) ( defined( HAVE_ ## H ) && HAVE_ ## H )
#elif defined BACKEND_BUILD_MPI
#include <config-backend-mpi.h>
#define HAVE_BACKEND( H ) ( defined( HAVE_ ## H ) && HAVE_ ## H )
#elif defined BACKEND_BUILD_SHMEM
#include <config-backend-shmem.h>
#define HAVE_BACKEND( H ) ( defined( HAVE_ ## H ) && HAVE_ ## H )
#else
#error "You cannot use config.h without defining either FRONTEND_BUILD, BACKEND_BUILD_NOMPI, BACKEND_BUILD_MPI or BACKEND_BUILD_SHMEM."
#endif

#elif defined NOCROSS_BUILD
#if defined BACKEND_BUILD_NOMPI
#include <config-backend.h>
#elif defined BACKEND_BUILD_MPI
#include <config-backend-mpi.h>
#elif defined BACKEND_BUILD_SHMEM
#include <config-backend-shmem.h>
#else
#error "You cannot use config.h without defining either BACKEND_BUILD_NOMPI, BACKEND_BUILD_MPI or BACKEND_BUILD_SHMEM."
#endif

#define HAVE_BACKEND( H ) ( defined( HAVE_ ## H ) && HAVE_ ## H )

#elif defined SCORE_BUILD
#include <config-score.h>

#elif defined GCC_PLUGIN_BUILD
#include <config-gcc-plugin.h>

#elif defined LLVM_PLUGIN_BUILD
       #include <config-llvm-plugin.h>

#elif defined LIBWRAP_BUILD
#include <config-libwrap.h>

#else
#error "You cannot use config.h without defining either CROSS_BUILD or NOCROSS_BUILD."
#endif

#include <config-common.h>

#include <config-custom.h>

#endif /* CONFIG_H */
