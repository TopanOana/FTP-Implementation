//
// Created by Oana on 12/1/2023.
//

#ifndef PROIECTSECURITATE_SOCKETFUNCTIONS_H
#define PROIECTSECURITATE_SOCKETFUNCTIONS_H

#include <winsock.h>
#include <iostream>

using namespace std;

int sendValue(SOCKET ClientSocket, size_t length, char *value) {
    size_t copyLength = htonl(length);

    int result = send(ClientSocket, (char *) &copyLength, sizeof(size_t), 0);

    if (result <= 0) {
        cout << "Error occurred on sending size of buffer: " << WSAGetLastError() << endl;
        return result;
    }

    result = send(ClientSocket, value, sizeof(char) * length, 0);

    if (result <= 0) {
        cout << "Error occurred on sending buffer: " << WSAGetLastError() << endl;
        return result;
    }

    return result;

}

int receiveValue(SOCKET ClientSocket, size_t &length, char *value) {
    int result = recv(ClientSocket, (char *) &length, sizeof(size_t), 0);

    if (result <= 0) {
        cout << "Error occured on reading size of buffer: " << WSAGetLastError() << endl;
        return result;
    }

    length = ntohl(length);

    result = recv(ClientSocket, value, sizeof(char) * length, 0);

    if (result <= 0) {
        cout << "Error occured on reading buffer: " << WSAGetLastError() << endl;
        return result;
    }

    value[length] = 0;
    return result;
}



#endif //PROIECTSECURITATE_SOCKETFUNCTIONS_H
