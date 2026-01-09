/*
    TCP Echo Server
    To perform send() and recv()
    It receives data from a client and sends the same data back
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490"
#define BACKLOG 10

int main() {

    struct addrinfo hints, *res, *p;
    int sockfd, new_fd;
    int yes = 1;

    char buffer[ 1024 ];

    // Clear hints structure
    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_flags    = AI_PASSIVE;    // Use my IP

    // Get address info
    int status;
    status = getaddrinfo( NULL, PORT, &hints, &res );

    if( status != 0 ) {

        fprintf( stderr, "getaddrinfo error: %s\n", gai_strerror( status ) );

        exit( 1 );
    }

    // Loop through all results and bind to first possible
    for( p = res; p != NULL; p = p -> ai_next ) {

        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( sockfd == -1 ) {
            continue;
        }

        // Allow reuse of address
        setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes );

        if( bind( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {
            close( sockfd );
            continue;
        }

        break;   // Successfully bound
    }

    freeaddrinfo( res );

    // Check if bind failed
    if( p == NULL ) {
        printf( "Failed to bind socket\n" );
        exit( 1 );
    }

    // Start listening
    if( listen( sockfd, BACKLOG ) == -1 ) {
        perror( "listen" );
        exit( 1 );
    }

    printf( "Server is listening on %s...\n", PORT );

    // Accept incoming connection
    new_fd = accept( sockfd, NULL, NULL );

    if( new_fd == -1 ) {
        perror( "accept" );
        exit( 1 );
    }

    // Communication loop
    while( 1 ) {

        int bytes = recv( new_fd, buffer, sizeof buffer - 1, 0 );

        if( bytes == -1 ) {
            perror( "recv" );
            break;
        }

        if( bytes == 0 ) {
            printf( "Client disconnected\n" );
            break;
        }

        buffer[ bytes ] = '\0';

        printf( "Client says: %s\n", buffer );

        // Echo back
        if( send( new_fd, buffer, bytes, 0 ) == -1 ) {
            perror( "send" );
            break;
        }
        
    }

    close( new_fd );
    close( sockfd );

    return 0;
}