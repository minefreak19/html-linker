@echo off
cd ./src
@echo on
gcc -std=c11 -pedantic -o ../bin/run.exe *.c
@echo off
cd ..
pause
