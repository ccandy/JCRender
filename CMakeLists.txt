cmake_minimum_required(VERSION 3.20)
project(JianJianRenderer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

add_executable(swr
    src/main.c
)

target_include_directories(swr PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(swr PRIVATE ${SDL2_LIBRARIES})

target_compile_options(swr PRIVATE -Wall -Wextra -Wpedantic)
