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
 * Reaps ( cleans up ) zombie child processes created by fork()
 */
void sigchld_handler( int s ) {
    (void)s; // Avoid unused parameter warning

    // Clean up all terminated child processes
    // i.e., Reaping zombie processes
    while( waitpid( -1, NULL, WNOHANG ) > 0 );
}
// This signal handler catches SIGCHLD and uses waitpid() with WNOHANG to reap all terminated child processes, preventing zombie processes in a multi - process server.

/*
 * Returns pointer to the actual IP address (IPv4 or IPv6)
 * Used only for printing client IP address
 */
void *get_in_addr( struct sockaddr *sa ) {

    /*
        Takes a generic socket address
        Returns a generic pointer (void *) to the IP address
        void * is used because:
            IPv4 and IPv6 address types are different sizes
    */

    if( sa -> sa_family == AF_INET ) {
        // IPv4 address
        return &( ( ( struct sockaddr_in* ) sa )-> sin_addr );
    }

    // IPv6 address
    /* 
        Return &( ( ( struct sockaddr_in6* ) sa ) -> sin6_addr );
        Cast generic pointer to struct sockaddr_in *
        Access sin_addr ( IPv4 address field )
        Return address of that field
    */
}
// Extracts and returns a pointer to the IP address from a generic socket address structure (IPv4 or IPv6).

