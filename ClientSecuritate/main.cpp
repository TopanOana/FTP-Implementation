#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <unistd.h>
#include <fcntl.h>
#include "SocketFunctions.h"
#include <shlwapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include "Modes.h"

#define CMD_PORT "21"
#define WHITESPACE " \n\r\t\f\v"

using namespace std;

bool checkCode(char *buffer, int position, char code) {
    return (buffer[position] == code);
}

string left_trim(string value) {
    size_t start = value.find_first_not_of(WHITESPACE);
    return (start == string::npos) ? "" : value.substr(start);
}

string right_trim(string value) {
    size_t end = value.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : value.substr(0, end + 1);
}

string getCommandWord(string command) {
    command = left_trim(command);
    string cmd = command.substr(0, command.find(' '));
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    return cmd;
}

string getCommandArguments(string command) {
    string arguments = right_trim(left_trim(command.substr(command.find(' '))));
    if (std::find(arguments.begin(), arguments.end(), '%') != arguments.end() ||
        std::find(arguments.begin(), arguments.end(), ';') != arguments.end())
        return "";
    return arguments;
}


SOCKET startup(char *here) {
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

    iResult = getaddrinfo(here, CMD_PORT, &hints, &result);
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
    return ConnectSocket;
}

int main(int argc, char **argv) {

    SOCKET ConnectSocket = startup(argv[1]);

    cout << "connected to server" << endl;
    int iResult;

    bool authenticated = false;
    int counter = 0;

    while (!authenticated && counter < 3) {
        char command[1000], aux[100], buffer[100];
        strcpy(command, "user ");
        cout << "user: " << endl;
        cin.getline(aux, 100);
        strncat(command, aux, 1000 - strlen(command) - 1);
        size_t size = strlen(command);

        iResult = sendValue(ConnectSocket, size, command);
        if (iResult <= 0) {
            closesocket(ConnectSocket);
            WSACleanup();
            exit(1);
        }

        iResult = receiveValue(ConnectSocket, size, buffer);
        if (iResult <= 0) {
            closesocket(ConnectSocket);
            WSACleanup();
            exit(1);
        }
        cout << buffer << endl;

        if (!checkCode(buffer, 0, '4') && !checkCode(buffer, 0, '5')) {
            cout << endl << "password: " << endl;
            memset(aux, 0, 100);
            cin.getline(aux, 100);
            strcpy(command, "pass ");
            strncat(command, aux, 1000 - strlen(command) - 1);
            size = strlen(command);

            iResult = sendValue(ConnectSocket, size, command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            iResult = receiveValue(ConnectSocket, size, buffer);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }
            cout << buffer << endl;

            if (!checkCode(buffer, 0, '4') && !checkCode(buffer, 0, '5')) {
                authenticated = true;
                break;
            } else {
//                cout << buffer << endl;
                counter++;
            }

        } else {
//            cout << buffer << endl;
            counter++;
        }
    }

    if (counter == 3) {
        closesocket(ConnectSocket);
        WSACleanup();
        exit(1);
    }

    int mode = 0; //0 unchosen, 1 active, 2 passive

    while (mode == 0) {
        cout << "choose command port or pasv" << endl;
        char command[1000], aux[100];
        cin.getline(aux, 100);

        string forTrimming = aux;
        strcpy(aux, right_trim(left_trim(forTrimming)).c_str());

        if (strcmpi("port", aux) == 0) {
            strcpy(command, "port ");

            char auxiliary[100];
            strcpy(auxiliary, DATA_IP);

            char *p = strtok(auxiliary, ".");
            strcat(command, p);
            strcat(command, ",");
            p = strtok(NULL, ".");
            strcat(command, p);
            strcat(command, ",");
            p = strtok(NULL, ".");
            strcat(command, p);
            strcat(command, ",");
            p = strtok(NULL, ".");
            strcat(command, p);
            strcat(command, ",");

            int p1 = atoi(DATA_PORT) / 256;
            int p2 = atoi(DATA_PORT) % 256;

            strcat(command, to_string(p1).c_str());
            strcat(command, ",");
            strcat(command, to_string(p2).c_str());

            iResult = sendValue(ConnectSocket, strlen(command), command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            char buffer[500];
            size_t size;
            iResult = receiveValue(ConnectSocket, size, buffer);

            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }
            cout << buffer << endl;

            if (checkCode(buffer, 0, '2')) {
                mode = 1;
                break;
            } else {
                cout << buffer << endl;
            }

        } else if (strcmpi("pasv", aux) == 0) {
            strcpy(command, "pasv");

            iResult = sendValue(ConnectSocket, strlen(command), command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            char buffer[500];
            size_t size;
            iResult = receiveValue(ConnectSocket, size, buffer);

            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            cout << buffer << endl;

            if (checkCode(buffer, 0, '2')) {
                mode = 2;
                char *p = strchr(buffer, '(') + 1;
                p = strtok(p, ","); //H1
                strcpy(DATA_IP, p);
                strcat(DATA_IP, ".");
                p = strtok(NULL, ",");//H2
                strcat(DATA_IP, p);
                strcat(DATA_IP, ".");
                p = strtok(NULL, ",");//H3
                strcat(DATA_IP, p);
                strcat(DATA_IP, ".");
                p = strtok(NULL, ",");//H4
                strcat(DATA_IP, p);
                strcat(DATA_IP, ".");
                p = strtok(NULL, ",");//p1
                int p1 = atoi(p);
                p = strtok(NULL, ",");//p2
                int p2 = atoi(p);
                int finalport = 256 * p1 + p2;
                strcpy(DATA_PORT, to_string(finalport).c_str());
                break;
            } else {
                cout << buffer << endl;
            }
        }
    }

    while (true) {
        char command[1000];
        cout << "command: " << endl;
        cin.getline(command, 1000);
        string current_command = command;
        string cmd_word = getCommandWord(current_command);
        if (strcmpi(cmd_word.c_str(), "CWD") == 0) {

            iResult = sendValue(ConnectSocket, strlen(command), command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            char buffer[500];
            size_t size;

            iResult = receiveValue(ConnectSocket, size, buffer);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            cout << buffer << endl;

        } else if (strcmpi(cmd_word.c_str(), "LIST") == 0) {

            iResult = sendValue(ConnectSocket, strlen(command), command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            SOCKET DataSocket;
            if (mode == 1)
                DataSocket = CreateDataSocketActiveMode(ConnectSocket);
            else
                DataSocket = CreateDataSocketPassiveMode(ConnectSocket);

            char buffer[500];
            size_t size;
            iResult = receiveValue(ConnectSocket, size, buffer);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            if (checkCode(buffer, 0, '2')) {
                memset(buffer, 0, 500);
                int k;
                while ((k = recv(DataSocket, buffer, 100 * sizeof(char), 0) )> 0) {
                    buffer[k] = 0;
                    cout << buffer;
                }

            } else {
                cout << buffer << endl;
            }
            closesocket(DataSocket);

        } else if (strcmpi(cmd_word.c_str(), "RETR") == 0) {
            iResult = sendValue(ConnectSocket, strlen(command), command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            SOCKET DataSocket;
            if (mode == 1)
                DataSocket = CreateDataSocketActiveMode(ConnectSocket);
            else
                DataSocket = CreateDataSocketPassiveMode(ConnectSocket);

            string cmd_arguments = getCommandArguments(current_command);

            char filepath[_MAX_PATH];
            strcpy(filepath, "D:\\UniversityWork\\SecuritateFTP\\");
            char *p = strrchr(cmd_arguments.c_str(), '\\');
            if (p != NULL)
                strcat(filepath, strrchr(cmd_arguments.c_str(), '\\') + 1);
            else
                strcat(filepath, cmd_arguments.c_str());

            FILE *dst_fd = fopen(filepath, "wb");
            if (dst_fd != NULL) {
                char filebuf[100];
                int count = 0, k = 0;

                while ((k = recv(DataSocket, filebuf, sizeof(char) * 99, 0)) > 0) {
                    filebuf[k] = 0;
                    count += k;
                    cout << filebuf;
                    fwrite(filebuf, 1, k, dst_fd);
                }
                fclose(dst_fd);
            } else {
                cout << "aaaaa" << endl;
            }

            char buffer[500];
            size_t size;
            iResult = receiveValue(ConnectSocket, size, buffer);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            cout << buffer << endl;
            closesocket(DataSocket);


        } else if (strcmpi(cmd_word.c_str(), "STOR") == 0) {
            string cmd_arguments = getCommandArguments(current_command);

            struct _stat structure;
            int fileCheck = _stat(cmd_arguments.c_str(), &structure);

            if (fileCheck < 0) {
                cout << "Error accessing file" << endl;
            } else {
                iResult = sendValue(ConnectSocket, strlen(command), command);
                if (iResult <= 0) {
                    closesocket(ConnectSocket);
                    WSACleanup();
                    exit(1);
                }

                SOCKET DataSocket;
                if (mode == 1)
                    DataSocket = CreateDataSocketActiveMode(ConnectSocket);
                else
                    DataSocket = CreateDataSocketPassiveMode(ConnectSocket);

                FILE *src_fd = fopen(cmd_arguments.c_str(), "rb");
                if (src_fd != NULL) {
                    int k, count = structure.st_size, total = 0;
                    char buffer[100];
                    while ((k = fread(buffer, 1, 99, src_fd)) > 0) {
                        total += k;
                        buffer[k] = 0;
                        int res = send(DataSocket, buffer, sizeof(char) * k, 0);
                        if (res <= 0) {
                            pthread_exit(nullptr);
                        }
                    }

                    fclose(src_fd);
                }
                closesocket(DataSocket);
            }
            size_t size;
            iResult = receiveValue(ConnectSocket, size, command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }
            cout << command << endl;

        } else if (strcmpi(command, "QUIT") == 0) {
            iResult = sendValue(ConnectSocket, strlen(command), command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }

            size_t size;
            iResult = receiveValue(ConnectSocket, size, command);
            if (iResult <= 0) {
                closesocket(ConnectSocket);
                WSACleanup();
                exit(1);
            }
            cout << command << endl;
            break;
        } else {
            cout << "command not accepted" << endl;
        }


    }

    closesocket(ConnectSocket);


    std::cout << "Hello, World!" << std::endl;
    return 0;
}
