cmake_minimum_required(VERSION 3.12)
project(httpServer)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

set(CMAKE_CXX_STANDARD 17)

set(VENDOR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vendor/spdlog/include)

message(STATUS "VENDOR_INCLUDE_DIR: ${VENDOR_INCLUDE_DIR}")

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)

set_source_files_properties( tags PROPERTIES GENERATED true)
add_custom_command ( OUTPUT tags
    COMMAND ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )

# Add executable
add_executable(httpServer main.cpp Server.cpp HttpMessage.cpp SocketIOHandler.cpp Log.cpp SandMethod.cpp)

# Include Boost headers
target_include_directories(httpServer PRIVATE
   ${Boost_INCLUDE_DIRS}
   ${VENDOR_INCLUDE_DIR}
)

# Link Boost libraries
target_link_libraries(httpServer PRIVATE ${Boost_LIBRARIES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

