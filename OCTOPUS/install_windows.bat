@echo off
REM Copy OCTOPUS.vst3 to the system VST3 folder (run as administrator).
set SRC=%~dp0build\OCTOPUS_artefacts\Release\VST3\OCTOPUS.vst3
set DST=%CommonProgramFiles%\VST3\OCTOPUS.vst3
if not exist "%SRC%" (echo Build first with build_windows.bat & pause & exit /b 1)
xcopy /E /I /Y "%SRC%" "%DST%" || (echo Failed: run this file as administrator. & pause & exit /b 1)
echo Installed to %DST%. In FL Studio: Options ^> Manage plugins ^> Find installed plugins.
pause
