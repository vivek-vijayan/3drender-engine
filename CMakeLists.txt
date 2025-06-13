cmake_minimum_required(VERSION 3.16)
project(Engine3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vendor/SFML)
include_directories(vendor/SFML/include)

file(COPY src/Engine/Core/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY src/Engine/Core/configs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(Engine3D ${SOURCES} "src/Engine/Core/Engine.cpp")

target_link_libraries(Engine3D sfml-graphics sfml-window sfml-audio sfml-system)
