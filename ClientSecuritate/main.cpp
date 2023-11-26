#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <unistd.h>

#define DEFAULT_PORT "27015"

using namespace std;

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

    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
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

    char buffer[100] = "user oana";

    size_t size = htonl(strlen(buffer));

    iResult = send(ConnectSocket, (char*)size, sizeof(size_t), 0);
    if (iResult <= 0) {
        cout << "error on send: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }


    iResult = send(ConnectSocket, buffer, strlen(buffer) * sizeof(char), 0);
    if (iResult <= 0) {
        cout << "error on send: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    strcpy(buffer, "");

    iResult = recv(ConnectSocket, (char*)size, sizeof(size_t), 0);
    if (iResult <= 0) {
        cout << "error on recv: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    size = ntohl(size);

    iResult = recv(ConnectSocket, buffer, size * sizeof(char), 0);
    if (iResult <= 0) {
        cout << "error on recv: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    buffer[strlen(buffer)] = 0;

    cout << buffer << endl;

    sleep(1000);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
