@echo off
chcp 65001 >nul
echo ==========================================
echo 导入简化测试数据（不依赖 PostGIS）
echo ==========================================
echo.

echo [步骤 1/2] 创建简化表结构...
psql -U postgres -d ugims -f "database\schema_simple.sql"
if %ERRORLEVEL% NEQ 0 (
    echo ❌ 表结构创建失败
    pause
    exit /b 1
)
echo ✅ 表结构创建完成
echo.

echo [步骤 2/2] 导入测试数据...
psql -U postgres -d ugims -f "database\test_data_simple.sql"
if %ERRORLEVEL% NEQ 0 (
    echo ❌ 测试数据导入失败
    pause
    exit /b 1
)
echo ✅ 测试数据导入完成
echo.

echo ==========================================
echo 数据导入完成！
echo ==========================================
echo.
echo ⚠️  注意：当前使用的是简化版数据结构
echo    - 管线使用 GeoJSON 文本存储（非标准 GEOMETRY 类型）
echo    - 设施使用经纬度字段（非 POINT 类型）
echo.
echo 💡 要使用完整功能，请安装 PostGIS：
echo    https://postgis.net/windows_downloads/
echo.
pause

