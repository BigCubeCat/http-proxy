set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME proxy_lib)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-g -O3 -Wall")
set(CMAKE_C_FLAGS "-g -O3 -Wall")

find_package(spdlog REQUIRED)

include_directories(include)
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h include/*.hpp)
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")


include_directories(${CMAKE_SOURCE_DIR}/src/proxy/include)

target_link_libraries(${PROJECT_NAME} PUBLIC cache_lib exceptions_lib task_lib picohttpparser PRIVATE spdlog::spdlog)

