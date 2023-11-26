//
// Created by Oana on 11/26/2023.
//

#ifndef PROIECTSECURITATE_AUTHENTICATION_H
#define PROIECTSECURITATE_AUTHENTICATION_H
#include <vector>
#include <string>
using namespace std;

vector<pair<string, string>> users;

void populateUsers(){
    users.emplace_back(make_pair("oana", "pass"));
    users.emplace_back(make_pair("naomi", "pass"));
    users.emplace_back(make_pair("user", "pass"));
}

bool checkIfUserExists(string user){
    auto it =std::find_if(users.begin(), users.end(), [user](pair<string, string> item ){return item.first == user;});
    return it != users.end();
}

bool checkIfUserAndPassMatch(string user, string pass){
    for (auto &item: users)
        if (item.first == user && item.second == pass)
            return true;
    return false;
}


#endif //PROIECTSECURITATE_AUTHENTICATION_H
