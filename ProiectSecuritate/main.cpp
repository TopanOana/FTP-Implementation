#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include "CommandCenter.h"

#define DEFAULT_PORT "27015"

using namespace std;

vector<thread> allThreads;

void handlerForKill(int sig) {
    cout << "handling the kills" << endl;
    for (auto &item: allThreads) {
        item.join();
    }
    exit(0);
}

BOOL WINAPI consoleHandler(DWORD signal) {
    cout << "before handling the kills" << endl;
    if (signal == CTRL_C_EVENT) {
        cout << "handling the kills" << endl;
        for (auto &item: allThreads) {
            item.join();
        }
        WSACleanup();
        exit(0);
    }
    return true;
}

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


void workerThread(SOCKET ClientSocket) {
    cout << "connected to client!" << endl;
    ///SET AUTHENTICATED TO FALSE
    bool authenticated = false;

    ///where to set current user
    char current_user[10];

    ///WHERE TO READ CURRENT COMMAND
    char current_command[100];
    char current_command_word[100];
    char command_arguments[100];
    /*
     * FOR RECV
     *  WSAEMSGSIZE -> error
            The message was too large to fit into the specified buffer and was truncated.
     */

    size_t bufferSize;

    int iResult = receiveValue(ClientSocket, bufferSize, current_command);

    if (iResult <= 0) {
        return;
    }

    strcpy(current_command_word, getCommandWord(current_command).c_str());

    while (strcmp(current_command_word, "quit") != 0) {

        string useCommand = current_command;
        char return_val[100];
        strcpy(return_val, goToCommand(useCommand));

        size_t size = strlen(return_val);

        iResult = sendValue(ClientSocket, size, return_val);

        if (iResult <= 0) {
            return;
        }
        if (strcmp(current_command_word, "user") == 0) {
            strncpy(current_user, return_val, strlen(return_val));
        }

        strcpy(current_command_word, "quit");


    }

    cout << "end of client" << endl;

    return;
}


int main() {
    //POPULATE COMMANDS
    populateCommands();

    if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
        cout << "Console Ctrl Handler failed" << endl;
        return 1;
    }

    ///START UP
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartUp failed: " << iResult << endl;
        return 1;
    }

    ///CREATE SOCKET
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        return 1;
    }


    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        cout << "Error at socket() : " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }


    ///BIND SOCKET
    iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    ///LISTEN ON SOCKET

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    ///ACCEPT CONNECTION
    while (true) {
        cout << "waiting for clients..." << endl;
        SOCKET ClientSocket;
        ClientSocket = INVALID_SOCKET;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            cout << "Accept failed with error: " << WSAGetLastError() << endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        allThreads.emplace_back(thread(workerThread, ClientSocket));
    }


    std::cout << "Hello, World!" << std::endl;
    return 0;
}
