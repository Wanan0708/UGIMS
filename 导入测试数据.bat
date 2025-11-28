@echo off
chcp 65001 >nul
echo ========================================
echo 导入测试管网数据到 PostgreSQL
echo ========================================
echo.

echo [步骤 1/4] 创建数据库（如果不存在）...
psql -U postgres -c "CREATE DATABASE ugims;" 2>nul
if %ERRORLEVEL% EQU 0 (
    echo ✅ 数据库创建成功
) else (
    echo ℹ️  数据库可能已存在
)
echo.

echo [步骤 2/4] 启用 PostGIS 扩展...
psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS postgis;"
echo.

echo [步骤 3/4] 创建表结构...
if exist "database\schema.sql" (
    psql -U postgres -d ugims -f "database\schema.sql"
    echo ✅ 表结构创建完成
) else (
    echo ❌ 找不到 database\schema.sql
    pause
    exit /b 1
)
echo.

echo [步骤 4/4] 导入测试数据...
if exist "database\test_data.sql" (
    psql -U postgres -d ugims -f "database\test_data.sql"
    echo ✅ 测试数据导入完成
) else (
    echo ❌ 找不到 database\test_data.sql
    pause
    exit /b 1
)
echo.

echo ========================================
echo 检查导入结果...
echo ========================================
psql -U postgres -d ugims -c "SELECT COUNT(*) as pipeline_count FROM pipelines;"
psql -U postgres -d ugims -c "SELECT COUNT(*) as facility_count FROM facilities;"
echo.

echo ========================================
echo 数据导入完成！现在可以重新启动程序了。
echo ========================================
pause

