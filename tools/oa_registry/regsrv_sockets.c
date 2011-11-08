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
 * @file regsrv_sockets.c
 * @maintainer Yury Oleynik <oleynik@in.tum.de>
 * @status alpha
 *
 * This file contains implementation of sockets communication functions
 */
#include <config.h>

#include "regsrv_sockets.h"
#include "scorep_oa_registry_protocol.h"

#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static int   read_cnt;
static char* read_ptr;
static char  read_buf[ 1000 ];


int
scorep_oa_sockets_server_startup_retry
(
    int* init_port,
    int  retries,
    int  step
)
{
    int                sock;
    int                yes  = 1;
    int                stat = -1;
    int                port;
    struct sockaddr_in my_addr;                 /* my address information */
    ////SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "Entering %s\n", __FUNCTION__ );

    /**
     * create a new socket socket() returns positive interger on success
     */

    for ( port = *init_port; port <= *init_port + retries * step && stat == -1; port = port + step )
    {
        stat = 0;

        if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
        {
            if ( port + step > *init_port + retries * step )
            {
                //SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "socket_server_startup::socket()\n" );
            }
            stat = -1;
        }
        else
        {
            if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) < 0 )
            {
                if ( port + step > *init_port + retries * step )
                {
                    //SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "socket_server_startup::setsockopt()\n" );
                }
                stat = -1;
            }
            else
            {
                my_addr.sin_family      = AF_INET;                        /** host byte order */
                my_addr.sin_port        = htons( port );                  /** short, network byte order */
                my_addr.sin_addr.s_addr = INADDR_ANY;                     /** automatically fill with my IP */
                memset( &( my_addr.sin_zero ), '\0', 8 );                 /** zero the rest of the struct */

                if ( bind( sock, ( struct sockaddr* )&my_addr, sizeof( struct sockaddr ) ) < 0 )
                {
                    if ( port + step > *init_port + retries * step )
                    {
                        //SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "socket_server_startup::bind()\n" );
                    }
                    stat = -1;
                }
                else
                {
                    if ( listen( sock, 1 ) < 0 )
                    {
                        if ( port + step > *init_port + retries * step )
                        {
                            //SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "socket_server_startup::listen()\n" );
                        }
                        stat = -1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    if ( stat == -1 )
    {
        return -1;
    }
    else
    {
        //SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "Exiting %s with successs, port = %d\n", __FUNCTION__, port );
        *init_port = port;
        return sock;
    }
}



int
scorep_oa_sockets_client_connect_retry
(
    const char* hostname,
    int         port,
    int         retries
)
{
    int                sock;
    struct sockaddr_in pin;
    struct hostent*    hp;
    int                success, i;

    success = -1;
    for ( i = 0; i < retries && success == -1; i++ )
    {
        sleep( 4 );
        success = 0;
        if ( ( hp = gethostbyname( hostname ) ) == 0 )
        {
            if ( i == retries - 1 )
            {
                perror( "socket_client_connect::gethostbyname()" );
            }
            success = -1;
        }
        else
        {
            /* fill in the socket structure with host information */
            memset( &pin, 0, sizeof( pin ) );
            pin.sin_family      = AF_INET;
            pin.sin_addr.s_addr = ( ( struct in_addr* )( hp->h_addr_list[ 0 ] ) )->s_addr;
            pin.sin_port        = htons( port );

            /* grab an Internet domain socket */
            if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
            {
                if ( i == retries - 1 )
                {
                    perror( "socket_client_connect::socket()" );
                }
                success = -1;
            }
            else
            {
                /* connect to PORT on HOST */
                if ( connect( sock, ( struct sockaddr* )&pin, sizeof( pin ) ) == -1 )
                {
                    if ( i == retries - 1 )
                    {
                        perror( "socket_client_connect::connect()" );
                    }
                    success = -1;
                }
            }
        }
    }
    if ( success == -1 )
    {
        sock = -1;
    }
    return sock;
}




void
scorep_oa_sockets_write_line
(
    int         sock,
    const char* str
)
{
    int result = write( sock, str, strlen( str ) );
}

void
scorep_oa_sockets_write_data
(
    int         sock,
    const void* buf,
    int         nbyte
)
{
    int result = write( sock, buf, nbyte );
}

int
scorep_oa_sockets_socket_my_read
(
    int   fd,
    char* ptr
)
{
    if ( read_cnt <= 0 )
    {
again:
        if ( ( read_cnt = read( fd, read_buf, sizeof( read_buf ) ) ) < 0 )
        {
            if ( errno == EINTR )
            {
                goto again;
            }
            return -1;
        }
        else
        if ( read_cnt == 0 )
        {
            return 0;
        }
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}


int
scorep_oa_sockets_blockread
(
    int   sock,
    char* ptr,
    int   size
)
{
    int  n, rc;
    char c;

    for ( n = 1; n <= size; n++ )
    {
        rc = scorep_oa_sockets_socket_my_read( sock, &c );
        if ( rc == 1 )
        {
            *ptr++ = c;
        }
        else
        if ( rc == 0 )
        {
            sleep( 1 );
        }
        else
        {
            return -1;
        }
    }
    return n - 1;
}



int
scorep_oa_sockets_read_line
(
    int   sock,
    char* str,
    int   maxlen
)
{
    int  n, rc;
    char c, * ptr;

    ptr = str;
    for ( n = 1; n < maxlen; n++ )
    {
        if ( ( rc = scorep_oa_sockets_socket_my_read( sock, &c ) ) == 1 )
        {
            if ( c  == '\n' )
            {
                break;                          /* newline is stored, like fgets() */
            }
            *ptr++ = c;
        }
        else
        if ( rc == 0 )
        {
            *ptr = 0;
            return n - 1;                           /* EOF, n - 1 bytes were read */
        }
        else
        {
            return -1;                              /* error, errno set by read() */
        }
    }

    *ptr = 0;          /* null terminate like fgets() */

    return n;
}

int
scorep_oa_sockets_server_accept_client
(
    int sock
)
{
    int                newsock;

    struct sockaddr_in client_addr;       /* client's address information */

//size_t sin_size;
    unsigned int sin_size;

    sin_size = sizeof( struct sockaddr_in );

    if ( ( newsock = accept( sock, ( struct sockaddr* )&client_addr, &sin_size ) ) < 0 )
    {
        //SCOREP_DEBUG_PRINTF( SCOREP_DEBUG_OA, "socket_server_accept_client::accept() error" );
        return -1;
    }

    return newsock;
}
