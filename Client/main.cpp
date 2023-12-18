#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>


#define PORT 8081
#define BUFFER_SIZE 1024


int createClientSocket() {
    int client_fd;
    struct sockaddr_in serv_addr{};

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "10.225.1.98", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

void sendFilenameToServer(int client_fd, const char *file_path) {
    char filename_buffer[256];
    strncpy(filename_buffer, file_path, sizeof(filename_buffer));

    size_t data_length = strlen(filename_buffer);

    char *filename = strrchr(filename_buffer, '/');
    if (filename != NULL) {
        filename++;
        std::cout << "filename: " << filename << std::endl;
    } else {
        filename = filename_buffer;
        std::cout << "filename = Null: " << filename << std::endl;
    }

    std::cout << "filename size : " << data_length << std::endl;

    send(client_fd, &data_length, sizeof(size_t), 0);

    send(client_fd, filename, data_length, 0);
}



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

void sendFilenameAndFileToServer(int client_fd, const char *file_path) {

    sendFilenameToServer(client_fd, file_path);
    sendFileToServer(client_fd, file_path);

    printf("File uploaded\n");

    close(client_fd);
}

void receiveFileFromServer(int client_fd, const char *file_path) {

    FILE *file = fopen(file_path, "wb");
    std::cout<< "File path: "<< file_path<<std::endl;
    if (file == NULL) {
        printf("\nError opening file for writing\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
    }

    fclose(file);
}

void downloadFileFromServer(int client_fd, const char *file_path) {

    sendFilenameToServer(client_fd, file_path);

    receiveFileFromServer(client_fd, file_path);

    printf("File downloaded\n");

    close(client_fd);
}

void sendRequesttoServer(int client_fd, int mode) {
    std::cout << "Mode: " << mode << std::endl;
    send(client_fd, &mode, sizeof(int), 0);
}


int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <file_path> <mode>\n", argv[0]);
        return -1;
    }

    const char *file_path = argv[1];
    int mode = atoi(argv[2]);
    if (mode == 0) {
        int client_fd = createClientSocket();
        sendRequesttoServer(client_fd, mode);
        sendFilenameAndFileToServer(client_fd, file_path);
        close(client_fd);
    } else {
        int client_fd = createClientSocket();
        sendRequesttoServer(client_fd, mode);
        downloadFileFromServer(client_fd, file_path);
        close(client_fd);
    }


    return 0;
}