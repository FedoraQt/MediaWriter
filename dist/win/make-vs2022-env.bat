@ECHO OFF

set OLDPATH=%PATH%

set VCARCH=%1
if "%VCARCH%"=="" set VCARCH=x64

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %VCARCH% > NUL:

echo export INCLUDE='%INCLUDE%'
echo export LIB='%LIB%'
echo export LIBPATH='%LIBPATH%'

call set NEWPATH=%%PATH:%OLDPATH%=%%
set NEWPATH=%NEWPATH:C:=/c%
set NEWPATH=%NEWPATH:\=/%
set NEWPATH=%NEWPATH:;=:%
echo export PATH="%NEWPATH%:$PATH"
