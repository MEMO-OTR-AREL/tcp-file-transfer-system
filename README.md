# TCP File Transfer System

This project is a TCP-based client-server file transfer system developed in C++ using socket programming.

The project demonstrates TCP communication, file transfer, data encoding and decoding, and checksum-based file integrity verification.

## Features

- TCP client-server communication
- Configurable server IP address and port
- File selection from the client
- File name and file size transfer
- File information encoding and decoding
- File transfer in 1024-byte chunks
- Checksum calculation and verification
- Transfer status (`SUCCESS` / `CHECKSUM_ERROR`)
- Received file storage on the server

## How It Works

1. The server starts and waits for a client connection on the selected port.
2. The client connects to the server using an IP address and port number through TCP.
3. The client selects a file and calculates its checksum.
4. The file name, file size, and checksum are encoded and sent to the server.
5. The file is transferred to the server in 1024-byte chunks.
6. The server decodes the file information, receives the file, and saves it in the `received_files` directory.
7. The server calculates the checksum of the received file.
8. The client and server checksum values are compared.
9. The server sends `SUCCESS` or `CHECKSUM_ERROR` to the client.

## Project Structure

```text id="dt13ga"
tcp-file-transfer-system/
├── src/
│   ├── client.cpp
│   └── server.cpp
├── received_files/
├── test.txt
├── test2.txt
├── .gitignore
└── README.md
```

## Compile

Compile the server:

```bash id="pc95wm"
clang++ -std=c++17 src/server.cpp -o server
```

Compile the client:

```bash id="al0njh"
clang++ -std=c++17 src/client.cpp -o client
```

## Run

Start the server:

```bash id="90s6yc"
./server
```

Enter a port number, for example:

```text id="9m91nw"
9090
```

Start the client in another terminal:

```bash id="3d31as"
./client
```

Example client inputs:

```text id="9t4ffm"
Server IP: 127.0.0.1
Server Port: 9090
File Name: test2.txt
```

## Error Control

The client calculates a checksum before sending the file. After receiving the file, the server calculates the checksum again.

If the client and server checksum values are equal, the file transfer is verified successfully. Otherwise, the server reports a checksum error.

## BER and SNR

Bit Error Rate (BER) and Signal-to-Noise Ratio (SNR) are considered in the theoretical analysis of the project to explain transmission errors and communication channel quality.

The implemented application uses TCP for reliable communication and checksum verification for file integrity.

## Technologies

- C++
- TCP/IP
- Socket Programming
- POSIX/Berkeley Sockets
- Visual Studio Code
- Git and GitHub