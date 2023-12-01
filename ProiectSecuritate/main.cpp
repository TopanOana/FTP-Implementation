#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include "CommandCenter.h"

#define DEFAULT_PORT "27015"
#define CMD_PORT "21"
#define ARGUMENT_ERROR "argument not accepted. cannot use forbidden characters: \% \;"

using namespace std;

char DATA_PORT[100] = "20";
char DATA_IP[100] = "127.0.0.1";

vector<thread> allThreads;

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

SOCKET CreateDataSocket() {
    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;


    int iResult = getaddrinfo(DATA_IP, DATA_PORT, &hints, &result);
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


    return DataSocket;
}


void workerThread(SOCKET ClientSocket) {
    cout << "connected to client!" << endl;
    ///SET AUTHENTICATED TO FALSE
    bool authenticated = false;

    ///where to set current user
    char current_user[10];

    ///where to set current directory
    char current_directory[500] = ".";

    ///WHERE TO READ CURRENT COMMAND
    char current_command[100];
    char current_command_word[100];
    char command_arguments[100];


    ///mode set
    /// 0 not chosen, 1 active, 2 passive
    int mode = 0;
    /*
     * FOR RECV
     *  WSAEMSGSIZE -> error
            The message was too large to fit into the specified buffer and was truncated.
     */

    int i = 0;
    size_t bufferSize;

    int iResult;



    //AUTHENTICATION

    while (!authenticated) {
        iResult = receiveValue(ClientSocket, bufferSize, current_command);

        if (iResult <= 0) {
            return;
        }

        strcpy(current_command_word, getCommandWord(current_command).c_str());

        char return_val[1024];
        string arguments;


        if (strcmp(current_command, current_command_word) != 0)
            arguments = getCommandArguments(current_command);


        if (strcmp(current_command_word, USER_COMMAND) == 0) {
            if (arguments.empty()) {
                strcpy(return_val, ARGUMENT_ERROR);
            } else if (strcmp(current_user, "") == 0) {
                strcpy(return_val, userCommand(arguments));
                if (strcmp(return_val, "false") == 0) {
                    cout << "user does not exist!" << endl;
                    strcpy(return_val, "user does not exist!");
                } else {
                    strcpy(current_user, return_val);
                }
            } else {
                cout << "user already logged in!";
                strcpy(return_val, "user already logged in!");
            }
        } else if (strcmp(current_command_word, PASS_COMMAND) == 0) {
            if (arguments.empty()) {
                strcpy(return_val, ARGUMENT_ERROR);
            } else if (strcmp(current_user, "") == 0) {
                strcpy(return_val, "user hasn't been saved!");
            } else {
                strcpy(return_val, passCommand(current_user, arguments));
                if (strcmp(return_val, "false") == 0) {
                    cout << "user and password do not match!" << endl;
                    strcpy(return_val, "user and password do not match!");
                } else {
                    authenticated = true;
                }
            }
        } else {
            strcpy(return_val, "command not accepted for authentication!");
        }

        size_t size = strlen(return_val);

        iResult = sendValue(ClientSocket, size, return_val);

        if (iResult <= 0) {
            return;
        }

    }

    while (mode == 0) {
        iResult = receiveValue(ClientSocket, bufferSize, current_command);

        if (iResult <= 0) {
            return;
        }

        strcpy(current_command_word, getCommandWord(current_command).c_str());

        char toSend[100] = "";

        if (strcmp(current_command_word, "port") == 0) {
            strcpy(command_arguments, getCommandArguments(current_command).c_str());

            size_t lengthOfArgs = strlen(command_arguments);
            char newAddress[100] = "";
            int index = 0;
            char *p = strtok(command_arguments, ","); //primul strtok
            strcat(newAddress, p);
            strcat(newAddress, ".");
            p = strtok(NULL, ","); //al doilea strtok
            strcat(newAddress, p);
            strcat(newAddress, ".");
            p = strtok(NULL, ","); //al treilea strtok
            strcat(newAddress, p);
            strcat(newAddress, ".");
            p = strtok(NULL, ","); //al patrulea strtok
            strcat(newAddress, p);

            strcpy(DATA_IP, newAddress);


            p = strtok(NULL, ",");
            int p1 = atoi(p);
            p = strtok(NULL, ",");
            int p2 = atoi(p);
            int finalPort = (p1 * 256) + p2;
            strcpy(DATA_PORT, to_string(finalPort).c_str());


            strcpy(toSend, "ack");

            iResult = sendValue(ClientSocket, strlen(toSend), toSend);
            if (iResult <= 0)
                return;

            mode = 1;


        } else if (strcmp(current_command_word, "pasv") == 0) {

            mode = 2;

        } else {

            strcpy(toSend, "To start the server, use port or pasv to determine the mode");
            iResult = sendValue(ClientSocket, strlen(toSend), toSend);
            if (iResult <= 0)
                return;
        }
    }


    iResult = receiveValue(ClientSocket, bufferSize, current_command);

    if (iResult <= 0) {
        return;
    }

    strcpy(current_command_word, getCommandWord(current_command).c_str());

    while (strcmp(current_command_word, "quit") != 0) {

        SOCKET DataSocket = CreateDataSocket();

        char return_val[1024];
        string arguments;

        if (strcmp(current_command, current_command_word) != 0)
            arguments = getCommandArguments(current_command);


        if (strcmp(current_command_word, LIST_COMMAND) == 0) {

            if (strcmp(current_command, current_command_word) != 0 && arguments.empty()) {
                strcpy(return_val, ARGUMENT_ERROR);
            } else {
                strcpy(return_val, "");
                if (arguments.empty())
                    arguments = current_directory;
                listCommand(return_val, 1024, arguments);
            }

        }

        if (strcmp(current_command_word, CWD_COMMAND) == 0) {

            if (arguments.empty()) {
                if (strcmp(current_command, current_command_word) == 0) {
                    strcpy(return_val, "an path must be provided");
                } else {
                    strcpy(return_val, ARGUMENT_ERROR);
                }
            } else {
                char auxiliary[500];
                strcpy(auxiliary, cwdCommand(arguments));
                if (strcmp(auxiliary, "false") == 0) {
                    strcpy(return_val, "File path doesn't exist or is inaccessible.");
                } else {
                    strcpy(current_directory, auxiliary);
                }
            }

        }


        size_t size = strlen(return_val);

        iResult = sendValue(DataSocket, size, return_val);

        if (iResult <= 0) {
            return;
        }

        closesocket(DataSocket);

        iResult = receiveValue(ClientSocket, size, current_command);

        if (iResult <= 0) {
            return;
        }

        strcpy(current_command_word, getCommandWord(current_command).c_str());

        i++;
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

    iResult = getaddrinfo(NULL, CMD_PORT, &hints, &result);
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
