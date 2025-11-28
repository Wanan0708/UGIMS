@echo off
chcp 65001 >nul
echo ========================================
echo 启动 UGIMS 城市地下管网智能管理系统
echo ========================================
echo.

echo [1/3] 检查目录结构...
if not exist "config" (
    echo ❌ 错误: config 目录不存在
    pause
    exit /b 1
)

if not exist "config\app.ini" (
    echo ❌ 错误: config\app.ini 文件不存在
    pause
    exit /b 1
)

if not exist "logs" (
    echo ℹ️  创建 logs 目录...
    mkdir logs
)

echo ✅ 目录结构正常
echo.

echo [2/3] 检查可执行文件...
if not exist "release\CustomTitleBarApp.exe" (
    echo ❌ 错误: release\CustomTitleBarApp.exe 不存在
    echo 请先编译项目
    pause
    exit /b 1
)
echo ✅ 可执行文件存在
echo.

echo [3/3] 启动程序...
echo ========================================
echo.

start "" "%~dp0release\CustomTitleBarApp.exe"

echo.
echo 程序已启动！
echo.
echo 如果程序没有显示窗口，请查看：
echo   - logs\app.log （运行日志）
echo   - 是否有错误弹窗
echo.

timeout /t 3 /nobreak >nul
exit /b 0

