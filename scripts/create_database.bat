@echo off
chcp 65001 >nul
echo ========================================
echo 创建 UGIMS 数据库
echo ========================================
echo.

cd /d "%~dp0.."

echo [1/5] 检查 PostgreSQL 连接...
echo 请输入 PostgreSQL 密码（用户：postgres）
echo.

REM 测试连接
psql -U postgres -d postgres -c "SELECT version();" >nul 2>&1
if errorlevel 1 (
    echo ❌ 无法连接到 PostgreSQL
    echo.
    echo 请检查：
    echo   1. PostgreSQL 服务是否运行
    echo   2. 密码是否正确
    pause
    exit /b 1
)

echo ✅ PostgreSQL 连接成功
echo.

echo [2/5] 检查 ugims 数据库是否存在...
psql -U postgres -d postgres -c "\l" 2>nul | findstr "ugims" >nul
if errorlevel 1 (
    echo ⚠️  ugims 数据库不存在，准备创建...
    
    echo.
    echo [3/5] 创建 ugims 数据库...
    psql -U postgres -c "CREATE DATABASE ugims ENCODING 'UTF8';"
    
    if errorlevel 1 (
        echo ❌ 创建数据库失败
        pause
        exit /b 1
    )
    echo ✅ 数据库创建成功
    
) else (
    echo ✅ ugims 数据库已存在
    
    set /p RECREATE="是否要重新创建数据库（会删除所有数据）? (Y/N): "
    if /i "%RECREATE%"=="Y" (
        echo 删除旧数据库...
        psql -U postgres -c "DROP DATABASE IF EXISTS ugims;"
        echo 创建新数据库...
        psql -U postgres -c "CREATE DATABASE ugims ENCODING 'UTF8';"
        echo ✅ 数据库已重新创建
    ) else (
        echo 跳过数据库创建
        goto :SKIP_CREATE
    )
)
echo.

:SKIP_CREATE
echo [4/5] 启用 PostGIS 扩展...
psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS postgis;"
if errorlevel 1 (
    echo ❌ 启用 PostGIS 失败
    pause
    exit /b 1
)
echo ✅ PostGIS 扩展已启用
echo.

psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\";"
echo ✅ UUID 扩展已启用
echo.

echo [5/5] 导入数据库结构...
if exist "database\schema.sql" (
    psql -U postgres -d ugims -f database\schema.sql
    if errorlevel 1 (
        echo ⚠️  结构导入失败（可能部分成功）
    ) else (
        echo ✅ 数据库结构导入成功
    )
) else (
    echo ⚠️  database\schema.sql 不存在
)
echo.

echo [6/6] 导入测试数据...
if exist "database\test_data.sql" (
    psql -U postgres -d ugims -f database\test_data.sql
    if errorlevel 1 (
        echo ⚠️  测试数据导入失败（可能部分成功）
    ) else (
        echo ✅ 测试数据导入成功
    )
) else (
    echo ⚠️  database\test_data.sql 不存在
)
echo.

echo ========================================
echo 数据库创建完成
echo ========================================
echo.

echo 验证数据...
echo.
echo === 数据表 ===
psql -U postgres -d ugims -c "\dt"
echo.
echo === 管线数据统计 ===
psql -U postgres -d ugims -c "SELECT pipeline_type, COUNT(*) as count FROM pipelines GROUP BY pipeline_type;"
echo.
echo === 设施数据统计 ===
psql -U postgres -d ugims -c "SELECT facility_type, COUNT(*) as count FROM facilities GROUP BY facility_type;"
echo.

echo ========================================
echo 现在可以启动程序测试管网可视化了！
echo ========================================
pause

