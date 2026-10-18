@echo off
REM Build MIDNIGHT_REELS (Release, VST3 + Standalone). Requirements: Visual Studio 2022 (C++ workload) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DMIDNIGHT_REELS_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target MIDNIGHT_REELS_VST3 MIDNIGHT_REELS_Standalone || goto :err
echo.
echo OK : build\MIDNIGHT_REELS_artefacts\Release\VST3\MIDNIGHT REELS.vst3
echo Run install_windows.bat (as administrator) to install it.
pause
exit /b 0
:err
echo Build FAILED.
pause
exit /b 1
