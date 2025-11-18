// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    char buffer[1024];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Connect
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    // Send message
    char *msg = "Hello from client!";
    send(sock, msg, strlen(msg), 0);

    // Receive response
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[bytes] = '\0';

    printf("Server replies: %s\n", buffer);

    close(sock);
    return 0;
}
