@echo off
cls
echo Running test...

@echo on
.\bin\run.exe ./test/src/index.html -o ./test/out/index.html %*
@echo off

pause
