#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <iostream>

#define PORT 8081
#define BUFFER_SIZE 1024
#define SAVE_PATH "/root/Desktop/WorkSpace/Lite_FileShare/Server/Saved_file/"

int createServerSocket() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    return server_fd;
}

int acceptClientConnection(int server_fd) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int new_socket;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("Accept");
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    return new_socket;
}

#define FILE_INFO_PATH "/root/Desktop/WorkSpace/Lite_FileShare/Server/file_info.txt"

void writeFileInfo(const char *filename) {
    FILE *file_info = fopen(FILE_INFO_PATH, "a");
    if (file_info != NULL) {
        time_t raw_time;
        struct tm *timeinfo;
        char time_buffer[80];

        time(&raw_time);
        timeinfo = localtime(&raw_time);

        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        fprintf(file_info, "File: %s - Received at: %s\n", filename, time_buffer);

        fclose(file_info);
    } else {
        printf("error\n");
    }
}

void receiveFileFromClient(int client_fd) {
    char buffer[BUFFER_SIZE];
    char filename_buffer[256];
    int valread = recv(client_fd, filename_buffer, sizeof(filename_buffer), 0);
    char *filename = strrchr(filename_buffer, '/');
    if (filename != NULL) {
        filename++;
    } else {
        filename = filename_buffer;
    }

    char file_save_path[512];
    snprintf(file_save_path, sizeof(file_save_path), "%s%s", SAVE_PATH, filename);
    FILE *received_file = fopen(file_save_path, "wb");

    if (received_file == NULL) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    size_t total_bytes_received = 0;
    int bytes_received;

    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        total_bytes_received += bytes_received;
        fwrite(buffer, 1, bytes_received, received_file);
    }

    fclose(received_file);
    printf("File received and saved successfully. Total bytes received: %zu\n", total_bytes_received);

    writeFileInfo(filename);
}
void sendFileToClient(int client_fd, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_fd, buffer, bytesRead, 0);
    }

    fclose(file);
}

int receiveModeFromClient(int client_fd) {
    int mode;
    std::cout << "Waiting for mode from client..." << std::endl;
    int bytes_received = recv(client_fd, &mode, sizeof(int), 0);
    if (bytes_received <= 0) {
        // Xử lý khi không nhận được dữ liệu
        return -1; // Hoặc giá trị khác đặc biệt để phân biệt lỗi
    }
    std::cout << "Received mode: " << mode << std::endl;
    return mode;
}



int main(int argc, char const *argv[]) {
    int server_fd = createServerSocket();
    int client_fd = acceptClientConnection(server_fd);
    int mode = receiveModeFromClient(client_fd);
    if (mode == 0) {
        receiveFileFromClient(client_fd);
    } else {
        //sendFileToClient(client_fd, file_path);
    }
    close(client_fd);
    close(server_fd);


    close(client_fd);
    close(server_fd);

    return 0;
}
