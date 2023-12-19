cmake_minimum_required(VERSION 3.12)

project(Signer)

set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

if(WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

add_executable(Signer
    main.cpp
    ../utils/argparse.cpp
    ../utils/MRConstants.cpp
    ../utils/MRContainer.cpp
    ../utils/MRCrypt.cpp
    ../utils/MRDateTime.cpp
    ../utils/MRFileSystem.cpp
    ../utils/MRString.cpp
    ../utils/MRTypedefs.cpp
    ../utils/StdAfx.cpp
    ../utils/tinyxml2.cpp
)

target_include_directories(Signer PRIVATE
    ${CMAKE_BINARY_DIR}/../../utils
)

install(TARGETS Signer RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_FILE_NAME "Signer")
set(CPACK_PACKAGE_CONTACT art.shestakov@icloud.com)
include(CPack)
