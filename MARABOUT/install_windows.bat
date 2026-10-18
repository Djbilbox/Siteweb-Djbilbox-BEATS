@echo off
REM Copies MARABOUT.vst3 into the system VST3 folder (run as administrator).
set SRC=%~dp0build\MARABOUT_artefacts\Release\VST3\MARABOUT.vst3
set DST=%CommonProgramFiles%\VST3\MARABOUT.vst3
if not exist "%SRC%" (echo Build first with build_windows.bat & pause & exit /b 1)
xcopy /E /I /Y "%SRC%" "%DST%" || (echo Failed: run this file as administrator. & pause & exit /b 1)
echo Installed to %DST%. In FL Studio: Options ^> Manage plugins ^> Find installed plugins.
pause
