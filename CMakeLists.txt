# i need to check the file

cmake_minimum_required(VERSION 3.14)
project(chess_game CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

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

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()

set(WEBSOCKETPP_INCLUDE_DIR ${websocketpp_SOURCE_DIR})
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include)

# nlohmann/json - header-only, single-include, MIT license. Fetched the same
# way as websocketpp/asio (FetchContent_Populate, no add_subdirectory) so
# there's nothing to manually download. Not wired into any target's JSON
# serialization yet in this step - just made available on the include path
# for whichever target needs it in a later step.
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
        network/protocol/JsonCodec.cpp
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
    target_include_directories(chess_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/backend ${CMAKE_CURRENT_SOURCE_DIR}/backend/third_party ${CMAKE_CURRENT_SOURCE_DIR}/frontend ${CMAKE_CURRENT_SOURCE_DIR} ${NLOHMANN_JSON_INCLUDE_DIR})

    add_test(NAME chess_tests COMMAND chess_tests)

    # GameEngine::snapshot now reads real config.json/sprite files from
    # assets/pieces_classic/ at runtime (frame-index computation), so
    # chess_tests needs the same asset copy the demo targets already get.
    add_custom_command(TARGET chess_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/frontend/assets"
            "$<TARGET_FILE_DIR:chess_tests>/assets"
    )

    option(ENABLE_COVERAGE "Build chess_tests with coverage instrumentation" OFF)

    if(ENABLE_COVERAGE)
        target_compile_options(chess_tests PRIVATE --coverage -O0 -g)
        target_link_options(chess_tests PRIVATE --coverage)
    endif()
endif()

# --- img_demo: lecturer-provided main.cpp, uses Img/OpenCV -----------------
add_executable(img_demo
    frontend/main.cpp
    frontend/ui/img.cpp
)
target_include_directories(img_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend)

add_custom_command(TARGET img_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/frontend/assets"
        "$<TARGET_FILE_DIR:img_demo>/assets"
)

# --- Local OpenCV support (optional) -------------------------------------------------
option(USE_LOCAL_OPENCV "Use local OpenCV in OpenCV_451 folder" OFF)

