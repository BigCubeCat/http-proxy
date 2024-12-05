cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_NAME http_proxy)
project(${EXECUTABLE_NAME})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

add_subdirectory(lib/picohttpparser)

add_subdirectory(src/proxy)
add_subdirectory(src/thread_pool)
add_subdirectory(src/task)
add_subdirectory(src/exceptions)
add_subdirectory(src/cache)
add_subdirectory(src/selector)

add_executable(${EXECUTABLE_NAME} src/main.cpp)

enable_testing()
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib/picohttpparser)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC
picohttpparser
selector_lib
cache_lib
exceptions_lib
task_lib 
proxy_lib
thread_pool_lib
)

