cmake_minimum_required(VERSION 3.3)
project(interprocess)

add_definitions(-std=c++1z -DASIO_STANDALONE -DBOOST_LOG_DYN_LINK)

include_directories(include/)

FIND_PACKAGE(Boost 1.60 COMPONENTS log REQUIRED)
FIND_PACKAGE(Boost 1.60 COMPONENTS system REQUIRED)
FIND_PACKAGE(Boost 1.60 COMPONENTS filesystem REQUIRED)

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp)

add_executable(${PROJECT_NAME} ${HEADERS}
        main.cpp)

target_link_libraries(${PROJECT_NAME}
        pthread
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_LOG_LIBRARY}
        rt
        )

install(DIRECTORY include/ DESTINATION include/dcm/ FILES_MATCHING PATTERN "*.hpp")
