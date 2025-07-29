@echo off

REM Check if curl is installed
echo Checking if curl is installed...
where curl >nul
if errorlevel 1 (
    echo Error: Curl is not installed or not in the PATH.
    pause
    exit /b 1
)

REM Define target directory
set "TARGET_DIR=cheat\engine\sdk\offsets\static"
echo Updating files in "%TARGET_DIR%"...

REM Ensure target directory exists
echo Checking if target directory exists...
if not exist "%TARGET_DIR%" (
    echo Error: Target directory does not exist: "%TARGET_DIR%"
    pause
    exit /b 1
)

REM Download the files
echo Downloading client_dll.hpp...
curl -sSL https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.hpp -o "%TARGET_DIR%\client_dll.hpp"
if errorlevel 1 (
    echo Error: Failed to download client_dll.hpp
    pause
    exit /b 1
)

echo Downloading offsets.hpp...
curl -sSL https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.hpp -o "%TARGET_DIR%\offsets.hpp"
if errorlevel 1 (
    echo Error: Failed to download offsets.hpp
    pause
    exit /b 1
)

echo Files updated successfully.
pause
