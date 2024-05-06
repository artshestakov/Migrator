@ECHO OFF

IF EXIST build RMDIR /Q /S build

cmake . -B build
IF %errorlevel% neq 0 EXIT /b %errorlevel%

cmake --build build --config Release
IF %errorlevel% neq 0 EXIT /b %errorlevel%

IF NOT EXIST deploy MKDIR deploy

COPY /Y build\bin\Release\Migrator.exe deploy
COPY /Y build\bin\Release\Signer.exe deploy
DIR deploy
