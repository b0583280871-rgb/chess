cmake_minimum_required(VERSION 3.14)
project(chess_game CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chess_game
    main.cpp
    model/Board.cpp
    rules/PieceRules.cpp
    rules/RuleEngine.cpp
    realtime/RealTimeArbiter.cpp
    engine/Engine.cpp
    io/BoardParser.cpp
    io/BoardPrinter.cpp
)
target_include_directories(chess_game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_TESTS "Build the doctest test suite" ON)

if(BUILD_TESTS)
    enable_testing()

    add_executable(chess_tests
        tests/doctest_main.cpp
        tests/Config.tests.cpp
        tests/Board.tests.cpp
        tests/Movement.tests.cpp
        tests/Engine.tests.cpp
        model/Board.cpp
        rules/PieceRules.cpp
        rules/RuleEngine.cpp
        realtime/RealTimeArbiter.cpp
        engine/Engine.cpp
        io/BoardParser.cpp
        io/BoardPrinter.cpp
    )
    target_include_directories(chess_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} third_party)

    add_test(NAME chess_tests COMMAND chess_tests)
endif()
