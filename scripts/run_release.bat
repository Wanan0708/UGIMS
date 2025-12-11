@echo off
chcp 65001 >nul
echo ========================================
echo 启动 UGIMS (Release版本)
echo ========================================
echo.

if not exist "release\UGIMS.exe" (
    echo ❌ 错误: Release 版本不存在
    echo.
    echo 请先编译 Release 版本:
    echo   运行: build_release.bat
    echo.
    echo 或使用 Debug 版本:
    echo   运行: run_debug.bat
    pause
    exit /b 1
)

echo ✅ 找到 Release 版本
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
start "" "%~dp0release\UGIMS.exe"

echo 程序已启动！
echo.
echo 📝 提示:
echo   - 日志文件: logs\app.log
echo   - 配置文件: config\database.ini
echo   - 地图缓存: tilemap\
echo.
timeout /t 3 /nobreak >nul

