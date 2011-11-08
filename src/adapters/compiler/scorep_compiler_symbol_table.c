/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2011,
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

/**
 * @status     alpha
 * @file       scorep_compiler_symbol_table.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief Symbol table analysis functions.
 * Contains functions that read the symbol table of a executable and add all functions
 * found to a hashtable. For this it offers 2 possibilities:
 * @li A bfd based implementation is the best choice.
 * @li If no bfd is available, nm can be used instead. However, it involves system calls
 *     and scales worse.
 * @li If none of both is enabled, a dummy is compiled, which disables the adapter.
 *
 * It may be compiled with different defines:
 * @li If HAVE_LIBBFD is defined, it uses the bfd library to read the symbols.
 * @li If HAVE_NM is defined, it uses a system call to nm to read the symbols. This
 *     option is only available if HAVE_LIBBFD is undefined, because of worse scaling.
 * @li If GNU_DEMANGLE is defined it uses cplus_demangle() to demangle function names.
 * @li If INTEL_COMPILER is defined, regions do not use the address as key, but get a
 *     32 bit integer id as key.
 */

#include <config.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>

#ifdef HAVE_LIBBFD
#include <bfd.h>
#elif defined HAVE_NM
#include <SCOREP_Timing.h>
#endif /* HAVE_LIBBFD / HAVE_NM */

#include <SCOREP_Types.h>
#include <scorep_utility/SCOREP_Utils.h>
#include <SCOREP_Events.h>
#include <SCOREP_Definitions.h>
#include <SCOREP_RuntimeManagement.h>
#include <SCOREP_Filter.h>

#include <SCOREP_Compiler_Init.h>
#include <scorep_compiler_data.h>

#define SCOREP_COMPILER_BUFFER_LEN 512

extern char* scorep_compiler_executable;

#ifdef INTEL_COMPILER
extern void
scorep_compiler_name_add( const char* name,
                          int32_t     id );

#endif /* INTEL_COMPILER */

/* ***************************************************************************************
   Demangling declarations
*****************************************************************************************/

#if defined( GNU_DEMANGLE )
/* Declaration of external demangeling function */
/* It is contained in "demangle.h" */
extern char*
cplus_demangle( const char* mangled,
                int         options );

/* cplus_demangle options */
#define SCOREP_COMPILER_DEMANGLE_NO_OPTS   0
#define SCOREP_COMPILER_DEMANGLE_PARAMS    ( 1 << 0 )  /* include function arguments */
#define SCOREP_COMPILER_DEMANGLE_ANSI      ( 1 << 1 )  /* include const, volatile, etc. */
#define SCOREP_COMPILER_DEMANGLE_VERBOSE   ( 1 << 3 )  /* include implementation details */
#define SCOREP_COMPILER_DEMANGLE_TYPES     ( 1 << 4 )  /* include type encodings */

/* Demangeling style. */
static int scorep_compiler_demangle_style = SCOREP_COMPILER_DEMANGLE_PARAMS  |
                                            SCOREP_COMPILER_DEMANGLE_ANSI    |
                                            SCOREP_COMPILER_DEMANGLE_VERBOSE |
                                            SCOREP_COMPILER_DEMANGLE_TYPES;
#endif /* GNU_DEMANGLE */

/* ***************************************************************************************
   helper functions for symbaol table analysis
*****************************************************************************************/

/**
   Writes the path/filename of the executable into @a path. If the path is longer
   than the @a path, the last @a length characters of the path are written.
   @param path    A buffer into which the pat is written. The memoty for the buffer
                  must be allocated already.
   @param length  The size of the buffer @a path. If the retrieved path/filename of
                  the executable is longer than @a length, the path's head is truncated.
   @retruns true if the path of the executable was obtained successfully. Else false is
                 returned.
 */
