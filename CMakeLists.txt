set(NAME dnTemplate)

cmake_minimum_required(VERSION 4.0)
project(${NAME})

set(CMAKE_CXX_STANDARD 20)

# Add files
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS source/*.cpp)

# Main library
add_library(${NAME} STATIC ${SOURCE_FILES})

# Add include
target_include_directories(${NAME} PUBLIC include)
