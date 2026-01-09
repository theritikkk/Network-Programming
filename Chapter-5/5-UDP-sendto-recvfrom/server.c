/*
    server.c

    UDP server using recvfrom() and sendto()
    What it does :
        - connectionless communication
        - recvfrom() blocking behavior
        - getting client address
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490"   // Port number as string

int main() {

    struct addrinfo hints, *res, *p;   // For address info
    
    int sockfd;                       // Socket file descriptor

    char buffer[ 1024 ];                // Buffer to store messages

    struct sockaddr_storage client_addr; // Stores client address
    socklen_t addr_len;                   // Length of address

    // Clear garbage values of hints structure
    memset( &hints, 0, sizeof hints );

    // Setup hints
    hints.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;    // UDP socket
    hints.ai_flags    = AI_PASSIVE;    // Use my IP

    // Get address info
    int status;
    status = getaddrinfo( NULL, PORT, &hints, &res );

    // Check for error
    if( status != 0 ) {

        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( status ) );

        exit( 1 );

    }

    // Loop through all results and bind
    for( p = res; p != NULL; p = p -> ai_next ) {

        // Create socket
        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( sockfd == -1 ) {
            continue;
        }

        // Bind socket to port
        if( bind( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {

            close( sockfd );

            continue;
        }

        break;   // Successfully bound
    }

    freeaddrinfo( res );   // Free memory

    // Check bind success
    if( p == NULL ) {
        printf( "Failed to bind\n" );
        exit( 1 );
    }

    printf( "UDP Server listening on %s...\n", PORT );

    // Main communication loop
    while( 1 ) {

        addr_len = sizeof client_addr;

        // Receive message from client
        int bytes = recvfrom( sockfd, buffer, sizeof buffer - 1, 0, ( struct sockaddr * ) &client_addr, &addr_len );

        // Error check
        if( bytes == -1 ) {
          
            perror("recvfrom");
            continue;

        }

        // Null terminate message
        buffer[ bytes ] = '\0';

        printf( "Client says: %s\n", buffer );

        // Send same message back (echo)
        if( sendto( sockfd, buffer, bytes, 0, ( struct sockaddr * ) &client_addr, addr_len ) == -1 ) {

            perror("sendto");

        }

    }

    close( sockfd );   // Close socket

    return 0;
    
}
