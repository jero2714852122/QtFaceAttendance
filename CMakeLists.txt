cmake_minimum_required(VERSION 3.24)

project(QtFaceAttendance VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Network Sql Widgets)

qt_standard_project_setup()

add_subdirectory(apps/client)
add_subdirectory(apps/server)
