@echo OFF
del test.exe
gcc -g test.c ..\src\csv.c -o test.exe
test.exe