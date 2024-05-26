@echo off

set VCVARS="True"
set TRIAL="True"
set LIBDB="True"
set UTILS="True"

rem parse arguments: [NOVARS] [NOTRIAL] [NODEBUG] [NOUTILS]
:LOOP
if "%1" == "NOVARS" set VCVARS="False"
if "%1" == "NOTRIAL" set TRIAL="False"
if "%1" == "NODEBUG" set LIBDB="False"
if "%1" == "NOUTILS" set UTILS="False"
SHIFT
if not "%1" == "" goto LOOP

rem set MSBuild environment variables
if %VCVARS% == "True" (
   pushd
   call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" x86
   popd
)
pushd

rem generate a pseudo build number
call build_no.bat

rem build the trial program
if %TRIAL% == "True" ( 
   call msbuild.exe .\Trial\slc_test.vcxproj /t:Clean;Build /p:"Configuration=Debug";"Platform=Win32"
   if errorlevel 1 goto end
)
rem build the CAN API V3 C library (dynamic and static)
call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Release_lib";"Platform=Win32"
if errorlevel 1 goto end

if %LIBDB% == "True" (
   call msbuild.exe .\Libraries\CANAPI\uvcanslc.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=Win32"
   if errorlevel 1 goto end
)
rem build the CAN API V3 C++ library (dynamic and static)
call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Release_dll";"Platform=Win32"
if errorlevel 1 goto end

call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Release_lib";"Platform=Win32"
if errorlevel 1 goto end

if %LIBDB% == "True" (
   call msbuild.exe .\Libraries\SerialCAN\SerialCAN.vcxproj /t:Clean;Build /p:"Configuration=Debug_lib";"Platform=Win32"
   if errorlevel 1 goto end
)
rem copy the arifacts into the Binaries folder
set BIN=.\Binaries
if not exist %BIN% mkdir %BIN%
set BIN=%BIN%\x86
if not exist %BIN% mkdir %BIN%
echo Copying dynamic libraries...
copy /Y .\Libraries\CANAPI\Release_dll\u3canslc.dll %BIN%
copy /Y .\Libraries\CANAPI\Release_dll\u3canslc.exp %BIN%
copy /Y .\Libraries\CANAPI\Release_dll\u3canslc.lib %BIN%
copy /Y .\Libraries\CANAPI\Release_dll\u3canslc.pdb %BIN%
copy /Y .\Libraries\SerialCAN\Release_dll\uvSerialCAN.dll %BIN%
copy /Y .\Libraries\SerialCAN\Release_dll\uvSerialCAN.exp %BIN%
copy /Y .\Libraries\SerialCAN\Release_dll\uvSerialCAN.lib %BIN%
copy /Y .\Libraries\SerialCAN\Release_dll\uvSerialCAN.pdb %BIN%
set BIN=%BIN%\lib
if not exist %BIN% mkdir %BIN%
echo Copying static libraries...
copy /Y .\Libraries\CANAPI\Release_lib\u3canslc.lib %BIN%
copy /Y .\Libraries\CANAPI\Release_lib\u3canslc.pdb %BIN%
copy /Y .\Libraries\SerialCAN\Release_lib\uvSerialCAN.lib %BIN%
copy /Y .\Libraries\SerialCAN\Release_lib\uvSerialCAN.pdb %BIN%
echo "Static libraries (x86)" > %BIN%\readme.txt
set BIN=%BIN%\Debug
if %LIBDB% == "True" (
   echo Copying static debug libraries...
   if not exist %BIN% mkdir %BIN%
   copy /Y .\Libraries\CANAPI\Debug_lib\u3canslc.lib %BIN%
   copy /Y .\Libraries\CANAPI\Debug_lib\u3canslc.pdb %BIN%
   copy /Y .\Libraries\CANAPI\Debug_lib\u3canslc.idb %BIN%
   copy /Y .\Libraries\SerialCAN\Debug_lib\uvSerialCAN.lib %BIN%
   copy /Y .\Libraries\SerialCAN\Debug_lib\uvSerialCAN.pdb %BIN%
   copy /Y .\Libraries\SerialCAN\Debug_lib\uvSerialCAN.idb %BIN%
   echo "Static debug libraries (x86)" > %BIN%\readme.txt
)
rem build the utilities 'can_moni' and 'can_test'
if %UTILS% == "True" (
   call msbuild.exe .\Utilities\can_moni\can_moni.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=Win32"
   if errorlevel 1 goto end

   call msbuild.exe .\Utilities\can_test\can_test.vcxproj /t:Clean;Build /p:"Configuration=Release";"Platform=Win32"
   if errorlevel 1 goto end
)
set BIN=.\Binaries
if not exist %BIN% mkdir %BIN%
set BIN=%BIN%\x86
if not exist %BIN% mkdir %BIN%
if %UTILS% == "True" (
   echo Copying utilities...
   copy /Y .\Utilities\can_moni\Release\can_moni.exe %BIN%
   copy /Y .\Utilities\can_test\Release\can_test.exe %BIN%
)
rem copy the header files into the Includes folder
echo Copying header files...
set INC=.\Includes
if not exist %INC% mkdir %INC%
copy /Y .\Sources\SerialCAN.h %INC%
copy /Y .\Sources\CANAPI\SerialCAN_Defines.h %INC%
copy /Y .\Sources\CANAPI\CANAPI.h %INC%
copy /Y .\Sources\CANAPI\CANAPI_Types.h %INC%
copy /Y .\Sources\CANAPI\CANAPI_Defines.h %INC%
copy /Y .\Sources\CANAPI\CANBTR_Defaults.h %INC%
copy /Y .\Sources\CANAPI\can_api.h %INC%
copy /Y .\Sources\CANAPI\can_btr.h %INC%

rem end of the job
:end
popd
if %VCVARS% == "True" (
   pause
)
