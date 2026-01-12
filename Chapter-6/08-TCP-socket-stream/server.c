/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"      // Port server will listen on
#define BACKLOG 10       // Max pending connections queue

/* ================= SIGCHLD HANDLER ================= */

/*
    Reaps zombie child processes
*/
void sigchld_handler(int s)
{
    (void)s;

    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

/* ================= ADDRESS HELPER ================= */

/*
    Returns IPv4 or IPv6 address from sockaddr
*/
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;

    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    /* ================= SETUP HINTS ================= */

    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    /* ================= GET ADDRESS INFO ================= */

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {

        fprintf(stderr, "getaddrinfo: %s\n",
                gai_strerror(rv));
        return 1;
    }

    /* ================= CREATE + BIND SOCKET ================= */

    for (p = servinfo; p != NULL; p = p->ai_next) {

        sockfd = socket(p->ai_family,
                        p->ai_socktype,
                        p->ai_protocol);

        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }

        // Allow port reuse
        if( setsockopt(sockfd, SOL_SOCKET,
                        SO_REUSEADDR,
                        &yes, sizeof yes) == -1 ) {

            perror("setsockopt");
            close(sockfd);
            continue;
        }

        if (bind(sockfd,
                 p->ai_addr,
                 p->ai_addrlen) == -1) {

            perror("server: bind");
            close(sockfd);
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    /* ================= LISTEN ================= */

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /* ================= HANDLE ZOMBIE CHILDREN ================= */

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    /* ================= ACCEPT LOOP ================= */

    while (1) {

        sin_size = sizeof their_addr;

        new_fd = accept(sockfd,
                       (struct sockaddr *)&their_addr,
                       &sin_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);

        printf("server: got connection from %s\n", s);

        /* ================= FORK ================= */

        pid_t pid = fork();

        if( pid == 0 ) {        // CHILD PROCESS

            close(sockfd);     // child doesn't need listener

            if (send(new_fd,
                     "Hello, world!\n",
                     14, 0) == -1)
                perror("send");

            close(new_fd);
            exit(0);
        }

        else if( pid < 0 ) {   // FORK FAILED
            perror("fork");
        }

        else {                 // PARENT PROCESS
            close(new_fd);
        }
    }

    return 0;
}
