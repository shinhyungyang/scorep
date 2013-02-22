/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2013,
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

/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2011                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2010-2011                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  Copyright (c) 2003-2008                                                **
**  University of Tennessee, Innovative Computing Laboratory               **
**                                                                         **
**  See the file COPYRIGHT in the package base directory for details       **
****************************************************************************/


/**
 * @file  SCOREP_Fmpi_Err.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 * @status     alpha
 * @ingroup    MPI_Wrapper
 *
 * @brief Fortran interface wrappers for error handling
 */

#include <config.h>
#include "SCOREP_Fmpi.h"

/* uppercase defines */
/** @def MPI_Add_error_class_U
    Exchange MPI_Add_error_class_U by MPI_ADD_ERROR_CLASS.
    It is used for the Fortran wrappers of MPI_Add_error_class.
 */
#define MPI_Add_error_class_U MPI_ADD_ERROR_CLASS

/** @def MPI_Add_error_code_U
    Exchange MPI_Add_error_code_U by MPI_ADD_ERROR_CODE.
    It is used for the Fortran wrappers of MPI_Add_error_code.
 */
#define MPI_Add_error_code_U MPI_ADD_ERROR_CODE

/** @def MPI_Add_error_string_U
    Exchange MPI_Add_error_string_U by MPI_ADD_ERROR_STRING.
    It is used for the Fortran wrappers of MPI_Add_error_string.
 */
#define MPI_Add_error_string_U MPI_ADD_ERROR_STRING

/** @def MPI_Errhandler_create_U
    Exchange MPI_Errhandler_create_U by MPI_ERRHANDLER_CREATE.
    It is used for the Fortran wrappers of MPI_Errhandler_create.
 */
#define MPI_Errhandler_create_U MPI_ERRHANDLER_CREATE

/** @def MPI_Errhandler_free_U
    Exchange MPI_Errhandler_free_U by MPI_ERRHANDLER_FREE.
    It is used for the Fortran wrappers of MPI_Errhandler_free.
 */
#define MPI_Errhandler_free_U MPI_ERRHANDLER_FREE

/** @def MPI_Errhandler_get_U
    Exchange MPI_Errhandler_get_U by MPI_ERRHANDLER_GET.
    It is used for the Fortran wrappers of MPI_Errhandler_get.
 */
#define MPI_Errhandler_get_U MPI_ERRHANDLER_GET

/** @def MPI_Errhandler_set_U
    Exchange MPI_Errhandler_set_U by MPI_ERRHANDLER_SET.
    It is used for the Fortran wrappers of MPI_Errhandler_set.
 */
#define MPI_Errhandler_set_U MPI_ERRHANDLER_SET

/** @def MPI_Error_class_U
    Exchange MPI_Error_class_U by MPI_ERROR_CLASS.
    It is used for the Fortran wrappers of MPI_Error_class.
 */
#define MPI_Error_class_U MPI_ERROR_CLASS

/** @def MPI_Error_string_U
    Exchange MPI_Error_string_U by MPI_ERROR_STRING.
    It is used for the Fortran wrappers of MPI_Error_string.
 */
#define MPI_Error_string_U MPI_ERROR_STRING


/* lowercase defines */
/** @def MPI_Add_error_class_L
    Exchanges MPI_Add_error_class_L by mpi_add_error_class.
    It is used for the Fortran wrappers of MPI_Add_error_class.
 */
#define MPI_Add_error_class_L mpi_add_error_class

/** @def MPI_Add_error_code_L
    Exchanges MPI_Add_error_code_L by mpi_add_error_code.
    It is used for the Fortran wrappers of MPI_Add_error_code.
 */
#define MPI_Add_error_code_L mpi_add_error_code

/** @def MPI_Add_error_string_L
    Exchanges MPI_Add_error_string_L by mpi_add_error_string.
    It is used for the Fortran wrappers of MPI_Add_error_string.
 */
#define MPI_Add_error_string_L mpi_add_error_string

/** @def MPI_Errhandler_create_L
    Exchanges MPI_Errhandler_create_L by mpi_errhandler_create.
    It is used for the Fortran wrappers of MPI_Errhandler_create.
 */
#define MPI_Errhandler_create_L mpi_errhandler_create

/** @def MPI_Errhandler_free_L
    Exchanges MPI_Errhandler_free_L by mpi_errhandler_free.
    It is used for the Fortran wrappers of MPI_Errhandler_free.
 */
#define MPI_Errhandler_free_L mpi_errhandler_free

/** @def MPI_Errhandler_get_L
    Exchanges MPI_Errhandler_get_L by mpi_errhandler_get.
    It is used for the Fortran wrappers of MPI_Errhandler_get.
 */
#define MPI_Errhandler_get_L mpi_errhandler_get

