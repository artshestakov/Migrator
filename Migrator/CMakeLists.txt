cmake_minimum_required(VERSION 3.12)
project(Migrator)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(VER_REVISION 1 CACHE INTERNAL "Revision")
set(VER_BRANCH branch_name CACHE INTERNAL "Branch")
set(VER_HASH hash CACHE INTERNAL "Hash")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../src_share/MRVersion.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/../src_share/MRVersion.h
)

if (WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        "/MP"
    )
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Migrator
    main.cpp
    MRMigrator.cpp
    MRUtils.cpp
)

if (WIN32)
    target_link_libraries(Migrator PRIVATE
        ${CMAKE_BINARY_DIR}/bin/Release/ISCore.lib
        ${CMAKE_BINARY_DIR}/bin/Release/ISDatabase.lib
    )
endif()

target_include_directories(Migrator PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../LibraryExt/ISCore
    ${CMAKE_CURRENT_SOURCE_DIR}/../LibraryExt/ISDatabase
    ${CMAKE_CURRENT_SOURCE_DIR}/../src_share
)
