@echo off
@REM Starts a client and server.
set Q4ModName=xanzcoop
set Q4InstallPath=D:\SteamLibrary\steamapps\common\Quake 4
set CWD=%cd%
cd /d %Q4InstallPath%
start "" "%Q4InstallPath%\Quake4.exe" +set com_allowConsole 1 +set logfile 2 +set fs_game xanzcoop +set si_pure 0 +set developer 1 +set win_allowmultipleInstances 1 +set fs_savepath "D:\SteamLibrary\steamapps\common\Quake 4\host\" +set si_map pukke_mp +set si_gameType "Co-op" +spawnserver
start "" "%Q4InstallPath%\Quake4.exe" +set com_allowConsole 1 +set fs_game xanzcoop +set win_allowmultipleInstances 1 +set fs_savepath "D:\SteamLibrary\steamapps\common\Quake 4\client\" +connect localhost
cd /d %CWD%