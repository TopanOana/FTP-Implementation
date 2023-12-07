//
// Created by Oana on 12/1/2023.
//

#ifndef PROIECTSECURITATE_SOCKETFUNCTIONS_H
#define PROIECTSECURITATE_SOCKETFUNCTIONS_H

#include <winsock.h>
#include <iostream>

using namespace std;

int sendValue(SOCKET ClientSocket, size_t length, char *value) {
    strncat(value, "\n", 1024 - strlen(value) - 1);
    value[1023] = 0;
    value[1022] = '\n';
    length = strlen(value);

//
//    size_t copyLength = htonl(length);
//
//
//    int result = send(ClientSocket, (char *) &copyLength, sizeof(size_t), 0);

//    if (result <= 0) {
//        cout << "Error occurred on sending size of buffer: " << WSAGetLastError() << endl;
//        return result;
//    }

    int result = send(ClientSocket, value, sizeof(char) * length, 0);

    if (result <= 0) {
        cout << "Error occurred on sending buffer: " << WSAGetLastError() << endl;
        return result;
    }

    return result;

}

int receiveValue(SOCKET ClientSocket, size_t &length, char *value) {

    int i = 0, max = 1024;
    int k = 0, c;
    while (((c = recv(ClientSocket, value + k, sizeof(char), 0)) > 0) && k < max) {
        if (value[k] == '\n') {
            value[k] = 0;
            break;
        }
        k++;
        i++;
    }
    if (c < 0)
        return c;
    if (i == max) {
        value[1022] = '\n';
        value[1023] = 0;
    }

    return 1;
//
//    int result = recv(ClientSocket, (char *) &length, sizeof(size_t), 0);
//
//    if (result <= 0) {
//        cout << "Error occured on reading size of buffer: " << WSAGetLastError() << endl;
//        return result;
//    }
//
//    length = ntohl(length);
//
//    result = recv(ClientSocket, value, sizeof(char) * length, 0);
//
//    if (result <= 0) {
//        cout << "Error occured on reading buffer: " << WSAGetLastError() << endl;
//        return result;
//    }
//
//    value[length] = 0;
//    return result;
}


#endif //PROIECTSECURITATE_SOCKETFUNCTIONS_H
