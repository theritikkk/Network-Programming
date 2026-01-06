/*
   showip.c
 
   Demonstrates hostname to IP address resolution using getaddrinfo().
   Supports both IPv4 and IPv6.
 
   This program takes a hostname as a command-line argument
   and prints all IP addresses associated with it.
   
   Compile:
    gcc -Wall -Wextra -pedantic showip.c -o showip
  
   Run:
     ./showip google.com
 */

#include <stdio.h>      // printf(), fprintf() and other basic functions
#include <stdlib.h>     // exit() function
#include <string.h>     // memset() and other string related funstions

#include <sys/types.h>  // System data types i.e., built-in for this program
#include <sys/socket.h> // Socket structures like structs
#include <netdb.h>      // getaddrinfo(), freeaddrinfo(), gai_strerror()
#include <arpa/inet.h>  // inet_ntop() for conversion

int main( int argc, char *argv[] ) {

    /* 
        argc ( argument count ) - number of command-line arguments
            includes the program name itself
        argv ( argument vector ) - array of strings
            each string is one argument

        run : ./showip google.com
        argv[0] = "./showip"
        argv[1] = "google.com"
        argc = 2

        - No hostname provided = ( No argv[1] exists → segmentation fault risk )
        - Too many arguments = ( Program behavior becomes ambiguous )

        Below are the structures used by getaddrinfo() 
    */
    struct addrinfo hints;   // 'hints' to tell OS what we want like a pre-condition
    struct addrinfo *res;    // head of linked list returned for the final output traversal
    struct addrinfo *p;      // iterator for the linked list i.e., ( *res )

    int status;
    char ipstr[ INET6_ADDRSTRLEN ]; // buffer to store IP address as string

    /* Validate command - line arguments */
    if( argc != 2 ) {
        fprintf( stderr, "Usage: %s <hostname>\n", argv[0] );
        // stdout - Normal output
        // stderr - Error messages
        exit( 1 );
    }

    /* ================= STEP 1: SETUP HINTS ================= */

    /*
       Clear the hints structure.
       This avoids undefined behavior due to garbage values.
    */
    memset( &hints, 0, sizeof hints );

    /*
       AF_UNSPEC:
         Allow both IPv4 ( AF_INET ) and IPv6 ( AF_INET6 )
    */
    hints.ai_family = AF_UNSPEC;

    /*
       SOCK_STREAM : We are to create TCP - style addresses
         ( protocol - independent, not actually creating a socket here )
    */
    hints.ai_socktype = SOCK_STREAM;
    /*
        Setting hints.ai_socktype = SOCK_STREAM tells getaddrinfo() to return only addresses suitable for TCP-style, connection-oriented sockets, without actually creating a socket.
    */

    /* ================= STEP 2: RESOLVE HOSTNAME ================= */

    /*
       getaddrinfo() : converts hostname into a linked list of address structures
        Performs DNS lookup inside the OS
    */
    status = getaddrinfo( argv[1], NULL, &hints, &res );
    if( status != 0 ) {
        /*
           getaddrinfo() does NOT use errno
           gai_strerror() converts error code to readable text
        */
        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( status ) );
        exit( 2 );
        /*
            exit( n ) - immediately terminate the program and 
                return status 'n' to the operating system
            
            as a program ends, it always returns an exit status to the OS ( and the shell )
            exit(0);   // success
            exit(1);   // wrong usage ( argc != 2 )
                user error ( wrong arguments )
            exit(2);   // DNS / getaddrinfo() failure
                system / DNS resolution error
        */
    }

    printf( "IP addresses for %s:\n\n", argv[ 1 ] );


    /* ================= STEP 3: ITERATE RESULTS ================= */

    /*
       getaddrinfo() returns a linked list.
       Each node may represent an IPv4 or IPv6 address.
    */
    for( p = res; p != NULL; p = p -> ai_next ) {
        
        /*
            IPv4 address type → struct in_addr
            IPv6 address type → struct in6_addr
            therefor : different sizes and types
            void * - allows us to use this variable 'addr' for both address types
        */
        void *addr;    // Will point to actual IP address
        char *ipver;   // IPv4 or IPv6 string label

        /*
            struct sockaddr is generic socket address : 
                struct sockaddr {
                    sa_family_t sa_family;  // AF_INET or AF_INET6
                    char sa_data[14];
                };
            This is the actual address structure
            It is a generic socket address and must be cast to:
                - struct sockaddr_in ( for IPv4 )
                - struct sockaddr_in6 ( for IPv6 )
            
            struct addrinfo ( returned by getaddrinfo() ) - contains ai_family 
            ai_family tells you what kind of address this entry represents
            it is the summary / metadata - // AF_INET or AF_INET6


           1. Check address family to determine IPv4 or IPv6
        */
        if( p -> ai_family == AF_INET ) {
            /* IPv4 */
            
            /* 
                p -> ai_addr is a struct sockaddr *
                We know ( from ai_family ) it’s IPv4
                So we safely cast it to struct sockaddr_in *

                2. Casts the generic address to the correct structure i.e., sockaddr_in
            */
            struct sockaddr_in *ipv4 = ( struct sockaddr_in * ) p -> ai_addr;

            // 3. Extracts the actual IP address field
            addr = &( ipv4 -> sin_addr );

            // 4. Stores a pointer to it in a version-independent way

            ipver = "IPv4";
        } 

        else {
            /* IPv6 */

            struct sockaddr_in6 *ipv6 = ( struct sockaddr_in6 * ) p -> ai_addr;
            addr = &( ipv6 -> sin6_addr );
            ipver = "IPv6";
        }


        /* ================= STEP 4: CONVERT IP TO STRING ================= */

        /*
           inet_ntop():
             Converts binary IP address into human-readable text
        */
        inet_ntop( p -> ai_family, addr, ipstr, sizeof ipstr );

        printf( "  %s: %s\n", ipver, ipstr );
    }


    /* ================= STEP 5: CLEANUP ================= */

    /*
       freeaddrinfo():
         Frees the entire linked list allocated by getaddrinfo()
         Prevents memory leaks
    */
    freeaddrinfo( res );

    return 0;
}
