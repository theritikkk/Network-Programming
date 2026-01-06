# 🚀 Concurrent TCP Server using POSIX Sockets (C)

This project implements a **concurrent TCP server** using **POSIX system calls** in C.  
It supports **IPv4 and IPv6**, handles **multiple clients concurrently using `fork()`**, and correctly manages **kernel resources** such as sockets, processes, and signals.

The goal of this project is not just functionality, but to **deeply understand how user programs interact with the operating system kernel** during network communication.

---

## 📌 Key Features

- ✅ IPv4 + IPv6 support using `getaddrinfo()`
- ✅ Concurrent client handling using **fork-per-connection**
- ✅ Proper cleanup of **zombie processes**
- ✅ Demonstrates **user space ↔ kernel space transitions**
- ✅ Stress-tested with **hundreds of clients**
- ✅ Written using **portable POSIX APIs**

---

## 🧠 High-Level Server Lifecycle

<pre>
        Prepare address info
                ↓
           Create socket
                ↓
        Bind socket to IP + port
                ↓
        Listen for connections
                ↓
           Accept client
                ↓
             fork()
         ↙            ↘
      Child          Parent
 Handle client    Keep listening
</pre>



This architecture is known as the **fork-per-connection server model**.

---

## 🔧 POSIX System Calls Used

These functions **cross the user–kernel boundary** and invoke kernel services.

| System Call | Purpose                       | Kernel Involvement                     |
|------------|-------------------------------|----------------------------------------|
| `socket()` | Create communication endpoint | Kernel allocates socket structure      |
| `bind()`   | Attach IP + port              | Kernel updates network tables          |
| `listen()` | Mark socket as passive        | Kernel creates accept queue            |
| `accept()` | Accept client connection      | Kernel creates new connected socket    |
| `send()`   | Send data                     | Kernel sends data via network stack    |
| `recv()`   | Receive data                  | Kernel reads from network buffers      |
| `fork()`   | Create new process            | Kernel duplicates process context      |
| `close()`  | Release resource              | Kernel frees file descriptor           |
| `waitpid()`| Reap child process            | Kernel cleans process table entry      |

---

## 🧑‍💻 Normal C Library Functions (User Space)

These functions **do not invoke the kernel directly**:

- `strlen()`
- `printf()`
- `memset()`

They run entirely in **user space**.

---

## 🔁 What Happens Internally (User → Kernel)

<pre>
            Your C program
                ↓
    POSIX function (e.g., socket())
                ↓
    System call interface (syscall / trap)
                ↓
            Kernel
                ↓   
    Hardware / OS - managed resources
</pre>



POSIX provides a **standardized interface**, allowing the same code to work across:
- Linux
- macOS
- BSD
- UNIX systems

---

## 📦 Important Headers Explained

### `<sys/socket.h>`
Provides core **POSIX socket system calls**:
- `socket()`, `bind()`, `listen()`, `accept()`

These functions enable **network communication through the kernel**.

---

### `<netdb.h>`
Provides **protocol-independent address resolution**.

Functions:
- `getaddrinfo()`
- `freeaddrinfo()`
- `gai_strerror()`

Structures:
| Structure         | Purpose                               |
|------------------|---------------------------------------|
| `struct addrinfo`| Holds resolved socket address info    |
| `struct hostent` | Legacy hostname structure             |
| `struct servent` | Service/port information              |

Why `getaddrinfo()`?
- Works with IPv4 **and** IPv6
- Replaces older, IPv4-only APIs

---

### `<arpa/inet.h>` — Internet Address Conversion

Used to convert IP addresses between **binary** and **human-readable** formats.

Why needed?
- IPs are stored internally in binary
- Printing binary data gives unreadable output

Key functions:
| Function      | Purpose                 |
|--------------|-------------------------|
| `inet_ntop()`| Binary → readable text  |
| `inet_pton()`| Text → binary           |
| `htonl()`    | Host → network (32-bit) |
| `htons()`    | Host → network (16-bit) |
| `ntohl()`    | Network → host          |
| `ntohs()`    | Network → host          |

IPv4 vs IPv6 support:
| Function      | IPv4 | IPv6 |
|--------------|------|------|
| `inet_ntoa()`| ✅   | ❌   |
| `inet_ntop()`| ✅   | ✅   |

---

