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

if /I "%1"=="Release" (
    set "BUILD_DIR=!BUILD_ROOT_DIR!\build_release"
    set "BUILD_TYPE=Release"
) else (
    set "BUILD_DIR=!BUILD_ROOT_DIR!\build_debug"
    set "BUILD_TYPE=Debug"
)
set "EXECUTABLE_NAME=CRUX.exe"

set "CMAKE_DIR=%PROJECT_ROOT%\.CRUX\cmake"
if not exist "!CMAKE_DIR!\CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found in "!CMAKE_DIR!".
    exit /b 1
)

if not exist "!BUILD_DIR!" (
    mkdir "!BUILD_DIR!"
)

echo Configuring project with CMake...
if defined VCPKG_ROOT (
    set "TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
    echo Using VCPKG toolchain file: "!TOOLCHAIN_FILE!"
    set "TOOLCHAIN_ARG=-DCMAKE_TOOLCHAIN_FILE=!TOOLCHAIN_FILE!"
) else (
    echo VCPKG_ROOT is not defined. Continuing without toolchain file.
    set "TOOLCHAIN_ARG="
)

cmake -S "!CMAKE_DIR!" -B "!BUILD_DIR!" ^
    -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=!BUILD_TYPE! ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    !TOOLCHAIN_ARG!

if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

echo Building the project...
cmake --build "!BUILD_DIR!" --config !BUILD_TYPE!
if errorlevel 1 (
    echo ERROR: Build failed.
    exit /b 1
)

if exist "!BUILD_DIR!\!EXECUTABLE_NAME!" (
    "!BUILD_DIR!\!EXECUTABLE_NAME!"
) else (
    echo ERROR: Could not find the executable "!EXECUTABLE_NAME!" in "!BUILD_DIR!".
    echo Searching for .exe files in the build directory:
    dir /s "!BUILD_DIR!\*.exe"
    exit /b 1
)

endlocal
