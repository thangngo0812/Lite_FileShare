#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

// Hàm gửi tên file đến server
void sendFilenameToServer(int client_fd, const char *file_path) {
    send(client_fd, file_path, strlen(file_path), 0);
}

// Hàm gửi dữ liệu từ file đến server
void sendFileToServer(int client_fd, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("\nError opening file\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_fd, buffer, bytesRead, 0);
    }

    fclose(file);
}

void sendRequestToServer(int client_fd, const char *file_name) {
    send(client_fd, file_name, strlen(file_name), 0);
}

void receiveFileFromServer(int client_fd, const char *save_path) {
    FILE *received_file = fopen(save_path, "wb");
    if (received_file == NULL) {
        printf("\nError opening file\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytesReceived;

    while ((bytesReceived = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, received_file);
    }

    fclose(received_file);
    printf("File received successfully\n");
}


int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file_path>\n", argv[0]);
        return -1;
    }

    const char *file_path = argv[1];

    int status, client_fd;
    struct sockaddr_in serv_addr;

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

    sendFilenameToServer(client_fd, file_path);

    sendFileToServer(client_fd, file_path);

    printf("File sent\n");

    close(client_fd);

    return 0;
}
