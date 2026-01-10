/*
 client.c

 Simple TCP client
 Connects to server and receives greeting message
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

int main() {

    struct addrinfo hints, *res;   // For storing address info
    int sockfd;                   // Socket file descriptor

    // Clear hints structure
    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP socket

    int status;

    // Get server address info
    status = getaddrinfo( "localhost", "3490", &hints, &res );

    if( status != 0 ) {
    
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror(status) );
    
        exit( 1 );
    }

    // Create socket
    sockfd = socket( res -> ai_family, res -> ai_socktype, res -> ai_protocol );

    if( sockfd == -1 ) {

        perror( "socket" );

        exit(1);
    }

    // Connect to server
    if ( connect( sockfd, res -> ai_addr, res -> ai_addrlen ) == -1 ) {
        
        perror( "connect" );
        
        exit(1);
    }

    freeaddrinfo( res );

    char buffer[ 100 ];   // Buffer to store server message

    // Receive message from server
    int bytes = recv( sockfd, buffer, sizeof buffer - 1, 0 );

    if( bytes == -1 ) {
        
        perror( "recv" );

        exit( 1 );
    }

    if( bytes == 0 ) {
    
        printf( "Server closed connection\n" );

        exit(0);
    }

    buffer[ bytes ] = '\0';

    // Print server response
    printf( "Server says: %s\n", buffer );

    // Close socket
    close( sockfd );

    return 0;

}
