/*
   client.c
  
   TCP client using getaddrinfo() and connect()
   Supports IPv4 and IPv6
 
   Demonstrates:
    - DNS + service resolution
    - Client-side socket creation
    - connect() without bind()
    - Kernel-assigned ephemeral ports
 
   Compile the code using the following command :
    gcc -Wall -Wextra -pedantic client.c -o client
  
   Run:
    ./client google.com 80
*/

#include <stdio.h>      // printf(), fprintf()
#include <stdlib.h>     // exit()
#include <string.h>     // memset(), strlen()
#include <unistd.h>     // close()
/*
    close() is a generic OS primitive, not a networking-specific one

    close(sockfd);
    On above command the kernel performs the following actions :
        - Removes 'sockfd' from your process’s file descriptor table
        - Decrements the reference count
        - If no references remain:
            - Closes the TCP connection ( FIN )
            - Frees kernel socket structures
        - Makes the descriptor number reusable
    This is real resource cleanup, not just a variable reset.

    It prevents:
        - File Descriptor Table leaks
        - Process table exhaustion
        - Socket exhaustion
*/
#include <errno.h>      // errno

#include <sys/types.h>  // system data types
#include <sys/socket.h> // socket(), connect(), send(), recv()
#include <netdb.h>      // getaddrinfo(), freeaddrinfo(), gai_strerror()

