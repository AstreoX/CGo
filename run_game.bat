@echo off
REM 设置DLL搜索路径
set PATH=%~dp0libs;%PATH%

REM 运行游戏
echo 正在启动围棋游戏...
start cgogame.exe

exit 