project(XenDX)

add_executable(testbed
        ${COMMON}/Filesystem.hpp
        ${COMMON}/Filesystem.cpp
        ${COMMON}/Color.hpp
        ${COMMON}/Color.cpp
        ${COMMON}/Panic.inl
        ${COMMON}/Types.hpp
        main.cpp
)