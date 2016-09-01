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
    )
)

if "%TOOLCHAIN%"=="mingw" (
    echo TOOLCHAIN = MinGW
    if "%CONFIGURATION%"=="static" (
        echo CONFIG = static
        echo. >> configure\CONFIG_SITE
        echo SHARED_LIBRARIES=NO >> configure\CONFIG_SITE
        echo STATIC_BUILD=YES >> configure\CONFIG_SITE
    )
    if "%OS%"=="64BIT" (
        echo [INFO] Installing MinGW (64bit)
        cinst mingw || cinst mingw
    ) else (
        echo [INFO] Installing MinGW (32bit)
        cinst mingw --x86 || cinst mingw --x86
    )
)
