# 4D Chess Engine — Makefile
# Requires: g++ (C++17)

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic
INCLUDES := -Iinclude

# Source files (all .cpp under src/ except main.cpp)
ENGINE_SRCS := $(wildcard src/types.cpp src/board.cpp src/zobrist.cpp \
                        src/movegen.cpp src/state.cpp src/eval.cpp \
                        src/search.cpp src/engine.cpp)

TEST_SRCS := $(wildcard tests/test_*.cpp)

ENGINE_OBJ := $(ENGINE_SRCS:.cpp=.o)
TEST_OBJ   := $(TEST_SRCS:.cpp=.o)

.PHONY: all clean test run

# --- Default target: CLI executable ---
all: chess4d

chess4d: $(ENGINE_OBJ) src/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# --- Compile rules ---
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(shell pwd)/tests/googletest/googletest/include -c $< -o $@

# --- Run the demo ---
run: chess4d
	./chess4d

# --- Tests (requires GoogleTest headers available) ---
# To build tests, either:
#   1. Have googletest installed system-wide, or
#   2. Clone into tests/googletest/
#   git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git tests/googletest
#
# Then: make test
test: chess4d tests/engine_tests
	./tests/engine_tests

tests/engine_tests: $(ENGINE_OBJ) $(TEST_OBJ) tests/googletest/build/lib/libgtest.a
	$(CXX) $(CXXFLAGS) $^ -lpthread -o $@

tests/googletest/build/lib/libgtest.a:
	cd tests/googletest && mkdir -p build && cd build && cmake .. -DCMAKE_INSTALL_PREFIX=.. && make install

# --- Clean ---
clean:
	rm -f src/*.o tests/*.o chess4d tests/engine_tests
	rm -rf tests/googletest/build