static bool
scorep_compiler_get_exe( char   path[],
                         size_t length )
{
    int         pid;
    int         err;
    struct stat status;

    /**
     * by default, use /proc mechanism to obtain path to executable
     * in other cases, do it by examining SCOREP_APPPATH variable
     */

    /* First trial */
    /* Does not work this way. Must either use readlink (creates lots of
       problems with intel compilers) or leave it out.
       sprintf( path, "/proc/self/exe" );
       err = stat( path, &status );
       if ( err == 0 )
       {
        return true;
       }
     */

    /* Second trial */
    pid = getpid();
    sprintf( path, "/proc/%d/exe", pid );
    err = stat( path, &status );
    if ( err == 0 )
    {
        return true;
    }

    /* Third trial */
    sprintf( path, "/proc/%d/object/a.out", pid );
    err = stat( path, &status );
    if ( err == 0 )
    {
        return true;
    }
    else
    {
        /* try to get the path via environment variable */
        if ( scorep_compiler_executable == NULL )
        {
            SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER,
                                 "Meanwhile, you have to set the configuration variable"
                                 "'executable' to your local executable." );
            SCOREP_ERROR( SCOREP_ERROR_ENOENT, "Could not determine path of executable." );
            return false;
            /* we should abort here */
        }
        else
        {
            char*  exepath   = scorep_compiler_executable;
            size_t exelength = strlen( exepath );
            if ( exelength > length )
            {
                /* If path is too long for buffer, truncate the head.
                   add one charakter for the terminating 0.           */
                exepath   = scorep_compiler_executable + exelength - length + 1;
                exelength = length;
            }
            SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "  exepath = %s ", exepath );

            strncpy( path, exepath, exelength );
            return true;
        }
    }
}

void
scorep_compiler_process_symbol( long         addr,
                                const char*  funcname,
                                const char*  filename,
                                unsigned int lno )
{
    if ( filename != NULL )
    {
        SCOREP_IO_SimplifyPath( ( char* )filename );
    }

#ifdef GNU_DEMANGLE
    /* use demangled name if possible */
    char* dem_name = 0;
    if ( scorep_compiler_demangle_style >= 0 )
    {
        dem_name = cplus_demangle( funcname,
                                   scorep_compiler_demangle_style );
        if ( dem_name != NULL )
        {
            funcname = dem_name;
        }
    }
#endif  /* GNU_DEMANGLE */

    if ( ( strncmp( funcname, "POMP", 4 ) != 0 ) &&
         ( strncmp( funcname, "Pomp", 4 ) != 0 ) &&
         ( strncmp( funcname, "pomp", 4 ) != 0 ) &&
         ( strncmp( funcname, "SCOREP_", 7 ) != 0 ) &&
         ( strncmp( funcname, "scorep_", 7 ) != 0 ) &&
         ( strncmp( funcname, "OTF2_", 5 ) != 0 ) &&
         ( strncmp( funcname, "otf2_", 5 ) != 0 ) &&
         ( strncmp( funcname, "cube_", 5 ) != 0 ) &&
         ( !SCOREP_Filter_Match( filename, funcname, true ) ) )
    {
#ifdef INTEL_COMPILER
        /* Counter for the last assigned region id.
           When using the Intel VT_ instrumentation, the functions provide a 32 bit storage.
           Thus, addresses may not fit, thus, we provide an other id as key.
         */
        static int32_t region_counter = 1;

        /* ifort constructs function names like <module>_mp_<function>,
           while __VT_Entry gets something like <module>.<function>
           => replace _mp_ with a dot. */
        char* name = SCOREP_CStr_dup( funcname );
        for ( int i = 1; i + 5 < strlen( name ); i++ )
        {
            if ( strncmp( &name[ i ], "_mp_", 4 ) == 0 )
            {
                name[ i ] = '.';
                for ( int j = i + 1; j <= strlen( name ) - 2; j++ )
                {
                    name[ j ] = name[ j + 3 ];
                }
                break;
            }
        }

        /* icpc appends the signature of the function. Unfortunately,
           __VT_Entry gives a string without signature.
           => cut off signature  */
        for ( int i = 1; i + 1 < strlen( name ); i++ )
        {
            if ( name[ i ] == '(' )
            {
                name[ i ] = '\0';
                break;
            }
        }

        /* Store new symbol in hash table */
        scorep_compiler_hash_put( region_counter, name, filename, lno );
        scorep_compiler_name_add( name, region_counter );
        region_counter++;
        free( name );
#else
        scorep_compiler_hash_put( addr, funcname, filename, lno );
#endif      /* INTEL_COMPILER */
    }
}

/* ***************************************************************************************
   BFD based symbol table analysis
*****************************************************************************************/
#ifdef HAVE_LIBBFD
/**
 * Get symbol table using BFD. Stores all functions obtained from the symbol table
 * in a hashtable. The key of the hashtable is the function pointer. This must be done
 * during initialization of the GNU compiler adapter, becuase enter and exit events
 * provide only a file pointer.
 * It also collects information about source file and line number.
 */
