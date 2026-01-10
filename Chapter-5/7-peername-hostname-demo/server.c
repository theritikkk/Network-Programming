/*
 server.c

 Demonstrates:
  - getpeername()   → Who is connected to me?
  - gethostname()   → Who am I?
  - Real TCP server behavior
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490"   // Server port number

int main() {

    struct addrinfo hints, *res, *p;   // Address info structures
    int sockfd, newfd;                // Socket descriptors

    // Clear hints structure
    memset( &hints, 0, sizeof hints );

    // Configure hints
    hints.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP socket
    hints.ai_flags    = AI_PASSIVE;   // Use local IP

    // Get address information
    int status;
    status = getaddrinfo( NULL, PORT, &hints, &res );

    // Error check
    if( status != 0 ) {

        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror(status) );

        exit( 1 );
    }

    // Loop through results to bind socket
    for( p = res; p != NULL; p = p->ai_next ) {

        // Create socket
        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );

        // If socket failed, try next
        if( sockfd == -1 ) {
        
            continue;
        }

        int yes = 1;
        
        setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes );

        // Bind socket to port
        if( bind( sockfd, p -> ai_addr,p -> ai_addrlen ) == -1 ) {

            close( sockfd );

            continue;
        }

        break;   // Successfully bound

    }

    // Free address info
    freeaddrinfo( res );

    // Check if bind failed
    if( p == NULL ) {

        printf( "Bind failed\n" );

        exit( 1 );
    }

    // Start listening
    if( listen( sockfd, 5 ) == -1 ) {

        perror( "listen" );
        
        exit( 1 );
    }

    printf( "Server waiting on port %s...\n", PORT );

    // Accept incoming connection
    newfd = accept( sockfd, NULL, NULL );

    if( newfd == -1 ) {

        perror("accept");

        exit(1);
    }

    /* ================= getpeername() ================= */

    /*
        getpeername():
        Retrieves the address of the connected client
    */

    struct sockaddr_storage peer;   // Stores client address
    socklen_t len = sizeof peer;

    if( getpeername( newfd, ( struct sockaddr* ) &peer, &len ) == -1 ) {

        perror( "getpeername" );

        exit( 1 );
    }

    char ipstr[ INET6_ADDRSTRLEN ];  // Stores IP string

    // If client is IPv4
    if( peer.ss_family == AF_INET ) {

        struct sockaddr_in *ipv4 = ( struct sockaddr_in* ) &peer;

        inet_ntop( AF_INET, &ipv4 -> sin_addr, ipstr, sizeof ipstr );

        printf( "Client IPv4: %s\n", ipstr );

        printf( "Client Port: %d\n", ntohs( ipv4 -> sin_port ) );

    }

    // If client is IPv6
    else {

        struct sockaddr_in6 *ipv6 = ( struct sockaddr_in6* ) &peer;

        inet_ntop( AF_INET6, &ipv6 -> sin6_addr, ipstr, sizeof ipstr );

        printf( "Client IPv6: %s\n", ipstr );

        printf( "Client Port: %d\n", ntohs( ipv6->sin6_port ) );

    }

    /* ================= gethostname() ================= */

    /*
        gethostname():
        Returns the name of this machine
    */

    char hostname[ 256 ];   // Buffer for hostname

    if( gethostname( hostname, sizeof hostname ) == -1 ) {

        perror( "gethostname" );
        
        exit( 1 );

    }

    printf( "Server hostname: %s\n", hostname );

    /* ================= Communication ================= */

    // Send greeting message to client
    if( send( newfd, "Hello client!\n", 14, 0 ) == -1 ) {

        perror( "send" );
    }

    // Close client socket
    close( newfd );

    // Close server socket
    close( sockfd );

    return 0;

}
