#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstdint>

uint32_t calculateChecksum(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);

    if (!file.is_open()) {
        return 0;
    }

    uint32_t checksum = 0;
    char byte;

    while (file.get(byte)) {
        checksum += static_cast<unsigned char>(byte);
    }

    file.close();

    return checksum;
}
bool sendAll(int socket, const char* data, int totalBytes) {
    int bytesSent = 0;

    while (bytesSent < totalBytes) {
        int result = send(
            socket,
            data + bytesSent,
            totalBytes - bytesSent,
            0
        );

        if (result <= 0) {
            return false;
        }

        bytesSent += result;
    }

    return true;
}
int main() {
    std::cout << "[CLIENT] Program started." << std::endl;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1) {
        std::cerr << "[CLIENT] Socket creation failed." << std::endl;
        return 1;
    }

    std::cout << "[CLIENT] Socket created successfully." << std::endl;

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        std::cerr << "[CLIENT] Invalid IP address." << std::endl;
        close(clientSocket);
        return 1;
    }

    if (connect(clientSocket,
                (struct sockaddr*)&serverAddress,
                sizeof(serverAddress)) == -1) {

        std::cerr << "[CLIENT] Connection failed." << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "[CLIENT] Connected to server successfully." << std::endl;

    std::string fileName;

    std::cout << "[CLIENT] Enter file name: ";
    std::getline(std::cin, fileName);

    std::ifstream file(fileName, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "[CLIENT] File could not be opened." << std::endl;
        close(clientSocket);
        return 1;
    }

    std::cout << "[CLIENT] File opened successfully: " << fileName << std::endl;

    file.seekg(0, std::ios::end);
    long long fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "[CLIENT] File size: " << fileSize << " bytes" << std::endl;

    uint32_t checksum = calculateChecksum(fileName);

    std::cout << "[CLIENT] Checksum: " << checksum << std::endl;

    std::string fileInfo =
        fileName + "\n" +
        std::to_string(fileSize) + "\n" +
        std::to_string(checksum) + "\n";

if (!sendAll(clientSocket, fileInfo.c_str(), fileInfo.size())) {        std::cerr << "[CLIENT] File information could not be sent." << std::endl;
        file.close();
        close(clientSocket);
        return 1;
    }

    std::cout << "[CLIENT] File information sent successfully." << std::endl;

    char buffer[1024];

    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int bytesToSend = file.gcount();

        if (!sendAll(clientSocket, buffer, bytesToSend)) {
            std::cerr << "[CLIENT] File transfer failed." << std::endl;
            file.close();
            close(clientSocket);
            return 1;
        }
    }

    std::cout << "[CLIENT] File sent successfully." << std::endl;

file.close();

char resultBuffer[1024] = {0};

int resultBytes = recv(
    clientSocket,
    resultBuffer,
    sizeof(resultBuffer) - 1,
    0
);

if (resultBytes <= 0) {
    std::cerr << "[CLIENT] Server response could not be received." << std::endl;
    close(clientSocket);
    return 1;
}

std::string serverResult(resultBuffer, resultBytes);

if (serverResult == "SUCCESS") {
    std::cout << "[CLIENT] Transfer completed successfully." << std::endl;
} else if (serverResult == "CHECKSUM_ERROR") {
    std::cout << "[CLIENT] Transfer failed: checksum error." << std::endl;
} else {
    std::cout << "[CLIENT] Unknown server response." << std::endl;
}

close(clientSocket);

    return 0;
}