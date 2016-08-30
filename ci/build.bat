:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::     TOOLCHAIN      -  Toolchain Version  [9.0/10.0/11.0/12.0/14.0/cygwin/mingw]
::     CONFIGURATION  -  determines EPICS build   [dynamic/static]
::
:: All command line args are passed to make

set "ST="
if /i "%CONFIGURATION%"=="static" set ST=-static

set OS=64BIT
if "%PLATFORM%"=="x86" set OS=32BIT

echo [INFO] Platform: %OS%

if "%TOOLCHAIN%"=="cygwin" (
    if "%OS%"=="64BIT" (
        set EPICS_HOST_ARCH=cygwin-x86_64
        set "INCLUDE=C:\cygwin64\include;%INCLUDE%"
        set "PATH=C:\cygwin64\bin;%PATH%"
    ) else (
        set EPICS_HOST_ARCH=cygwin-x86
        set "INCLUDE=C:\cygwin\include;%INCLUDE%"
        set "PATH=C:\cygwin\bin;%PATH%"
    )
    echo [INFO] Cygwin Toolchain
    echo [INFO] Compiler Version
    gcc
    goto Finish
)

if "%TOOLCHAIN%"=="mingw" (
    if "%OS%"=="64BIT" (
        set EPICS_HOST_ARCH=windows-x64-mingw
        set "INCLUDE=C:\mingw-w64\i686-5.3.0-posix-dwarf-rt_v4-rev0\mingw32\include;%INCLUDE%"
        set "PATH=C:\mingw-w64\i686-5.3.0-posix-dwarf-rt_v4-rev0\mingw32\bin;%PATH%"
    ) else (
        set EPICS_HOST_ARCH=win32-x86-mingw
        set "INCLUDE=C:\MinGW\include;%INCLUDE%"
        set "PATH=C:\MinGW\bin;%PATH%"
    )
    echo [INFO] MinGW Toolchain
    echo [INFO] Compiler Version
    gcc
    goto Finish
)

set "VSINSTALL=C:\Program Files (x86)\Microsoft Visual Studio %TOOLCHAIN%"

if "%OS%"=="64BIT" (
    set EPICS_HOST_ARCH=windows-x64%ST%
    if exist "%VSINSTALL%\VC\bin\amd64\vcvars64.bat" (
        call "%VSINSTALL%\VC\bin\amd64\vcvars64.bat"
    ) else (
        set "INCLUDE=%VSINSTALL%\VC\include;%INCLUDE%"
        set "PATH=%VSINSTALL%\VC\bin\amd64;%PATH%"
        set "LIB=%VSINSTALL%\VC\lib\amd64;%LIB%"
        set "LIBPATH=%VSINSTALL%\VC\lib\amd64;%LIBPATH%"
    )
) else (
    set EPICS_HOST_ARCH=win32-x86%ST%
    if exist "%VSINSTALL%\Common7\Tools\vsvars32.bat" (
        call "%VSINSTALL%\Common7\Tools\vsvars32.bat"
    ) else (
        set "INCLUDE=%VSINSTALL%\VC\include;%INCLUDE%"
        set "PATH=%VSINSTALL%\VC\bin;%PATH%"
        set "LIB=%VSINSTALL%\VC\lib;%LIB%"
        set "LIBPATH=%VSINSTALL%\VC\lib;%LIBPATH%"
    )
)
echo [INFO] Microsoft Visual Studio Toolchain %TOOLCHAIN%
echo [INFO] Compiler Version
cl

:Finish
echo [INFO] EPICS_HOST_ARCH: %EPICS_HOST_ARCH%

C:\MinGW\bin\mingw32-make %*
