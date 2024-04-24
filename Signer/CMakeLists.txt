cmake_minimum_required(VERSION 3.12)
project(Signer)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        "/MP"
    )
endif()

add_executable(Signer
    main.cpp
)

if (WIN32)
    target_link_libraries(Signer PRIVATE
        ${CMAKE_BINARY_DIR}/bin/Release/ISCore.lib
    )
endif()

target_include_directories(Signer PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../LibraryExt/ISCore
    ${CMAKE_CURRENT_SOURCE_DIR}/../src_share
)
