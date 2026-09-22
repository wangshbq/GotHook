cmake_minimum_required(VERSION 3.22.1)

project(jiagu_native)

add_library(
    jiagu_native
    SHARED
    native-lib.cpp
)
find_package(bytehook REQUIRED CONFIG)
find_library(log-lib log)

target_link_libraries(
    jiagu_native
        bytehook::bytehook
    ${log-lib}
)
