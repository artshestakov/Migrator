cmake_minimum_required(VERSION 3.12)

project(Migrator)

set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

if(WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()

add_executable(Migrator
    IDatabase.cpp
    IDatabasePostgreSQL.cpp
    IDatabaseSQLite.cpp
    IQuery.cpp
    IQueryPostgreSQL.cpp
    IQuerySQLite.cpp
    main.cpp
    MRConfig.cpp
    MRDatabase.cpp
    MRDefDatabase.cpp
    MRException.cpp
    MRLibrary.cpp
    MRLogger.cpp
    MRMetaData.cpp
    MRMigrator.cpp
    MRQuery.cpp
    MRSQLProcessor.cpp
    MRTemplate.cpp
    MRUtils.cpp
    SimpleIni.cpp
    TMetaClass.cpp
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

target_include_directories(Migrator PRIVATE
    ${CMAKE_BINARY_DIR}/../../utils
)

if(WIN32)
    target_link_libraries(Migrator rpcrt4.lib)
else()
    target_link_libraries(Migrator uuid dl crypto)
endif()

install(TARGETS Migrator RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_FILE_NAME "Migrator")
set(CPACK_PACKAGE_CONTACT art.shestakov@icloud.com)
include(CPack)
