@echo off
@REM Starts the game. Typically used for singleplayer mods.
set Q4ModName=xanzcoop
set Q4InstallPath=D:\SteamLibrary\steamapps\common\Quake 4
set CWD=%cd%
cd /d "%Q4InstallPath%"
start "Quake 4" "%Q4InstallPath%\Quake4.exe" +set com_allowConsole 1  +set fs_game %Q4ModName% +set developer 1 +disconnect  +set fs_savepath "%Q4InstallPath%\client\"  +set si_gameType singleplayer +editor
cd /d "%CWD%"