# TCP Echo Client-Server (C Socket Programming)

A simple **TCP Echo Server and Client** implemented in C to demonstrate
basic socket programming using `send()` and `recv()`.

This project shows how a client sends messages to a server and
the server **echoes back the same message**.

---

## 🚀 Features

✔ TCP communication  
✔ IPv4 & IPv6 support  
✔ Proper error handling  
✔ Clean and well-commented code  
✔ Client exit command  
✔ Works on Linux & macOS  

---

## 📂 Project Structure

```text
tcp-echo-client-server/
|
├──screenshots/
│   └── tcp-echo-client-server-demo.png
|
├──server.c
├──client.c
│   
└── README.md
```


---

## 🛠 Requirements

- GCC compiler
- Linux / macOS terminal
- Basic knowledge of C

---

## ⚙️ Compilation

Compile the server:

```bash
gcc server.c -o server
```

Compile the client:

```bash
gcc client.c -o client
```

---

## ▶️ How to Run

Step 1: Start Server

```bash
./server
```

Output:

```bash
Server is listening on 3490...
```

---

Step 2: Run Client (in another terminal)
```bash
./client localhost 3490
```

---

Step 3: Start chatting

Example:

```bash
Enter message ( type exit to quit ): hello there !!!
Server replied : hello there !!!

Enter message ( type exit to quit ): I am working on C language based network programming.
Server replied : I am working on C language based network programming.

Enter message ( type exit to quit ): 👍
Server replied : 👍

Enter message ( type exit to quit ): exit
```

---

## 🖼 Demo Output

![demo](/screenshots/tcp-echo-client-server-demo.png)

(Screenshot showing server & client communication)

---

## 📌 How It Works

Server:
- Creates socket
- Binds to port 3490
- Listens for connections
- Accepts client
- Receives message
- Sends same message back (echo)

Client:
- Connects to server
- Sends message
- Receives echoed response
- Displays it
- Type exit to quit

---

## 🧠 Learning Outcomes

- TCP socket creation
- getaddrinfo() usage
- bind(), listen(), accept()
- send() and recv()
- Client-server architecture
- Error handling in sockets


---

### 👨‍💻 Author

Ritik Raj

![GitHub](https://github.com/theritikkk)

![LinkedIn](https://www.linkedin.com/in/ritik-raj-773383167/)

---

