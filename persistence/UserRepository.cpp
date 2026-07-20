#include "UserRepository.hpp"

#include <sqlite3.h>
#include <picosha2.h>

#include <iomanip>
#include <random>
#include <sstream>

namespace {

    const int HASH_ITERATIONS = 100000;

    std::string generateSalt() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<int> byteDist(0, 255);

        std::ostringstream oss;
        for (int i = 0; i < 16; ++i) {
            oss << std::hex << std::setfill('0') << std::setw(2) << byteDist(gen);
        }
        return oss.str();
    }

    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string digest = picosha2::hash256_hex_string(salt + password);
        for (int i = 1; i < HASH_ITERATIONS; ++i) {
            digest = picosha2::hash256_hex_string(digest + salt);
        }
        return digest;
    }

    bool userExists(Database& db, const std::string& email) {
        const char* sql = "SELECT 1 FROM users WHERE email = ?;";
        sqlite3_stmt* stmt = nullptr;
        sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

        bool found = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
        return found;
    }

}

RegisterResult registerUser(Database& db, const std::string& email, const std::string& password) {
    if (userExists(db, email)) {
        return {false, "email_taken"};
    }

    std::string salt = generateSalt();
    std::string hash = hashPassword(password, salt);

    const char* sql = "INSERT INTO users (email, username, password_hash, salt, rating) VALUES (?, ?, ?, ?, 1200);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return {false, "database_error"};
    }
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, salt.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return {false, "database_error"};
    }

    return {true, ""};
}

LoginResult loginUser(Database& db, const std::string& email, const std::string& password) {
    const char* sql = "SELECT password_hash, salt, rating FROM users WHERE email = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return {false, "database_error", 0};
    }
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return {false, "user_not_found", 0};
    }

    std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string storedSalt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    int rating = sqlite3_column_int(stmt, 2);
    sqlite3_finalize(stmt);

    if (hashPassword(password, storedSalt) != storedHash) {
        return {false, "wrong_password", 0};
    }

    return {true, "", rating};
}
