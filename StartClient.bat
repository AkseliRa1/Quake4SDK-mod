@echo off
@REM Runs a client game.
set Q4ModName=xanzcoop
set Q4InstallPath=
set CWD=%cd%
cd /d "%Q4InstallPath%"
start "" "%Q4InstallPath%\Quake4.exe" +set com_allowConsole 1 +set logfile 2 +set fs_game %Q4ModName% +set win_allowmultipleInstances 1 +set fs_savepath "%Q4InstallPath%\client\" +connect localhost
cd /d "%CWD%"