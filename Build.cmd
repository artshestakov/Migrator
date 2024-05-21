@ECHO OFF

REM Вытаскиваем информацию о будущей версии
FOR /f %%i IN ('git rev-list --count --all') DO SET REVISION=%%i
FOR /f %%i IN ('git rev-parse --abbrev-ref HEAD') DO SET BRANCH=%%i
FOR /f %%i IN ('git rev-parse --verify HEAD') DO SET HASH=%%i

IF EXIST build RMDIR /Q /S build

cmake . -B build -DVER_REVISION=%REVISION% -DVER_BRANCH=%BRANCH% -DVER_HASH=%HASH%
IF %errorlevel% neq 0 EXIT /b %errorlevel%

cmake --build build --config Release
IF %errorlevel% neq 0 EXIT /b %errorlevel%

IF NOT EXIST deploy MKDIR deploy

COPY /Y build\bin\Release\Migrator.exe deploy
COPY /Y build\bin\Release\Signer.exe deploy
DIR deploy
