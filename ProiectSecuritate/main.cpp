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

void workerThread(SOCKET ClientSocket) {
    cout << "connected to client!" << endl;
    ///SET AUTHENTICATED TO FALSE
    bool authenticated = false;

    ///WHERE TO READ CURRENT COMMAND
    char current_command[100];
    /*
     * FOR RECV
     *  WSAEMSGSIZE -> error
            The message was too large to fit into the specified buffer and was truncated.
     */

    size_t bufferSize;

    int iResult = recv(ClientSocket, (char *) &bufferSize, sizeof(size_t), 0);
    if (iResult == 0) {
        cout << "CONNECTION CLOSED!" << endl;
        return;
    } else if (iResult < 0) {
        cout << "error has occurred: " << WSAGetLastError() << endl;
        return;
    }

    bufferSize = ntohl(bufferSize);


    iResult = recv(ClientSocket, current_command, bufferSize, 0);
    if (iResult == 0) {
        cout << "connection closed!" << endl;
        return;
    } else if (iResult < 0) {
        if (iResult == WSAEMSGSIZE) {
            cout << "a possible buffer overflow was attempted, the size of the command was truncated" << endl;
            strcpy(current_command, "");
        }
    }
    while (strcmp(current_command, "QUIT") != 0) {
        if (strcmp(current_command, "") == 0) {
            strcpy(current_command,
                   "please retry with a valid command that doesn\'t surpass the 100 character threshold");
            cout << "cmd surpassed limit and was truncated, not processed" << endl;
            size_t size = strlen(current_command);
            iResult = send(ClientSocket, (char *) size, sizeof(size_t), 0);
            if (iResult <= 0) {
                cout << "error has occurred: " << WSAGetLastError() << endl;
                return;
            }
            iResult = send(ClientSocket, current_command, size * sizeof(char), 0);
            if (iResult == SOCKET_ERROR) {
                cout << "send failed: " << WSAGetLastError() << endl;
//                closesocket()
            }
        } else {
            string useCommand = current_command;
            char return_val[100];
            strcpy(return_val, goToCommand(useCommand));

            size_t size = htonl(strlen(return_val));

            iResult = send(ClientSocket, (char*)size, sizeof(size_t), 0);

            if (iResult <= 0) {
                cout << "error on send: " << WSAGetLastError() << endl;
                closesocket(ClientSocket);
                return;
            }

            iResult = send(ClientSocket, return_val, size * sizeof(char) + 1, 0);
            if (iResult <= 0) {
                cout << "error on send: " << WSAGetLastError() << endl;
                closesocket(ClientSocket);
                return;
            }

            strcpy(current_command, "QUIT");

        }
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
