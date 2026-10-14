@echo off
REM Compile OCTOPUS (Release, VST3 + Standalone). Prerequis : Visual Studio 2022 (charge de travail C++) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DOCTOPUS_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target OCTOPUS_VST3 OCTOPUS_Standalone || goto :err
echo.
echo OK : build\OCTOPUS_artefacts\Release\VST3\OCTOPUS.vst3
echo Lance install_windows.bat (en administrateur) pour l installer.
pause
exit /b 0
:err
echo ECHEC de la compilation.
pause
exit /b 1
