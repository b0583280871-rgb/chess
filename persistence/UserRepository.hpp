#pragma once

#include <string>

#include "Database.hpp"

struct RegisterResult {
    bool success;
    std::string reason;   
};

struct LoginResult {
    bool success;
    std::string reason;   
    int rating;           
};

RegisterResult registerUser(Database& db, const std::string& email, const std::string& password);

LoginResult loginUser(Database& db, const std::string& email, const std::string& password);
