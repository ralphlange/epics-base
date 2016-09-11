:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::     TOOLCHAIN      -  Toolchain Version  [9.0/10.0/11.0/12.0/14.0/cygwin/mingw]
::     CONFIGURATION  -  determines EPICS build   [dynamic/static, -debug]
::
:: All command line args are passed to make

Setlocal EnableDelayedExpansion

set OS=64BIT
if "%PLATFORM%"=="x86" set OS=32BIT

echo [INFO] Platform: %OS%

if "%TOOLCHAIN%"=="cygwin" (
    echo.%CONFIGURATION% | findstr /C:"static">nul && (
        echo SHARED_LIBRARIES=NO>> configure\CONFIG_SITE
        echo STATIC_BUILD=YES>> configure\CONFIG_SITE
        echo [INFO] EPICS set up for static build
    ) || (
        echo [INFO] EPICS set up for dynamic build
    )
    echo.%CONFIGURATION% | findstr /C:"debug">nul && (
        echo HOST_OPT=NO>> configure\CONFIG_SITE
        echo [INFO] EPICS set up for debug build
    ) || (
        echo [INFO] EPICS set up for optimized build
    )
    if "%OS%"=="64BIT" (
        echo [INFO] Installing dependencies
        C:\cygwin64\setup-x86_64.exe -q -P "libreadline-devel,libncursesw-devel"
    ) else (
        echo [INFO] Installing dependencies
        C:\cygwin\setup-x86.exe -q -P "libreadline-devel,libncursesw-devel"
    )
)

if "%TOOLCHAIN%"=="mingw" (
    echo.%CONFIGURATION% | findstr /C:"static">nul && (
        echo SHARED_LIBRARIES=NO>> configure\CONFIG_SITE
        echo STATIC_BUILD=YES>> configure\CONFIG_SITE
        echo [INFO] EPICS set up for static build
    ) || (
        echo [INFO] EPICS set up for dynamic build
    )
    echo.%CONFIGURATION% | findstr /C:"debug">nul && (
        echo HOST_OPT=NO>> configure\CONFIG_SITE
        echo [INFO] EPICS set up for debug build
    ) || (
        echo [INFO] EPICS set up for optimized build
    )
    if "%OS%"=="64BIT" (
        echo [INFO] Installing MinGW 64bit
        cinst mingw || cinst mingw
    ) else (
        echo [INFO] Installing MinGW 32bit
        cinst mingw --x86 || cinst mingw --x86
    )
)

echo [INFO] Installing Make 4.1
@powershell -Command "(new-object net.webclient).DownloadFile('https://www.aps.anl.gov/epics/download/tools/make-4.1-win64.zip', 'C:\tools\make-4.1.zip')"
cd \tools
"C:\Program Files\7-Zip\7z" e make-4.1.zip
