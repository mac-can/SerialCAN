rem Requires administrator rights!
@echo off
set PWD="%~dp0"
pushd
cd /D %PWD%
copy /Y .\Binaries\x86\u3canslc.dll C:\Windows\SysWOW64
copy /Y .\Binaries\x86\uvSerialCAN.dll C:\Windows\SysWOW64
popd
dir C:\Windows\SysWOW64\u*can*.dll
pause
