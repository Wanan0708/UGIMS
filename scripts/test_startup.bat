@echo off
chcp 65001 >nul
echo ========================================
echo 测试程序启动（带详细输出）
echo ========================================
echo.

echo 🔍 先检查是否有残留进程...
tasklist /FI "IMAGENAME eq CustomTitleBarApp.exe" 2>nul | find /I "CustomTitleBarApp.exe" >nul
if %errorlevel% equ 0 (
    echo ⚠️  发现残留进程，正在清理...
    taskkill /F /IM CustomTitleBarApp.exe >nul 2>&1
    timeout /t 1 /nobreak >nul
)
echo ✅ 无残留进程
echo.

set EXE_PATH=release\CustomTitleBarApp.exe

if not exist "%EXE_PATH%" (
    set EXE_PATH=build\Desktop_Qt_6_8_1_MinGW_64_bit-Debug\debug\CustomTitleBarApp.exe
)

if not exist "%EXE_PATH%" (
    echo ❌ 找不到可执行文件
    pause
    exit /b 1
)

echo 📂 可执行文件: %EXE_PATH%
echo.

echo 🚀 启动程序（带控制台输出）...
echo ========================================
echo.

REM 设置Qt调试环境变量
set QT_DEBUG_PLUGINS=1
set QT_LOGGING_RULES=*.debug=true

REM 在当前窗口运行（可以看到qDebug输出）
"%EXE_PATH%"

echo.
echo ========================================
echo 程序已退出
echo 退出代码: %errorlevel%
echo.

if exist "logs\app.log" (
    echo 📝 日志内容:
    echo ========================================
    type "logs\app.log"
    echo ========================================
)

pause

