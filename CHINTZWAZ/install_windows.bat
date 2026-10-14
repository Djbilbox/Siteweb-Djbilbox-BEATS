@echo off
REM Copie CHINTZWAZ.vst3 dans le dossier VST3 systeme (executer en administrateur).
set SRC=%~dp0build\CHINTZWAZ_artefacts\Release\VST3\CHINTZWAZ.vst3
set DST=%CommonProgramFiles%\VST3\CHINTZWAZ.vst3
if not exist "%SRC%" (echo Compile d abord avec build_windows.bat & pause & exit /b 1)
xcopy /E /I /Y "%SRC%" "%DST%" || (echo Echec : lance ce fichier en administrateur. & pause & exit /b 1)
echo Installe dans %DST%. Dans FL Studio : Options > Manage plugins > Find installed plugins.
pause
