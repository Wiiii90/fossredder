@echo off
REM Builds the project in a clean x64 MSVC/Ninja environment.
REM Run this from a normal CMD (not PowerShell) or double-click in Explorer.

REM Try to locate Visual Studio installation via vswhere (preferred)
set VSINSTALL=
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath 2^>nul`) do set VSINSTALL=%%i
if defined VSINSTALL (
  set VSPATH="%VSINSTALL%\Common7\Tools\VsDevCmd.bat"
) else (
  REM Fallback to common location; adjust if your VS is installed elsewhere
  set VSPATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
)

if not exist %VSPATH% (
  echo VsDevCmd not found at %VSPATH%
  echo Please install Visual Studio or update scripts\build_win_x64.bat to point to your VsDevCmd.bat
  exit /b 1
)

:: Initialize Visual Studio x64 environment
call %VSPATH% -arch=amd64 -host_arch=amd64
if errorlevel 1 (
  echo Failed to initialize VS developer environment
  exit /b 1
)

echo === Tool locations ===
where cl
where rc
where mt
where cmake
where ninja
echo ======================

:: Paths (adjust if your repo is located elsewhere)
set SRC_DIR=P:\fossredder
set BUILD_DIR=P:\fossredder\build\x64-Debug-vcpkg
set VCPKG_ROOT=P:\fossredder\third_party\vcpkg
set VCPKG_INSTALLED=P:\fossredder\vcpkg_installed

:: Remove previous build dir if present
if exist "%BUILD_DIR%" (
  rmdir /s /q "%BUILD_DIR%"
)

:: Configure
"C:\Program Files\CMake\bin\cmake.exe" -S "%SRC_DIR%" -B "%BUILD_DIR%" -G "Ninja" -DCMAKE_MAKE_PROGRAM="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" -DCMAKE_C_COMPILER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe" -DCMAKE_CXX_COMPILER="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.44.35207/bin/Hostx64/x64/cl.exe" -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET="x64-windows" -DVCPKG_INSTALLED_DIR="%VCPKG_INSTALLED%" -DVCPKG_APPLOCAL_DEPS=ON
if errorlevel 1 (
  echo Configure failed
  exit /b 1
)

:: Build serially to surface first error clearly
"C:\Program Files\CMake\bin\cmake.exe" --build "%BUILD_DIR%" --config Debug -- -v -j1
if errorlevel 1 (
  echo Build failed
  exit /b 1
)

:: Ensure Qt6Quick debug DLL is available in the app bin (helps plugin load failures when a debug QtQuick DLL wasn't copied)
set QTQUICK_DEBUG_SRC=%VCPKG_INSTALLED%\debug\bin\Qt6Quickd.dll
if exist "%QTQUICK_DEBUG_SRC%" (
  echo Copying Qt6Quickd.dll from %QTQUICK_DEBUG_SRC% to %BUILD_DIR%\bin\
  copy /Y "%QTQUICK_DEBUG_SRC%" "%BUILD_DIR%\bin\" >nul
) else (
  set QTQUICK_RELEASE_SRC=%VCPKG_INSTALLED%\bin\Qt6Quickd.dll
  if exist "%QTQUICK_RELEASE_SRC%" (
    echo Copying Qt6Quickd.dll from %QTQUICK_RELEASE_SRC% to %BUILD_DIR%\bin\
    copy /Y "%QTQUICK_RELEASE_SRC%" "%BUILD_DIR%\bin\" >nul
  ) else (
    echo Qt6Quickd.dll not found in vcpkg installed tree; skipping copy
  )
)

echo Build succeeded.
exit /b 0
