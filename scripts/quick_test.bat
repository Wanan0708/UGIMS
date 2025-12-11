@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
echo ========================================
echo UGIMS 管网可视化快速测试
echo ========================================
echo.

cd /d "%~dp0.."

REM 设置颜色（Windows 10+）
set "GREEN=[92m"
set "RED=[91m"
set "YELLOW=[93m"
set "NC=[0m"

echo 🔍 快速环境检查...
echo.

REM 1. 检查PostgreSQL
echo [1/6] PostgreSQL服务...
sc query | findstr /C:"postgresql" >nul 2>&1
if errorlevel 1 (
    echo %RED%❌ PostgreSQL服务未运行%NC%
    echo.
    echo 请先启动PostgreSQL:
    echo   net start postgresql-x64-16
    echo.
    set /p CONTINUE="是否继续（仅测试地图功能）? (Y/N): "
    if /i not "!CONTINUE!"=="Y" exit /b 1
    set DB_AVAILABLE=0
) else (
    echo %GREEN%✅ PostgreSQL运行中%NC%
    set DB_AVAILABLE=1
)
echo.

REM 2. 检查配置
echo [2/6] 配置文件...
if not exist "config\database.ini" (
    echo %RED%❌ config\database.ini 不存在%NC%
    set DB_AVAILABLE=0
) else (
    echo %GREEN%✅ 配置文件存在%NC%
)
echo.

REM 3. 检查数据库（如果PostgreSQL可用）
if !DB_AVAILABLE!==1 (
    echo [3/6] ugims数据库...
    psql -U postgres -d postgres -c "\l" 2>nul | findstr "ugims" >nul
    if errorlevel 1 (
        echo %YELLOW%⚠️  ugims数据库不存在%NC%
        set /p CREATE="是否创建数据库? (Y/N): "
        if /i "!CREATE!"=="Y" (
            echo 创建数据库...
            psql -U postgres -c "CREATE DATABASE ugims;" 2>nul
            psql -U postgres -d ugims -c "CREATE EXTENSION postgis;" 2>nul
            
            if exist "database\schema.sql" (
                echo 导入结构...
                psql -U postgres -d ugims -f database\schema.sql 2>nul
            )
            if exist "database\test_data.sql" (
                echo 导入数据...
                psql -U postgres -d ugims -f database\test_data.sql 2>nul
            )
            echo %GREEN%✅ 数据库创建完成%NC%
        ) else (
            set DB_AVAILABLE=0
        )
    ) else (
        echo %GREEN%✅ 数据库存在%NC%
        
        REM 检查数据
        psql -U postgres -d ugims -c "SELECT COUNT(*) FROM pipelines;" 2>nul | findstr "[0-9]" >nul
        if errorlevel 1 (
            echo %YELLOW%⚠️  数据表为空或不存在%NC%
            if exist "database\schema.sql" (
                echo 导入数据...
                psql -U postgres -d ugims -f database\schema.sql 2>nul
                psql -U postgres -d ugims -f database\test_data.sql 2>nul
            )
        )
    )
) else (
    echo [3/6] 数据库... %YELLOW%跳过（PostgreSQL不可用）%NC%
)
echo.

REM 4. 检查可执行文件
echo [4/6] 可执行文件...
if not exist "release\UGIMS.exe" (
    echo %RED%❌ release\UGIMS.exe 不存在%NC%
    echo.
    echo 请先编译:
    echo   qmake
    echo   mingw32-make -j4
    pause
    exit /b 1
) else (
    echo %GREEN%✅ 可执行文件存在%NC%
)
echo.

REM 5. 检查地图缓存
echo [5/6] 地图瓦片...
if not exist "tilemap" (
    echo %YELLOW%⚠️  tilemap目录不存在（首次运行会自动创建）%NC%
) else (
    dir /s /b tilemap\*.png 2>nul | find /c ".png" > temp_tile_count.txt
    set /p TILE_COUNT=<temp_tile_count.txt
    del temp_tile_count.txt 2>nul
    if !TILE_COUNT! GTR 0 (
        echo %GREEN%✅ 已缓存 !TILE_COUNT! 个瓦片%NC%
    ) else (
        echo %YELLOW%⚠️  瓦片缓存为空（首次运行会下载）%NC%
    )
)
echo.

REM 6. 准备日志目录
echo [6/6] 日志目录...
if not exist "logs" mkdir logs
echo %GREEN%✅ 已准备%NC%
echo.

echo ========================================
echo 环境检查完成
echo ========================================
echo.

REM 显示状态摘要
echo 📊 功能状态:
if !DB_AVAILABLE!==1 (
    echo   - 地图功能: %GREEN%✅ 可用%NC%
    echo   - 管网可视化: %GREEN%✅ 可用%NC%
) else (
    echo   - 地图功能: %GREEN%✅ 可用%NC%
    echo   - 管网可视化: %YELLOW%⚠️  不可用（数据库未配置）%NC%
)
echo.

echo 🚀 启动程序...
echo.
echo ----------------------------------------
echo.

REM 启动程序
start "" release\UGIMS.exe

echo 程序已在后台启动
echo.
echo 💡 提示:
echo   - 窗口应该在2秒内出现
echo   - 查看状态栏了解加载进度
echo   - 查看日志: logs\app.log
echo.

if !DB_AVAILABLE!==1 (
    echo   - 管网数据加载需要5-10秒
    echo   - 缩放到北京天安门附近查看测试数据
    echo   - 坐标: 116.4074°E, 39.9042°N
) else (
    echo   - 仅地图功能可用
    echo   - 要启用管网可视化，请先配置PostgreSQL
    echo   - 参考: docs\启用管网可视化指南.md
)

echo.
echo ========================================
pause

