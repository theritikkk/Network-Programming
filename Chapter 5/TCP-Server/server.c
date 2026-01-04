/*
 * server.c -- Fully commented TCP server using getaddrinfo()
 * Supports IPv4 and IPv6
 */

#include <stdio.h>      // printf(), fprintf()
#include <stdlib.h>     // exit()

#include <unistd.h>     // close(), fork()
#include <errno.h>      // errno

/*
    errno is an integer variable
    Automatically set by the kernel / C library
    Updated only when a system call fails
    Holds an error code representing the reason for failure
    If a function succeeds, errno is not reset automatically.
    
System calls like: 
    socket(), bind(), listen(), accept(), connect(), read(), write() 
    usually return: -1 on failure
    But -1 does not tell you the reason.

    errno tells you what exactly went wrong.
*/

/*
Code: 
    perror("bind failed");
Output:
    bind failed: Address already in use

perror() internally uses errno

Code: 
    printf("Error: %s\n", strerror(errno));
Output: 
    Error: Address already in use
*/

#include <string.h>     // memset(), strlen()
/* 
    functions to work with memory and strings
    Even though the name says string, 
    it actually includes raw memory functions too.

    memset() - 
        Fills a block of memory with a specific value,
        Works on bytes, not strings
    
    strlen() -
        Returns the number of characters in a string
*/        

#include <sys/types.h>  // data types
/* 
    It defines standard system data types that are used by: 
    Kernel interfaces, POSIX system calls, Networking and file I/O functions

    It does not define functions, only types.
    Like:
        size_t	         Sizes of memory blocks
        ssize_t          Signed size ( read / write return )
        socklen_t	     Socket address length
            - Tells kernel how much space is available for address info
        pid_t	         Process IDs
        off_t	         File offsets
        time_t	         Time values
        mode_t           File permissions
*/

/* 
POSIX: Portable Operating System Interface
That means the same code works on: Linux, macOS, BSD, UNIX

    User programs cannot directly access hardware or kernel resources, 
        so they must ask the kernel via system calls.
    Examples of kernel services: Creating processes, 
        Reading/writing files, Network communication, Memory allocation
*/

#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <netdb.h>      // getaddrinfo(), freeaddrinfo()
/* 
<netdb.h> is a POSIX networking header that provides: 
    Network database functions
    Protocol - independent address resolution
    It helps convert: 
        hostnames → IP addresses
        service names / ports → usable socket structures
    Support for IPv4 and IPv6

    functions like:
        getaddrinfo(), freeaddrinfo(), gai_strerror()
    are declared in <netdb.h>.

    getaddrinfo() - Converts a hostname + service into a linked list of socket address structures.
    freeaddrinfo() - Frees memory allocated by getaddrinfo(), Prevents memory leaks
*/

#include <arpa/inet.h>  // inet_ntop()
// is used for IP address conversion between binary and human-readable form.

#include <sys/wait.h>   // waitpid()
// used for process control, especially when your server creates child processes using fork()

#include <signal.h>     // sigaction()
// used for signal handling, which lets your program respond to asynchronous events sent by the operating system.

#define PORT "3490"     // Port number (string form required by getaddrinfo)
#define BACKLOG 10      // Max pending connections in queue

/*
 * SIGCHLD handler
 * Reaps zombie child processes created by fork()
 */
void sigchld_handler(int s)
{
    (void)s; // Avoid unused parameter warning

    // Clean up all terminated child processes
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*
 * Returns pointer to the actual IP address (IPv4 or IPv6)
 * Used only for printing client IP address
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        // IPv4 address
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    // IPv6 address
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;         // Listening socket file descriptor
    int new_fd;         // Connected client socket descriptor
    struct addrinfo hints;      // Requirements for getaddrinfo
    struct addrinfo *servinfo;  // Linked list of results
    struct addrinfo *p;         // Iterator over results
    struct sockaddr_storage their_addr; // Client address info
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char client_ip[INET6_ADDRSTRLEN];
    int status;

    /* ================= STEP 1: PREPARE HINTS ================= */

    // Clear the hints structure to avoid garbage values
    memset(&hints, 0, sizeof hints);
    // Set all bytes of the hints structure to 0

    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP socket
    hints.ai_flags = AI_PASSIVE;        // Automatically fill local IP

    /* ================= STEP 2: GET ADDRESS INFO ================= */

    // Ask OS for suitable address structures for binding
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    /* ================= STEP 3: CREATE & BIND SOCKET ================= */

    // Loop through all returned addresses until bind succeeds
    for (p = servinfo; p != NULL; p = p->ai_next) {

        // Create socket
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("server: socket");
            continue;
        }

        // Allow address reuse (prevents "Address already in use" error)
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // Bind socket to IP address and port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        // Successfully bound
        break;
    }

    // Free address info list (no longer needed)
    freeaddrinfo(servinfo);

    // If no address was successfully bound
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    /* ================= STEP 4: LISTEN FOR CONNECTIONS ================= */

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /* ================= STEP 5: HANDLE ZOMBIE PROCESSES ================= */

    // Setup signal handler for cleaning up child processes
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections on port %s...\n", PORT);

    /* ================= STEP 6: ACCEPT CLIENT CONNECTIONS ================= */

    while (1) {
        sin_size = sizeof their_addr;

        // Accept incoming client connection
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        // Convert client IP to readable form
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  client_ip, sizeof client_ip);

        printf("server: got connection from %s\n", client_ip);

        /* ================= STEP 7: HANDLE CLIENT ================= */

        if (!fork()) { // Child process
            close(sockfd); // Child doesn't need listening socket

            // Send message to client
            const char *msg = "Hello client! Connection established.\n";
            send(new_fd, msg, strlen(msg), 0);

            close(new_fd); // Close client socket
            exit(0);       // Terminate child process
        }

        // Parent closes connected socket and waits for more clients
        close(new_fd);
    }

    return 0;
}