/** @def MPI_Errhandler_set_L
    Exchanges MPI_Errhandler_set_L by mpi_errhandler_set.
    It is used for the Fortran wrappers of MPI_Errhandler_set.
 */
#define MPI_Errhandler_set_L mpi_errhandler_set

/** @def MPI_Error_class_L
    Exchanges MPI_Error_class_L by mpi_error_class.
    It is used for the Fortran wrappers of MPI_Error_class.
 */
#define MPI_Error_class_L mpi_error_class

/** @def MPI_Error_string_L
    Exchanges MPI_Error_string_L by mpi_error_string.
    It is used for the Fortran wrappers of MPI_Error_string.
 */
#define MPI_Error_string_L mpi_error_string


/**
 * @name Fortran wrappers
 * @{
 */

#ifndef NEED_F2C_CONV

/* If MPI_Comm_f2c is HAVE as a simple macro (which typically does nothing)
 * as it would require a "real" function if it is really needed
 * => we can save the f2c and c2s conversions */

#if HAVE( DECL_PMPI_ADD_ERROR_CLASS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Add_error_class )
/**
 * Measurement wrapper for MPI_Add_error_class
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup err
 */
void
FSUB( MPI_Add_error_class )( int* errorclass, int* ierr )
{
    *ierr = MPI_Add_error_class( errorclass );
}
#endif
#if HAVE( DECL_PMPI_ADD_ERROR_CODE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Add_error_code )
/**
 * Measurement wrapper for MPI_Add_error_code
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup err
 */
void
FSUB( MPI_Add_error_code )( int* errorclass, int* errorcode, int* ierr )
{
    *ierr = MPI_Add_error_code( *errorclass, errorcode );
}
#endif
#if HAVE( DECL_PMPI_ADD_ERROR_STRING ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Add_error_string )
/**
 * Measurement wrapper for MPI_Add_error_string
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup err
 */
void
FSUB( MPI_Add_error_string )( int* errorcode, char* string, int* ierr, int string_len )
{
    char* c_string = NULL;
    c_string = ( char* )malloc( ( string_len + 1 ) * sizeof( char ) );
    if ( !c_string )
    {
        exit( EXIT_FAILURE );
    }
    strncpy( c_string, string, string_len );
    c_string[ string_len ] = '\0';



    *ierr = MPI_Add_error_string( *errorcode, c_string );

    free( c_string );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_CREATE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_create )
