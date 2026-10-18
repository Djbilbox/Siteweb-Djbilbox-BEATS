@echo off
REM Compile HIGHROLLER_GOLD (Release, VST3 + Standalone). Prerequis : Visual Studio 2022 (charge de travail C++) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DHIGHROLLER_GOLD_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target HIGHROLLER_GOLD_VST3 HIGHROLLER_GOLD_Standalone || goto :err
echo.
echo OK : build\HIGHROLLER_GOLD_artefacts\Release\VST3\HIGHROLLER GOLD.vst3
echo Run install_windows.bat (as administrator) to install it.
pause
exit /b 0
:err
echo Build FAILED.
pause
exit /b 1
