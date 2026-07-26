#include "Database.hpp"

#include <sqlite3.h>

#include <stdexcept>

Database::Database(const std::string& path) {
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown sqlite3_open failure";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Database: failed to open '" + path + "': " + msg);
    }

    createSchemaIfNeeded();
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::createSchemaIfNeeded() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  email TEXT PRIMARY KEY,"
        "  username TEXT NOT NULL,"
        "  password_hash TEXT NOT NULL,"
        "  salt TEXT NOT NULL,"
        "  rating INTEGER NOT NULL DEFAULT 1200"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string msg = errMsg ? errMsg : "unknown schema creation error";
        sqlite3_free(errMsg);
        throw std::runtime_error("Database: failed to create users schema: " + msg);
    }
}
