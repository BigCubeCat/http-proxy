set(PROJECT_NAME test_http_proxy)

find_package(spdlog REQUIRED)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.cpp)

set(SOURCES ${TEST_SOURCES})

SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")

include_directories(${CMAKE_SOURCE_DIR}/src/proxy/include)
include_directories(${CMAKE_SOURCE_DIR}/src/thread_pool/include)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

find_package(proxy_lib)
find_package(thread_pool_lib)
find_package(task_lib)
target_link_libraries(${PROJECT_NAME} PUBLIC task_lib proxy_lib thread_pool_lib gtest PRIVATE spdlog::spdlog)
