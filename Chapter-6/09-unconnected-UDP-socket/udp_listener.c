/*
   udp_listener.c

   UDP datagram socket "server"
   Uses recvfrom()

   Demonstrates:
    - UDP sockets (SOCK_DGRAM)
    - IPv6 usage
    - recvfrom()
    - sender address extraction

   Compile:
    gcc -Wall -Wextra -pedantic udp_listener.c -o udp_listener

   Run:
    ./udp_listener
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

#define MYPORT "4950"   // Port we listen on
#define MAXBUFLEN 100   // Max bytes to receive


/* ================= HELPER FUNCTION ================= */

// Extract IP address (IPv4 or IPv6)
void *get_in_addr( struct sockaddr *sa ) {

    // // Check address family
    if( sa -> sa_family == AF_INET ) {

        // IPv4 → return sin_addr
        return &( ( ( struct sockaddr_in* ) sa ) -> sin_addr );

    }

    // IPv6 → return sin6_addr
    return &( ( ( struct sockaddr_in6* ) sa ) -> sin6_addr );

}



/* ================= MAIN FUNCTION ================= */

int main( void ) {

    int sockfd;                   // Socket file descriptor

    struct addrinfo hints;        // Input to getaddrinfo()
    struct addrinfo *servinfo;    // Result list
    struct addrinfo *p;           // Iterator
    int rv;

    char buf[ MAXBUFLEN ];        // Receive buffer

    struct sockaddr_storage their_addr;     // Sender address
    socklen_t addr_len;     

    int numbytes;

    char ipstr[ INET6_ADDRSTRLEN ];     // Printable IP


    /* STEP 1: SETUP HINTS */

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_INET6;   // force IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags    = AI_PASSIVE; // bind to my IP



    /* STEP 2: GET ADDRESS */

    // to resolve local address
    rv = getaddrinfo( NULL, MYPORT, &hints, &servinfo );

    if( rv != 0 ) {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );
        exit( 1 );
    }


    /* STEP 3: CREATE SOCKET + BIND */

    for( p = servinfo; p != NULL; p = p -> ai_next ) {

        // create UDP socket
        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( sockfd == -1 ) {
            perror( "listener: socket" );
            continue;
        }

        // bind socket to port
        if( bind( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {
            close( sockfd );
            perror( "listener: bind" );
            continue;
        }

        break;  // success
    }

    if( p == NULL ) {
        fprintf( stderr, "listener: failed to bind\n ");
        exit( 2 );
    }

    freeaddrinfo( servinfo );


    printf( "listener: waiting to recvfrom...\n" );


    /* STEP 4: RECEIVE PACKET */

    addr_len = sizeof their_addr;

    numbytes = recvfrom( sockfd, buf, MAXBUFLEN - 1, 0, ( struct sockaddr * ) &their_addr, &addr_len );

    if( numbytes == -1 ) {
        perror( "recvfrom" );
        exit( 3 );
    }

    buf[ numbytes ] = '\0';     // Null terminate


    /* STEP 5: PRINT INFO */

    inet_ntop( their_addr.ss_family, get_in_addr( ( struct sockaddr * ) &their_addr ), ipstr, sizeof ipstr );

    printf( "listener: got packet from %s\n", ipstr );
    printf( "listener: packet size = %d bytes\n", numbytes );
    printf( "listener: message = \"%s\"\n", buf );


    /* STEP 6: CLEANUP */

    close( sockfd );

    return 0;

}
