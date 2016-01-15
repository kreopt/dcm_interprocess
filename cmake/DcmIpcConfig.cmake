# - Try to find DCM IPC
# Once done this will define
#  DCMIPC_FOUND - System has DCM IPC
#  DCMIPC_INCLUDE_DIRS - The DCM IPC include directories
#  DCMIPC_LIBRARIES - The libraries needed to use DCM IPC
#  DCMIPC_DEFINITIONS - Compiler switches required for using DCM IPC

add_definitions(-DASIO_STANDALONE)

set(DCMIPC_INSTALL_PREFIX "/usr/local")
set(CMAKECONFIG_INSTALL_DIR "lib/cmake/DcmIpc")

find_path(DCMIPC_INCLUDE_DIR ${DCMIPC_INSTALL_PREFIX}/dcm/interprocess/listener_fctory.hpp PATH_SUFFIXES dcm )

include(${DCMIPC_INSTALL_PREFIX}/${CMAKECONFIG_INSTALL_DIR}/ExtLibs.cmake)

set(DCMIPC_INCLUDE_DIRS, ${DCMIPC_INCLUDE_DIR})
#set(DCMIPC_LIBRARIES  ${DCMIPC_EXT_LIBRARIES} ${DCMIPC_INSTALL_PREFIX}/lib/dcm/libdcm_ipc.so)

mark_as_advanced(DCMIPC_INCLUDE_DIR STDCPP_FS FS_LIB OS_SPECIFIC)