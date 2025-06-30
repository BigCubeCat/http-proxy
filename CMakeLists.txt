cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_NAME http_proxy)
project(${EXECUTABLE_NAME})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

add_subdirectory(lib/picohttpparser)

add_subdirectory(src/thread_pool)
add_subdirectory(src/task)
add_subdirectory(src/exceptions)
add_subdirectory(src/context)
add_subdirectory(src/utils)
add_subdirectory(src/proxy)

add_executable(${EXECUTABLE_NAME} src/main.cpp)
link_directories(${CMAKE_BINARY_DIR}/lib/picohttpparser)

find_package(spdlog REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC
picohttpparser
context_lib
exceptions_lib
task_lib 
proxy_lib
thread_pool_lib
utils_lib
PRIVATE
spdlog::spdlog
)


find_program (BASH_PROGRAM bash)

enable_testing()
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
  if (BASH_PROGRAM)
    add_test(basics ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/0.sh)
    add_test(sequential ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/1.sh)
    add_test(concurrent-batches ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/2.sh)
    add_test(cache-invalidation ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/3.sh)
    add_test(parallel-clients ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/4.sh)
    add_test(incremental-interrupt ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/5.sh)
  endif(BASH_PROGRAM)
endif (BUILD_TESTING)


