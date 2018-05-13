@echo off
rmdir /S /Q build
rmdir /S /Q bin
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017 Win64"
pause
