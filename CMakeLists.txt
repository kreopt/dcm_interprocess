cmake_minimum_required(VERSION 2.8.4)
project(interprocess)

add_definitions(-std=c++14 -DASIO_STANDALONE)

include_directories(include/)


file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp)

add_executable(test ${HEADERS} main.cpp)
target_link_libraries(test
    pthread
    boost_system
    boost_filesystem
)

install(DIRECTORY include/ DESTINATION include/dcm/ FILES_MATCHING PATTERN "*.hpp")
