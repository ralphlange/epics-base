:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::     TOOLCHAIN      -  Visual Studio Toolchain Version  [9.0/10.0/11.0/12.0/14.0]
::     CONFIGURATION  -  determines EPICS build   [dynamic/static]
::
:: All command line args are passed to make

set "ST="
if /i "%CONFIGURATION%"=="static" set ST=-static

set OS=64BIT
if "%PLATFORM%"=="x86" set OS=32BIT

set "VSINSTALL=C:\Program Files (x86)\Microsoft Visual Studio %TOOLCHAIN%"

if "%OS%"=="64BIT" (
    set EPICS_HOST_ARCH=windows-x64%ST%
    if exist "%VSINSTALL%\VC\bin\amd64\vcvars64.bat" (
        call "%VSINSTALL%\VC\bin\amd64\vcvars64.bat"
    ) else (
        set "INCLUDE=%VSINSTALL%\VC\include;%INCLUDE%"
        set "PATH=%VSINSTALL%\VC\bin\amd64;%PATH%"
    )
) else (
    set EPICS_HOST_ARCH=win32-x86%ST%
    if exist "%VSINSTALL%\Common7\Tools\vsvars32.bat" (
        call "%VSINSTALL%\Common7\Tools\vsvars32.bat"
    ) else (
        set "INCLUDE=%VSINSTALL%\VC\include;%INCLUDE%"
        set "PATH=%VSINSTALL%\VC\bin;%PATH%"
    )
)

echo [INFO] Platform: %OS%
echo [INFO] Microsoft Visual Studio Toolchain %TOOLCHAIN%
echo [INFO] Compiler Version
cl
echo [INFO] EPICS_HOST_ARCH: %EPICS_HOST_ARCH%

C:\MinGW\bin\mingw32-make %*
