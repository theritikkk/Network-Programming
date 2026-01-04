Examples of POSIX system calls (from your server code):
| System call | Purpose                       | Kernel involvement            |
| ----------- | ----------------------------- | ----------------------------- |
| `socket()`  | Create communication endpoint | Kernel allocates socket       |
| `bind()`    | Attach IP + port              | Kernel updates network tables |
| `listen()`  | Mark socket as passive        | Kernel queue created          |
| `accept()`  | Accept client                 | Kernel creates new socket     |
| `send()`    | Send data                     | Kernel sends via NIC          |
| `recv()`    | Receive data                  | Kernel reads network buffer   |
| `fork()`    | Create process                | Kernel duplicates process     |
| `close()`   | Release resource              | Kernel frees descriptor       |

Normal C functions: strlen(), printf(), memset()
Runs entirely in user space
No kernel involvement

POSIX system call: socket(), read(), write(), fork()
Switches to kernel mode
Uses CPU instructions like syscall / trap

What happens internally:
Your C program
    ↓
POSIX function (socket())
    ↓
System call interface
    ↓
Kernel
    ↓
Hardware / OS resources

POSIX system calls are standardized interfaces defined by POSIX 
that allow user programs to request services from the operating system kernel 
in a portable and consistent way.

<sys/socket.h> exposes POSIX-defined socket system calls that 
allow programs to perform network communication through the kernel.

<netdb.h> provides network database functions such as getaddrinfo() and freeaddrinfo(), which are used to resolve hostnames and services into protocol-independent socket address structures.
Structures defined in <netdb.h>:
| Structure         | Purpose                         |
| ----------------- | ------------------------------- |
| `struct addrinfo` | Holds socket address info       |
| `struct hostent`  | Old hostname structure (legacy) |
| `struct servent`  | Service/port info               |


#include <arpa/inet.h>  // inet_ntop()
    is used for IP address conversion between binary and human-readable form.
arpa → Advanced Research Projects Agency (historical)
inet → Internet

What is <arpa/inet.h>?
<arpa/inet.h> is a POSIX networking header that provides:
    Internet address manipulation functions
    Conversion between network format (binary) and presentation format (text)
    The name:
        arpa → Advanced Research Projects Agency (historical)
        inet → Internet

inet_ntop() converts an IP address from binary (network format) to text (readable format).
What’s happening:
    Detect IPv4 or IPv6
    Extract raw IP address
    Convert to readable string
    Store in client_ip

Why not print IP directly?
Because internally:
    IP addresses are stored in binary
    Printing binary data gives garbage
inet_ntop() makes it readable.

Related functions in <arpa/inet.h>:
| Function      | Purpose                 |
| ------------- | ----------------------- |
| `inet_ntop()` | Binary → string         |
| `inet_pton()` | String → binary         |
| `htonl()`     | Host → network (32-bit) |
| `htons()`     | Host → network (16-bit) |
| `ntohl()`     | Network → host          |
| `ntohs()`     | Network → host          |

IPv4 vs IPv6 support:
| Function        | IPv4 | IPv6 |
| --------------- | ---- | ---- |
| `inet_ntoa()`   | Yes  | No   |
| `inet_ntop()`   | Yes  | Yes  |

<arpa/inet.h> provides internet address conversion functions such as inet_ntop(), which converts binary IP addresses into human-readable string format for IPv4 and IPv6.

What is <sys/wait.h>?
    <sys/wait.h> is a POSIX header that provides:
        Functions to wait for child processes
        Macros to inspect child exit status
    It is essential in multi-process programs.

What problem does waitpid() solve?
    When a child process exits:
        Kernel keeps its exit status
        The child becomes a zombie process
        Zombies waste system resources.
    
    waitpid():
    Collects (reaps) the child’s exit status
    Frees kernel resources

What happens without <sys/wait.h>?
    Zombie processes pile up
    Server may exhaust process table
    Eventually crashes or refuses new forks

Related macros in <sys/wait.h>:
| Macro                 | Purpose               |
| --------------------- | --------------------- |
| `WNOHANG`             | Non-blocking wait     |
| `WIFEXITED(status)`   | Child exited normally |
| `WEXITSTATUS(status)` | Exit code             |
| `WIFSIGNALED(status)` | Killed by signal      |


What is <signal.h>?

<signal.h> is a POSIX header that provides:

Signal constants (e.g. SIGCHLD, SIGINT)

Signal handling functions

Signal-related data structures

It allows a program to catch, ignore, or handle signals.

What is a signal?

A signal is:

A notification sent by the OS to a process to inform it that an event has occurred.

Examples:

A child process exits

User presses Ctrl + C

Illegal memory access

Why is sigaction() used?

sigaction() is the modern, reliable way to handle signals.

It replaces the older signal() function.

✔ More predictable
✔ Portable
✔ Safe for servers

Why is this needed in your server?

Your server creates child processes using fork().

When a child exits:

OS sends SIGCHLD to parent

Parent must handle it

Otherwise → zombie processes



```
struct sigaction sa;
sa.sa_handler = sigchld_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
sigaction(SIGCHLD, &sa, NULL);
```
| Line            | Explanation                         |
| --------------- | ----------------------------------- |
| `sa.sa_handler` | Function to call when signal occurs |
| `sigemptyset()` | Clear blocked signals               |
| `SA_RESTART`    | Restart interrupted system calls    |
| `SIGCHLD`       | Signal sent when child exits        |
Why SA_RESTART is important

Without it:

accept() may fail with EINTR

Server may crash or behave incorrectly

With it:

Interrupted calls restart automatically

Common signals from <signal.h>:
| Signal    | Meaning                       |
| --------- | ----------------------------- |
| `SIGCHLD` | Child process terminated      |
| `SIGINT`  | Ctrl + C                      |
| `SIGTERM` | Termination request           |
| `SIGKILL` | Force kill (cannot be caught) |
| `SIGSEGV` | Segmentation fault            |

What happens without <signal.h>?

sigaction() undefined

SIGCHLD undefined

Server leaks zombie processes

<signal.h> provides signal handling facilities such as sigaction(), which allows a program to handle signals like SIGCHLD to clean up child processes and ensure stable server execution.