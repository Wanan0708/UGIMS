@echo off
chcp 65001 >nul
echo ========================================
echo 编译 UGIMS Release 版本
echo ========================================
echo.

echo [1/5] 清理旧的编译文件...
if exist "release" (
    rmdir /S /Q release 2>nul
    echo ✅ 已清理 release 目录
)
if exist "Makefile" (
    del /Q Makefile* 2>nul
)

echo.
echo [2/5] 运行 qmake 生成 Makefile...
qmake CustomTitleBarApp.pro -spec win32-g++ "CONFIG+=release"

if errorlevel 1 (
    echo ❌ qmake 失败
    pause
    exit /b 1
)
echo ✅ Makefile 生成成功

echo.
echo [3/5] 编译项目 (这可能需要几分钟)...
mingw32-make -j4

if errorlevel 1 (
    echo ❌ 编译失败
    pause
    exit /b 1
)
echo ✅ 编译成功

echo.
echo [4/5] 部署 Qt 依赖...
cd release
windeployqt CustomTitleBarApp.exe --no-translations
cd ..
echo ✅ Qt 依赖部署完成

echo.
echo [5/5] 检查文件...
if exist "release\CustomTitleBarApp.exe" (
    echo ✅ 可执行文件: release\CustomTitleBarApp.exe
    
    for %%F in (release\CustomTitleBarApp.exe) do (
        echo    大小: %%~zF 字节
    )
) else (
    echo ❌ 错误: 找不到可执行文件
    pause
    exit /b 1
)

echo.
echo ========================================
echo 🎉 Release 版本编译完成！
echo ========================================
echo.
echo 运行方式:
echo   1. 双击: release\CustomTitleBarApp.exe
echo   2. 或运行: run_release.bat
echo.
pause