void
scorep_compiler_get_sym_tab( void )
{
    bfd*      bfd_image = 0;
    int       nr_all_syms;
    int       i;
    size_t    size;
    asymbol** canonic_symbols;
    char*     exepath;
    char      path[ 512 ] = { 0 };

    /* initialize BFD */
    bfd_init();

    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "Read symbol table using BFD" );

    /* get the path from system */
    if ( !scorep_compiler_get_exe( path, SCOREP_COMPILER_BUFFER_LEN ) )
    {
        return;
    }
    bfd_image = bfd_openr( ( const char* )&path, 0 );
    if ( !bfd_image )
    {
        SCOREP_ERROR( SCOREP_ERROR_ENOENT, "BFD image not present at path: %s \n", path );
        return;
    }

    /* check image format   */
    if ( !bfd_check_format( bfd_image, bfd_object ) )
    {
        SCOREP_ERROR( SCOREP_ERROR_EIO, "BFD: bfd_check_format(): failed\n" );
        return;
    }

    /* return if file has no symbols at all */
    if ( !( bfd_get_file_flags( bfd_image ) & HAS_SYMS ) )
    {
        SCOREP_ERROR( SCOREP_ERROR_FILE_INTERACTION,
                      "BFD: bfd_get_file_flags(): failed \n" );
        return;
    }

    /* get the upper bound number of symbols */
    size = bfd_get_symtab_upper_bound( bfd_image );

    /* HAS_SYMS can be set even with no symbols in the file! */
    if ( size < 1 )
    {
        SCOREP_ERROR( SCOREP_ERROR_INVALID_SIZE_GIVEN,
                      "BFD: bfd_get_symtab_upper_bound(): < 1 \n" );
        return;
    }

    /* read canonicalized symbols  */
    canonic_symbols = ( asymbol** )malloc( size );

    nr_all_syms = bfd_canonicalize_symtab( bfd_image, canonic_symbols );
    if ( nr_all_syms < 1 )
    {
        SCOREP_ERROR( SCOREP_ERROR_INVALID_SIZE_GIVEN,
                      "BFD: bfd_canonicalize_symtab(): < 1\n" );
    }
    for ( i = 0; i < nr_all_syms; ++i )
    {
        long         addr;
        const char*  filename;
        const char*  funcname;
        unsigned int lno;

        /* Process only symbols of type function */
        if ( !( canonic_symbols[ i ]->flags & BSF_FUNCTION ) )
        {
            continue;
        }

        /* ignore system functions */
        if ( strncmp( canonic_symbols[ i ]->name, "bfd_", 4 ) == 0 ||
             strncmp( canonic_symbols[ i ]->name, "_bfd_", 5 ) == 0 ||
             strstr( canonic_symbols[ i ]->name, "@@" ) != NULL )
        {
            continue;
        }

        /* get filename and linenumber from debug info */
        /* needs -g */
        filename = NULL;
        lno      = SCOREP_INVALID_LINE_NO;

        /* calculate function address */
        addr = canonic_symbols[ i ]->section->vma + canonic_symbols[ i ]->value;

        /* get the source info for every function in case of gnu by default */
        /* calls BFD_SEND */
        bfd_find_nearest_line( bfd_image,
                               bfd_get_section( canonic_symbols[ i ] ),
                               canonic_symbols,
                               canonic_symbols[ i ]->value,
                               &filename,
                               &funcname,
                               &lno );

#ifndef INTEL_COMPILER
        /* The debug information has often undecorated function names,
           thus, they are nicer to use.
           If no debugging symbols are found and thus funcname is NULL,
           set it from the always present canonic symbols.
           However, to have the trace comparable to Scalasca and Vampir, we use
           always canonical symbols */

        //if ( funcname == NULL )
        {
            funcname = canonic_symbols[ i ]->name;
        }
#else
        /* However, because the intel compiler provides a fortran mangled
           function name to __VT_Entry for fortran routines. We use the
           non-debug name */
        funcname = canonic_symbols[ i ]->name;
#endif  /* INTEL_COMPILER */

        scorep_compiler_process_symbol( addr, funcname, filename, lno );
    }
    free( canonic_symbols );
    bfd_close( bfd_image );
    return;
}

#elif HAVE_NM

/* ***************************************************************************************
   nm based symbol table analysis
*****************************************************************************************/

/**
 * Write output from nm for @a exefile to @a nmfile.
 * @param exefile Filename of the executable which is analysed.
 * @param nmfile  Filename of the file to which the output is written.
 * @returns true if the nm output was created successfully, else it returns false.
 */
