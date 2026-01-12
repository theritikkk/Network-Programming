/*
   nonblocking_client.c

   Demonstrates:
    - socket()
    - fcntl() to set NON-BLOCKING mode
    - non-blocking connect()
    - recv() with EAGAIN / EWOULDBLOCK handling

   Compile:
    gcc -Wall -Wextra -pedantic nonblocking_client.c -o nonblocking_client

   Run:
    ./nonblocking_client localhost 3490
*/


#include <stdio.h>      // printf(), fprintf()
#include <stdlib.h>     // exit()
#include <string.h>     // memset()
#include <unistd.h>     // close(), sleep()
#include <errno.h>      // errno, EINPROGRESS, EAGAIN
#include <fcntl.h>     // fcntl(), O_NONBLOCK

#include <sys/types.h>
#include <sys/socket.h> // socket(), connect(), recv()
#include <netdb.h>     // getaddrinfo()

#define BUFSIZE 1024
// max size of receive buffer

int main(int argc, char *argv[])
{
    int sockfd;                 // socket descriptor
    struct addrinfo hints;      // filter for getaddrinfo
    struct addrinfo *res;       // result list
    struct addrinfo *p;         // iterator

    int flags;                  // socket flags

    char buffer[ BUFSIZE ];     // recv buffer
    int rv;                     // return value


    /* ================= ARG CHECK ================= */

    if( argc != 3 ) {
        fprintf( stderr, "Usage: %s <hostname> <port>\n", argv[ 0 ] );
        exit( 1 );
    }
    // for client to run : ./client hostname port
    // example : ./client localhost 3490


    /* ================= DNS RESOLUTION ================= */

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;          // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;        // TCP socket

    // hostname + port => IP addresses
    rv = getaddrinfo( argv[ 1 ], argv[ 2 ], &hints, &res );

    if( rv != 0 ) {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );
        exit( 2 );
    }


    /* ================= CREATE SOCKET ================= */

    for( p = res; p != NULL; p = p -> ai_next ) {
        // loop through all resolved addresse

        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );
        //creates socket

        if( sockfd == -1 ) {
            perror( "socket" );

            // If fail : try next address
            continue;
        }

        // stop when socket created
        break;
    }

    if( p == NULL ) {
        
        // No socket created - exit.

        fprintf( stderr, "client: failed to create socket\n" );
        exit( 3 );
    }


    /* ================= SET NON-BLOCKING ================= */

    flags = fcntl( sockfd, F_GETFL, 0 );
    // gets current socket flags

    if( flags == -1 ) {
        perror( "fcntl F_GETFL" );

        // error handling
        exit( 4 );
    }

    if( fcntl( sockfd, F_SETFL, flags | O_NONBLOCK ) == -1) {
        // O_NONBLOCK : now socket is NON - BLOCKING

        perror( "fcntl F_SETFL" );
        
        exit( 5 );
    }

    printf( "Socket set to NON-BLOCKING mode\n" );



    /* ================= NON-BLOCKING CONNECT ================= */

    // tries to connect
    rv = connect( sockfd, p -> ai_addr, p -> ai_addrlen );

    // connect failed
    if( rv == -1 ) {

        // connection is still in progress ( normal for non - blocking )
        if( errno == EINPROGRESS ) {

            printf( "Connection in progress...\n" );
        }

        else {

            perror( "connect" );

            // real error : exit
            exit( 6 );
        }
    }

    else {

        // connected instantly ( rare )
        printf( "Connected immediately!\n" );
    }

    // Demo wait
    sleep( 2 );
    // to allow server time to respond ( not required in real programs )



    /* ================= TRY RECV ================= */

    // attempts to receive data
    int bytes = recv( sockfd, buffer, sizeof( buffer ) - 1, 0 );

    if( bytes == -1 ) {
        // recv failed

        if( errno == EAGAIN || errno == EWOULDBLOCK ) {
            /*
                No data available YET
                NOT an error
                Because socket is NON - BLOCKING
            */

            printf( "No data available yet " "(non-blocking)\n" );
        }

        else {
            // real error
            perror( "recv" );
        }

    }
    else if( bytes == 0 ) {

        // server closed connection
        printf( "Server closed connection\n" );
    }
    else {

        // valid data received
        buffer[ bytes ] = '\0';

        printf( "Received: %s\n", buffer );
    }



    /* ================= CLEANUP ================= */

    freeaddrinfo( res );
    // release memory
    
    // close socket
    close( sockfd );

    return 0;

}