### `<sys/wait.h>` — Zombie Process Management

When a child process exits:
- Kernel keeps its exit status
- The process becomes a **zombie**
- Zombies waste kernel resources

`waitpid()`:
- Reaps the child
- Frees kernel process table entry

Important macros:
| Macro                 | Purpose                     |
|----------------------|-----------------------------|
| `WNOHANG`             | Non-blocking wait           |
| `WIFEXITED(status)`  | Child exited normally       |
| `WEXITSTATUS(status)`| Exit code                   |
| `WIFSIGNALED(status)`| Child killed by signal      |

---

### `<signal.h>` — Signal Handling

Signals are **asynchronous notifications** sent by the OS.

Examples:
- Child exits → `SIGCHLD`
- Ctrl + C → `SIGINT`

Why `sigaction()`?
- Modern replacement for `signal()`
- Predictable and portable
- Safe for servers

Used in this server to:
- Catch `SIGCHLD`
- Reap zombie processes
- Keep the server stable

```c
struct sigaction sa;
sa.sa_handler = sigchld_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
sigaction(SIGCHLD, &sa, NULL);
```


Why SA_RESTART?
- Prevents accept() from failing with EINTR
- Ensures uninterrupted server operation


## 🔁 Socket Roles in the Server
| Socket   | Purpose                                   |
| -------- | ----------------------------------------- |
| `sockfd` | Listening socket (never talks to clients) |
| `new_fd` | Connected socket (talks to one client)    |

Visual flow:
<pre>
listen socket (sockfd)
        |
     accept()
        |
   new_fd (client)
        |
     fork()
     /     \
 child    parent
 handle   accept more
 client   clients
</pre>


## ⚠️ Understanding BACKLOG

```
#define BACKLOG 10
listen(sockfd, BACKLOG);
```

Important clarification:
BACKLOG limits the number of pending connections waiting to be accepted,
NOT the total number of clients the server can handle.

Since this server accepts connections immediately, the accept queue rarely fills,
allowing far more than 10 clients to connect even when `BACKLOG` is set to 10.

---

## 🧪 Stress Testing & Observations

### Experiments performed

- Connected **dozens of concurrent clients** using `nc`
- Burst-tested with **hundreds of background clients**
- Observed operating system limits using:
  ```bash
  ulimit -u
  ps aux | wc -l

### Observed behavior

Fork-per-connection servers are limited by:
- Process limits (per-user and system-wide)
- File descriptor limits
- Memory overhead per process

Eventually, the operating system reports:

<pre> 
fork failed: resource temporarily unavailable
</pre>

This clearly demonstrates why modern high-performance servers use event-driven models
instead of creating one process per client.

## 🧠 What This Project Demonstrates
- How user programs interact with the kernel
- How TCP servers work internally
- Why zombie processes occur
- Why fork() does not scale indefinitely
- How OS-enforced limits protect system stability

## 📸 Screenshots & Experiments

The following screenshots document real experiments performed while running and
stress-testing the concurrent TCP server on macOS.

---

### 🔹 Server Startup and Single Client Connection

This screenshot shows:
- Successful compilation of the server
- Server listening on port `3490`
- A client connecting using `nc`
- Server logging the IPv6 loopback address (`::1`)

![Server startup and single client connection](screenshots/server-startup-single-client.png)

---

### 🔹 Multiple Concurrent Client Connections

This screenshot demonstrates:
- Multiple terminal windows acting as clients
- Each client connecting independently using `nc localhost 3490`
- The server handling each connection concurrently using `fork()`

This validates the **fork-per-connection concurrency model**.

![Multiple concurrent clients](screenshots/multiple-concurrent-clients.png)

---

### 🔹 Stress Testing and Process Exhaustion

This screenshot captures:
- A large number of simultaneous client connections
- Repeated `fork()` calls by the server
- The operating system eventually rejecting new process creation

Observed error:
```text
fork failed: resource temporarily unavailable
```
![Multiple concurrent clients](screenshots/stress-test-process-exhaustion.png)

## 🏁 Summary

This project implements a correct, portable, concurrent TCP server
using POSIX APIs and demonstrates real operating-system behavior under load.

It serves as:
- 📘 A learning reference for network programming
- 🧠 A demonstration of systems-level understanding
- 🚀 A foundation for exploring event-driven servers (select, poll, kqueue)

