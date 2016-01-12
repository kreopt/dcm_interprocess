ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

FIND_PACKAGE(Boost 1.58 COMPONENTS log REQUIRED)
FIND_PACKAGE(Boost 1.58 COMPONENTS system REQUIRED)
FIND_PACKAGE(Boost 1.58 COMPONENTS filesystem OPTIONAL)

# FIXME: failed to find stdc++fs
FIND_LIBRARY(STDCPP_FS stdc++fs)

if (STDCPP_FS)
    message(STATUS "Using stdc++ filesystem library")
    set(FS_LIB ${STDCPP_FS})
    add_definitions(-DUSE_STD_FS)
else()
    set(FS_LIB ${Boost_FILESYSTEM_LIBRARY})
endif()

set(OS_SPECIFIC pthread rt)

set(DCMIPC_INCLUDE_DIRS, ${DCMIPC_INCLUDE_DIR})
set(DCMIPC_EXT_LIBRARIES  ${OS_SPECIFIC} ${Boost_SYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${FS_LIB})