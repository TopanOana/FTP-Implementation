#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <unistd.h>

#define DEFAULT_PORT "27015"

using namespace std;

int sendValue(SOCKET ConnectSocket, size_t length, char *value) {
    size_t copyLength = htonl(length);

    int result = send(ConnectSocket, (char *) &copyLength, sizeof(size_t), 0);

    if (result <= 0) {
        cout << "Error occurred on sending size of buffer: " << WSAGetLastError() << endl;
        return result;
    }

    result = send(ConnectSocket, value, sizeof(char) * length, 0);

    if (result <= 0) {
        cout << "Error occurred on sending buffer: " << WSAGetLastError() << endl;
        return result;
    }

    return result;

}

int receiveValue(SOCKET ConnectSocket, size_t &length, char *value) {
    int result = recv(ConnectSocket, (char *) &length, sizeof(size_t), 0);

    if (result <= 0) {
        cout << "Error occured on reading size of buffer: " << WSAGetLastError() << endl;
        return result;
    }

    length = ntohl(length);

    result = recv(ConnectSocket, value, sizeof(char) * length, 0);

    if (result <= 0) {
        cout << "Error occured on reading buffer: " << WSAGetLastError() << endl;
        return result;
    }

    value[length] = 0;
    return result;
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
    iResult = receiveValue(ConnectSocket, size, buffer);
    if (iResult <= 0) {
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    cout << buffer << endl;
//list
    iResult = receiveValue(ConnectSocket, size, buffer);
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
