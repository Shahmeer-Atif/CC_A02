cmake_minimum_required(VERSION 3.10)
project(CompilerAssignment)

# 1. Require C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 2. THE MAGIC LINE: Force the compiled executable to be saved in the ROOT folder
# This ensures your program can easily find the input/ and output/ folders!
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# 3. Tell CMake that all our headers (.h) are inside the src/ folder
include_directories(src)

# 4. Grab all the .cpp files explicitly from the src/ folder
file(GLOB SOURCES "src/*.cpp")

# 5. Create the final executable
add_executable(compiler_test ${SOURCES})