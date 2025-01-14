cmake_minimum_required(VERSION 3.30)
project(XenDX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(COMMON ${CMAKE_CURRENT_SOURCE_DIR}/Code/Common)
set(ENGINE ${CMAKE_CURRENT_SOURCE_DIR}/Code/XenEngine)
#set(TOOLS ${CMAKE_CURRENT_SOURCE_DIR}/Code/Tools)
#set(VENDOR ${CMAKE_CURRENT_SOURCE_DIR}/Code/Vendor)

include_directories(
        ${COMMON}
        ${ENGINE}
        #        ${TOOLS}
        #        ${VENDOR}
)

add_subdirectory(Code/XenEngine)
add_subdirectory(Code/Testbed)
