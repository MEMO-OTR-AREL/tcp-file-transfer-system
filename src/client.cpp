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
std::string encodeFileInfo(
    const std::string& fileName,
    long long fileSize,
    uint32_t checksum
) {
    return fileName + "\n" +
           std::to_string(fileSize) + "\n" +
           std::to_string(checksum) + "\n";
}

int main() {
    std::cout << "[CLIENT] Program started." << std::endl;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1) {
        std::cerr << "[CLIENT] Socket creation failed." << std::endl;
        return 1;
    }

    std::cout << "[CLIENT] Socket created successfully." << std::endl;
    std::string serverIP;
    int serverPort;

    std::cout << "[CLIENT] Enter server IP: ";
    std::getline(std::cin, serverIP);

    std::cout << "[CLIENT] Enter server port: ";
    std::cin >> serverPort;
    std::cin.ignore();

    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP.c_str() , &serverAddress.sin_addr) <= 0) {
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

    size_t dotPosition = fileName.find_last_of('.');
    std::string fileType = "unknown";

    if (dotPosition != std::string::npos) {
    fileType = fileName.substr(dotPosition);
    }

std::cout << "[CLIENT] File type: " << fileType << std::endl;

    uint32_t checksum = calculateChecksum(fileName);
    std::cout << "[CLIENT] Checksum: " << checksum << std::endl;

    std::string fileInfo = encodeFileInfo(
        fileName,
        fileSize,
        checksum

    );
        

if (!sendAll(clientSocket, fileInfo.c_str(), fileInfo.size())) {        std::cerr << "[CLIENT] File information could not be sent." << std::endl;
        file.close();
        close(clientSocket);
        return 1;
    }

    std::cout << "[CLIENT] File information sent successfully." << std::endl;

    const int chunkSize = 1024;
    char buffer[chunkSize];
    long long totalBytesSent = 0;

    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int bytesToSend = file.gcount();

        if (!sendAll(clientSocket, buffer, bytesToSend)) {
            std::cerr << "[CLIENT] File transfer failed." << std::endl;
            file.close();
            close(clientSocket);
            return 1;
        }
        totalBytesSent += bytesToSend;

        int progress = static_cast<int>((totalBytesSent * 100) / fileSize);
        std::cout << "[CLIENT] Transfer progress: " << progress << "%" << std::endl;
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