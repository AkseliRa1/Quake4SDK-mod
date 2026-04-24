@echo off
@REM Starts a client and server.
set Q4ModName=xanzcoop
set Q4InstallPath=
set CWD=%cd%
cd %Q4InstallPath%
start "" "%Q4InstallPath%\Quake4.exe" +set com_allowConsole 1 +set logfile 2 +set fs_game %Q4ModName% +set si_pure 0 +set developer 1 +set win_allowmultipleInstances 1 +set fs_savepath "%Q4InstallPath%\host\" +set si_map mp/q4dm1 +spawnserver
start "" "%Q4InstallPath%\Quake4.exe" +set com_allowConsole 1 +set logfile 2 +set fs_game %Q4ModName% +set win_allowmultipleInstances 1 +set fs_savepath "%Q4InstallPath%\client\" +connect localhost
cd %CWD%