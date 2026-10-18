@echo off
REM Build OCCITANIE (Release, VST3 + Standalone). Requirements: Visual Studio 2022 (C++ workload) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DOCCITANIE_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target OCCITANIE_VST3 OCCITANIE_Standalone || goto :err
echo.
echo OK : build\OCCITANIE_artefacts\Release\VST3\OCCITANIE.vst3
echo Run install_windows.bat (as administrator) to install it.
pause
exit /b 0
:err
echo Build FAILED.
pause
exit /b 1
