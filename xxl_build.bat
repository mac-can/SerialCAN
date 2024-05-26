@echo off
rem set MSBuild environment variables
pushd
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" x86
popd
set PWD="%~dp0"
pushd

rem build libraries, utilities and the trial program
call .\x64_build.bat NOVARS
if not errorlevel 0 goto end
call .\x86_build.bat NOVARS
if not errorlevel 0 goto end

rem rem build test suites
rem cd /D .\Tests
rem call .\x64_build.bat NOVARS NORUN
rem if not errorlevel 0 goto end
rem call .\x86_build.bat NOVARS NORUN
rem if not errorlevel 0 goto end

rem end of the job
:end
popd
