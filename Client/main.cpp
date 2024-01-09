#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <sys/select.h>


#define PORT 8081
#define BUFFER_SIZE 2048
#define folder_download_path "/root/Desktop/WorkSpace/Lite_FileShare/Client/Download"


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

    const char *filename = strrchr(file_path, '/');
    if (filename == NULL) {
        filename = file_path;
    } else {
        filename++;
    }
    std::cout<< "name_of_filesent:  "<< filename<<std::endl;

    size_t data_length = strlen(filename);
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

    fseek(file, 0, SEEK_END);

    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::cout<< "file_size_to_send: "<< file_size<<std::endl;

    send(client_fd, &file_size, sizeof(size_t), 0);
    char buffer[BUFFER_SIZE];
    size_t bytes_send;

    while ((bytes_send = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_fd, buffer, bytes_send, 0);
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
    size_t file_size =0;
    int filesize_recv = recv(client_fd, &file_size, sizeof(size_t), 0);
    char full_path[100];
    if (snprintf(full_path, sizeof(full_path), "%s/%s", folder_download_path, file_path) >= sizeof(full_path)) {
        return;
    }
    FILE *file = fopen(full_path, "wb");
    std::cout<< "File path: "<< full_path<<std::endl;
    if (file == NULL) {
        printf("\nError opening file for writing\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    size_t total_bytes_received = 0;
    fd_set readfds;
    struct timeval timeout;

    while (total_bytes_received < file_size) {
        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);

        timeout.tv_sec = 90;
        timeout.tv_usec = 0;

        int ready = select(client_fd + 1, &readfds, NULL, NULL, &timeout);

        if (ready == -1) {
            perror("Select error");
            fclose(file);
            close(client_fd);
            remove(full_path);
            return;
        } else if (ready == 0) {
            printf("\nTimeout: No additional data received from server\n");
            break;
        }

        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            fwrite(buffer, 1, bytes_received, file);
            total_bytes_received += bytes_received;
            std::cout << "total_bytes_received: " << total_bytes_received << std::endl;
        } else if (bytes_received == 0) {
            printf("\nThe connection was closed before the complete file was received, or the file was invalid\n");
            break;
        } else {
            perror("Error receiving data");
            fclose(file);
            close(client_fd);
            remove(full_path);
            return;
        }

        if (total_bytes_received == file_size) {
            break;
        }
    }

    if (total_bytes_received != file_size) {
        std::cout << "total_bytes_received: " << total_bytes_received << " filesize: " << file_size << std::endl;
        printf("\nReceived file size doesn't match expected size\n");
        fclose(file);
        close(client_fd);
        remove(full_path);
        return;
    }
}
void downloadFileFromServer(int client_fd, const char *file_path) {

    sendFilenameToServer(client_fd, file_path);

    receiveFileFromServer(client_fd, file_path);

    close(client_fd);
}

void sendRequesttoServer(int client_fd, int mode) {
    std::cout << "Mode: " << mode << std::endl;
    send(client_fd, &mode, sizeof(int), 0);
}

enum TransferMode {
    UPLOAD = 0,
    DOWNLOAD = 1
};


int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <file_path> <mode>\n";
        return -1;
    }

    const char *file_path = argv[1];
    std::string mode_str = argv[2];

    std::map<std::string, TransferMode> modeMap = {
            {"0", UPLOAD},
            {"1", DOWNLOAD}
    };

    if (modeMap.find(mode_str) != modeMap.end()) {
        TransferMode mode = modeMap[mode_str];
        int client_fd = createClientSocket();

        switch (mode) {
            case UPLOAD:
                sendRequesttoServer(client_fd, mode);
                sendFilenameAndFileToServer(client_fd, file_path);
                break;
            case DOWNLOAD:
                sendRequesttoServer(client_fd, mode);
                downloadFileFromServer(client_fd, file_path);
                break;
            default:
                std::cout << "Invalid mode\n";
                break;
        }

        close(client_fd);
    } else {
        std::cout << "Invalid mode\n";
    }

    return 0;
}