@echo off
REM Compile K2000 (Release, VST3 + Standalone). Prerequis : Visual Studio 2022 (charge de travail C++) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DK2000_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target K2000_VST3 K2000_Standalone || goto :err
echo.
echo OK : build\K2000_artefacts\Release\VST3\K2000.vst3
echo Run install_windows.bat (as administrator) to install it.
pause
exit /b 0
:err
echo Build FAILED.
pause
exit /b 1
