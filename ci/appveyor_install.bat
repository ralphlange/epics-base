:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::     TOOLCHAIN      -  Toolchain Version  [9.0/10.0/11.0/12.0/14.0/cygwin/mingw]
::     CONFIGURATION  -  determines EPICS build   [dynamic/static]
::
:: All command line args are passed to make

Setlocal EnableDelayedExpansion

set OS=64BIT
if "%PLATFORM%"=="x86" set OS=32BIT

echo [INFO] Platform: %OS%

if "%TOOLCHAIN%"=="cygwin" (
    if "%CONFIGURATION%"=="static" (
        echo. >> configure\CONFIG_SITE
        echo "SHARED_LIBRARIES=NO" >> configure\CONFIG_SITE
        echo "STATIC_BUILD=YES" >> configure\CONFIG_SITE
        echo [INFO] EPICS set up for static build
    ) else (
        echo [INFO] EPICS set up for dynamic build
    )
    if "%OS%"=="64BIT" (
::        echo [INFO] Installing Cygwin 64bit
::        cinst cyg-get --dir=C:\tools\cygwin64 || cinst cyg-get --dir=C:\tools\cygwin64
        cinst libreadline-devel --source cygwin
        cinst ncursesw-devel --source cygwin
    ) else (
::        echo [INFO] Installing Cygwin 32bit
::        cinst cyg-get --x86 --dir=C:\tools\cygwin32 || cinst cyg-get --x86 --dir=C:\tools\cygwin32
        cinst libreadline-devel --x86 --source cygwin
        cinst ncursesw-devel --x86 --source cygwin
    )
::    @powershell cyg-get libreadline-devel,ncursesw-devel
)

if "%TOOLCHAIN%"=="mingw" (
    if "%CONFIGURATION%"=="static" (
        echo. >> configure\CONFIG_SITE
        echo SHARED_LIBRARIES=NO >> configure\CONFIG_SITE
        echo STATIC_BUILD=YES >> configure\CONFIG_SITE
        echo [INFO] EPICS set up for static build
    ) else (
        echo [INFO] EPICS set up for dynamic build
    )
    if "%OS%"=="64BIT" (
        echo [INFO] Installing MinGW 64bit
        cinst mingw || cinst mingw
    ) else (
        echo [INFO] Installing MinGW 32bit
        cinst mingw --x86 || cinst mingw --x86
    )
)
