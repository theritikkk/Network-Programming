/*
    client.c

    UDP Client using sendto() and recvfrom()

    What it does :
    - sends message to server
    - receives echoed response
    - demonstrates connectionless communication
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main( int argc, char *argv[] ) {

    if( argc != 3 ) {

        printf( "Usage: %s <server-ip> <port>\n", argv[ 0 ] );
        exit( 1 );

    }

    struct addrinfo hints, *res, *p;
    int sockfd;

    char msg[ 1024 ];
    char buffer[ 1024 ];

    struct sockaddr_storage server_addr;
    socklen_t addr_len;

    // Clear hints structure
    memset( &hints, 0, sizeof hints );

    // Setup hints
    hints.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;    // UDP socket

    // Get server address info
    int status;
    status = getaddrinfo( argv[ 1 ], argv[ 2 ], &hints, &res );

    if( status != 0 ) {

        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( status ) );
        exit( 1 );

    }

    // Loop through results and create socket
    for( p = res; p != NULL; p = p -> ai_next ) {

        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( sockfd == -1 ) {
            continue;
        }

        break;   // Socket created successfully
    }

    if( p == NULL ) {
        printf( "Failed to create socket\n" );
        freeaddrinfo( res );
        exit( 1 );
    }

    // Store server address
    memcpy( &server_addr, p -> ai_addr, p -> ai_addrlen );
    addr_len = p -> ai_addrlen;

    freeaddrinfo( res );

    printf( "Connected to UDP server %s:%s\n", argv[1], argv[2] );

    // Communication loop
    while( 1 ) {

        printf( "Enter message (type exit to quit): " );
        fgets( msg, sizeof msg, stdin );

        // exit condition
        if( strncmp( msg, "exit", 4 ) == 0 ) {
            break;
        }

        // Send message to server
        if( sendto( sockfd, msg, strlen( msg ), 0, ( struct sockaddr * ) &server_addr, addr_len ) == -1 ) {

            perror( "sendto" );

            break;

        }

        // Receive echoed message
        int bytes = recvfrom( sockfd, buffer, sizeof buffer - 1, 0, NULL, NULL );

        if( bytes == -1 ) {

            perror( "recvfrom" );
            
            break;

        }

        buffer[ bytes ] = '\0';

        printf( "Server replied: %s\n", buffer );
    }

    close( sockfd );

    return 0;
    
}
