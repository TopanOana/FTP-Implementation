//
// Created by Oana on 12/3/2023.
//

#ifndef PROIECTSECURITATE_MODES_H
#define PROIECTSECURITATE_MODES_H


#include <winsock.h>
#include <iostream>
#include <ws2tcpip.h>
#include "SocketFunctions.h"

using namespace std;

char DATA_PORT[100] = "2000";
char DATA_IP[100] = "127.0.0.1";
int currentCommand = 1;

SOCKET CreateDataSocketActiveMode(SOCKET ClientSocket) {
    char portBuffer[10];
    size_t size;

    int iResult = receiveValue(ClientSocket, size, portBuffer);
    if (iResult <= 0) {
        cout << "receiving port failed " << iResult << endl;
        pthread_exit(nullptr);
    }

    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char port[10];
    strcpy(port, portBuffer);


    iResult = getaddrinfo(DATA_IP, port, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
//        return 1;
        pthread_exit(nullptr);
    }

    SOCKET DataSocket = INVALID_SOCKET;
    ptr = result;

    DataSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (DataSocket == INVALID_SOCKET) {
        cout << "Error at socket() : " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        pthread_exit(nullptr);
//        return 1;
    }


    //CONNECT TO A SOCKET
    iResult = connect(DataSocket, ptr->ai_addr, (int) ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Error on connect(): " << WSAGetLastError() << endl;
        closesocket(DataSocket);
        DataSocket = INVALID_SOCKET;
        pthread_exit(nullptr);
    }


    freeaddrinfo(result);

    if (DataSocket == INVALID_SOCKET) {
        cout << "Unable to connect to server" << endl;
        WSACleanup();
        pthread_exit(nullptr);
    }

    currentCommand++;

    return DataSocket;
}

SOCKET CreateDataSocketPassiveMode(SOCKET ClientSocket) {
    //will listen on port
    char portBuffer[10];
    strcpy(portBuffer, to_string(atoi(DATA_PORT) + currentCommand).c_str());

    int iResult = sendValue(ClientSocket, strlen(portBuffer), portBuffer);
    if (iResult <= 0) {
        cout << "sending port failed " << iResult << endl;
        pthread_exit(nullptr);
    }

    ///CREATE SOCKET
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(DATA_IP, portBuffer, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        pthread_exit(nullptr);
    }


    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        cout << "Error at socket() : " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        pthread_exit(nullptr);
    }


    ///BIND SOCKET
    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        pthread_exit(nullptr);
    }

    freeaddrinfo(result);

    ///LISTEN ON SOCKET

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        pthread_exit(nullptr);
    }

    ///ACCEPT CONNECTION

    SOCKET DataSocket = INVALID_SOCKET;
    DataSocket = accept(ListenSocket, NULL, NULL);
    if (DataSocket == INVALID_SOCKET) {
        cout << "Accept failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        pthread_exit(nullptr);
    }

    currentCommand++;

    return DataSocket;

}


#endif //PROIECTSECURITATE_MODES_H
