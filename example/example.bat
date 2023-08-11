@echo OFF
del ex.exe
gcc -g example.c ..\src\csv.c -o ex.exe
ex.exe