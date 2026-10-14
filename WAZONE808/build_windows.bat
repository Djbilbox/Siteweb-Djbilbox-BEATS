@echo off
REM Compile WARZONE808 (Release, VST3 + Standalone). Prerequis : Visual Studio 2022 (C++), CMake, Git.
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DWZ_BUILD_TESTS=OFF || goto :err
cmake --build build --config Release --target WARZONE808_VST3 WARZONE808_Standalone || goto :err
echo.
echo OK : build\WARZONE808_artefacts\Release\VST3\WARZONE808.vst3
echo Lance install_windows.bat (en administrateur) pour l'installer.
pause
exit /b 0
:err
echo ECHEC de la compilation.
pause
exit /b 1
