#include <stdio.h>              // printf()
#include <string.h>             // memcpy(), strlen()
#include <stdint.h>             // uint32_t
#include <unistd.h>             // close()
#include <arpa/inet.h>          // sockets, sockaddr_in, htons(), inet_pton()


void packi32( unsigned char *buf, uint32_t val ) {
    /* 
        convert a 32 - bit integer into 4 bytes i.e., big - endian
        example : 
                777 = 0x00000309
            
            stored as:
            [ 00 ] [ 00 ] [ 03 ] [ 09 ]
    */

    buf[ 0 ] = val >> 24;
    buf[ 1 ] = val >> 16;
    buf[ 2 ] = val >> 8;
    buf[ 3 ] = val;
}


void packf( unsigned char *buf, float f ) {
    // to copy raw float bytes into buffer
    // as float is stored as raw bytes to begin with

    memcpy( buf, &f, sizeof( float ) );
    // no change in endianness
    // it just assumes to have same architecture on both sides
}


int main() {

    int sockfd;
    // sockfd - socket file descriptor

    struct sockaddr_in server;
    // server - server address

    unsigned char buffer[ 1024 ];
    // buffer - packet for the storage of the data to be sent

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    // creation of socket

    // setting up of server address
    server.sin_family = AF_INET;
    server.sin_port = htons( 5050 );
    inet_pton( AF_INET, "127.0.0.1", &server.sin_addr );
    // IP : localhost - 127.00.00.01
    // PORT - 5050

    // now then we have to connect to the server
    connect( sockfd, ( struct sockaddr* ) &server, sizeof( server ) );
    // this triggers TCP -> 3 way handshake : ( SYN -> SYN - ACK -> ACK )

    // this is the data to be sent
    int num = 777;
    float pi = 3.14f;
    char msg[] = " Hello from client to server using pack and unpack ";

    int offset = 0;
    // offset - the cursor on the 'buffer'

    packi32( buffer + offset, num );
    // [ int ( 4 bytes ) ]
    offset += 4;

    packf( buffer + offset, pi );
    // [ float ( 4 bytes ) ]
    offset += sizeof( float );

    // pack the string length - to let the server know about how many bytes to read from the string
    packi32( buffer + offset, strlen( msg ) );
    // [ strlen ( 4 bytes ) ]
    offset += 4;

    memcpy( buffer + offset, msg, strlen( msg ) );
    // [ string bytes ] - copy the string itself into 'buffer'
    offset += strlen( msg );

    // buffer = [ int ][ float ][ strlen ][ string bytes ]


    send( sockfd, buffer, offset, 0 );
    // sending the packet

    close( sockfd );
    // to end TCP connection

    return 0;

}
