@echo off
echo ===================================

if not exist bin mkdir bin

windres src/resource.rc -O coff -o bin/resource.o
if %errorlevel% neq 0 (
    echo [ERR] Khong the bien dich src/resource.rc! Hay kiem tra lai file manifest.
    pause
    exit /b %errorlevel%
)

g++ -std=c++17 src/main.cpp bin/resource.o ^
    -I"%CD%/lib/wxWidgets/include" ^
    -I"%CD%/lib/wxWidgets/lib/gcc_lib/mswu" ^
    -L"%CD%/lib/wxWidgets/lib/gcc_lib" ^
    -lwxmsw33u_core -lwxbase33u ^
    -lwxtiff -lwxjpeg -lwxpng -lwxzlib ^
    -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm ^
    -lshell32 -lole32 -loleaut32 -luuid -lrpcrt4 ^
    -ladvapi32 -lversion -lws2_32 -luxtheme -lshlwapi ^
    -lgdiplus -loleacc -lmsimg32 -lcomctl32 ^
    -static -static-libgcc -static-libstdc++ ^
    -mwindows ^
    -o bin/photoblack.exe

if %errorlevel% neq 0 (
    echo.
    echo [ERR]
    pause
    exit /b %errorlevel%
)

echo.
echo ===================================
start bin/photoblack.exe