int main( int argc, char *argv[] ) {

    /* ================= STEP 0: ARGUMENT VALIDATION ================= */

    /*
        Client needs:
            argv[1] -> hostname
            argv[2] -> port or service (e.g. 80, http)
    */
    if( argc != 3 ) {
        fprintf( stderr, "Usage: %s <hostname> <port>\n", argv[ 0 ] );
        exit( 1 );   // user error
    }

    

    /* ================= STEP 1: DECLARATION OF VARIABLES ================= */

    struct addrinfo hints;   // input hints for getaddrinfo()
    struct addrinfo *res;    // head of results linked list
    struct addrinfo *p;      // iterator through 'res' linked list

    int sockfd;              // socket file descriptor
    int status;              // return value of getaddrinfo()

    char buffer[ 1024 ];       // buffer for receiving data



    /* ================= STEP 2: SETUP HINTS ================= */

    /*
        Clear hints to avoid garbage values.
    */
    memset( &hints, 0, sizeof hints );

    /*
        AF_UNSPEC:
            Allow IPv4 or IPv6
        SOCK_STREAM:
            TCP socket
    */
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;



    /* ================= STEP 3: DNS + SERVICE RESOLUTION ================= */

    /*
        DNS resolution     :   hostname → IP address        :   google.com → 142.250.182.46
        Service resolution :   service → port + protocol    :   http → TCP port 80

        Service resolution is the process of converting a service name ( like "http", "https", "ftp" ) or a port string ( like "80", "443" ) into:
            - a port number
            - a transport protocol ( TCP or UDP )
        
        It is done by the OS, not by DNS.
        getaddrinfo():
            resolves hostname + port into a linked list of addresses
            and handles IPv4 / IPv6 / protocol selection
    */
    status = getaddrinfo( argv[ 1 ], argv[ 2 ], &hints, &res );
    // argv[1] -> hostname ( DNS )
    // argv[2] -> port or service ( SERVICE resolution )

    if( status != 0 ) {
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( status ) );
        exit( 2 );   // resolution failure
    }



    /* ================= STEP 4: CREATE SOCKET + CONNECT ================= */

    /*
        Try each address until a connection succeeds
        This is REQUIRED for real - world robustness
        
            Because getaddrinfo() returns multiple valid addresses for redundancy and IPv4 / IPv6 compatibility, and real networks frequently fail on some paths, robust programs must try each address until a connection succeeds
    */
    for( p = res; p != NULL; p = p -> ai_next ) {

        /*
            Create socket using parameters supplied by OS
        */
        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );
        if( sockfd == -1 ) {
            perror( "socket" );
            continue;
        }

        /*
            connect():
                - No bind() required
                - Kernel auto - assigns local IP + ephemeral port
                - Performs TCP handshake

            An ephemeral port is a temporary 'local port' automatically assigned by the OS kernel to identify a client - side TCP or UDP connection when the application does not explicitly bind a port.
        */
        if( connect( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {
            perror( "connect" );
            /*
                connect() returns : 
                    0 → success
                    -1 → failure ( and sets errno )

                perror() prints : 
                    string "connect"
                    A human-readable explanation of errno

                At this point:
                    - socket was successfully created
                    - connect() failed
                    - and the socket is now useless
                    - so we use clode( sockfd ) to handle this
            */
            close( sockfd );
            continue;
        }

        /*
            If we reach here, connect() succeeded
        */
        break;
    }

    /*
        We no longer need the address list
    */
    freeaddrinfo( res );

    if( p == NULL ) {
        fprintf( stderr, "client: failed to connect\n" );
        exit( 3 );   // connection failure
    }

    printf( "Connected to %s:%s\n", argv[ 1 ], argv[ 2 ] );



    /* ================= STEP 5: SEND A HTTP REQUEST ================= */

    /*
        Minimal HTTP/1.0 request
        Servers respond even without Host header

        request - is raw HTTP text, not a function call
        It is a protocol message - an HTTP request, written in plain text

        This line is part of the HTTP protocol, which is an application - layer communication protocol
        Therefore,
            - "GET / HTTP/1.0" → application-layer instruction
            - TCP → delivers it reliably
            - Server → parses and acts on it

        It is:
            - sent to the server
            - parsed by the server’s HTTP parser
            - used to decide what response to send
        
        The response, not the request, may contain: HTML, JSON, images, or anything else

        "GET / HTTP/1.0" is a protocol-level instruction written in plain text
        It is neither HTML nor an API by itself, but the foundation on which both web pages and APIs are built
    */
    const char *request = "GET / HTTP/1.0\r\n\r\n";
    send( sockfd, request, strlen( request ), 0 );
    /*
        send() is a POSIX system call
            That means:
                Your program is in user space
                The kernel actually sends the data
                A user → kernel mode switch happens

        sockfd              // connected TCP socket
            it is the file descriptor returned by socket() + connect()
            it identifies one TCP connection
            which is already connected to a remote server

        request            // pointer to bytes to send
            a pointer to raw memory
            no HTTP awareness here.
            kernel does not know or care that this is HTTP

        strlen(request),     // number of bytes
            it is the number of bytes to send
            send() does not stop at \0
            must explicitly tell it how many bytes to transmit
        
        "GET / HTTP/1.0\r\n\r\n" : length ≈ 18 bytes

        0                    // default behavior
    */



    /* ================= STEP 6: RECEIVE RESPONSE ================= */

    int bytes;
    /*
        recv() is a POSIX system call that:
            - switches from user space → kernel
            - copies data from the kernel receive buffer → user buffer
            - reads data sent by the remote peer over TCP
        
        TCP is a stream, not messages - no message boundaries, and no request / response sizes
        
        sockfd              :   connected TCP socket
        buffer              :   where received bytes go
        sizeof buffer - 1   :   leave room for '\0'
            as without -1, this could overflow
        0                   :   default behavior

        recv() returns  number > 0 -> number of bytes received
        recv() returns  number = 0 -> peer closed the connection
        recv() returns  number < 0 -> Error ( check errno )
    */
    while( ( bytes = recv( sockfd, buffer, sizeof buffer - 1, 0 ) ) > 0 ) {
        // while conditions says that : Keep reading until the server closes the connection
        buffer[ bytes ] = '\0';
        printf( "%s", buffer );
    }
    /*
        This loop continuously reads data from a TCP stream into a user buffer until the remote server closes the connection, which is how HTTP/1.0 signals the end of a response.
    */



    /* ================= STEP 7: CLEANUP ================= */

    close( sockfd );   // close connection
    return 0;        // success

}
