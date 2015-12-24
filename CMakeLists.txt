cmake_minimum_required(VERSION 3.2)
project(interprocess)

include_directories(include/)
add_definitions(-std=c++1z -DUSE_BOOST_LOG)

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.hpp)

add_executable(${PROJECT_NAME} ${HEADERS}
        main.cpp src/util.cpp)

add_library(dcm_ipc SHARED src/util.cpp)

include(cmake/DcmIpcConfig.cmake)

target_link_libraries(${PROJECT_NAME} ${DCMIPC_LIBRARIES})

set(CMAKECONFIG_INSTALL_DIR "lib/cmake/DcmIpc")

install(TARGETS dcm_ipc DESTINATION lib/dcm/)
install(DIRECTORY include/ DESTINATION include/dcm/ FILES_MATCHING PATTERN "*.hpp")
install(FILES "cmake/DcmIpcConfig.cmake" DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(FILES "cmake/DcmIpcVersion.cmake" DESTINATION ${CMAKECONFIG_INSTALL_DIR})
