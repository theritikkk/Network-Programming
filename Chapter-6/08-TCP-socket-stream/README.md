# 📡 TCP Socket Programming in C
Concurrent Server–Client Communication (IPv4 & IPv6)

This project demonstrates real TCP client–server communication in C using Berkeley sockets.
It supports IPv4 and IPv6, handles multiple clients concurrently using fork(), and includes robust error handling.


---

## 📂 Project Structure

```text
8-TCP-socket-stream/
│
├── server.c
├── client.c
├── screenshots/
│   ├── server_waiting.png
│   ├── single_client.png
│   ├── multiple_clients.png
│
└── README.md
```

---

## 🚀 Features

Server:

- Listens on TCP port 3490
- Supports IPv4 & IPv6
- Handles multiple clients simultaneously using fork()
- Displays:
    - Client IP address
    - Connection logs
- Prevents zombie processes using SIGCHLD handler
- Uses setsockopt() for port reuse

Client:

- Resolves hostname using getaddrinfo()
- Connects via TCP
- Receives server message
- Prints:
    - Connection attempts
    - Server response
- Works with IPv4 & IPv6

---

## 🛠 Compilation

```bash
gcc -Wall -Wextra -pedantic server.c -o server
gcc -Wall -Wextra -pedantic client.c -o client
```

---


## ▶️ Running the Program

### 1️⃣ Start Server (Terminal 1):

```bash
./server
```

Expected output:
```bash
server: waiting for connections...
```

---

### 2️⃣ Run Client (Terminal 2):

```bash
./client localhost
```

Output:
```bash
client: attempting connection to ::1
client: connected to ::1
client: received 'Hello, world!'
```

---


### 3️⃣ Multiple Clients (Concurrency Test):

Open multiple terminals and run:

```bash
./client localhost
```

Server output:

```bash
server: got connection from ::1
server: got connection from ::1
server: got connection from ::1
```

This proves concurrent handling using fork().

---



## 📸 Screenshots

### 🔹 Server Waiting

![demo1](screenshots/server-setup.png)

---

### 🔹 Single Client Connection

![demo2](screenshots/server-client-setup.png)


---

### 🔹 Multiple Clients

![demo3](screenshots/server-multiple-client.png)


---


## 🧪 How to Check Number of Clients Handled

Your server prints this line for every client:

```bash
printf("server: got connection from %s\n", s);
```

Count how many times this appears.
Each line = 1 client handled


---

## 🧠 Concepts Demonstrated

- TCP socket lifecycle
- getaddrinfo()
- connect()
- accept()
- fork() for concurrency
- SIGCHLD handling
- Zombie process prevention
- IPv4 & IPv6 compatibility
- Error handling best practices

---

## ⚠️ Important Notes

- Server runs continuously
- Each client handled by a separate child process
- Parent keeps listening
- Clean resource management
- Production-style structure

---
