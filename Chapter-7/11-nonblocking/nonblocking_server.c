/*
   nonblocking_server.c

   Demonstrates:
    - socket()
    - fcntl() NON-BLOCKING
    - accept() without blocking
    - handling EAGAIN / EWOULDBLOCK

   Compile:
    gcc -Wall -Wextra -pedantic nonblocking_server.c -o nonblocking_server

   Run:
    ./nonblocking_server 3490
*/


#include <stdio.h>              // printf(), fprintf()
#include <stdlib.h>             // exit()
#include <string.h>             // memset()
#include <unistd.h>             // close(), sleep()
#include <errno.h>              // errno error codes
#include <fcntl.h>              // fcntl() for non - blocking

#include <sys/types.h>          // system data types
#include <sys/socket.h>         // socket(), bind(), listen(), accept()
#include <netdb.h>              // getaddrinfo()

// argc - argument count
// argv - argument values
int main( int argc, char *argv[] ) {

    int sockfd;     // listening socket
    int newfd;      // client socket

    struct addrinfo hints;      // tells getaddrinfo what we want 
    struct addrinfo *res;       // stores result from getaddrinfo
    int flags;                  // used for fcntl()


    /* ================= ARG CHECK ================= */

    if( argc != 2 ) {

        // user must pass port number
        fprintf( stderr, "Usage: %s <port>\n", argv[0] );
        // ./server 3490

        exit( 1 );
        // If not : program exits.
    }


    /* ================= SETUP ADDRESS ================= */

    // clears structure : avoids garbage values
    memset( &hints, 0, sizeof hints );

    hints.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP socket
    hints.ai_flags    = AI_PASSIVE;   // bind to local IP
    // server mode

    getaddrinfo( NULL, argv[ 1 ], &hints, &res );
    // converts port into usable socket address
    // stores result in res


    /* ================= CREATE SOCKET ================= */

    // creates: TCP socket and IPv4 or IPv6 depending on result
    sockfd = socket( res -> ai_family, res -> ai_socktype, res -> ai_protocol );
    
    // returns: file descriptor - sockfd


    /* ================= BIND ================= */

    bind( sockfd, res -> ai_addr, res -> ai_addrlen );
    // binds socket to: local IP and given port


    /* ================= LISTEN ================= */

    // puts socket into listening mode
    listen( sockfd, 10 );
    // 10 = backlog queue size ( max waiting clients )


    /* ================= SET NON-BLOCKING ================= */

    // gets current socket flags
    flags = fcntl( sockfd, F_GETFL, 0 );

    // O_NONBLOCK --- Now socket is: NOT blocking : returns immediately
    fcntl( sockfd, F_SETFL, flags | O_NONBLOCK );

    // status message
    printf( "Non-blocking server running...\n" );


    /* ================= ACCEPT LOOP ================= */

    while( 1 ) {
        // infinite loop
        // server always runs

        newfd = accept( sockfd, NULL, NULL );
        /* Attempts: 
            1. accept the client
            2. but in non - blocking
            3. returns immediately

            In case of Blocking server :
                --- If no client: accept();     // waits here forever i.e, sleeps
            
                In case of NON-BLOCKING SEREVR: 
                --- If no client: accept();     // returns immediately i.e., return -1
        */

        if( newfd == -1 ) {

            // if accept failed:
            // Linux   -   EAGAIN   |||||   macOS   -   EWOULDBLOCK
            // as it is checking both i.e., portable code
            // meaning : no client yet
            if( errno == EAGAIN || errno == EWOULDBLOCK ) {
                // both mean : Operation would block, but socket is NON - BLOCKING
                printf( "No clients yet...\n" );
                
                sleep( 1 );
                // sleeps 1 second
                // tries again
                continue;
            }

            else {
                // any OTHER error - print it
                perror( "accept" );
            }

        }

        printf( "Client connected!\n" );

        // sends message to client
        send( newfd, "Hello from NON-BLOCKING server\n", 32, 0 );

        // closes client connection
        close( newfd );

    }

    // program never exits - runs forever
    
}
