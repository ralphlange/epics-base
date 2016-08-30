:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::     TOOLCHAIN      -  Visual Studio Toolchain Version  [9.0/10.0/11.0/12.0/14.0]
::     CONFIGURATION  -  determines EPICS build   [dynamic/static]
::
:: All command line args are passed to make

set "ST="
if /i "%CONFIGURATION%"=="static" set ST=-static

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if "%OS%"=="64BIT" (
    set EPICS_HOST_ARCH=windows-x64%ST%
    set "PATH=C:\Program Files (x86)\Microsoft Visual Studio %TOOLCHAIN%\VC\bin\amd64;%PATH%"
) else (
    set EPICS_HOST_ARCH=win32-x86%ST%
    if exist "C:\Program Files (x86)\Microsoft Visual Studio %TOOLCHAIN%\Common7\Tools\vsvars32.bat" call "C:\Program Files (x86)\Microsoft Visual Studio %TOOLCHAIN%\Common7\Tools\vsvars32.bat"
)

echo [INFO] Platform: %OS%
echo [INFO] Microsoft Visual Studio Toolchain %TOOLCHAIN%
echo [INFO] Compiler Version
cl
echo [INFO] EPICS_HOST_ARCH: %EPICS_HOST_ARCH%

rem C:\MinGW\bin\mingw32-make %*
