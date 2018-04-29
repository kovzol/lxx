cmake_minimum_required(VERSION 3.0)
project(lxx)

set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(sword REQUIRED sword)

include_directories(/usr/include/sword)

add_executable(lxx main.cpp)
target_link_libraries(lxx sword)