static bool
scorep_compiler_create_nm_file( char* nmfile,
                                char* exefile )
{
    char command[ 1024 ];
#ifdef GNU_DEMANGLE
    sprintf( command, "nm -Aol %s > %s", exefile, nmfile );
#else /* GNU_DEMANGLE */
    sprintf( command, "nm -ol %s > %s", exefile, nmfile );
#endif /* GNU_DEMANGLE */
    if ( system( command ) != EXIT_SUCCESS )
    {
        SCOREP_ERROR( SCOREP_ERROR_ON_SYSTEM_CALL,
                      "Failed to get symbol table output for binary %s using nm",
                      exefile );
        return false;
    }
    return true;
}


/**
 * Get symbol table by parsing the oputput from nm. Stores all functions obtained
 * from the symbol table
 * in a hashtable. The key of the hashtable is the function pointer. This must be done
 * during initialization of the GNU compiler adapter, becuase enter and exit events
 * provide only a file pointer.
 * It also collects information about source file and line number.

 */
void
scorep_compiler_get_sym_tab( void )
{
#ifdef INTEL_COMPILER
    /* Counter for the last assigned region id.
       When using the Intel VT_ instrumentation, the functions provide a 32 bit storage.
       Thus, addresses may not fit, thus, we provide an other id as key.
     */
    int32_t region_counter = 1;
#endif /* INTEL_COMPILER */
    FILE*   nmfile;
    char    line[ 1024 ];
    char    path[ SCOREP_COMPILER_BUFFER_LEN ] = { 0 };
    char    nmfilename[ 64 ];

    SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "Read symbol table using nm" );

    /* get the path from system */
    if ( !scorep_compiler_get_exe( path, SCOREP_COMPILER_BUFFER_LEN ) )
    {
        return;
    }

    /* open nm-file */
    sprintf( nmfilename, "scorep_nm_file.%" PRIu64, SCOREP_GetClockTicks() );
    if ( !scorep_compiler_create_nm_file( nmfilename, path ) )
    {
        return;
    }
    if ( !( nmfile = fopen( nmfilename, "r" ) ) )
    {
        SCOREP_ERROR_POSIX();
    }

    /* read lines */
    while ( fgets( line, sizeof( line ) - 1, nmfile ) )
    {
        char*        col;
        char         delim[ 2 ] = " ";
        int          col_num    = 0;
        int          length     = 0;

        long         addr     = -1;
        char*        filename = NULL;
        char*        funcname = NULL;
        unsigned int line_no  = SCOREP_INVALID_LINE_NO;

        if ( strlen( line ) == 0 || line[ 0 ] == ' ' )
        {
            continue;
        }

        if ( line[ strlen( line ) - 1 ] == '\n' )
        {
            line[ strlen( line ) - 1 ] = '\0';
        }

        /* split line to columns */
        col = strtok( line, delim );
        do
        {
            if ( col_num == 0 ) /* column 1 (address) */
            {
                length = strlen( col );
                addr   = strtol( col + length - sizeof( void* ) * 2, NULL, 16 );
                if ( addr == 0 )
                {
                    break;
                }
            }
            else if ( col_num == 1 ) /* column 2 (type) */
            {
                strcpy( delim, "\t" );
            }
            else if ( col_num == 2 ) /* column 3 (symbol) */
            {
                funcname = col;
                strcpy( delim, ":" );
            }
            else if ( col_num == 3 ) /* column 4 (filename) */
            {
                filename = col;
            }
            else /* column 5 (line number) */
            {
                line_no = atoi( col );
                if ( line_no == 0 )
                {
                    line_no = SCOREP_INVALID_LINE_NO;
                }
                break;
            }

            col_num++;
        }
        while ( ( col = strtok( 0, delim ) ) );

        if ( col_num >= 3 )
        {
            scorep_compiler_process_symbol( addr, funcname, filename, line_no );
        }
    }

    /* close nm-file */
    fclose( nmfile );
    remove( nmfilename );
}

#else /* HAVE_LIBBFD / HAVE_NM */

/* ***************************************************************************************
   dummy implememtation of symbol table analysis
*****************************************************************************************/

#warning Neither BFD nor nm are available. Thus, the symbol table can not be analyzed.
#warning The compiler adapter will be disabled.

/**
   Dummy implementation of symbol table analysis for the case that neither BFD
   nor nm are available. It allows to compile without error, but the compiler adapter
   is will not generate events, though the compiler instrumented the code, because
   it can not map the function pointers to names.
 */
void
scorep_compiler_get_sym_tab( void )
{
}

#endif /* HAVE_LIBBFD / HAVE_NM */
