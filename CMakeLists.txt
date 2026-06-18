set(NAME PIS)

cmake_minimum_required(VERSION 4.0)
project(${NAME})

set(CMAKE_CXX_STANDARD 20)

# For profiling
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

# Add files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS source/*.cpp)

# Main library
add_executable(${NAME} ${SOURCE_FILES})

# Add include
target_include_directories(${NAME} PUBLIC include)
