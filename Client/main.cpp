#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file_path>\n", argv[0]);
        return -1;
    }

    const char *file_path = argv[1];

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    FILE *file;

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "10.225.1.98", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("\nError opening file\n");
        close(client_fd);
        return -1;
    }

    // Gửi tên file đến server trước
    send(client_fd, file_path, strlen(file_path), 0);

    char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_fd, buffer, bytesRead, 0);
    }

    printf("File sent\n");

    close(client_fd);
    fclose(file);

    return 0;
}
