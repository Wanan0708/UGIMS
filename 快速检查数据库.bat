@echo off
chcp 65001 >nul
echo ========================================
echo 检查 PostgreSQL 数据库中的管网数据
echo ========================================
echo.

echo [1/3] 检查数据库是否存在...
psql -U postgres -lqt | findstr /C:"ugims"
if %ERRORLEVEL% EQU 0 (
    echo ✅ 数据库 ugims 存在
) else (
    echo ❌ 数据库 ugims 不存在！
    echo.
    echo 请先创建数据库：
    echo   psql -U postgres -c "CREATE DATABASE ugims;"
    echo   psql -U postgres -d ugims -c "CREATE EXTENSION postgis;"
    pause
    exit /b 1
)
echo.

echo [2/3] 检查表是否存在...
psql -U postgres -d ugims -c "\dt"
echo.

echo [3/3] 检查管网数据数量...
psql -U postgres -d ugims -c "SELECT COUNT(*) as pipeline_count FROM pipelines;"
psql -U postgres -d ugims -c "SELECT COUNT(*) as facility_count FROM facilities;"
echo.

echo [4/3] 检查管网类型分布...
psql -U postgres -d ugims -c "SELECT pipeline_type, COUNT(*) as count FROM pipelines GROUP BY pipeline_type;"
echo.

echo ========================================
echo 检查完成！
echo ========================================
pause

