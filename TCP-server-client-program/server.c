// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[1024];
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Server address setup
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port 8080...\n");

    // Accept
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept");
        exit(1);
    }

    // Receive data
    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    buffer[bytes] = '\0';

    printf("Client says: %s\n", buffer);

    // Send response
    char *reply = "Hello from server!";
    send(client_fd, reply, strlen(reply), 0);

    close(client_fd);
    close(server_fd);

    return 0;
}
