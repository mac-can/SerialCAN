@echo off
set PWD="%~dp0"
pushd
cd /D %PWD%
copy /Y .\Libraries\CANAPI\uvcanslc.vcxproj\Release_dll\u3canslc.dll C:\Windows\SysWOW64
copy /Y .\Libraries\SerialCAN\SerialCAN.vcxproj\Release_dll\uvSerialCAN.dll C:\Windows\SysWOW64
popd
dir C:\Windows\SysWOW64\u*can*.dll
pause
