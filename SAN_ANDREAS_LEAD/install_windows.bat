@echo off
REM Copie SAN ANDREAS LEAD.vst3 dans le dossier VST3 systeme (executer en administrateur).
set SRC=%~dp0build\SAN_ANDREAS_LEAD_artefacts\Release\VST3\SAN ANDREAS LEAD.vst3
set DST=%CommonProgramFiles%\VST3\SAN ANDREAS LEAD.vst3
if not exist "%SRC%" (echo Compile d abord avec build_windows.bat & pause & exit /b 1)
xcopy /E /I /Y "%SRC%" "%DST%" || (echo Echec : lance ce fichier en administrateur. & pause & exit /b 1)
echo Installe dans %DST%. Dans FL Studio : Options > Manage plugins > Find installed plugins.
pause
