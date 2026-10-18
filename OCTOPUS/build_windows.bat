@echo off
REM Build OCTOPUS (Release, VST3 + Standalone). Requirements: Visual Studio 2022 (C++ workload) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DOCTOPUS_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target OCTOPUS_VST3 OCTOPUS_Standalone || goto :err
echo.
echo OK : build\OCTOPUS_artefacts\Release\VST3\OCTOPUS.vst3
echo Run install_windows.bat (as administrator) to install it.
pause
exit /b 0
:err
echo Build FAILED.
pause
exit /b 1
