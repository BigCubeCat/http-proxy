set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME thread_pool_lib)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
set(CMAKE_CXX_FLAGS "-g -Ofast -Wall")
set(CMAKE_C_FLAGS "-g -Ofast -Wall -W")

include_directories(include)
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h include/*.hpp)


add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")


include_directories(${CMAKE_SOURCE_DIR}/src/thread_pool/include)

find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC exceptions_lib task_lib PRIVATE spdlog::spdlog)
