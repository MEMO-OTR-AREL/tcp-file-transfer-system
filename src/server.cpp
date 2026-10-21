#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
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

void decodeFileInfo(
    const std::string& fileInfo,
    std::string& fileName,
    long long& fileSize,
    uint32_t& checksum
) {
    size_t firstNewLine = fileInfo.find('\n');
    size_t secondNewLine = fileInfo.find('\n', firstNewLine + 1);
    size_t thirdNewLine = fileInfo.find('\n', secondNewLine + 1);

    fileName = fileInfo.substr(0, firstNewLine);

    fileSize = std::stoll(
        fileInfo.substr(
            firstNewLine + 1,
            secondNewLine - firstNewLine - 1
        )
    );

    checksum = std::stoul(
        fileInfo.substr(
            secondNewLine + 1,
            thirdNewLine - secondNewLine - 1
        )
    );
}

int main() {
    std::cout << "[SERVER] Program started." << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        std::cerr << "[SERVER] Socket creation failed." << std::endl;
        return 1;
    }

    std::cout << "[SERVER] Socket created successfully." << std::endl;

    sockaddr_in serverAddress{};

    int serverPort;

    std::cout << "[SERVER] Enter port: ";
    std::cin >> serverPort;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(
        serverSocket,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == -1) {
        std::cerr << "[SERVER] Bind failed." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout
        << "[SERVER] Socket bound to port "
        << serverPort
        << "."
        << std::endl;

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "[SERVER] Listen failed." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "[SERVER] Waiting for a client connection..." << std::endl;

    int clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == -1) {
        std::cerr << "[SERVER] Client connection failed." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "[SERVER] Client connected successfully." << std::endl;

    std::string fileInfo;
    char infoBuffer[1];

    while (true) {
        int bytesReceived = recv(clientSocket, infoBuffer, 1, 0);

        if (bytesReceived <= 0) {
            std::cerr
                << "[SERVER] File information could not be received."
                << std::endl;

            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        fileInfo += infoBuffer[0];

        int newLineCount = 0;

        for (char c : fileInfo) {
            if (c == '\n') {
                newLineCount++;
            }
        }

        if (newLineCount == 3) {
            break;
        }
    }

    std::string fileName;
    long long fileSize;
    uint32_t clientChecksum;

    decodeFileInfo(
        fileInfo,
        fileName,
        fileSize,
        clientChecksum
    );

    std::cout << "[SERVER] File name: " << fileName << std::endl;
    std::cout << "[SERVER] File size: " << fileSize << " bytes" << std::endl;
    std::cout << "[SERVER] Client checksum: " << clientChecksum << std::endl;

    std::string savePath = "received_files/" + fileName;

    std::ofstream outputFile(savePath, std::ios::binary);

    if (!outputFile.is_open()) {
        std::cerr << "[SERVER] File could not be created." << std::endl;
        close(clientSocket);
        close(serverSocket);
        return 1;
    }

    char buffer[1024];
    long long totalBytesReceived = 0;

    while (totalBytesReceived < fileSize) {
        long long remainingBytes = fileSize - totalBytesReceived;

        int bytesToReceive;

        if (remainingBytes < sizeof(buffer)) {
            bytesToReceive = static_cast<int>(remainingBytes);
        } else {
            bytesToReceive = sizeof(buffer);
        }

        int bytesReceived = recv(
            clientSocket,
            buffer,
            bytesToReceive,
            0
        );

        if (bytesReceived <= 0) {
            std::cerr << "[SERVER] File transfer failed." << std::endl;

            outputFile.close();
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        outputFile.write(buffer, bytesReceived);
        totalBytesReceived += bytesReceived;
    }

    outputFile.close();

    std::cout << "[SERVER] File received successfully." << std::endl;
    std::cout << "[SERVER] Saved to: " << savePath << std::endl;

    uint32_t serverChecksum = calculateChecksum(savePath);

    std::cout << "[SERVER] Server checksum: " << serverChecksum << std::endl;

    if (clientChecksum == serverChecksum) {
        std::cout
            << "[SERVER] Checksum verified successfully."
            << std::endl;

        std::string result = "SUCCESS";

        sendAll(
            clientSocket,
            result.c_str(),
            result.size()
        );
    } else {
        std::cout
            << "[SERVER] Checksum verification failed."
            << std::endl;

        std::string result = "CHECKSUM_ERROR";

        sendAll(
            clientSocket,
            result.c_str(),
            result.size()
        );
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}