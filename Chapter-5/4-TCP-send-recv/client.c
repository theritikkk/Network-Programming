/*
    TCP Client
    For practicing send() and recv()
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

int main( int argc, char *argv[] ) {

    if( argc != 3 ) {
        printf( "Usage : %s host port\n", argv[0] );
        exit( 1 );
    }

    struct addrinfo hints, *res, *p;
    int sockfd;
    char msg[ 1024 ], buffer[ 1024 ];

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get server address
    int status;
    status = getaddrinfo( argv[1], argv[2], &hints, &res );

    if( status != 0 ) {
        fprintf( stderr, "getaddrinfo error: %s\n", gai_strerror( status ) );
        exit( 1 );
    }

    // Try all results
    for( p = res; p != NULL; p = p -> ai_next ) {

        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( sockfd == -1 ) {
            continue;
        }

        if( connect( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {
            close( sockfd );
            continue;
        }

        break;   // Connected successfully
    }

    freeaddrinfo( res );

    if( p == NULL ) {
        printf( "Failed to connect\n" );
        exit( 1 );
    }

    // Chat loop
    while( 1 ) {

        printf( "Enter message ( type exit to quit ): " );
        fgets( msg, sizeof msg, stdin );

        // Exit condition
        if( strncmp( msg, "exit", 4 ) == 0 ) {
            break;
        }

        // Send message
        if( send( sockfd, msg, strlen( msg ), 0 ) == -1 ) {
            perror( "send" );
            break;
        }

        // Receive echo
        int bytes = recv( sockfd, buffer, sizeof buffer - 1, 0 );

        if( bytes == -1 ) {
            perror( "recv" );
            break;
        }

        if( bytes == 0 ) {
            printf( "Server closed connection\n" );
            break;
        }

        buffer[ bytes ] = '\0';

        printf( "Server replied : %s\n", buffer );
    }

    close( sockfd );

    return 0;
    
}
