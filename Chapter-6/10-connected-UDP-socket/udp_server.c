/*
   udp_server.c

   Connected UDP server
   Uses connect() + recv()

   Demonstrates:
    - UDP socket (SOCK_DGRAM)
    - connect() on datagram socket
    - recv() instead of recvfrom()

   Key idea:
    Even though UDP is connectionless, calling connect()
    locks this socket to ONE peer, allowing:
        - send()
        - recv()
    instead of:
        - sendto()
        - recvfrom()

   Compile:
    gcc -Wall -Wextra -pedantic udp_server.c -o udp_server

   Run:
    ./udp_server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "5050"     // Server listens on this port
#define BUFSIZE 1024    // Buffer size for incoming data


int main( void ) {

    int sockfd;     // UDP socket descriptor

    struct addrinfo hints;  // Input for getaddrinfo()
    struct addrinfo *res;   // Result list
    struct addrinfo *p;     // Iterator

    struct sockaddr_storage client_addr; 
    // Will store client IP + port

    socklen_t addr_len;

    char buffer[ BUFSIZE ];
    int bytes;



    /* ================= STEP 1: SETUP HINTS ================= */

    /*
        We tell the OS what kind of socket we want
    */

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_INET6;     // Force IPv6
    hints.ai_socktype = SOCK_DGRAM;   // UDP socket
    hints.ai_flags    = AI_PASSIVE;   // Bind to my local IP



    /* ================= STEP 2: GET LOCAL ADDRESS ================= */

    /*
        getaddrinfo():
            - Resolves local address
            - Creates structures with IP + port info
    */

    if( getaddrinfo( NULL, PORT, &hints, &res ) != 0 ) {
        perror( "getaddrinfo" );
        exit( 1 );
    }

    

    /* ================= STEP 3: CREATE SOCKET + BIND ================= */

    /*
        Try each returned address until bind() works
    */

    for( p = res; p != NULL; p = p -> ai_next ) {

        /*
            socket():
                Kernel creates UDP socket
        */
        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );
        
        if( sockfd == -1 ) {
            continue;
        }

        /*
            bind():
                Attach socket to:
                    - local IP
                    - port 5050
        */
        if( bind( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {

            close( sockfd );
            continue;
        }

        break;  // success

    }

    freeaddrinfo( res );

    if( p == NULL ) {

        fprintf( stderr, "server: failed to bind\n" );

        exit( 2 );
    }

    printf( "server: waiting for first packet...\n" );



    /* ================= STEP 4: RECEIVE FIRST PACKET ================= */

    /*
        recvfrom():
            - Blocks until packet arrives
            - Fills:
                buffer        -> message
                client_addr   -> sender IP + port

        This is REQUIRED:
            We don't know client address yet
            So we must use recvfrom() first
    */

    addr_len = sizeof client_addr;

    bytes = recvfrom( sockfd, buffer, BUFSIZE - 1, 0, ( struct sockaddr* ) &client_addr, &addr_len );

    buffer[ bytes ] = '\0';

    printf( "server: first message = %s\n", buffer );



    /* ================= STEP 5: CONNECT UDP SOCKET ================= */

    /*
        connect() on UDP:

            - NO handshake
            - NO reliability
            - Simply stores:
                - peer IP
                - peer port

        After this:
            Socket will ONLY accept packets
            from this client
    */

    connect( sockfd, ( struct sockaddr* ) &client_addr, addr_len );

    printf( "server: UDP socket now CONNECTED\n" );



    /* ================= STEP 6: NOW USE recv() ================= */

    /*
        Now we can use:

            recv()

        because:
            - peer is locked
            - kernel filters packets

        No sender address needed anymore
    */

    bytes = recv( sockfd, buffer, BUFSIZE - 1, 0 );

    buffer[ bytes ] = '\0';

    printf( "server: second message = %s\n", buffer );



    /* ================= STEP 7: CLEANUP ================= */

    close( sockfd );   // Free kernel socket

}
