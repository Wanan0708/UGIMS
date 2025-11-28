@echo off
chcp 65001 >nul
echo ========================================
echo 路径诊断工具
echo ========================================
echo.

echo [1/5] 检查项目结构...
if exist "CustomTitleBarApp.pro" (
    echo ✅ 当前在项目根目录
    echo 根目录: %CD%
) else (
    echo ❌ 未在项目根目录
    cd ..
    if exist "CustomTitleBarApp.pro" (
        echo ✅ 找到项目根目录: %CD%
    ) else (
        echo ❌ 无法找到项目根目录
        pause
        exit /b 1
    )
)
echo.

echo [2/5] 检查资源文件...
if exist "resources\images\OrangeCat.png" (
    echo ✅ resources\images\OrangeCat.png 存在
) else (
    echo ❌ resources\images\OrangeCat.png 不存在
)

if exist "resources\styles\style.qss" (
    echo ✅ resources\styles\style.qss 存在
) else (
    echo ❌ resources\styles\style.qss 不存在
)
echo.

echo [3/5] 检查瓦片地图目录...
if exist "tilemap" (
    echo ✅ tilemap\ 目录存在
    dir /s /b tilemap\*.png 2>nul | find /c ".png" > temp_count.txt
    set /p TILE_COUNT=<temp_count.txt
    del temp_count.txt
    echo    包含 %TILE_COUNT% 个瓦片文件
) else (
    echo ⚠️  tilemap\ 目录不存在（首次运行会自动创建）
)
echo.

echo [4/5] 检查可执行文件...
if exist "release\CustomTitleBarApp.exe" (
    echo ✅ release\CustomTitleBarApp.exe 存在
    echo    大小: 
    dir /b release\CustomTitleBarApp.exe | find "CustomTitleBarApp.exe"
) else (
    echo ❌ release\CustomTitleBarApp.exe 不存在
    echo    请先编译项目
)
echo.

echo [5/5] 检查日志目录...
if not exist "logs" mkdir logs
echo ✅ logs\ 目录已准备
echo.

echo ========================================
echo 诊断完成
echo ========================================
echo.
echo 现在启动程序（按Ctrl+C取消）...
timeout /t 3
echo.

if exist "release\CustomTitleBarApp.exe" (
    echo 启动中...
    cd release
    start CustomTitleBarApp.exe
    cd ..
    echo.
    echo 程序已启动！
    echo 如果遇到问题，请查看:
    echo   - logs\app.log
    echo   - 控制台输出
) else (
    echo 可执行文件不存在，无法启动
)

echo.
pause

