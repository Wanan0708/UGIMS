@echo off
chcp 65001 >nul
echo ========================================
echo 详细启动测试（控制台输出）
echo ========================================
echo.

cd /d "%~dp0.."

if not exist "release\CustomTitleBarApp.exe" (
    echo ❌ 可执行文件不存在
    echo 请先编译: qmake ^&^& mingw32-make
    pause
    exit /b 1
)

echo ✅ 可执行文件存在
echo.
echo 正在启动程序...
echo 【注意查看控制台输出】
echo.
echo ----------------------------------------
echo.

REM 设置Qt环境（如果需要）
REM set PATH=E:\Softwore\Qt\6.8.1\mingw_64\bin;%PATH%

REM 从项目根目录运行，保持控制台窗口
cd release
CustomTitleBarApp.exe

echo.
echo ----------------------------------------
echo.
echo 程序已退出
echo.
pause

