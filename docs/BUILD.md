# Building CoreMiner from Source

## Table of Contents

- [Building CoreMiner from Source](#building-coreminer-from-source)
  - [Table of Contents](#table-of-contents)
  - [Requirements](#requirements)
    - [Common](#common)
    - [Linux](#linux)
    - [macOS](#macos)
    - [Windows](#windows)
  - [Instructions](#instructions)
    - [Windows-specific Script](#windows-specific-script)
  - [CMake Configuration Options](#cmake-configuration-options)
  - [Disable Hunter](#disable-hunter)

## Requirements

This project uses [CMake] and [Hunter] package manager.

### Common

1. [CMake] >= 3.5
2. [Git](https://git-scm.com/downloads)
3. [Perl](https://www.perl.org/get.html) (required for OpenSSL build)

### Linux

1. GCC version >= 4.8
2. DBUS development libraries (if building with `-DETHDBUS`). For Ubuntu:

   ```bash
   sudo apt install libdbus-1-dev
   ```

### macOS

1. GCC version >= TBF

### Windows

1. [Visual Studio 2017](https://www.visualstudio.com/downloads/) (Community Edition is sufficient)
   > **Note**: Make sure you install MSVC 2015 toolkit (v140).

## Instructions

1. Update git submodules:

   ```bash
   git submodule update --init --recursive
   ```

2. Create a build directory:

   ```bash
   mkdir build
   cd build
   ```

3. Configure the project with CMake. See [configuration options](#cmake-configuration-options) for additional settings:

   ```bash
   cmake ..
   ```

   For Windows:

   ```bash
   cmake .. -G "Visual Studio 15 2017 Win64"
   # If you encounter build errors, try:
   cmake .. -G "Visual Studio 15 2017 Win64" -T v140
   ```

4. Build the project using CMake Build Tool Mode:

   ```bash
   cmake --build .
   ```

   > **Note**: On Windows, specify the build config if you encounter compiler issues:

   ```bash
   cmake --build . --config Release
   ```

5. _(Optional, Linux only)_ Install the built executable:

   ```bash
   sudo make install
   ```

### Windows-specific Script

Here's a complete Windows batch file example. **Adapt it to your system**. The script assumes:

- It's placed one folder up from the CoreMiner source folder
- CMake is installed
- Perl is installed

```batch
@echo off
setlocal

rem Add MSVC to PATH
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\Common7\Tools\VsMSBuildCmd.bat"

rem Add Perl to PATH (needed for OpenSSL build)
set "PERL_PATH=C:\Perl\perl\bin"
set "PATH=%PERL_PATH%;%PATH%"

rem Switch to CoreMiner's source folder
cd "%~dp0\coreminer\"

if not exist "build\" mkdir "build\"

cmake -G "Visual Studio 15 2017 Win64" -H. -Bbuild -DAPICORE=ON ..
cmake --build . --config Release --target package

endlocal
pause
```

## CMake Configuration Options

Add these options to your CMake configuration command:

- `-DAPICORE=ON` - Enable API Server (default: ON)
- `-DETHDBUS=ON` - Enable D-Bus support (default: OFF)

## Disable Hunter

To install dependencies manually or use your system's package manager, disable Hunter by adding `-DHUNTER_ENABLED=OFF` to your CMake configuration options.

[CMake]: https://cmake.org/
[Hunter]: https://docs.hunter.sh/
