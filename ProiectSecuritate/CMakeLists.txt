cmake_minimum_required(VERSION 3.26)
project(ProiectSecuritate)

set(CMAKE_CXX_STANDARD 17)


link_libraries(ws2_32 wsock32 Shlwapi)

add_executable(ProiectSecuritate main.cpp
        Authentication.h
        CommandCenter.h
        SocketFunctions.h
        Modes.h
)
