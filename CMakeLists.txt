cmake_minimum_required(VERSION 3.12)
project(BuildSolution)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/LibraryExt/ISCore)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/LibraryExt/ISDatabase)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Migrator)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Signer)