// The entire lifecycle of a concurrent TCP server
int main( void ) {

    /*
    Create a TCP server that:
        Works with IPv4 and IPv6
        Binds to a port
        Listens for incoming connections
        Accepts multiple clients
        Handles each client in a separate child process
        Prevents zombie processes
    */

    // listening socket: used only to accept new connections
    int sockfd;         // Listening socket file descriptor

    // connected socket: used to communicate with a specific client
    int new_fd;         // Connected client socket descriptor

    // hints → tells OS what kind of socket we want
    struct addrinfo hints;      // Requirements for getaddrinfo
    
    // servinfo → linked list returned by getaddrinfo()
    struct addrinfo *servinfo;  // Linked list of results

    // p → iterator to walk through that linked list
    struct addrinfo *p;         // Iterator over results

    // stores client address: Large enough for IPv4 or IPv6
    struct sockaddr_storage their_addr; // Client address info

    // size of client address structure: required by accept()
    socklen_t sin_size;

    // used to handle SIGCHLD: prevents zombie processes
    struct sigaction sa;

    // used by setsockopt() to enable address reuse
    int yes = 1;

    // stores client IP in human - readable form
    char client_ip[ INET6_ADDRSTRLEN ];

    // stores return value of getaddrinfo()
    int status;


    /* ================= STEP 1: PREPARE HINTS ================= */

    // clear the hints structure to avoid garbage values
    memset( &hints, 0, sizeof hints );
    // set all bytes of the hints structure to 0

    hints.ai_family = AF_UNSPEC;        // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TCP socket, not UDP i.e., SOCK_DGRAM
    hints.ai_flags = AI_PASSIVE;        // automatically fill local IP
    // server socket: OS fills local IP automatically (0.0.0.0 / ::)

    /* ================= STEP 2: GET ADDRESS INFO ================= */

    // Ask OS for suitable address structures for binding
    if( ( status = getaddrinfo( NULL, PORT, &hints, &servinfo ) ) != 0 ) {
        /*
            getaddrinfo : converts port + requirements into usable socket addresses
            returns linked list ( IPv4 + IPv6 possibilities )
            reason for linked list : system may support multiple protocols

            getaddrinfo : returns 0 → success : ( status = 0 → success )
            getadrinfo : returns non-zero → error code : ( status = EAI_FAIL → error )

            call getaddrinfo(), store its return value in status, 
            and if the result is not zero, handle the error
            in the given below manner
        */
        fprintf( stderr, "getaddrinfo error: %s\n", gai_strerror( status ) );
        /*
            getaddrinfo() does not use errno
            it returns its own error codes
            gai_strerror(status) converts them to text
        */
        exit( 1 );
    }


    /* ================= STEP 3: CREATE & BIND SOCKET ================= */

    // Loop through all returned addresses until bind succeeds
    for( p = servinfo; p != NULL; p = p -> ai_next ) {
        // i.e., try each address until one works

        // create a socket
        sockfd = socket( p -> ai_family, p -> ai_socktype, p -> ai_protocol );
        // kernel allocates a socket descriptor

        if( sockfd == -1 ) {
            perror( "server: socket" );
            continue;
        }

        // Allow address reuse ( prevents "Address already in use" error )
        if( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) == -1 ) {
            /*
                setsockopt() allow to configure how a socket behaves at the kernel level
                setsockopt() returns : 0 → success
                setsockopt() returns : -1 → failure ( sets errno )
                
                sockfd = socket you just created using socket()
                        the option applies only to this socket
                
                        SOL_SOCKET = specifies the level of the option
                SOL_SOCKET means: this is a generic socket - level option
                
                SO_REUSEADDR = the actual option name
                it allows reuse of a local address / port
                i.e., allows binding to the same port,
                enables quick server restarts,
                essential for development and production
                
                &yes : Pointer to the option value
                    yes = 1 → enable the option
                    yes = 0 → disable the option
                
                sizeof(int) : size of the option value
                    kernel needs to know : how many bytes to read
            */

            perror( "setsockopt" );
            /*
                setsockopt() sets errno on failure
                perror() prints a human - readable error
                example output : ( setsockopt: Permission denied )
            */
            exit( 1 );

            // This code enables the reuse of a socket’s local address and port, 
            // preventing bind errors when restarting a server.
        }

        // Bind socket to IP address and port : therefore, server is now reachable
        /*
            sockfd : socket already created by socket()
            at this point:
                the socket exists
                but it has no address
            
            p -> ai_addr : pointer to a socket address structure
            contains:
                IP address ( IPv4 or IPv6 )
                Port number ( 3490 )
            filled by getaddrinfo()
            this structure is already in binary network format, ready for the kernel.

            p -> ai_addrlen : size ( in bytes ) of the address structure
            needed because:
                IPv4 and IPv6 have different sizes
                Kernel must know how much memory to read
        */
        if( bind( sockfd, p -> ai_addr, p -> ai_addrlen ) == -1 ) {
            /* 
                bind() checks address availability
                option must be active before the check
                bind() attaches a socket to a specific local IP address and port number, 
                making the server reachable at that address
            */
            close( sockfd );
            perror( "server : bind" );
            continue;

            /*
                After bind():
                    The socket has its own internal address
                    It does not depend on servinfo anymore
                So freeing servinfo does not affect the socket which is the next part of code
            */
        }

        // Successfully bound
        break;
        // break out of the loop
    }

    // Free address info list ( no longer needed )
    // freeaddrinfo(servinfo) releases the dynamically allocated linked list of address structures created by getaddrinfo(), preventing memory leaks after the server finishes binding.
    freeaddrinfo( servinfo );
    /* 
        This frees dynamically allocated address list 
        prevents memory leaks

        servinfo points to a linked list of struct addrinfo
        This list is allocated by getaddrinfo()
        Each node contains:
            Address family ( IPv4 / IPv6 )
            Socket type
            Protocol
            Socket address ( ai_addr )
            Pointer to next node ( ai_next )
    
        getaddrinfo() did allocated this memory internally

        After the for loop of servinfo :
            Successfully bound ( p != NULL ), or
            Failed for all addresses ( p == NULL )
        
        At this point: we no longer need the address list
            The kernel has already copied what it needed during bind()
            So it is safe to free it here
    */


    // If no address was successfully bound
    if( p == NULL ) {
        // p == NULL : means loop finished i.e., no address could be bound
        fprintf( stderr, "server: failed to bind\n" );
        exit( 1 );
    }


    /* ================= STEP 4: LISTEN FOR CONNECTIONS ================= */

    if( listen( sockfd, BACKLOG ) == -1 ) {
        
        // marks socket as passive
        // Kernel starts queueing incoming connections
        
        perror( "listen" );
        exit( 1 );

        // At this point: server is ready, but no client is connected yet.
    }


    /* ================= STEP 5: HANDLE ZOMBIE PROCESSES ================= */

    // Setup signal handler for cleaning up child processes
    sa.sa_handler = sigchld_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;   // With it : interrupted system calls restart automatically
    // Without it : accept() may fail with EINTR
    // Server loop breaks unexpectedly

    // Meaning : when SIGCHLD occurs, call sigchld_handler()


    /*
        sigaction(SIGCHLD, &sa, NULL) installs a signal handler that reaps( clean up a finished child process ) terminated child processes, preventing zombie processes in a server that uses fork().
    */
    if( sigaction( SIGCHLD, &sa, NULL ) == -1 ) {
        /* 
            This registers a signal handler for the signal SIGCHLD - child process termination ( exits or stops )
            Each client is handled by a child process
            When the child finishes -> OS sends 'SIGCHLD' to the parent

            &sa : how to handle the signal
            NULL : don’t care about old handler
        */
        perror( "sigaction" );
        exit( 1 );
    }

    printf( "server: waiting for connections on port %s...\n", PORT );



    /* ================= STEP 6: ACCEPT CLIENT CONNECTIONS ================= */

    while( 1 ) {
        // Infinite loop → server runs continuously
        sin_size = sizeof their_addr;

        // Accept incoming client connection
        new_fd = accept( sockfd, ( struct sockaddr * ) &their_addr, &sin_size );
        if( new_fd == -1 ) {
            perror( "accept" );
            continue;
        }

        // Convert client IP to readable form
        inet_ntop( their_addr.ss_family,
                  get_in_addr( ( struct sockaddr * ) &their_addr ),
                  client_ip, sizeof client_ip 
                );

        printf( "server: got connection from %s\n", client_ip );

        /* ================= STEP 7: HANDLE CLIENT ================= */

        if( !fork() ) { // Child process

            close( sockfd ); // Child doesn't need listening socket

            // Send message to client
            const char *msg = "Hello client! Connection established.\n";
            send( new_fd, msg, strlen(msg), 0 );

            close( new_fd ); // Close client socket
            exit( 0 );       // Terminate child process
        }

        // Parent closes connected socket and waits for more clients
        close( new_fd );
    }

    return 0;
}
