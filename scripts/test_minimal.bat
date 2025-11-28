@echo off
chcp 65001 >nul
echo ========================================
echo 编译并运行最小测试程序
echo ========================================
echo.

echo [1/3] 编译测试程序...
g++ -o test_minimal.exe test_minimal.cpp ^
    -IE:\Softwore\Qt\6.8.1\mingw_64\include ^
    -IE:\Softwore\Qt\6.8.1\mingw_64\include\QtCore ^
    -IE:\Softwore\Qt\6.8.1\mingw_64\include\QtGui ^
    -IE:\Softwore\Qt\6.8.1\mingw_64\include\QtWidgets ^
    -LE:\Softwore\Qt\6.8.1\mingw_64\lib ^
    -lQt6Core -lQt6Gui -lQt6Widgets ^
    -std=c++17

if errorlevel 1 (
    echo ❌ 编译失败
    pause
    exit /b 1
)

echo ✅ 编译成功
echo.

echo [2/3] 部署Qt依赖...
windeployqt test_minimal.exe --no-translations

echo.
echo [3/3] 运行测试程序...
echo ========================================
echo.

start "" test_minimal.exe

echo 测试程序已启动！
echo.
echo 如果看到窗口：
echo   ✅ Qt环境正常，问题在主程序代码中
echo.
echo 如果没有看到窗口：
echo   ❌ Qt环境有问题或缺少DLL
echo.
timeout /t 3 /nobreak >nul

