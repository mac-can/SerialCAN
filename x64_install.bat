@echo off
set PWD="%~dp0"
pushd
cd /D %PWD%
copy /Y .\Libraries\CANAPI\uvcanslc.vcxproj\x64\Release_dll\u3canslc.dll C:\Windows\System32
copy /Y .\Libraries\SerialCAN\SerialCAN.vcxproj\x64\Release_dll\uvSerialCAN.dll C:\Windows\System32
popd
dir C:\Windows\System32\u*can*.dll
pause
