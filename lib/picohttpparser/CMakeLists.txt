cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME picohttpparser)
project(${PROJECT_NAME} C)

add_library(${PROJECT_NAME} STATIC picohttpparser.c picohttpparser.h)
target_include_directories(picohttpparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

