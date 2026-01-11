/*
   udp_client.c

   Connected UDP client
   Uses connect() + send()

   Demonstrates:
    - UDP socket (SOCK_DGRAM)
    - connect() on datagram socket
    - send() instead of sendto()

   Compile:
    gcc -Wall -Wextra -pedantic udp_client.c -o udp_client

   Run:
    ./udp_client localhost
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "5050"


int main( int argc, char *argv[] ) {

    int sockfd;                     // UDP socket descriptor
    
    struct addrinfo hints, *res;    // For DNS + service resolution



    /* ================= STEP 0: ARGUMENT CHECK ================= */

    /*
        Client needs:
            argv[1] -> hostname of server
    */

    if( argc != 2 ) {
        fprintf( stderr, "usage: %s hostname\n", argv[ 0 ] );
        exit( 1 );
    }


    /* ================= STEP 1: SETUP HINTS ================= */

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_INET6;    // Force IPv6
    hints.ai_socktype = SOCK_DGRAM;  // UDP socket


    /* ================= STEP 2: RESOLVE SERVER ADDRESS ================= */

    /*
        getaddrinfo():
            - resolves hostname -> IP
            - resolves service -> port
            - fills sockaddr structs
    */

    getaddrinfo( argv[ 1 ], PORT, &hints, &res );



    /* ================= STEP 3: CREATE SOCKET ================= */

    /*
        socket():
            Kernel allocates UDP socket
    */

    sockfd = socket( res -> ai_family, res -> ai_socktype, res -> ai_protocol );



    /* ================= STEP 4: CONNECT UDP SOCKET ================= */

    /*
        connect() on UDP:

            - NO handshake
            - NO reliability
            - Just stores peer address inside kernel

        After this:
            - socket is LOCKED to this server
            - send() auto fills destination
            - recv() only accepts from server
    */

    connect( sockfd, res -> ai_addr, res -> ai_addrlen );

    printf( "client: connected UDP socket\n" );



    /* ================= STEP 5: SEND DATA ================= */

    /*
        send():

            - copies bytes from user space -> kernel
            - kernel sends UDP packet
            - destination = connected peer
    */

    send( sockfd, "Hello server", 12, 0 );
    send( sockfd, "Second message", 14, 0 );



    /* ================= STEP 6: CLEANUP ================= */

    freeaddrinfo( res );   // free DNS results
    close( sockfd );       // close UDP socket

}
