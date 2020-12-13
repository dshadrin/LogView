@echo off
set PATH=c:\msys64\mingw64\bin;%PATH%
rmdir /S /Q build
mkdir build
cd build
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE:STRING="Release"
pause
mingw32-make install

