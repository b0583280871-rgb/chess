cmake_minimum_required(VERSION 3.14)
project(chess_game CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chess_game
    main.cpp
    model/Board.cpp
    model/Piece.cpp
    model/Position.cpp
    rules/PieceRules.cpp
    rules/RuleEngine.cpp
    realtime/RealTimeArbiter.cpp
    input/BoardMapper.cpp
    input/Controller.cpp
    engine/GameEngine.cpp
    texttests/ScriptRunner.cpp
    io/BoardParser.cpp
    io/BoardPrinter.cpp
    img.cpp
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
        tests/Piece.tests.cpp
        model/Board.cpp
        model/Piece.cpp
        model/Position.cpp
        rules/PieceRules.cpp
        rules/RuleEngine.cpp
        realtime/RealTimeArbiter.cpp
        input/BoardMapper.cpp
        input/Controller.cpp
        engine/GameEngine.cpp
        texttests/ScriptRunner.cpp
        io/BoardParser.cpp
        io/BoardPrinter.cpp
    )
    target_include_directories(chess_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} third_party)

    add_test(NAME chess_tests COMMAND chess_tests)

    option(ENABLE_COVERAGE "Build chess_tests with coverage instrumentation" OFF)

    if(ENABLE_COVERAGE)
        target_compile_options(chess_tests PRIVATE --coverage -O0 -g)
        target_link_options(chess_tests PRIVATE --coverage)
    endif()
endif()

    # --- Local OpenCV support (optional) -------------------------------------------------
    option(USE_LOCAL_OPENCV "Use local OpenCV in OpenCV_451 folder" OFF)

    if(USE_LOCAL_OPENCV)
        set(OPENCV_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenCV_451-20260714T084200Z-1-001/OpenCV_451")
        if(EXISTS "${OPENCV_ROOT}/include" AND (EXISTS "${OPENCV_ROOT}/bin/opencv_world451.lib" OR EXISTS "${OPENCV_ROOT}/bin/opencv_world451d.lib"))
            message(STATUS "Using local OpenCV at ${OPENCV_ROOT}")
            target_include_directories(chess_game PRIVATE "${OPENCV_ROOT}/include")
            set(OPENCV_LIBRARY "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.lib,${OPENCV_ROOT}/bin/opencv_world451.lib>")
            target_link_libraries(chess_game PRIVATE ${OPENCV_LIBRARY})

            if(BUILD_TESTS)
                target_include_directories(chess_tests PRIVATE "${OPENCV_ROOT}/include")
                target_link_libraries(chess_tests PRIVATE ${OPENCV_LIBRARY})
            endif()

            # Copy runtime DLLs next to the executables so they run without modifying PATH
            add_custom_command(TARGET chess_game POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                    $<TARGET_FILE_DIR:chess_game>
            )
            if(BUILD_TESTS)
            add_custom_command(TARGET chess_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<IF:$<CONFIG:Debug>,${OPENCV_ROOT}/bin/opencv_world451d.dll,${OPENCV_ROOT}/bin/opencv_world451.dll>"
                    $<TARGET_FILE_DIR:chess_tests>
            )
            endif()
        else()
            message(WARNING "Local OpenCV not found under ${OPENCV_ROOT}; disable USE_LOCAL_OPENCV or place OpenCV there")
        endif()
    else()
        find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)
        if(OpenCV_FOUND)
            target_include_directories(chess_game PRIVATE ${OpenCV_INCLUDE_DIRS})
            target_link_libraries(chess_game PRIVATE ${OpenCV_LIBS})
            if(BUILD_TESTS)
                target_include_directories(chess_tests PRIVATE ${OpenCV_INCLUDE_DIRS})
                target_link_libraries(chess_tests PRIVATE ${OpenCV_LIBS})
            endif()
        endif()
    endif()