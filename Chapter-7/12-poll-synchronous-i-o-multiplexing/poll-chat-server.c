/*
   poll-chat-server.c

   multi-client TCP chat server using poll()
   with usernames + timestamps

   Compile:
    gcc -Wall -Wextra -pedantic poll-chat-server.c -o poll-chat-server

   Run:
    ./poll-chat-server

   Connect:
    windows - telnet localhost 9034
    macOS - nc localhost 9034

*/

#include <stdio.h>      // printf(), perror()
// stdio : purpose - printing

#include <stdlib.h>     // exit(), malloc(), realloc()
// stdlib : purpose - memory

#include <string.h>     // memset(), strncpy(), strlen()
// string : purpose - string operations

#include <unistd.h>     // close()
// unistd : purpose - close()

#include <errno.h>      // errno values
// errno : purpose - error codes

#include <time.h>       // time(), localtime(), strftime()
// time	: purpose - timestamps

#include <sys/types.h>  
#include <sys/socket.h> // socket(), bind(), listen(), accept(), send(), recv()
// socket : headers	networking

#include <netdb.h>      // getaddrinfo()
#include <arpa/inet.h> // inet_ntop()
#include <poll.h>      // poll()
// poll : multiplexing

#define PORT "9034"
// PORT : server listens here
// must be string because getaddrinfo expects string

#define INITIAL_SIZE 5
// INITIAL_SIZE : initial poll array size

#define NAME_LEN 32
// NAME_LEN : max username length



// store username per client : representing ONE connected user
typedef struct {
    
    int fd;
    
    char name[ NAME_LEN ];

} client_t;
// fd - socket descriptor
// name - username

client_t clients[ 100 ];
// maximum 100 users

int client_count = 0;
// stores connected users



/* ================= IP HELPER - FUNCTION ================= */

void *get_in_addr( struct sockaddr *sa ) {

    // extracts IP address from: sa_family -> IPv4, IPv6
    if( sa -> sa_family == AF_INET ) {
        // access sin_addr field : return pointer to it
        return &( ( ( struct sockaddr_in* ) sa ) -> sin_addr );
    }

    return &( ( ( struct sockaddr_in6* ) sa ) -> sin6_addr );
}



/* ================= GET TIME - FUNCTION ================= */
// takes buffer to write timestamp
void get_time( char *buf ) {

    time_t now = time( NULL );
    // time() - current epoch time

    struct tm *t = localtime( &now );
    // localtime() - convert to human time

    strftime( buf, 16, "%H:%M:%S", t );
    // strftime() - format as HH:MM:SS
}


/* ================= FIND CLIENT - FUNCTION ================= */

client_t* get_client( int fd ) {

    for( int i = 0; i < client_count; i++ ) {
        // Searches client list: by using 'fd'
    
        // if 'fd' matches : return client
        if( clients[ i ].fd == fd ) {
            
            return &clients[ i ];
        }
        
    }
    
    // else → return NULL
    return NULL;
}



/* ================= ADD CLIENT ================= */

void add_client( int fd, char *name ) {
    // stores: socket fd , username , increments count

    clients[ client_count ].fd = fd;

    strncpy( clients[ client_count ].name, name, NAME_LEN );
    // safe copy

    client_count++;
}



/* ================= REMOVE CLIENT ================= */

void remove_client(int fd) {

    // deletes client : in O(1) time
    for( int i = 0; i < client_count; i++ ) {

        // swaps with last client and decreases count
        if( clients[ i ].fd == fd ) {
            
            clients[ i ] = clients[ client_count - 1 ];
            client_count--;
            // fast delete trick

            /*
                Index:   0   1   2   3   4
                Clients: A   B   C   D   E

                To delete C (index 2)

                1. Copy last element into the deleted slot :
                    clients[2] = clients[4];
                    Index:   0   1   2   3   4
                    Clients: A   B   E   D   E   <-- duplicated E

                    So: 
                        - C is gone
                        - E moved into position 2

                2. Decrement count :
                    
                    client_count--;

                    Now: client_count = 4

                    Index:   0   1   2   3
                    Clients: A   B   E   D

                3. The last slot (index 4) is now:
                    - logically dead
                    - ignored
                    - inaccessible

                Order is lost:
                    But for chat server:
                    - Order irrelevant
                    - Speed matters
            */

            return;
        }

    }
}



/* ================= LISTENER ================= */

