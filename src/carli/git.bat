@echo off

echo #ifndef GIT_H>git.h~
if ERRORLEVEL 1 exit /B 1
echo #define GIT_MODIFIED \>>git.h~
if ERRORLEVEL 1 exit /B 2
"C:\Program Files (x86)\Git\bin\git.exe" status | grep -c modified>>git.h~
REM if ERRORLEVEL 1 exit /B 3
echo #define GIT_REVISION \>>git.h~
if ERRORLEVEL 1 exit /B 4
"C:\Program Files (x86)\Git\bin\git.exe" log -n 1 | head -n 1 | sed "s/commit //">>git.h~
if ERRORLEVEL 1 exit /B 5
echo #define GIT_STR_STR(x) #x>>git.h~
if ERRORLEVEL 1 exit /B 6
echo #define GIT_STR(x) GIT_STR_STR(x)>>git.h~
if ERRORLEVEL 1 exit /B 7
echo #define GIT_MODIFIED_STR GIT_STR(GIT_MODIFIED)>>git.h~
if ERRORLEVEL 1 exit /B 8
echo #define GIT_REVISION_STR GIT_STR(GIT_REVISION)>>git.h~
if ERRORLEVEL 1 exit /B 9
echo #endif>>git.h~
if ERRORLEVEL 1 exit /B 10
diff git.h git.h~ > nul || cp git.h~ git.h
if ERRORLEVEL 1 exit /B 11
