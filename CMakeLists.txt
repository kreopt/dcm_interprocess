cmake_minimum_required(VERSION 3.3)
project(interprocess)

add_definitions(-std=c++1z -DASIO_STANDALONE)

include_directories(include/)


file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp)

add_executable(${PROJECT_NAME} ${HEADERS}
        main.cpp
        include/interprocess/core/sender.hpp)

target_link_libraries(${PROJECT_NAME}
        pthread
        boost_system
        boost_filesystem
        rt
        )

install(DIRECTORY include/ DESTINATION include/dcm/ FILES_MATCHING PATTERN "*.hpp")
