@echo off
chcp 65001 >nul
echo ========================================
echo 启动 UGIMS (Debug版本)
echo ========================================
echo.

set EXE_PATH=build\Desktop_Qt_6_8_1_MinGW_64_bit-Debug\debug\CustomTitleBarApp.exe

if not exist "%EXE_PATH%" (
    echo ❌ 错误: 找不到可执行文件
    echo 路径: %EXE_PATH%
    echo.
    echo 请在 Qt Creator 中编译项目
    pause
    exit /b 1
)

echo ✅ 找到可执行文件
echo 路径: %EXE_PATH%
echo.

if not exist "config" (
    echo ⚠️  警告: config 目录不存在，创建中...
    mkdir config
)

if not exist "logs" (
    mkdir logs
)

echo 🚀 启动程序...
echo.
start "" "%EXE_PATH%"

echo 程序已启动！
echo.
echo 📝 提示:
echo   - 日志文件: logs\app.log
echo   - 配置文件: config\database.ini
echo   - 如果没有数据库，程序仍可运行（瓦片地图功能可用）
echo.
timeout /t 3 /nobreak >nul

