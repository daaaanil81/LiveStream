cmake_minimum_required(VERSION 3.24.0)
project("webrtcdaemon")

if (${SANITIZE})
    message(STATUS "Sanitizer ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=undefined -fstack-protector-all -fno-omit-frame-pointer -g")
else ()
    message(STATUS "Sanitizer OFF")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# DEPENDENCIES

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    message(STATUS "Build for MACOS")
endif()

if (${MACOSX})
    set(CMAKE_LIBRARY_PATH "/Volumes/ACE_BUILD/ACE_wrappers/ace")
    link_directories("/Volumes/ACE_BUILD/ACE_wrappers/ace")
else ()
    set(CMAKE_LIBRARY_PATH "/usr/local/lib")
    link_directories("/usr/local/lib")
endif()

find_library(ACE_LIB ACE)
if (NOT ACE_LIB)
    message(FATAL_ERROR "ACE library not found")
endif()

project("webrtcdaemon")
set(CMAKE_CXX_STANDARD 17)

if (${MACOSX})
    include_directories("/Volumes/ACE_BUILD/ACE_wrappers")
else()
    include_directories("/usr/local/include")
endif()

# DAEMON

file(GLOB_RECURSE SOURCE_FILES_DAEMON
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )

file(GLOB_RECURSE HEADER_FILES_DAEMON
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    )

add_executable(daemon ${HEADER_FILES_DAEMON} ${SOURCE_FILES_DAEMON})

target_include_directories(daemon PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    )

target_link_libraries(daemon -lACE)

# CLIENT

file(GLOB_RECURSE SOURCE_FILES_CLIENT
    "${CMAKE_SOURCE_DIR}/client/*.cpp"
    )

add_executable(client ${SOURCE_FILES_CLIENT})

target_link_libraries(client -lACE)