if(USE_LOCAL_OPENCV)
    set(OPENCV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenCV_451-20260714T084200Z-1-001/OpenCV_451")
    if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
        message(STATUS "Using local OpenCV at ${OPENCV_ROOT}")
        target_include_directories(img_demo PRIVATE "${OPENCV_ROOT}/include")
        set(OPENCV_LIBRARY "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.lib,${OPENCV_ROOT}/bin/opencv_world451.lib>")
        target_link_libraries(img_demo PRIVATE ${OPENCV_LIBRARY})

        # Copy runtime DLLs next to the executable so it runs without modifying PATH
        add_custom_command(TARGET img_demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                $<TARGET_FILE_DIR:img_demo>
        )
    else()
        message(WARNING "Local OpenCV not found under ${OPENCV_ROOT}; disable USE_LOCAL_OPENCV or place OpenCV there")
    endif()
else()
    find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
    if(OpenCV_FOUND)
        target_include_directories(img_demo PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(img_demo PRIVATE ${OpenCV_LIBS})
    endif()
endif()

# --- sprite_demo: Step 1 of sprite-loading plan - single static sprite -----
add_executable(sprite_demo
    frontend/sprite_demo.cpp
    frontend/ui/img.cpp
    frontend/view/SpriteLoader.cpp
)
target_include_directories(sprite_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend)

add_custom_command(TARGET sprite_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/frontend/assets"
        "$<TARGET_FILE_DIR:sprite_demo>/assets"
)

if(USE_LOCAL_OPENCV)
    if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
        target_include_directories(sprite_demo PRIVATE "${OPENCV_ROOT}/include")
        target_link_libraries(sprite_demo PRIVATE ${OPENCV_LIBRARY})

        add_custom_command(TARGET sprite_demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                $<TARGET_FILE_DIR:sprite_demo>
        )
    endif()
else()
    if(OpenCV_FOUND)
        target_include_directories(sprite_demo PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(sprite_demo PRIVATE ${OpenCV_LIBS})
    endif()
endif()

# --- view_demo: Steps 4-5 of sprite-loading plan - static board+piece render -
add_executable(view_demo
    frontend/view_demo.cpp
    frontend/ui/img.cpp
    frontend/view/SpriteLoader.cpp
    frontend/view/Renderer.cpp
    backend/model/Board.cpp
    backend/model/Piece.cpp
    backend/model/Position.cpp
    backend/rules/PieceRules.cpp
    backend/realtime/RealTimeArbiter.cpp
    backend/io/BoardParser.cpp
    backend/engine/GameSnapshot.cpp
)
target_include_directories(view_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/frontend)

add_custom_command(TARGET view_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/frontend/assets"
        "$<TARGET_FILE_DIR:view_demo>/assets"
)

if(USE_LOCAL_OPENCV)
    if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
        target_include_directories(view_demo PRIVATE "${OPENCV_ROOT}/include")
        target_link_libraries(view_demo PRIVATE ${OPENCV_LIBRARY})

        add_custom_command(TARGET view_demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                $<TARGET_FILE_DIR:view_demo>
        )
    endif()
else()
    if(OpenCV_FOUND)
        target_include_directories(view_demo PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(view_demo PRIVATE ${OpenCV_LIBS})
    endif()
endif()

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

add_custom_command(TARGET live_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/frontend/assets"
        "$<TARGET_FILE_DIR:live_demo>/assets"
)

if(USE_LOCAL_OPENCV)
    if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
        target_include_directories(live_demo PRIVATE "${OPENCV_ROOT}/include")
        target_link_libraries(live_demo PRIVATE ${OPENCV_LIBRARY})

        add_custom_command(TARGET live_demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                $<TARGET_FILE_DIR:live_demo>
        )
    endif()
else()
    if(OpenCV_FOUND)
        target_include_directories(live_demo PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(live_demo PRIVATE ${OpenCV_LIBS})
    endif()
endif()

if(BUILD_TESTS)
    if(USE_LOCAL_OPENCV)
        if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
            target_include_directories(chess_tests PRIVATE "${OPENCV_ROOT}/include")
            target_link_libraries(chess_tests PRIVATE ${OPENCV_LIBRARY})

            add_custom_command(TARGET chess_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                    $<TARGET_FILE_DIR:chess_tests>
            )
        endif()
    else()
        if(OpenCV_FOUND)
            target_include_directories(chess_tests PRIVATE ${OpenCV_INCLUDE_DIRS})
            target_link_libraries(chess_tests PRIVATE ${OpenCV_LIBS})
        endif()
    endif()
endif()

add_executable(ws_echo_server network/ws_echo_server.cpp)
add_executable(ws_echo_client network/ws_echo_client.cpp)

add_executable(protocol_demo_server
    network/protocol_demo_server.cpp
    network/protocol/JsonCodec.cpp
)
add_executable(protocol_demo_client
    network/protocol_demo_client.cpp
    network/protocol/JsonCodec.cpp
)
target_include_directories(protocol_demo_server PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(protocol_demo_client PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

foreach(ws_target ws_echo_server ws_echo_client protocol_demo_server protocol_demo_client)
    target_include_directories(${ws_target} PRIVATE ${WEBSOCKETPP_INCLUDE_DIR} ${ASIO_INCLUDE_DIR})
    target_compile_definitions(${ws_target} PRIVATE
        ASIO_STANDALONE            
        _WEBSOCKETPP_CPP11_STL_    
        _WEBSOCKETPP_CPP11_THREAD_ 
                                   
                                   
    )

    if(WIN32)
        target_compile_definitions(${ws_target} PRIVATE _WIN32_WINNT=0x0601)
        target_link_libraries(${ws_target} PRIVATE ws2_32 wsock32 mswsock)
    endif()
endforeach()
