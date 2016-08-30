:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::     TOOLCHAIN_VERSION  -  Visual Studio Version  [12.0/13.0]
::     CONFIGURATION      -  determines EPICS build   [dynamic/static]
::
:: All command line args are passed to make

set VCVARSALL="C:\Program Files (x86)\Microsoft Visual Studio %TOOLCHAIN_VERSION%\VC\vcvarsall.bat"

set "ST="
if /i "%CONFIGURATION%"=="static" set ST=-static

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if "%OS%"=="64BIT" (
    set EPICS_HOST_ARCH=windows-x64%ST%
    set COMPILER=amd64
) else (
    set EPICS_HOST_ARCH=win32-x86%ST%
    set COMPILER=x86
)
call %VCVARSALL% %COMPILER%
if %ERRORLEVEL% NEQ 0 exit 1

echo [INFO] Microsoft Visual Studio Version %TOOLCHAIN_VERSION%
echo [INFO] Platform: %COMPILER%
echo [INFO] EPICS_HOST_ARCH: %EPICS_HOST_ARCH%

C:\MinGW\bin\mingw32-make %*