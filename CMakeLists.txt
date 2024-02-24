cmake_minimum_required(VERSION 3.12)
project(httpServer)

set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)

# Add executable
add_executable(httpServer main.cpp Server.cpp)

# Include Boost headers
target_include_directories(httpServer PRIVATE ${Boost_INCLUDE_DIRS})

# Link Boost libraries
target_link_libraries(httpServer PRIVATE ${Boost_LIBRARIES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

