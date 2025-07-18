@echo off
setlocal enabledelayedexpansion

echo [INFO] 正在初始化 include 目录...
if not exist "include" (
    mkdir include
)

cd include

echo [INFO] 克隆 vcpkg 仓库...
if not exist "vcpkg" (
    git clone https://github.com/microsoft/vcpkg.git
)

cd vcpkg

echo [INFO] 正在引导 vcpkg 工具链...
call bootstrap-vcpkg.bat

cd ../..

echo.
echo [SUCCESS] 项目初始化完成！
echo.

echo 构建项目的推荐指令如下（请根据需要在命令行中执行）:
echo ----------------------------------------
echo cmake --preset vs2022-debug
echo cmake --build --preset debug
echo ----------------------------------------

echo.
pause
