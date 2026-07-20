#include "doctest.h"

#include "../../persistence/Database.hpp"
#include "../../persistence/UserRepository.hpp"

TEST_CASE("registerUser succeeds for a new email") {
    Database db(":memory:");
    RegisterResult result = registerUser(db, "alice", "hunter2");
    CHECK(result.success);
    CHECK(result.reason.empty());
}

TEST_CASE("registerUser rejects a duplicate email") {
    Database db(":memory:");
    registerUser(db, "alice", "hunter2");
    RegisterResult result = registerUser(db, "alice", "differentpass");
    CHECK_FALSE(result.success);
    CHECK(result.reason == "email_taken");
}

TEST_CASE("loginUser succeeds with the correct password") {
    Database db(":memory:");
    registerUser(db, "alice", "hunter2");
    LoginResult result = loginUser(db, "alice", "hunter2");
    CHECK(result.success);
    CHECK(result.reason.empty());
}

TEST_CASE("loginUser rejects the wrong password") {
    Database db(":memory:");
    registerUser(db, "alice", "hunter2");
    LoginResult result = loginUser(db, "alice", "wrongpass");
    CHECK_FALSE(result.success);
    CHECK(result.reason == "wrong_password");
}

TEST_CASE("loginUser rejects a nonexistent user") {
    Database db(":memory:");
    LoginResult result = loginUser(db, "ghost", "whatever");
    CHECK_FALSE(result.success);
    CHECK(result.reason == "user_not_found");
}

TEST_CASE("a newly registered user has the default rating of 1200") {
    Database db(":memory:");
    registerUser(db, "alice", "hunter2");
    LoginResult result = loginUser(db, "alice", "hunter2");
    REQUIRE(result.success);
    CHECK(result.rating == 1200);
}
