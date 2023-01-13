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

link_directories("/usr/local/lib")
include_directories("/usr/local/include")

# DEPENDENCIES

project("webrtcdaemon")
set(CMAKE_CXX_STANDARD 17)

find_library(AVCODEC_LIB avcodec)
if (NOT AVCODEC_LIB)
    message(FATAL_ERROR "avcodec library not found")
endif()

find_library(AVFORMAT_LIB avformat)
if (NOT AVFORMAT_LIB)
    message(FATAL_ERROR "avformat library not found")
endif()

find_library(AVUTIL_LIB avutil)
if (NOT AVUTIL_LIB)
    message(FATAL_ERROR "avutil library not found")
endif()

find_library(SWSCALE_LIB swscale)
if (NOT SWSCALE_LIB)
    message(FATAL_ERROR "swscale library not found")
endif()

find_package(OpenCV 4.6.0 REQUIRED)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
)

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

target_link_libraries(daemon
    -lavcodec
    -lavformat
    -lavutil
    -lswscale
    ${OpenCV_LIBS}
    )
