@ECHO OFF

IF EXIST build RMDIR /Q /S build

cmake . -B build
cmake --build build --config Release
