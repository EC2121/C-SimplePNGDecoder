@echo off

if exist "bin" (
    RD /S /Q "bin"
)

MD "bin"
clang.exe -I .\SDL2-2.26.0\include\ -o source\PNGDecoder.exe .\source\main.c -L .\SDL2-2.26.0\lib\x64\ .\zlib\zlibwapi.lib -lSDL2