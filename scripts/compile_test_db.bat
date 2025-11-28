@echo off
chcp 65001 >nul
echo ========================================
echo 编译数据库连接测试程序
echo ========================================
echo.

cd /d "%~dp0.."

echo [1/3] 编译测试程序...
g++ -o test_db_connection.exe test_db_connection.cpp ^
    src/core/common/logger.cpp ^
    src/core/common/config.cpp ^
    src/core/database/databasemanager.cpp ^
    -Isrc ^
    -IE:/Softwore/Qt/6.8.1/mingw_64/include ^
    -IE:/Softwore/Qt/6.8.1/mingw_64/include/QtCore ^
    -IE:/Softwore/Qt/6.8.1/mingw_64/include/QtSql ^
    -LE:/Softwore/Qt/6.8.1/mingw_64/lib ^
    -lQt6Core -lQt6Sql ^
    -std=c++17 ^
    -fexceptions

if errorlevel 1 (
    echo ❌ 编译失败
    pause
    exit /b 1
)

echo ✅ 编译成功
echo.

echo [2/3] 准备运行环境...
if not exist "logs" mkdir logs
echo ✅ 日志目录已准备
echo.

echo [3/3] 运行测试程序...
echo.
echo ----------------------------------------
test_db_connection.exe
echo ----------------------------------------
echo.

pause

