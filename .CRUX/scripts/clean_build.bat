@echo off
setlocal EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

set "PROJECT_ROOT=%SCRIPT_DIR%\..\.."
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_ROOT=%%~fI"

set "BUILD_ROOT_DIR="
for /d %%D in ("%PROJECT_ROOT%\.*") do (
    if /i not "%%~nxD"=="." if /i not "%%~nxD"==".." (
        set "BUILD_ROOT_DIR=%%~fD\build"
        goto :found_build_root
    )
)

:found_build_root
if not defined BUILD_ROOT_DIR (
    echo ERROR: No hidden build root directory found in "%PROJECT_ROOT%".
    exit /b 1
)

set "DEBUG_BUILD_DIR=!BUILD_ROOT_DIR!\build_debug"

if exist "!DEBUG_BUILD_DIR!" (
    echo Cleaning Debug Build Directory: "!DEBUG_BUILD_DIR!"
    rmdir /s /q "!DEBUG_BUILD_DIR!"
    if exist "!DEBUG_BUILD_DIR!" (
        echo ERROR: Failed to delete Debug Build Directory: "!DEBUG_BUILD_DIR!"
    ) else (
        echo Debug Build Directory cleaned successfully.
    )
) else (
    echo Debug Build Directory does not exist: "!DEBUG_BUILD_DIR!"
)

set "RELEASE_BUILD_DIR=!BUILD_ROOT_DIR!\build_release"

if exist "!RELEASE_BUILD_DIR!" (
    echo Cleaning Release Build Directory: "!RELEASE_BUILD_DIR!"
    rmdir /s /q "!RELEASE_BUILD_DIR!"
    if exist "!RELEASE_BUILD_DIR!" (
        echo ERROR: Failed to delete Release Build Directory: "!RELEASE_BUILD_DIR!"
    ) else (
        echo Release Build Directory cleaned successfully.
    )
) else (
    echo Release Build Directory does not exist: "!RELEASE_BUILD_DIR!"
)

echo Cleanup process completed.

endlocal
