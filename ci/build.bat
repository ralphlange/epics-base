:: Universal build script for AppVeyor (https://ci.appveyor.com/)
:: Environment:
::             STATIC   -   YES/NO   determines EPICS build
::
:: All command line args are passed to make

set "ST="
if /i "%STATIC%"=="YES" set ST=-static
reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if "%OS%"=="64BIT" (
    set EPICS_HOST_ARCH=windows-x64%ST%
    set COMPILER=amd64
) else (
    set EPICS_HOST_ARCH=win32-x86%ST%
    set COMPILER=x86
)
for /l %%X in (14,-1,8) do (
    if exist "C:\Program Files (x86)\Microsoft Visual Studio %%X.0\VC\vcvarsall.bat" (
        "C:\Program Files (x86)\Microsoft Visual Studio %%X.0\VC\vcvarsall.bat" %COMPILER%
        echo [INFO] Using Microsoft Visual Studio %%X.0
        goto :BUILD
    )
)
:BUILD
echo [INFO] EPICS_HOST_ARCH = %EPICS_HOST_ARCH%
C:\MinGW\bin\mingw32-make %*