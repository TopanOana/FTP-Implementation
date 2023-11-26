//
// Created by Oana on 11/26/2023.
//

#ifndef PROIECTSECURITATE_COMMANDCENTER_H
#define PROIECTSECURITATE_COMMANDCENTER_H

#include <vector>
#include <string>
#include <algorithm>
#include "Authentication.h"


#define USER_COMMAND "user"
#define PASS_COMMAND "pass"
#define QUIT_COMMAND "quit"
#define CWD_COMMAND "cwd"
#define PORT_COMMAND "port"
#define LIST_COMMAND "list"
#define RETR_COMMAND "retr"
#define STOR_COMMAND "stor"
#define MODE_COMMAND "mode"
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

string left_trim(string value){
    size_t start = value.find_first_not_of(WHITESPACE);
    return (start == string::npos) ? "" : value.substr(start);
}

string right_trim(string value){
    size_t end = value.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : value.substr(0, end + 1);
}

char* goToCommand(string command){
    command = left_trim(command);
    string cmd = command.substr(0, command.find(' '));
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    string arguments = right_trim(left_trim(command.substr(command.find(' '))));
    if (cmd == USER_COMMAND){
        bool value = checkIfUserExists(arguments);
        if(value)
            return "true";
        else
            return "false";
    }
    if (cmd == PASS_COMMAND){

    }
    if (cmd == QUIT_COMMAND){

    }
    if (cmd == CWD_COMMAND){

    }
    if (cmd == PORT_COMMAND){

    }
    if (cmd == LIST_COMMAND){

    }
    if (cmd == RETR_COMMAND){

    }
    if (cmd == STOR_COMMAND){

    }
    if (cmd == MODE_COMMAND){

    }
    if (cmd == PASV_COMMAND){

    }
    return "";
}


#endif //PROIECTSECURITATE_COMMANDCENTER_H
