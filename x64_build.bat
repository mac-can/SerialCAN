@echo off

rem parse arguments: [[NOVARS] NOTRIAL]
if "%1" == "NOVARS" (
   set VCVARS="False"
) else (
   set VCVARS="True"
)
SHIFT
if "%1" == "NOTRIAL" (
  set TRIAL="False"
) else (
  set TRIAL="True"
)

rem set MSBuild environment variables
if %VCVARS% == "True" (
   pushd
   call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" x64
   popd
)
pushd

rem generate a pseudo build number
call build_no.bat

rem build the trial program
if %TRIAL% == "True" ( 
call msbuild.exe .\Trial\SerialCAN.vcxproj\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Debug";"Platform=x64"
if errorlevel 1 goto end
)
rem build the CAN API V3 C library (dynamic and static)
call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=x64"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=x64"
if errorlevel 1 goto end

rem build the CAN API V3 C++ library (dynamic and static)
call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=x64"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=x64"
if errorlevel 1 goto end

rem copy the arifacts into the Binaries folder
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
copy /Y .\Sources\CANAPI\CANBTR_Defaults.h %INC%
copy /Y .\Sources\CANAPI\can_api.h %INC%
copy /Y .\Sources\CANAPI\can_btr.h %INC%

rem build the utilities 'can_mone' and 'can_test'
rem call msbuild.exe .\Utilities\can_moni\can_moni.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=x64"
rem if errorlevel 1 goto end

rem call msbuild.exe .\Utilities\can_test\can_test.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=x64"
rem if errorlevel 1 goto end

rem copy the utilities into the Binaries folder
echo Copying utilities...
set BIN=".\Binaries"
if not exist %BIN% mkdir %BIN%
set BIN="%BIN%\x64"
if not exist %BIN% mkdir %BIN%
rem copy /Y .\Utilities\can_moni\x64\Release\can_moni.exe %BIN%
rem copy /Y .\Utilities\can_test\x64\Release\can_test.exe %BIN%

rem end of the job
:end
popd
if %VCVARS% == "True" (
   pause
)
