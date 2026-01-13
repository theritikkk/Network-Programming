/*
   select-chat-server.c

   Multi-client chat server using select()

   Demonstrates:
    - Blocking I/O
    - select() multiplexing
    - Handling multiple clients
    - Broadcasting messages

   Compile:
    gcc -Wall -Wextra -pedantic select-chat-server.c -o select-chat-server

   Run:
    ./select-chat-server

   Connect:
    nc localhost 9034
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "9034"
#define BACKLOG 10
#define MAXBUF 256


/* ================= HELPER ================= */

void *get_in_addr( struct sockaddr *sa )
{
    if( sa -> sa_family == AF_INET ) {
        return &( ( ( struct sockaddr_in* ) sa ) -> sin_addr );
    }

    return &( ( ( struct sockaddr_in6* ) sa ) -> sin6_addr );
}


/* ================= LISTENER ================= */

int get_listener_socket( void )
{
    struct addrinfo hints, *res, *p;
    int sockfd;
    int yes = 1;

    memset( &hints, 0, sizeof hints );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    getaddrinfo( NULL, PORT, &hints, &res );

    for( p = res; p; p = p -> ai_next ) {

        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if ( sockfd < 0 ) {
            continue;
        }

        setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );

        if( bind( sockfd, p -> ai_addr, p -> ai_addrlen ) < 0 ) {
            close( sockfd );
            continue;
        }

        break;
    }

    freeaddrinfo( res );

    if( !p ) {
        return -1;
    }

    listen( sockfd, BACKLOG );

    return sockfd;
}


/* ================= MAIN ================= */

int main( void ) {
    // master -> permanent record
    fd_set master;
    
    // read_fds -> temporary working copy
    fd_set read_fds;

    // select() DESTROYS the set you pass

    int fdmax;

    int listener = get_listener_socket();
    if( listener == -1 ) {
        perror( "listener" );
        exit( 1 );
    }

    FD_ZERO( &master );
    FD_ZERO( &read_fds );

    FD_SET( listener, &master );
    fdmax = listener;

    printf( "Server running on port %s\n", PORT );

    while( 1 ) {

        read_fds = master; // copy

        // select() : kernel sleeps until activity
        select( fdmax + 1, &read_fds, NULL, NULL, NULL );

        for( int i = 0; i <= fdmax; i++ ) {

            if( FD_ISSET( i, &read_fds ) ) {

                /* NEW CONNECTION */
                if( i == listener ) {
                    // If socket == listener : new client -> accept()

                    struct sockaddr_storage remote;
                    socklen_t len = sizeof remote;

                    int newfd = accept( listener, ( struct sockaddr* ) &remote, &len );

                    FD_SET( newfd, &master );
                    if( newfd > fdmax ) {
                        fdmax = newfd;
                    }

                    char ip[ INET6_ADDRSTRLEN ];
                    inet_ntop( remote.ss_family, get_in_addr( ( struct sockaddr* ) &remote ), ip, sizeof ip );

                    printf( "New client %s\n", ip );
                }

                /* CLIENT DATA */
                else{
                    // Client message -> recv()
                    char buf[ MAXBUF ];
                    int nbytes = recv( i, buf, sizeof buf, 0 );

                    if( nbytes <= 0 ) {

                        if( nbytes == 0 ) {
                            // recv() == 0 : client closed connection
                            printf( "Client %d left\n", i );
                        }
                        
                        else {
                            perror( "recv" );
                        }

                        close( i );
                        FD_CLR( i, &master );
                    }

                    else{
                        for( int j = 0; j <= fdmax; j++ ) {
                            // Broadcast : send message to all clients
                            
                            if( FD_ISSET( j, &master ) ) {

                                if( j != listener && j != i ) {
                                    send( j, buf, nbytes, 0 );
                                }

                            }
                        }
                    }
                }
            }
        }
    }
}
/*
select():
- Registers sockets
- Kernel waits
- Interrupts process
- Marks which fd fired
- Returns to user space
*/
