#include <stdio.h>
// printf(), perror() : user-space I/O utilities

#include <stdlib.h>
// exit(), memory utilities, general system functions

#include <string.h>
// strlen(), memset() : raw memory manipulation

#include <unistd.h>
// POSIX system calls : close(), read(), write()
// bridge between user space <--> kernel space

#include <sys/socket.h>
// socket API : socket(), connect(), send()
// core networking layer

#include <arpa/inet.h>
// IP conversion utilities : inet_pton(), htons()


int sendall( int s, char *buf, int *len ) {
    /*
        s : socket file descriptor,
        buf : pointer to data,
        len : pointer to length,

        total : bytes sent so far,
        bytesleft : remaining bytes to send
                    dereferencing pointer *len,
        n : return value of send()
            tracks the number of bytes sent
            also to track error,
    */

    int total = 0;
    int bytesleft = *len;
    int n;

    // keep sending until : total bytes sent == original length
    while( total < *len ) {

        n = send( s, buf + total, bytesleft, 0 );
        /*
            s : socket File Descriptor : identifies which connection to send data on,

            buf + total : move pointer forward
                buf : start of your data buffer
                total : number of bytes already sent,
            ensures you send only the remaining data, not what was already sent

            bytesleft : remaining bytes,

            0 : flags
                send data using default socket behavior
        */

        if( n == -1 ) {
            break;
        }

        total += n;
        bytesleft -= n;
    }

    *len = total;

    return ( n == -1 ) ? -1 : 0;
}

int main( void ){

    int sockfd;
    // will store the socket file descriptor : uniquely identifies your TCP connection.
    
    struct sockaddr_in serv;
    // structure holding server address info : used for connect()
    // contains : IP address, Port number, address family

    char msg[ 10000 ];
    // buffer to hold the message

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    /*
        AF_INET : IPv4
        SOCK_STREAM : TCP
        0 : default protocol

        kernel allocates: Network buffers, TCP state machine, returns File Descriptor
        
        If fail → returns -1
    */

    serv.sin_family = AF_INET;
    // IPv4 address

    serv.sin_port = htons( 9090 );
    // converts host byte order -> network byte order
    // required by TCP / IP standard

    inet_pton( AF_INET, "127.0.0.1", &serv.sin_addr );
    // converts string IPv4 address -> binary
    // stores inside serv.sin_addr

    connect( sockfd, ( struct sockaddr* ) &serv, sizeof serv );
    /*
        internally : TCP 3-way handshake
        SYN → SYN-ACK → ACK
        connection established
        socket now connected
    */

    memset( msg, 'A', sizeof msg );
    // fills entire buffer with 'A' : creates huge payload

    msg[ 9999 ] = '\0';         // Null terminator

    int len = strlen( msg );
    // counts characters until \0

    if( sendall( sockfd, msg, &len ) == -1 ) {
        perror( "sendall" );
        // prints error

        printf( "Sent only %d bytes\n", len );
        // shows how many bytes were sent before failure
    }

    else{
        printf( "Sent %d bytes successfully\n", len );
        // confirms full transmission
    }

    close( sockfd );

}
