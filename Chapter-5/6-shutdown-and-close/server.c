/*
    interactive_echo_server.c

    - Receives client messages
    - Echoes them back
    - Allows runtime shutdown control
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#define PORT "3490"

int main() {

    struct addrinfo hints, *res;
    
    int sockfd, newfd;

    char buffer[ 1024 ];

    int choice;

    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    getaddrinfo( NULL, PORT, &hints, &res );

    sockfd = socket( res -> ai_family, res -> ai_socktype, res -> ai_protocol );

    bind( sockfd, res -> ai_addr, res -> ai_addrlen );

    listen( sockfd, 5 );

    printf( "Server waiting...\n" );

    newfd = accept( sockfd, NULL, NULL );

    printf( "Client connected!\n" );

    while( 1 ) {

        printf( "\nChoose option:\n" );
        printf( "1 -> shutdown RECEIVE\n" );
        printf( "2 -> shutdown SEND\n" );
        printf( "3 -> shutdown BOTH\n" );
        printf( "4 -> CLOSE socket\n" );
        printf( "5 -> RECV + ECHO\n" );
        printf( "Enter choice: " );

        scanf( "%d", &choice );

        if( choice == 5 ) {

            int bytes = recv( newfd, buffer, sizeof buffer - 1, 0 );

            if( bytes <= 0 ) {
                printf( "Client disconnected\n" );
                break;
            }

            buffer[ bytes ] = '\0';

            printf( "Client says: %s\n", buffer );

            send( newfd, buffer, bytes, 0 );
        }

        else if( choice == 1 ) {

            printf( "Stopping RECEIVE channel\n" );
            shutdown( newfd, 0 );
        }

        else if( choice == 2 ) {

            printf( "Stopping SEND channel\n" );
            shutdown( newfd, 1 );
        }

        else if( choice == 3 ) {

            printf( "Stopping BOTH channels\n" );
            shutdown( newfd, 2 );
        }

        else if( choice == 4 ) {

            printf( "Closing connection\n" );
            close( newfd );
            break;
        }

        else {
            printf( "Invalid choice\n" );
        }
    }

    close( sockfd );
    return 0;
}
