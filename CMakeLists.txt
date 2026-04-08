include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- Test executable ---
add_executable(engine_tests
    test_board.cpp
    test_movegen.cpp
    test_zobrist.cpp
    test_search.cpp
    test_engine.cpp
)
target_link_libraries(engine_tests PRIVATE engine GTest::gtest_main)
add_test(NAME engine_tests COMMAND engine_tests)
