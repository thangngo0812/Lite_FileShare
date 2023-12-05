#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define SAVE_PATH "/root/Desktop/WorkSpace/Lite_FileShare/Server/Saved_file/"

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = { 0 };

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

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("Accept");
        exit(EXIT_FAILURE);
    }
    char filename_buffer[256];
    int valread = recv(new_socket, filename_buffer, sizeof(filename_buffer), 0);
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

    while ((bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        total_bytes_received += bytes_received;
        fwrite(buffer, 1, bytes_received, received_file);
    }

    fclose(received_file);
    printf("File received and saved successfully. Total bytes received: %zu\n", total_bytes_received);

    close(new_socket);
    close(server_fd);

    return 0;
}
