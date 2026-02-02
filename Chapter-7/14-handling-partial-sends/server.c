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

#include <netinet/in.h>
// Internet structures : sockaddr_in, htons(), INADDR_ANY

#define PORT 9090
// server will listen on port 9090

int main( void ){

    int s, newfd;
    /*
        s : server listening socket
        newfd : client connection socket
    */

    struct sockaddr_in addr;
    /*
        structure that stores : IP address, port, address family
    */

    char buf[ 1024 ];
    // buffer to receive client data
    // size = 1024 bytes

    s = socket( AF_INET, SOCK_STREAM, 0 );
    /*
        AF_INET : IPv4
        SOCK_STREAM : TCP
        0 : default protocol
        
        kernel creates a TCP socket
        returns file descriptor
    */

    addr.sin_family = AF_INET;
    addr.sin_port = htons( PORT );
    // converts port to : network byte order ( big - endian )

    addr.sin_addr.s_addr = INADDR_ANY;
    // bind to all network interfaces : ( 127.0.0.1, WiFi IP, Ethernet IP )

    bind( s, ( struct sockaddr* ) &addr, sizeof addr );
    // attaches : socket s to IP + port

    listen( s, 5 );
    // 5 : backlog queue size

    // puts socket into passive mode

    printf( "Server listening...\n" );

    newfd = accept( s, NULL, NULL );
    /*
        blocks until client connects
        returns : new socket for that client

        but, server socket s stays open
    */

    int n = recv( newfd, buf, sizeof buf, 0 );
    /*
        newfd : client socket
        buf : store data
        sizeof buf : max bytes
        0 : flags ( default )

        recv() blocks until data arrives
        returns : number of bytes received
    */

    buf[ n ] = '\0';

    printf( "Received: %s\n", buf );

    close( newfd );
    // close client socket

    close( s );
    // close server socket
    
}
