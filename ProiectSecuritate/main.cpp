#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <regex>
#include "CommandCenter.h"
#include "Modes.h"

#define DEFAULT_PORT "27015"
#define CMD_PORT "21"
#define ARGUMENT_ERROR "501 Syntax error in parameters or arguments"

using namespace std;


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


void workerThread(SOCKET ClientSocket) {
    cout << "connected to client!" << endl;
    ///SET AUTHENTICATED TO FALSE
    bool authenticated = false;

    ///where to set current user
    char current_user[10];

    ///where to set current directory
    char current_directory[1024] = ".";

    ///WHERE TO READ CURRENT COMMAND
    char current_command[1024];
    char current_command_word[1024];
    char command_arguments[1024];


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

    int counter = 0;

    while (!authenticated && counter < 3) {

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
                counter++;
            } else if (strcmp(current_user, "") == 0) {
                strcpy(return_val, userCommand(arguments));
                if (strcmp(return_val, "false") == 0) {
                    cout << "user does not exist!" << endl;
                    strcpy(return_val, "430 Invalid username or password.");
                    counter++;
                } else {
                    strcpy(current_user, return_val);
                    strcpy(return_val, "331 User name okay, need password.");
                    cout << return_val << endl;
                }
            } else {
                cout << "user already logged in!";
                strcpy(return_val, "331 User name okay, need password.");
            }
        } else if (strcmp(current_command_word, PASS_COMMAND) == 0) {
            if (arguments.empty()) {
                strcpy(return_val, ARGUMENT_ERROR);
                counter++;
            } else if (strcmp(current_user, "") == 0) {
                strcpy(return_val, "332 Need account for login");
            } else {
                strcpy(return_val, passCommand(current_user, arguments));
                if (strcmp(return_val, "false") == 0) {
                    cout << "user and password do not match!" << endl;
                    strcpy(return_val, "430 Invalid username or password.");
                    counter++;
                } else {
                    authenticated = true;
                    strcpy(return_val, "230 Logged in");
                    cout << return_val << endl;
                }
            }
        } else {
            strcpy(return_val, "530 Not logged in");
            counter++;
            cout << return_val << endl;
        }

        size_t size = strlen(return_val);

        iResult = sendValue(ClientSocket, size, return_val);

        if (iResult <= 0) {
            return;
        }

    }

    if (!authenticated && counter >= 3) {

        closesocket(ClientSocket);
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

            if (strlen(command_arguments) == 0 || !regex_match(command_arguments,
                                                               regex("^(([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5]),){5}([0-9]{1,2}|1[0-9]{2}|2[0-4][0-9]|25[0-5])$"))) {
                strcpy(toSend, ARGUMENT_ERROR);
            } else {

                portCommand(command_arguments);
                strcpy(toSend, "200 Command okay.");
                mode = 1;
            }

            if (strlen(toSend) > 0) {
                iResult = sendValue(ClientSocket, strlen(toSend), toSend);
                if (iResult <= 0) {
                    closesocket(ClientSocket);
                    pthread_exit(nullptr);
                }
            }


        } else if (strcmp(current_command_word, "pasv") == 0) {
            pasvCommand(ClientSocket);
            mode = 2;

        } else {

            strcpy(toSend, "503 Bad sequence of commands");
            iResult = sendValue(ClientSocket, strlen(toSend), toSend);
            if (iResult <= 0) {
                closesocket(ClientSocket);
                pthread_exit(nullptr);
            }
        }
    }


    iResult = receiveValue(ClientSocket, bufferSize, current_command);

    if (iResult <= 0) {
        closesocket(ClientSocket);
        pthread_exit(nullptr);
    }

    strcpy(current_command_word, getCommandWord(current_command).c_str());

    while (strcmp(current_command_word, "quit") != 0) {


        char return_val[1024];
        strcpy(return_val, "");
        string arguments;

        if (strcmp(current_command, current_command_word) != 0)
            arguments = getCommandArguments(current_command);


        if (strcmp(current_command_word, LIST_COMMAND) == 0) {
            cout << "i am in list" << endl;
            SOCKET DataSocket;
            if (mode == 1)
                DataSocket = CreateDataSocketActiveMode(ClientSocket);
            else {
                DataSocket = CreateDataSocketPassiveMode(ClientSocket);
            }

            if (strcmp(current_command, current_command_word) != 0 && arguments.empty()) {
                strcpy(return_val, ARGUMENT_ERROR);
            } else {
                strcpy(return_val, "");
                char directory[500];
                if (arguments.empty()) {
                    strcpy(directory, current_directory);
                } else {
                    strcpy(directory, arguments.c_str());
                }

                listCommand(DataSocket, directory, return_val);

                if (strlen(return_val) == 0){
                    strcpy(return_val, "200 Command okay.");
                }

                size_t size = strlen(return_val);

                iResult = sendValue(ClientSocket, size, return_val);

                if (iResult <= 0) {
                    return;
                }
                closesocket(DataSocket);
            }

        }

        if (strcmp(current_command_word, CWD_COMMAND) == 0) {

            if (arguments.empty()) {

                strcpy(return_val, ARGUMENT_ERROR);

            } else {
                char auxiliary[500];
                cwdCommand(arguments, auxiliary);
                if (strcmp(auxiliary, "false") == 0) {
                    strcpy(return_val, "550 Requested action not taken. File unavailable.");
                } else {
                    strcpy(current_directory, auxiliary);
                    strcpy(return_val, "250 The current directory has been changed to ");
                    strcat(return_val, current_directory);
                }

                size_t size = strlen(return_val);

                iResult = sendValue(ClientSocket, size, return_val);

                if (iResult <= 0) {
                    return;
                }

            }

        }

        if (strcmp(current_command_word, RETR_COMMAND) == 0) {
            SOCKET DataSocket;
            if (mode == 1)
                DataSocket = CreateDataSocketActiveMode(ClientSocket);
            else {
                DataSocket = CreateDataSocketPassiveMode(ClientSocket);
            }
            if (arguments.empty()) {
                closesocket(DataSocket);
                strcpy(return_val, ARGUMENT_ERROR);

                size_t size = strlen(return_val);

                iResult = sendValue(ClientSocket, size, return_val);

                if (iResult <= 0) {
                    return;
                }
            } else {
                retrCommand(DataSocket, arguments, current_directory, ClientSocket);
                closesocket(DataSocket);
            }
        }

        if (strcmp(current_command_word, STOR_COMMAND) == 0) {
            SOCKET DataSocket;
            if (mode == 1)
                DataSocket = CreateDataSocketActiveMode(ClientSocket);
            else {
                DataSocket = CreateDataSocketPassiveMode(ClientSocket);
            }
            if (arguments.empty()) {
                closesocket(DataSocket);
                strcpy(return_val, ARGUMENT_ERROR);

                size_t size = strlen(return_val);

                iResult = sendValue(ClientSocket, size, return_val);

                if (iResult <= 0) {
                    return;
                }
            } else {

                storCommand(DataSocket, arguments.c_str(), current_directory);
                closesocket(DataSocket);
                strcpy(return_val, "226 File transfer successful. Closing data connection");

                size_t size = strlen(return_val);

                iResult = sendValue(ClientSocket, size, return_val);

                if (iResult <= 0) {
                    return;
                }
            }
        }


        size_t size;

        iResult = receiveValue(ClientSocket, size, current_command);

        if (iResult <= 0) {
            return;
        }

        strcpy(current_command_word, getCommandWord(current_command).c_str());


    }

    cout << "end of client" << endl;

    char value[100] = "221 Service closing control connection";
    sendValue(ClientSocket, strlen(value), value);

    closesocket(ClientSocket);

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
