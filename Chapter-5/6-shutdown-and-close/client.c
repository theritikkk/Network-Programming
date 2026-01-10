/*
    client.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

int main() {

    struct addrinfo hints, *res;
    
    int sockfd;

    char buffer[ 1024 ];

    // Clear hints
    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get server address
    getaddrinfo( "localhost", "3490", &hints, &res );

    // Create socket
    sockfd = socket( res -> ai_family, res -> ai_socktype, res -> ai_protocol );

    // Connect to server
    connect( sockfd, res -> ai_addr, res -> ai_addrlen );

    while( 1 ) {

        printf( "Send message (type exit): " );
        fgets( buffer, sizeof buffer, stdin );

        if( strncmp( buffer, "exit", 4 ) == 0 ) {
            break;
        }

        send( sockfd, buffer, strlen( buffer ), 0 );

        int bytes = recv( sockfd, buffer, sizeof buffer - 1, 0 );

        if( bytes == 0 ) {

            printf( "Server closed sending side\n" );
            break;
        }

        if( bytes == -1 ) {

            perror( "recv" );
            break;
        }

        buffer[ bytes ] = '\0';

        printf( "Server: %s\n", buffer );

    }

    close( sockfd );

    return 0;
    
}
