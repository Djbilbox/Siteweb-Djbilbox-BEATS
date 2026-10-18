@echo off
REM Compile STREETLIGHT_TRAP (Release, VST3 + Standalone). Prerequis : Visual Studio 2022 (charge de travail C++) + CMake + Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSTREETLIGHT_TRAP_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target STREETLIGHT_TRAP_VST3 STREETLIGHT_TRAP_Standalone || goto :err
echo.
echo OK : build\STREETLIGHT_TRAP_artefacts\Release\VST3\STREETLIGHT TRAP.vst3
echo Lance install_windows.bat (en administrateur) pour l installer.
pause
exit /b 0
:err
echo ECHEC de la compilation.
pause
exit /b 1
