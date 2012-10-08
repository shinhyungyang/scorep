/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2009-2012,
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
 * @file       scorep_compiler_data_intel.c
 * @maintainer Daniel Lorenz <d.lorenz@fz-juelich.de>
 *
 * @brief Implementation of helper functions for managing region data.
 */

#include <config.h>
#include <string.h>
#include <unistd.h>

#include <SCOREP_Definitions.h>
#include <SCOREP_Mutex.h>
#include <UTILS_Error.h>
#include <UTILS_Debug.h>
#include <SCOREP_Hashtab.h>

#include <scorep_compiler_data_intel.h>

/**
   A hash table which stores information about regions under their name as
   key. Mainly used to obtain the region handle from the region name.
 */
scorep_compiler_hash_node* region_hash_table[ SCOREP_COMPILER_REGION_SLOTS ];

/**
   Hash table for mapping source file names to SCOREP file handles.
 */
SCOREP_Hashtab* scorep_compiler_file_table = NULL;

/**
   Mutex for mutual exclusive access to @ref scorep_compiler_file_table.
 */
SCOREP_Mutex scorep_compiler_file_table_mutex;

/* ***************************************************************************************
   File hash table functions
*****************************************************************************************/

/**
   Deletes one file table entry.
   @param entry Pointer to the entry to be deleted.
 */
static void
scorep_compiler_delete_file_entry( SCOREP_Hashtab_Entry* entry )
{
    UTILS_ASSERT( entry );

    free( ( SCOREP_SourceFileHandle* )entry->value );
    free( ( char* )entry->key );
}

/* Initialize the file table */
void
scorep_compiler_init_file_table()
{
    SCOREP_MutexCreate( &scorep_compiler_file_table_mutex );
    scorep_compiler_file_table = SCOREP_Hashtab_CreateSize( SCOREP_COMPILER_FILE_SLOTS,
                                                            &SCOREP_Hashtab_HashString,
                                                            &SCOREP_Hashtab_CompareStrings );
}

/* Finalize the file table */
void
scorep_compiler_final_file_table()
{
    SCOREP_Hashtab_Foreach( scorep_compiler_file_table, &scorep_compiler_delete_file_entry );
    SCOREP_Hashtab_Free( scorep_compiler_file_table );
    scorep_compiler_file_table = NULL;
    SCOREP_MutexDestroy( &scorep_compiler_file_table_mutex );
}

/* Returns the file handle for a given file name. */
SCOREP_SourceFileHandle
scorep_compiler_get_file( const char* file )
{
    size_t                index;
    SCOREP_Hashtab_Entry* entry = NULL;

    if ( file == NULL )
    {
        return SCOREP_INVALID_SOURCE_FILE;
    }

    SCOREP_MutexLock( scorep_compiler_file_table_mutex );

    entry = SCOREP_Hashtab_Find( scorep_compiler_file_table, file,
                                 &index );

    /* If not found, register new file */
    if ( !entry )
    {
        /* Reserve own storage for file name */
        char* file_name = ( char* )malloc( ( strlen( file ) + 1 ) * sizeof( char ) );
        strcpy( file_name, file );

        /* Register file to measurement system */
        SCOREP_SourceFileHandle* handle = malloc( sizeof( SCOREP_SourceFileHandle ) );
        *handle = SCOREP_DefineSourceFile( file_name );

        /* Store handle in hashtable */
        SCOREP_Hashtab_Insert( scorep_compiler_file_table, ( void* )file_name,
                               handle, &index );

        SCOREP_MutexUnlock( scorep_compiler_file_table_mutex );
        return *handle;
    }

    SCOREP_MutexUnlock( scorep_compiler_file_table_mutex );
    return *( SCOREP_SourceFileHandle* )entry->value;
}



/* ***************************************************************************************
   Region hash table functions
*****************************************************************************************/

/* Initialize slots of compiler hash table. */
void
scorep_compiler_hash_init()
{
    uint64_t i;

    scorep_compiler_init_file_table();

    for ( i = 0; i < SCOREP_COMPILER_REGION_SLOTS; i++ )
    {
        region_hash_table[ i ] = NULL;
    }
}

