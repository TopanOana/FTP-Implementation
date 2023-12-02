//
// Created by Oana on 12/2/2023.
//

#ifndef CLIENTSECURITATE_SOCKETFUNCTIONS_H
#define CLIENTSECURITATE_SOCKETFUNCTIONS_H

#include <winsock2.h>
#include <iostream>

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


#endif //CLIENTSECURITATE_SOCKETFUNCTIONS_H
