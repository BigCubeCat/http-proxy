cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME exceptions_lib)

project(${PROJECT_NAME}
LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(include)
file(GLOB_RECURSE SOURCES *.cpp)
file(GLOB_RECURSE HEADERS *.h *.hpp)

find_package(spdlog REQUIRED)

add_library(${PROJECT_NAME} INTERFACE ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>"
)

target_link_libraries(${PROJECT_NAME} INTERFACE spdlog::spdlog)


