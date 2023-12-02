#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <unistd.h>
#include <fcntl.h>
#include "SocketFunctions.h"

#define CMD_PORT "21"

using namespace std;

char DATA_IP[100] = "127.0.0.1";
char DATA_PORT[100] = "1285";
int currentCommand = 1;


SOCKET CreateDataSocketActiveMode() {
    ///CREATE SOCKET
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port[10];
    strcpy(port, to_string(atoi(DATA_PORT) + currentCommand).c_str());

    int iResult = getaddrinfo(DATA_IP, port, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        exit(1);
    }


    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        cout << "Error at socket() : " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        exit(1);
    }


    ///BIND SOCKET
    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    freeaddrinfo(result);

    ///LISTEN ON SOCKET

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    ///ACCEPT CONNECTION

    SOCKET DataSocket = INVALID_SOCKET;
    DataSocket = accept(ListenSocket, NULL, NULL);
    if (DataSocket == INVALID_SOCKET) {
        cout << "Accept failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        exit(1);
    }

    currentCommand++;

    return DataSocket;
}


int main(int argc, char **argv) {
    ///START UP
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartUp failed: " << iResult << endl;
        return 1;
    }

    ///CREATE SOCKET
    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(argv[1], CMD_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    ptr = result;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Error at socket() : " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }


    //CONNECT TO A SOCKET
    iResult = connect(ConnectSocket, ptr->ai_addr, (int) ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Error on connect(): " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }


    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Unable to connect to server" << endl;
        WSACleanup();
        return 1;
    }


    cout << "connected to server" << endl;

    char buffer[1024] = "user oana";

    size_t size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    strcpy(buffer, "");
//user
    iResult = receiveValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    cout << buffer << endl;
    //pass

    strcpy(buffer, "");
    strcpy(buffer, "pass pass");
    size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    iResult = receiveValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    cout << buffer << endl;

    ///port
    strcpy(buffer, "");
    strcpy(buffer, "port 127,0,0,1,5,5");
    size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    iResult = receiveValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    cout << buffer << endl;



    //list

    strcpy(buffer, "");
    strcpy(buffer, "list");
    size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    SOCKET DataSocket = CreateDataSocketActiveMode();

    iResult = receiveValue(DataSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(DataSocket);
    cout << buffer << endl;


    strcpy(buffer, "");
    strcpy(buffer, "cwd ..");
    size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    DataSocket = CreateDataSocketActiveMode();

    iResult = receiveValue(DataSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        closesocket(DataSocket);
        WSACleanup();
        return 1;
    }

    closesocket(DataSocket);
    cout << buffer << endl;


    strcpy(buffer, "");
    strcpy(buffer, "list");
    size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    DataSocket = CreateDataSocketActiveMode();

    iResult = receiveValue(DataSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(DataSocket);
    cout << buffer << endl;

    //retr
    strcpy(buffer, "");
    strcpy(buffer, "retr CMakeLists.txt");

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    DataSocket = CreateDataSocketActiveMode();

    char buf[500] = "";
    size_t length;
    iResult = receiveValue(DataSocket, length, buf);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        cout << "error receivijng file ok" << endl;
        WSACleanup();
        return 1;
    }

    if (!(strcmp(buf, "ok") == 0)) {
        closesocket(ConnectSocket);
        cout << "error file cannot be used" << endl;
        WSACleanup();
        return 1;
    }

    iResult = receiveValue(DataSocket, length, buf);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        cout << "error receivijng file name" << endl;
        WSACleanup();
        return 1;
    }

    cout << buf << endl;

    char filepath[500];
    strcpy(filepath, "D:\\UniversityWork\\SecuritateFTP\\");
    strcat(filepath, buf);



    size_t filesize;
    iResult = recv(DataSocket, (char *) &filesize, sizeof(size_t), 0);

    if (iResult <= 0) {
        closesocket(ConnectSocket);
        cout << "error receivijng file size" << endl;
        WSACleanup();
        return 1;
    }

    filesize = ntohl(filesize);
    int fileDescriptor = open(filepath, O_WRONLY | O_CREAT);
    if (fileDescriptor > 0){
        char filebuf[100];
        int count = 0, k = 0;
        size_t aux = 0;
        while (count < filesize && (k = receiveValue(DataSocket, aux, filebuf)) > 0) {
            count += aux;
            int check = write(fileDescriptor, filebuf, aux);
            cout << filebuf;
        }

        close(fileDescriptor);
    }





    //quit

    strcpy(buffer, "");
    strcpy(buffer, "quit");
    size = strlen(buffer);

    iResult = sendValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }


    cout << buffer << endl;

    sleep(1000);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
