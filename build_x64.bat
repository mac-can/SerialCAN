@echo off

pushd
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" x64
popd

pushd
call build_no.bat
rem type .\Sources\build_no.h
rem pause

call msbuild.exe .\Trial\SerialCAN.vcxproj\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Debug";"Platform=x64"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=x64"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=x64"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=x64"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=x64"
if errorlevel 1 goto end

echo Copying artifacts...
set BIN=".\Binaries"
if not exist %BIN% mkdir %BIN%
set BIN="%BIN%\x64"
if not exist %BIN% mkdir %BIN%
copy /Y .\Libraries\CANAPI\uvcanslc.vcxproj\x64\Release_dll\u3canslc.dll %BIN%
copy /Y .\Libraries\CANAPI\uvcanslc.vcxproj\x64\Release_dll\u3canslc.lib %BIN%
copy /Y .\Libraries\SerialCAN\SerialCAN.vcxproj\x64\Release_dll\uvSerialCAN.dll %BIN%
copy /Y .\Libraries\SerialCAN\SerialCAN.vcxproj\x64\Release_dll\uvSerialCAN.lib %BIN%
set BIN="%BIN%\lib"
if not exist %BIN% mkdir %BIN%
copy /Y .\Libraries\CANAPI\uvcanslc.vcxproj\x64\Debug_lib\u3canslc.lib %BIN%
copy /Y .\Libraries\CANAPI\uvcanslc.vcxproj\x64\Debug_lib\u3canslc.pdb %BIN%
copy /Y .\Libraries\SerialCAN\SerialCAN.vcxproj\x64\Debug_lib\uvSerialCAN.lib %BIN%
copy /Y .\Libraries\SerialCAN\SerialCAN.vcxproj\x64\Debug_lib\uvSerialCAN.pdb %BIN%
echo Static libraries (x64) > %BIN%\readme.txt

echo Copying header files...
set INC=".\Includes"
if not exist %INC% mkdir %INC%
copy /Y .\Sources\SerialCAN*.h %INC%
copy /Y .\Sources\CANAPI\CANAPI.h %INC%
copy /Y .\Sources\CANAPI\CANAPI_Types.h %INC%
copy /Y .\Sources\CANAPI\CANAPI_Defines.h %INC%
copy /Y .\Sources\CANAPI\can_api.h %INC%

rem call msbuild.exe .\Utilities\can_moni\can_moni.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=x64"
rem if errorlevel 1 goto end

rem call msbuild.exe .\Utilities\can_test\can_test.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=x64"
rem if errorlevel 1 goto end

echo Copying utilities...
set BIN=".\Binaries"
if not exist %BIN% mkdir %BIN%
set BIN="%BIN%\x86"
if not exist %BIN% mkdir %BIN%
rem copy /Y .\Utilities\can_moni\x64\Release\can_moni.exe %BIN%
rem copy /Y .\Utilities\can_test\x64\Release\can_test.exe %BIN%

:end
popd
pause