/* Get hash table entry for given name. */
scorep_compiler_hash_node*
scorep_compiler_hash_get( char* region_name )
{
    char* name;
    /* The intel compiler prepends the filename to the function name.
       -> Need to remove the file name. */
    name = region_name;
    while ( *name != '\0' )
    {
        if ( *name == ':' )
        {
            region_name = name + 1;
            break;
        }
        name++;
    }

    uint64_t hash_code = SCOREP_Hashtab_HashString( region_name ) % SCOREP_COMPILER_REGION_SLOTS;

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, " hash code %ld", hash_code );

    scorep_compiler_hash_node* curr = region_hash_table[ hash_code ];
    /* The tail after curr will never change because, new elements are inserted before
       curr. Thus, it allows a parallel @ref scorep_compiler_hash_put which can only
       insert a new element before curr.
     */
    while ( curr )
    {
        if ( strcmp( curr->region_name_mangled, region_name )  == 0 )
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/* Stores function name under hash code */
scorep_compiler_hash_node*
scorep_compiler_hash_put( uint64_t      key,
                          const char*   region_name_mangled,
                          const char*   region_name_demangled,
                          const char*   file_name,
                          SCOREP_LineNo line_no_begin )
{
    /* ifort constructs function names like <module>_mp_<function>,
     * while __VT_Entry gets something like <module>.<function>
     * => replace _mp_ with a dot. */
    char* name = UTILS_CStr_dup( region_name_mangled );
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
     * __VT_Entry gives a string without signature.
     * => cut off signature  */
    for ( int i = 1; i + 1 < strlen( name ); i++ )
    {
        if ( name[ i ] == '(' )
        {
            name[ i ] = '\0';
            break;
        }
    }

    uint64_t                   hash_code = SCOREP_Hashtab_HashString( name ) % SCOREP_COMPILER_REGION_SLOTS;
    scorep_compiler_hash_node* add       = ( scorep_compiler_hash_node* )
                                           malloc( sizeof( scorep_compiler_hash_node ) );
    add->key                   = key;
    add->region_name_mangled   = UTILS_CStr_dup( name );
    add->region_name_demangled = UTILS_CStr_dup( region_name_demangled );
    add->file_name             = UTILS_CStr_dup( file_name );
    add->line_no_begin         = line_no_begin;
    add->line_no_end           = SCOREP_INVALID_LINE_NO;
    add->region_handle         = SCOREP_INVALID_REGION;
    /* Inserting elements at the head allows parallel calls to
     * @ref scorep_compiler_hash_get
     */
    add->next                      = region_hash_table[ hash_code ];
    region_hash_table[ hash_code ] = add;

    free( name );
    return add;
}


/* Free elements of compiler hash table. */
void
scorep_compiler_hash_free()
{
    scorep_compiler_hash_node* next;
    scorep_compiler_hash_node* cur;
    uint64_t                   i;
    for ( i = 0; i < SCOREP_COMPILER_REGION_SLOTS; i++ )
    {
        if ( region_hash_table[ i ] )
        {
            cur = region_hash_table[ i ];
            while ( cur != NULL )
            {
                next = cur->next;
                if ( cur->region_name_mangled != NULL )
                {
                    free( cur->region_name_mangled );
                }
                if ( cur->region_name_demangled != NULL )
                {
                    free( cur->region_name_demangled );
                }
                if ( cur->file_name != NULL )
                {
                    free( cur->file_name );
                }
                free( cur );
                cur = next;
            }
            region_hash_table[ i ] = NULL;
        }
    }

    scorep_compiler_final_file_table();
}

/* Register a new region to the measuremnt system */
void
scorep_compiler_register_region( scorep_compiler_hash_node* node )
{
    SCOREP_SourceFileHandle file_handle = scorep_compiler_get_file( node->file_name );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "Define region %s", node->region_name_demangled );

    node->region_handle = SCOREP_DefineRegion( node->region_name_demangled,
                                               node->region_name_mangled,
                                               file_handle,
                                               node->line_no_begin,
                                               node->line_no_end,
                                               SCOREP_ADAPTER_COMPILER,
                                               SCOREP_REGION_FUNCTION );

    UTILS_DEBUG_PRINTF( SCOREP_DEBUG_COMPILER, "Define region %s done", node->region_name_demangled );
}
