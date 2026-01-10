/*
   client.c

   TCP client using getaddrinfo() and connect()
   Supports IPv4 and IPv6

   Demonstrates:
    - DNS resolution
    - Client-side socket creation
    - connect()
    - recv()
    - Proper cleanup

   Compile:
    gcc -Wall -Wextra -pedantic client.c -o client

   Run:
    ./client <hostname>

   Example:
    ./client localhost
*/

#include <stdio.h>      // printf(), fprintf()
#include <stdlib.h>     // exit()
#include <unistd.h>     // close()
#include <errno.h>      // errno
#include <string.h>     // memset()

#include <netdb.h>      // getaddrinfo(), freeaddrinfo()
#include <sys/types.h>  // system data types
#include <sys/socket.h> // socket(), connect(), recv()
#include <arpa/inet.h>  // inet_ntop()

#define PORT "3490"         // Server port
#define MAXDATASIZE 100    // Max bytes to receive


/* ================= HELPER FUNCTION ================= */

/*
    Extracts IP address from generic sockaddr
    Works for both IPv4 and IPv6
*/
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // IPv4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    // IPv6
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


/* ================= MAIN ================= */

int main(int argc, char *argv[])
{
    /* ================= STEP 0: ARGUMENT CHECK ================= */

    /*
        Client needs:
            argv[1] → hostname
    */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(1);
    }


    /* ================= STEP 1: VARIABLE DECLARATION ================= */

    int sockfd;            // socket descriptor
    int numbytes;          // bytes received

    char buf[MAXDATASIZE]; // receive buffer

    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo *p;

    int rv;
    char ipstr[INET6_ADDRSTRLEN];


    /* ================= STEP 2: SETUP HINTS ================= */

    memset(&hints, 0, sizeof hints);

    hints.ai_family   = AF_UNSPEC;    // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP


    /* ================= STEP 3: DNS RESOLUTION ================= */

    rv = getaddrinfo(argv[1], PORT, &hints, &servinfo);

    if (rv != 0) {
        fprintf(stderr,
                "getaddrinfo: %s\n",
                gai_strerror(rv));
        exit(2);
    }


    /* ================= STEP 4: CREATE SOCKET + CONNECT ================= */

    /*
        Try all returned addresses until one works
    */
    for (p = servinfo; p != NULL; p = p->ai_next) {

        /* Create socket */
        sockfd = socket(p->ai_family,
                        p->ai_socktype,
                        p->ai_protocol);

        if (sockfd == -1) {
            perror("client: socket");
            continue;
        }

        /* Print IP we are trying */
        inet_ntop(p->ai_family,
                  get_in_addr((struct sockaddr *)p->ai_addr),
                  ipstr,
                  sizeof ipstr);

        printf("client: attempting connection to %s\n",
               ipstr);

        /* Connect */
        if (connect(sockfd,
                    p->ai_addr,
                    p->ai_addrlen) == -1) {

            perror("client: connect");
            close(sockfd);
            continue;
        }

        break; // success
    }


    if (p == NULL) {
        fprintf(stderr,
                "client: failed to connect\n");
        exit(3);
    }


    printf("client: connected to %s\n", ipstr);

    freeaddrinfo(servinfo); // cleanup address list


    /* ================= STEP 5: RECEIVE DATA ================= */

    /*
        recv() blocks until:
            - data arrives
            - or server closes connection
    */
    numbytes = recv(sockfd,
                    buf,
                    MAXDATASIZE - 1,
                    0);

    if (numbytes == -1) {
        perror("recv");
        exit(4);
    }

    buf[numbytes] = '\0'; // null terminate

    printf("client: received '%s'\n", buf);


    /* ================= STEP 6: CLEANUP ================= */

    close(sockfd);
    return 0;
}
