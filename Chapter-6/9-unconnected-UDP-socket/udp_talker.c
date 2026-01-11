/*
   udp_talker.c

   UDP datagram socket client
   Uses sendto()

   Compile:
    gcc -Wall -Wextra -pedantic udp_talker.c -o udp_talker

   Run:
    ./udp_talker <hostname> <message>

   Example:
    ./udp_talker localhost "hello UDP"
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVERPORT "4950"


int main( int argc, char *argv[] ) {

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;


    /* STEP 0: ARG CHECK */

    if( argc != 3 ) {
        fprintf( stderr, "usage: %s hostname message\n", argv[ 0 ] );
        exit( 1 );
    }


    /* STEP 1: SETUP HINTS */

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_INET6;   // force IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP


    /* STEP 2: RESOLVE HOST */

    rv = getaddrinfo( argv[ 1 ], SERVERPORT, &hints, &servinfo );

    if( rv != 0 ) {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );
        exit( 2 );
    }


    /* STEP 3: CREATE SOCKET */

    for( p = servinfo; p != NULL; p = p -> ai_next ) {

        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( sockfd == -1 ) {
            perror( "talker: socket" );
            continue;
        }

        break;
    }

    if( p == NULL ) {
        fprintf( stderr, "talker: failed to create socket\n" );
        exit( 3 );
    }


    /* STEP 4: SEND PACKET */

    numbytes = sendto( sockfd, argv[ 2 ], strlen( argv[ 2 ] ), 0, p -> ai_addr, p -> ai_addrlen );

    if( numbytes == -1 ) {
        perror( "talker: sendto" );
        exit( 4 );
    }

    printf( "talker: sent %d bytes to %s\n", numbytes, argv[ 1 ] );


    /* STEP 5: CLEANUP */

    freeaddrinfo( servinfo );
    
    close( sockfd );

    return 0;
}
