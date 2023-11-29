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

char *cwdCommand(string arguments) {
    struct _stat structure;
    int result = _stat(arguments.c_str(), &structure);
    if (result < 0)
        return "false";
    else
        return "true";
}

void listCommand(char *result, int size, string arguments) {

    DIR *dir;
    struct dirent *en;
    dir = opendir(arguments.c_str()); //open directory from arguments

    if (dir) {
        while ((en = readdir(dir)) != NULL) {
            strncat(result, en->d_name, size - strlen(result) - 1);
            strncat(result, "\n", size - strlen(result) - 1);
        }
        closedir(dir);
    } else {
        strcpy(result, "not a directory");
    }
}



#endif //PROIECTSECURITATE_COMMANDCENTER_H