int get_listener_socket( void ) {
    // to create listener socket

    int listener;
    int yes = 1;

    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;
    
    // preparing hints for getaddrinfo
    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;          // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;        // TCP
    hints.ai_flags    = AI_PASSIVE;         // bind to all interfaces

    getaddrinfo( NULL, PORT, &hints, &res );
    // NULL = local host
    // PORT = "9034"

    // loop through results
    for( p = res; p; p= p -> ai_next ) {

        // create socket
        listener = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        if( listener < 0 ) {
            continue;
        }

        // enable port reuse
        setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );

        // bind : attach socket to port
        if( bind( listener, p -> ai_addr, p -> ai_addrlen ) < 0 ) {
            close( listener );
            continue;
        }

        break;

    }

    freeaddrinfo( res );

    if( p == NULL ) {
        // get_listener_socket() : returns: listener socket or -1 on failure
        return -1;
    }

    // Listen
    listen( listener, 10 );

    return listener;
}



/* ================= MAIN ================= */

int main( void ) {

    int listener;

    int fd_count = 0;
    int fd_size = INITIAL_SIZE;

    // stores all sockets that poll will monitor
    struct pollfd *pfds = malloc( sizeof *pfds * fd_size );
    // this is your monitored fd list

    // to create server socket
    listener = get_listener_socket();

    if( listener == -1 ) {
        perror( "listener" );
        exit( 1 );
    }

    // add listener to poll
    pfds[ 0 ].fd = listener;
    pfds[ 0 ].events = POLLIN;
    // now poll monitors :: index = 0 ->  server socket

    fd_count = 1;

    puts( "Chat server running..." );

    // infinite server loop
    while( 1 ) {
        // server loop

        poll( pfds, fd_count, -1 );
        /* waits forever : wakes when activity happens i.e.,
            Blocked until:
            - new connection
            - client message
        */

        // loop through active sockets
        for( int i = 0; i < fd_count; i++ ) {

            // Check events :
            if( pfds[ i ].revents & POLLIN ) {
                // meaning : data is ready

                // meaning : listening socket is readable -> new client
                /* NEW CONNECTION */
                if( pfds[ i ].fd == listener ) {
                    // Steps :

                    struct sockaddr_storage addr;
                    socklen_t len = sizeof addr;

                    // 1. accept() : creates new socket for client
                    int newfd = accept( listener, ( struct sockaddr* ) &addr, &len );

                    if( newfd == -1 ) {
                        continue;
                    }

                    // resize poll array if full : i.e., Double the size
                    if( fd_count == fd_size ){
                        fd_size *= 2;
                        pfds = realloc( pfds, sizeof( *pfds ) *fd_size );
                    }
                    
                    // add to poll list

                    pfds[ fd_count ].fd = newfd;
                    pfds[ fd_count ].events = POLLIN;

                    fd_count++;

                    // ask for username
                    send( newfd, "Enter username: ", 16, 0 );

                }

                /* CLIENT MESSAGE */
                else{
                    // meaning : this is client socket

                    char buf[ 256 ];
                    
                    // receive data
                    int n = recv( pfds[i].fd, buf, sizeof buf, 0 );

                    int fd = pfds[ i ].fd;

                    // if client left : i.e., client disconnected
                    if( n <= 0 ) {

                        client_t *c = get_client( fd );
                        
                        if( c ) {
                          printf( "%s left\n", c -> name );
                        }

                        // close socket
                        close( fd );

                        // remove client from list
                        remove_client( fd );

                        // delete from poll list
                        pfds[ i ] = pfds[ fd_count - 1 ];
                        fd_count--;
                        i--;
                        // O( 1 )

                        continue;
                    }

                    // trim newline
                    buf[ n - 1 ]='\0';

                    // check if username not set
                    client_t *c = get_client( fd );

                    // user not registered yet
                    if( !c ) {
                        /* FIRST MESSAGE = USERNAME */

                        add_client( fd, buf );

                        char msg[ 64 ];

                        // send welcome :
                        sprintf( msg, "Welcome %s!\n", buf );

                        send( fd, msg, strlen( msg ), 0 );

                        continue;
                    }

                    /* NORMAL MESSAGE */
                    char timebuf[ 16 ];
                    get_time( timebuf );

                    char out[ 512 ];
                    sprintf( out, "[%s] %s: %s\n", timebuf, c -> name, buf );

                    /* BROADCAST */
                    for( int j = 0; j < fd_count; j++ ) {
                        // send to all clients
                        // except listener
                        int dest = pfds[ j ].fd;

                        if( dest != listener ) {
                            // sender also receives own message
                            send( dest, out, strlen( out ), 0 );
                        }
                    }
                }
            }
        }
    }
}
/*

Client joins
 - asked username
 - stored
 - sends message
 - server timestamps
 - broadcast to all

System Level Design:
- poll() = kernel event loop
- No threads
- No fork
- Single process
- Scales to 1000s clients
- Low memory
- Non - blocking I/O

*/
