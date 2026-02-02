#include <stdio.h>              // printf()
#include <stdlib.h>             // general utilities like : strtol(), 
#include <string.h>             // memcpy()
#include <stdint.h>             // uint32_t
#include <unistd.h>             // close()
#include <arpa/inet.h>          // sockets + byte order : sockaddr_in, htons(), INADDR_ANY

uint32_t unpacki32( unsigned char *buf ) {
    /*
        rebuilds an integer from 4 bytes stored in big - endian format

        Example : buf = [00][00][30][39]
        It reconstructs : 
            ( 0x00 << 24 ) | ( 0x00 << 16 ) | ( 0x30 << 8 )  | ( 0x39 ) = 12345

        This is manual network byte order decoding
    */

    return ( 
        ( buf[0] << 24 ) | 
        ( buf[1] << 16 ) | 
        ( buf[2] << 8 )  | 
        buf[3] 
    );
}


float unpackf( unsigned char *buf ) {
    
    /*
        To unpack float :
            copies raw bytes into a float variable

        As the float variable is stored as a binary : IEEE 754 binary
        thats why, we copy byte to byte in here of float
    */
    float f;

    memcpy( &f, buf, sizeof( float ) );

    return f;
}


int main() {

    int sockfd, newfd;
    // sockfd - listening socket
    // newfd - connected client socket
    
    struct sockaddr_in server, client;
    // server - server address
    // client - client address

    socklen_t len = sizeof( client );
    // length of client address


    unsigned char buffer[ 1024 ];
    // buffer - holds received raw bytes


    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    // creation of TCP based socket - returns a file descriptior

    // setup of server address
    server.sin_family = AF_INET;
    server.sin_port = htons( 5050 );
    server.sin_addr.s_addr = INADDR_ANY;
    // to accept connections from any IP


    bind( sockfd, ( struct sockaddr* ) &server, sizeof( server ) );
    // this socket is now attached to port 5050


    listen( sockfd, 1 );
    // 1 - backlog -> OS will queue up to 1 pending client


    printf( " Server waiting... \n " );

    newfd = accept( sockfd, ( struct sockaddr* ) &client, &len );
    /*
        accept() - is blocked until a client connects
        it creates new socket for that client 

        but sockfd - stays open for more clients
    */


    int n = recv( newfd, buffer, sizeof( buffer ), 0 );
    // reading bytes from client ( newfd ) into 'buffer'
    // buffer -  [ int ]  [ float ]  [ strlen ]  [ string ]

    int offset = 0;
    // for deserialization process - used as cursor of buffer


    int num = unpacki32( buffer + offset );
    // reading 4 bytes -> and converted into integer
    offset += 4;
    // moving offset forward by 4


    float pi = unpackf( buffer + offset );
    // this is for reading float bytes
    offset += sizeof( float );
    // moving offset forward on the basis of size of float


    int strlen2 = unpacki32( buffer + offset );
    // reads string length
    offset += 4;


    // now to read string
    char msg[ 100 ];
    memcpy( msg, buffer + offset, strlen2 );
    // copying string bytes into buffer
    msg[ strlen2 ] = '\0';
    // adding a null terminator ( '\0' )


    // test cases
    printf( " Received : \n" );
    printf( " int = %d\n ", num );
    printf( " float = %f\n ", pi );
    printf( " string = %s\n ", msg );


    close( newfd );
    // close client connection
    
    close( sockfd );
    // close server connection
    
    return 0;
}
