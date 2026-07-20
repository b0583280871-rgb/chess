cmake_minimum_required(VERSION 3.14)
project(chess_game CXX C) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================
# Networking dependencies - fetched from source (header-only, no manual
# download/install), see network/ and PROTOCOL.md.
# =============================================================================
include(FetchContent)

# Deliberately using FetchContent_Populate (source-only) rather than
# FetchContent_MakeAvailable: none of these libraries' own CMakeLists.txt is
# invoked, since we only want their headers, not their own CMake-built
# targets. Silences CMake's "call MakeAvailable instead" dev warning.
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG        master
)
FetchContent_GetProperties(websocketpp)
if(NOT websocketpp_POPULATED)
    FetchContent_Populate(websocketpp)
endif()
set(WEBSOCKETPP_INCLUDE_DIR ${websocketpp_SOURCE_DIR})

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_GetProperties(nlohmann_json)
if(NOT nlohmann_json_POPULATED)
    FetchContent_Populate(nlohmann_json)
endif()
set(NLOHMANN_JSON_INCLUDE_DIR ${nlohmann_json_SOURCE_DIR}/single_include)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
)
FetchContent_GetProperties(sqlite3)
if(NOT sqlite3_POPULATED)
    FetchContent_Populate(sqlite3)
endif()
set(SQLITE3_SOURCE_DIR ${sqlite3_SOURCE_DIR})

FetchContent_Declare(
    picosha2
    GIT_REPOSITORY https://github.com/okdshin/PicoSHA2.git
    GIT_TAG        master
)
FetchContent_GetProperties(picosha2)
if(NOT picosha2_POPULATED)
    FetchContent_Populate(picosha2)
endif()
set(PICOSHA2_INCLUDE_DIR ${picosha2_SOURCE_DIR})

option(USE_LOCAL_OPENCV "Use local OpenCV in OpenCV_451 folder" OFF)

if(USE_LOCAL_OPENCV)
    set(OPENCV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenCV_451-20260714T084200Z-1-001/OpenCV_451")
    if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
        message(STATUS "Using local OpenCV at ${OPENCV_ROOT}")
        set(OPENCV_LIBRARY "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.lib,${OPENCV_ROOT}/bin/opencv_world451.lib>")
    else()
        message(WARNING "Local OpenCV not found under ${OPENCV_ROOT}; disable USE_LOCAL_OPENCV or place OpenCV there")
    endif()
else()
    find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
endif()

function(link_opencv target_name)
    if(USE_LOCAL_OPENCV)
        if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
            target_include_directories(${target_name} PRIVATE "${OPENCV_ROOT}/include")
            target_link_libraries(${target_name} PRIVATE ${OPENCV_LIBRARY})
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                    $<TARGET_FILE_DIR:${target_name}>
            )
        endif()
    else()
        if(OpenCV_FOUND)
            target_include_directories(${target_name} PRIVATE ${OpenCV_INCLUDE_DIRS})
            target_link_libraries(${target_name} PRIVATE ${OpenCV_LIBS})
        endif()
    endif()
endfunction()

function(copy_assets target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/frontend/assets"
            "$<TARGET_FILE_DIR:${target_name}>/assets"
    )
endfunction()

add_executable(chess_game
    backend/chess_main.cpp
    backend/model/Board.cpp
    backend/model/Piece.cpp
    backend/model/Position.cpp
    backend/rules/PieceRules.cpp
    backend/rules/RuleEngine.cpp
    backend/realtime/RealTimeArbiter.cpp
    backend/input/BoardMapper.cpp
    backend/input/Controller.cpp
    backend/engine/GameEngine.cpp
    backend/texttests/ScriptRunner.cpp
    backend/io/BoardParser.cpp
    backend/io/BoardPrinter.cpp
)
target_include_directories(chess_game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/backend)

option(BUILD_TESTS "Build the doctest test suite" ON)