/**
 * Measurement wrapper for MPI_Errhandler_create
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 */
void
FSUB( MPI_Errhandler_create )( MPI_Handler_function* function, MPI_Errhandler* errhandler, int* ierr )
{
    *ierr = MPI_Errhandler_create( function, errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_FREE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_free )
/**
 * Measurement wrapper for MPI_Errhandler_free
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 */
void
FSUB( MPI_Errhandler_free )( MPI_Errhandler* errhandler, int* ierr )
{
    *ierr = MPI_Errhandler_free( errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_get )
/**
 * Measurement wrapper for MPI_Errhandler_get
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 */
void
FSUB( MPI_Errhandler_get )( MPI_Comm* comm, MPI_Errhandler* errhandler, int* ierr )
{
    *ierr = MPI_Errhandler_get( *comm, errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_SET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_set )
/**
 * Measurement wrapper for MPI_Errhandler_set
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 */
void
FSUB( MPI_Errhandler_set )( MPI_Comm* comm, MPI_Errhandler* errhandler, int* ierr )
{
    *ierr = MPI_Errhandler_set( *comm, *errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERROR_CLASS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Error_class )
/**
 * Measurement wrapper for MPI_Error_class
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 */
void
FSUB( MPI_Error_class )( int* errorcode, int* errorclass, int* ierr )
{
    *ierr = MPI_Error_class( *errorcode, errorclass );
}
#endif
#if HAVE( DECL_PMPI_ERROR_STRING ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Error_string )
/**
 * Measurement wrapper for MPI_Error_string
 * @note Auto-generated by wrapgen from template: SCOREP_Fmpi_Std.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 */
void
FSUB( MPI_Error_string )( int* errorcode, char* string, int* resultlen, int* ierr, int string_len )
{
    char* c_string     = NULL;
    int   c_string_len = 0;
    c_string = ( char* )malloc( ( string_len + 1 ) * sizeof( char ) );
    if ( !c_string )
    {
        exit( EXIT_FAILURE );
    }



    *ierr = MPI_Error_string( *errorcode, c_string, resultlen );


    c_string_len = strlen( c_string );
    strncpy( string, c_string, c_string_len );
    memset( string + c_string_len, ' ', string_len - c_string_len );
    free( c_string );
}
#endif

#else /* !NEED_F2C_CONV */

#if HAVE( DECL_PMPI_ADD_ERROR_CLASS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Add_error_class )
/**
 * Measurement wrapper for MPI_Add_error_class
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup err
 * For the order of events see @ref MPI_Add_error_class
 */
void
FSUB( MPI_Add_error_class )( MPI_Fint* errorclass, MPI_Fint* ierr )
{
    *ierr = MPI_Add_error_class( errorclass );
}
#endif
#if HAVE( DECL_PMPI_ADD_ERROR_CODE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Add_error_code )
/**
 * Measurement wrapper for MPI_Add_error_code
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup err
 * For the order of events see @ref MPI_Add_error_code
 */
void
FSUB( MPI_Add_error_code )( MPI_Fint* errorclass, MPI_Fint* errorcode, MPI_Fint* ierr )
{
    *ierr = MPI_Add_error_code( *errorclass, errorcode );
}
#endif
#if HAVE( DECL_PMPI_ADD_ERROR_STRING ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Add_error_string )
/**
 * Measurement wrapper for MPI_Add_error_string
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-2
 * @ingroup err
 * For the order of events see @ref MPI_Add_error_string
 */
void
FSUB( MPI_Add_error_string )( MPI_Fint* errorcode, char* string, MPI_Fint* ierr, int string_len )
{
    char* c_string = NULL;
    c_string = ( char* )malloc( ( string_len + 1 ) * sizeof( char ) );
    if ( !c_string )
    {
        exit( EXIT_FAILURE );
    }
    strncpy( c_string, string, string_len );
    c_string[ string_len ] = '\0';



    *ierr = MPI_Add_error_string( *errorcode, c_string );

    free( c_string );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_CREATE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_create )
/**
 * Measurement wrapper for MPI_Errhandler_create
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 * For the order of events see @ref MPI_Errhandler_create
 */
void
FSUB( MPI_Errhandler_create )( void* function, void* errhandler, MPI_Fint* ierr )
{
    *ierr = MPI_Errhandler_create( ( MPI_Handler_function* )function, ( MPI_Errhandler* )errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_FREE ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_free )
/**
 * Measurement wrapper for MPI_Errhandler_free
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 * For the order of events see @ref MPI_Errhandler_free
 */
void
FSUB( MPI_Errhandler_free )( void* errhandler, MPI_Fint* ierr )
{
    *ierr = MPI_Errhandler_free( ( MPI_Errhandler* )errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_GET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_get )
/**
 * Measurement wrapper for MPI_Errhandler_get
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 * For the order of events see @ref MPI_Errhandler_get
 */
void
FSUB( MPI_Errhandler_get )( MPI_Fint* comm, void* errhandler, MPI_Fint* ierr )
{
    *ierr = MPI_Errhandler_get( PMPI_Comm_f2c( *comm ), ( MPI_Errhandler* )errhandler );
}
#endif
#if HAVE( DECL_PMPI_ERRHANDLER_SET ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Errhandler_set )
/**
 * Measurement wrapper for MPI_Errhandler_set
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 * For the order of events see @ref MPI_Errhandler_set
 */
void
FSUB( MPI_Errhandler_set )( MPI_Fint* comm, void* errhandler, MPI_Fint* ierr )
{
    *ierr = MPI_Errhandler_set( PMPI_Comm_f2c( *comm ), *( ( MPI_Errhandler* )errhandler ) );
}
#endif
#if HAVE( DECL_PMPI_ERROR_CLASS ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Error_class )
/**
 * Measurement wrapper for MPI_Error_class
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 * For the order of events see @ref MPI_Error_class
 */
void
FSUB( MPI_Error_class )( MPI_Fint* errorcode, MPI_Fint* errorclass, MPI_Fint* ierr )
{
    *ierr = MPI_Error_class( *errorcode, errorclass );
}
#endif
#if HAVE( DECL_PMPI_ERROR_STRING ) && !defined( SCOREP_MPI_NO_EXTRA ) && !defined( SCOREP_MPI_NO_ERR ) && !defined( MPI_Error_string )
/**
 * Measurement wrapper for MPI_Error_string
 * @note Auto-generated by wrapgen from template: SCOREP_Mpi_F2c_C2f.w
 * @note Fortran interface
 * @note Introduced with MPI-1
 * @ingroup err
 * For the order of events see @ref MPI_Error_string
 */
void
FSUB( MPI_Error_string )( MPI_Fint* errorcode, char* string, MPI_Fint* resultlen, MPI_Fint* ierr, int string_len )
{
    char* c_string     = NULL;
    int   c_string_len = 0;
    c_string = ( char* )malloc( ( string_len + 1 ) * sizeof( char ) );
    if ( !c_string )
    {
        exit( EXIT_FAILURE );
    }



    *ierr = MPI_Error_string( *errorcode, c_string, resultlen );


    c_string_len = strlen( c_string );
    strncpy( string, c_string, c_string_len );
    memset( string + c_string_len, ' ', string_len - c_string_len );
    free( c_string );
}
#endif

#endif

/**
 * @}
 */
