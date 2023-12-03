//
// Created by Oana on 11/26/2023.
//


#ifndef PROIECTSECURITATE_COMMANDCENTER_H
#define PROIECTSECURITATE_COMMANDCENTER_H

#include <vector>
#include <string>
#include <algorithm>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Authentication.h"
#include <windows.h>
#include <shlwapi.h>
#include <fcntl.h>
#include "SocketFunctions.h"
#include "Modes.h"


#define USER_COMMAND "user"
#define PASS_COMMAND "pass"
#define QUIT_COMMAND "quit"
#define CWD_COMMAND "cwd"
#define LIST_COMMAND "list"
#define RETR_COMMAND "retr"
#define STOR_COMMAND "stor"
#define PASV_COMMAND "pasv"

#define WHITESPACE " \n\r\t\f\v"

using namespace std;

vector<string> commands;

void populateCommands() {
    populateUsers();
    commands.emplace_back("user");
    commands.emplace_back("pass");
    commands.emplace_back("quit");
    commands.emplace_back("cwd");
    commands.emplace_back("port");
    commands.emplace_back("list");
    commands.emplace_back("retr");
    commands.emplace_back("stor");
    commands.emplace_back("mode");
    commands.emplace_back("pasv");
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

char *userCommand(const string &arguments) {
    bool value = checkIfUserExists(arguments);
    if (value)
        return const_cast<char *>(arguments.c_str());
    else
        return "false";
}

char *passCommand(char *user, string arguments) {
    bool value = checkIfUserAndPassMatch(user, arguments);
    if (value)
        return user;
    else
        return "false";
}

void cwdCommand(string arguments, char *path) {
    struct _stat structure;
    int result = _stat(arguments.c_str(), &structure);
    if (result < 0)
        strcpy(path, "false");
    else {
        _fullpath(path, arguments.c_str(), _MAX_PATH);
    }
}

void listCommand(SOCKET DataSocket, char *arguments, char *result) {

    DIR *dir;
    struct dirent *en;
    dir = opendir(arguments); //open directory from arguments
    char line[_MAX_PATH + 2] = "";

    if (dir) {
        while ((en = readdir(dir)) != NULL) {
            strcpy(line, en->d_name);
            strcat(line, "\n");
            int res = send(DataSocket, line, sizeof(char) * strlen(line), 0);
            if (res <= 0) {
                strcpy(result, "426 Connection closed; transfer aborted");
            }
//            strncat(result, en->d_name, size - strlen(result) - 1);
//            strncat(result, "\n", size - strlen(result) - 1);
        }
        closedir(dir);
    } else {
        strcpy(result, "550 File does not exist.");
    }
}


void retrCommand(SOCKET DataSocket, string arguments, char *currentDirectory, SOCKET ClientSocket) {
    struct _stat structure;
    char filepath[500] = "";
    strcat(filepath, currentDirectory);
    strcat(filepath, "\\");
    strcat(filepath, arguments.c_str());

    int result = _stat(filepath, &structure);
    char buffer[101] = "";
    if (result < 0) {
        strcpy(buffer, "550 File does not exist.");
        int res = sendValue(ClientSocket, strlen(buffer), buffer);
        if (res <= 0) {
            pthread_exit(nullptr);
        }
    } else {
        int k, count = structure.st_size, total = 0;
//        size_t copySize = htonl(count);
//        strcpy(buffer, "ok");
//        int res = sendValue(DataSocket, strlen(buffer), buffer);
//        if (res <= 0) {
//            pthread_exit(nullptr);
//        }


        strcpy(buffer, strrchr(filepath, '\\') + 1);
//        int res = sendValue(DataSocket, strlen(buffer), buffer);
//        if (res <= 0) {
//            pthread_exit(nullptr);
//        }

//        int res = send(DataSocket, (char *) &copySize, sizeof(size_t), 0);
//        if (res <= 0) {
//            pthread_exit(nullptr);
//        }


//        int fileDescriptor = open(filepath, O_RDONLY);
        FILE *src_fd = fopen(filepath, "rb");

        if (src_fd != NULL) {
            while (total < count && (k = fread(buffer, 1, 100, src_fd)) > 0) {
                total += k;
                buffer[k] = 0;
                int res = send(DataSocket, buffer, sizeof(char) * k, 0);
                if (res <= 0) {
                    char value[100] = "426 Connection closed; transfer aborted.";
                    res = sendValue(ClientSocket, strlen(value), value);
                    if (res <= 0) {
                        pthread_exit(nullptr);
                    }
                    break;
                }
            }
            fclose(src_fd);
            char value[100] = "226 File transfer successful. Closing data connection.";
            int res = sendValue(ClientSocket, strlen(value), value);
            if (res <= 0) {
                pthread_exit(nullptr);
            }
        }

    }
}

void storCommand(SOCKET DataSocket, const char *arguments, char *currentDirectory) {
    char filepath[500] = "";
    strcpy(filepath, currentDirectory);
    strcat(filepath, "\\");
    strcat(filepath, strrchr(arguments, '\\') + 1);

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
    }

}


void pasvCommand(SOCKET ClientSocket) {
    char send[100];
    char auxiliary[20];
    strcpy(auxiliary, DATA_IP);
    strcpy(send, "227 Entering Passive Mode (");

    char *p = strtok(auxiliary, ".");
    strcat(send, p);
    strcat(send, ",");
    p = strtok(NULL, ".");
    strcat(send, p);
    strcat(send, ",");
    p = strtok(NULL, ".");
    strcat(send, p);
    strcat(send, ",");
    p = strtok(NULL, ".");
    strcat(send, p);
    strcat(send, ",");

    int p1 = atoi(DATA_PORT) / 256;
    int p2 = atoi(DATA_PORT) % 256;

    strcat(send, to_string(p1).c_str());
    strcat(send, ",");
    strcat(send, to_string(p2).c_str());
    strcat(send, ")");

    int result = sendValue(ClientSocket, strlen(send), send);
    if (result <= 0) {
        closesocket(ClientSocket);
        pthread_exit(nullptr);
    }

}

void portCommand(char *command_arguments) {
    char newAddress[100] = "";

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
}

#endif //PROIECTSECURITATE_COMMANDCENTER_H
