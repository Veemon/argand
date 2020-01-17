@echo off
setlocal
pushd "%~dp0"
if not exist "build" mkdir build
cd build

set s=..\src\
set l=..\dep\
set i=/I %l%

set SOURCE=%s%argand.cpp

set INCLUDES=

set LIBRARIES=kernel32.lib gdi32.lib shell32.lib msvcrt.lib libcmt.lib user32.lib

if "%1" == "-t"     goto TESTING
if "%1" == "--test" goto TESTING

if "%1" == "-d"      goto DEBUGGING
if "%1" == "--debug" goto DEBUGGING

set ARGS=/O2
goto COMPILE

:TESTING
    echo [Tests Enabled]
    echo.
    set ARGS=/Zi /DTESTING_ENABLE
    goto COMPILE
:DEBUGGING
    echo [Debug Enabled]
    echo.
    set ARGS=/Zi /DDEBUG_ENABLE
    goto COMPILE

:COMPILE
cl %ARGS% /Feargand.exe %INCLUDES% %SOURCE% /link %LIBRARIES% /SUBSYSTEM:CONSOLE
if ERRORLEVEL 1 (
	popd
	exit /b 1
)

move argand.exe ..

popd
exit /b 0
