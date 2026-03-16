@echo off
setlocal
cd /d "%~dp0"
if "%TOOLCHAIN%"=="" set TOOLCHAIN=C:\msys64\mingw32\bin
set PATH=%TOOLCHAIN%;%PATH%
echo Using toolchain at "%TOOLCHAIN%"
echo Pass --skip-existing to skip compile commands when matching .o already exists.

if /I "%~1"=="--skip-existing" if exist "C:\Builder Projects\BmpToIcon\resource.o" goto :skip_1
"%TOOLCHAIN%\windres.exe" --target=pe-i386 "resource.rc" "C:\Builder Projects\BmpToIcon\resource.o"
if errorlevel 1 goto :fail
:skip_1
if /I "%~1"=="--skip-existing" if exist "C:\Builder Projects\BmpToIcon\WinMainUnit.o" goto :skip_2
"%TOOLCHAIN%\g++.exe" -c -m32 -O2 -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-deprecated-declarations -Wattributes -Wno-unknown-pragmas -Wno-missing-field-initializers -D_UNICODE -DUNICODE -DNOVTABLE= -D_WIN32_WINNT=0x0501 "WinMainUnit.cpp" -o "C:\Builder Projects\BmpToIcon\WinMainUnit.o"
if errorlevel 1 goto :fail
:skip_2
"%TOOLCHAIN%\g++.exe" -O2 -static -mwindows -m32 -municode "C:\Builder Projects\BmpToIcon\resource.o" "C:\Builder Projects\BmpToIcon\WinMainUnit.o" -o "BmpToIcon.exe" -lstdc++ -lshell32 -luuid -lcomdlg32 -lcomctl32 -lversion -lshlwapi
if errorlevel 1 goto :fail
"%TOOLCHAIN%\g++.exe" -O2 -static -mwindows -m32 -municode "C:\Builder Projects\BmpToIcon\resource.o" "C:\Builder Projects\BmpToIcon\WinMainUnit.o" -o "BmpToIcon.exe" -lstdc++ -lshell32 -luuid -lcomdlg32 -lcomctl32 -lversion -lshlwapi
if errorlevel 1 goto :fail
echo Build finished successfully.
goto :eof
:fail
echo Build failed with error %errorlevel%.
exit /b %errorlevel%
