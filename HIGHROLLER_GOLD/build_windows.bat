@echo off
REM Compile HIGHROLLER_GOLD (Release, VST3 + Standalone). Prerequis : Visual Studio 2022 (charge de travail C++) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DHIGHROLLER_GOLD_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target HIGHROLLER_GOLD_VST3 HIGHROLLER_GOLD_Standalone || goto :err
echo.
echo OK : build\HIGHROLLER_GOLD_artefacts\Release\VST3\HIGHROLLER_GOLD.vst3
echo Lance install_windows.bat (en administrateur) pour l installer.
pause
exit /b 0
:err
echo ECHEC de la compilation.
pause
exit /b 1