if(BUILD_TESTS)
    enable_testing()

    add_executable(chess_tests
        backend/tests/doctest_main.cpp
        backend/tests/Config.tests.cpp
        backend/tests/Board.tests.cpp
        backend/tests/Movement.tests.cpp
        backend/tests/Engine.tests.cpp
        backend/tests/Piece.tests.cpp
        backend/tests/SpriteLoader.tests.cpp
        backend/tests/EngineSnapshot.tests.cpp
        backend/tests/RealTimeArbiter.tests.cpp
        backend/tests/SpriteAnimation.tests.cpp
        backend/tests/Protocol.tests.cpp
        backend/tests/ProtocolJson.tests.cpp
        backend/tests/UserRepository.tests.cpp
        network/protocol/JsonCodec.cpp
        persistence/Database.cpp
        persistence/UserRepository.cpp
        ${SQLITE3_SOURCE_DIR}/sqlite3.c
        backend/model/Board.cpp
        backend/model/Piece.cpp
        backend/model/Position.cpp
        backend/rules/PieceRules.cpp
        backend/rules/RuleEngine.cpp
        backend/realtime/RealTimeArbiter.cpp
        backend/input/BoardMapper.cpp
        backend/input/Controller.cpp
        backend/engine/GameEngine.cpp
        backend/engine/GameSnapshot.cpp
        backend/texttests/ScriptRunner.cpp
        backend/io/BoardParser.cpp
        backend/io/BoardPrinter.cpp
        frontend/ui/img.cpp
        frontend/view/SpriteLoader.cpp
    )
    target_include_directories(chess_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/backend
        ${CMAKE_CURRENT_SOURCE_DIR}/backend/third_party
        ${CMAKE_CURRENT_SOURCE_DIR}/frontend
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${NLOHMANN_JSON_INCLUDE_DIR}
        ${SQLITE3_SOURCE_DIR}
        ${PICOSHA2_INCLUDE_DIR}
    )
    link_opencv(chess_tests)
    copy_assets(chess_tests)

    add_test(NAME chess_tests COMMAND chess_tests)

    option(ENABLE_COVERAGE "Build chess_tests with coverage instrumentation" OFF)
    if(ENABLE_COVERAGE)
        target_compile_options(chess_tests PRIVATE --coverage -O0 -g)
        target_link_options(chess_tests PRIVATE --coverage)
    endif()
endif()

add_executable(sprite_demo
    frontend/sprite_demo.cpp
    frontend/ui/img.cpp
    frontend/view/SpriteLoader.cpp
)
target_include_directories(sprite_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend)
link_opencv(sprite_demo)
copy_assets(sprite_demo)

add_executable(live_demo
    frontend/live_demo.cpp
    frontend/ui/img.cpp
    frontend/view/SpriteLoader.cpp
    frontend/view/Renderer.cpp
    backend/model/Board.cpp
    backend/model/Piece.cpp
    backend/model/Position.cpp
    backend/rules/PieceRules.cpp
    backend/rules/RuleEngine.cpp
    backend/realtime/RealTimeArbiter.cpp
    backend/input/BoardMapper.cpp
    backend/input/Controller.cpp
    backend/io/BoardParser.cpp
    backend/engine/GameSnapshot.cpp
    backend/engine/GameEngine.cpp
)
target_include_directories(live_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend)
link_opencv(live_demo)
copy_assets(live_demo)


# game_server / game_client - real GameEngine integration + real login
# (persistence/), single global game. protocol_demo_server/client are gone -
# superseded now that game_server/game_client do the real thing (see
# CLAUDE.md's demo/executable policy).
add_executable(game_server
    network/game_server.cpp
    network/protocol/JsonCodec.cpp
    persistence/Database.cpp
    persistence/UserRepository.cpp
    ${SQLITE3_SOURCE_DIR}/sqlite3.c
    backend/model/Board.cpp
    backend/model/Piece.cpp
    backend/model/Position.cpp
    backend/rules/PieceRules.cpp
    backend/rules/RuleEngine.cpp
    backend/realtime/RealTimeArbiter.cpp
    backend/input/BoardMapper.cpp
    backend/input/Controller.cpp
    backend/io/BoardParser.cpp
    backend/engine/GameSnapshot.cpp
    backend/engine/GameEngine.cpp
)
target_include_directories(game_server PRIVATE ${SQLITE3_SOURCE_DIR} ${PICOSHA2_INCLUDE_DIR})

add_executable(game_client
    network/game_client.cpp
    network/protocol/JsonCodec.cpp
    frontend/ui/img.cpp
    frontend/view/SpriteLoader.cpp
    frontend/view/Renderer.cpp
)
target_include_directories(game_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend)
link_opencv(game_client)
copy_assets(game_client)

foreach(net_target game_server game_client)
    target_include_directories(${net_target} PRIVATE
        ${WEBSOCKETPP_INCLUDE_DIR}
        ${ASIO_INCLUDE_DIR}
        ${NLOHMANN_JSON_INCLUDE_DIR}
    )
    target_compile_definitions(${net_target} PRIVATE
        ASIO_STANDALONE
        _WEBSOCKETPP_CPP11_STL_
        _WEBSOCKETPP_CPP11_THREAD_
    )

    if(WIN32)
        target_compile_definitions(${net_target} PRIVATE _WIN32_WINNT=0x0601)
        target_link_libraries(${net_target} PRIVATE ws2_32 wsock32 mswsock)
    endif()
endforeach()

add_executable(persistence_demo
    persistence/persistence_demo.cpp
    persistence/Database.cpp
    persistence/UserRepository.cpp
    ${SQLITE3_SOURCE_DIR}/sqlite3.c
)
target_include_directories(persistence_demo PRIVATE ${SQLITE3_SOURCE_DIR} ${PICOSHA2_INCLUDE_DIR})
