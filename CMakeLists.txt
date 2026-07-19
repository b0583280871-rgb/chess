# i need to check the file

cmake_minimum_required(VERSION 3.14)
project(chess_game CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    target_include_directories(chess_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/backend ${CMAKE_CURRENT_SOURCE_DIR}/backend/third_party ${CMAKE_CURRENT_SOURCE_DIR}/frontend)

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

# --- live_demo: Step 10 - live render loop (no mouse input yet) ------------
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

# --- chess_tests also needs OpenCV now that it links view/SpriteLoader.cpp -
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
