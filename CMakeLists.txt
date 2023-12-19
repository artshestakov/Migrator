cmake_minimum_required(VERSION 3.12)
project(BuildSolution)

include_directories(${CMAKE_BINARY_DIR}/../src/utils)

add_subdirectory(${CMAKE_BINARY_DIR}/../src/Migrator)
add_subdirectory(${CMAKE_BINARY_DIR}/../src/Signer)
