@echo off
chcp 65001 >nul
echo ==========================================
echo 诊断管网显示问题
echo ==========================================
echo.

echo [1/4] 检查数据库中的数据...
echo.
psql -U postgres -d ugims -c "SELECT COUNT(*) as pipeline_count FROM pipelines;"
psql -U postgres -d ugims -c "SELECT COUNT(*) as facility_count FROM facilities;"
psql -U postgres -d ugims -c "SELECT pipeline_type, COUNT(*) FROM pipelines GROUP BY pipeline_type;"
echo.

echo [2/4] 检查管线坐标范围...
echo.
psql -U postgres -d ugims -c "SELECT pipeline_id, ST_AsText(geom) FROM pipelines LIMIT 3;"
echo.

echo [3/4] 检查设施坐标...
echo.
psql -U postgres -d ugims -c "SELECT facility_id, ST_AsText(geom) FROM facilities;"
echo.

echo [4/4] 检查应用日志（最后50行）...
echo.
if exist "logs\app.log" (
    powershell -Command "Get-Content logs\app.log -Tail 50 | Select-String -Pattern 'Pipeline|Facility|Render'"
) else (
    echo logs\app.log not found
)
echo.

echo ==========================================
echo 诊断完成！
echo ==========================================
echo.
echo 请检查：
echo 1. 数据库中是否有 6 条管线和 3 个设施？
echo 2. 坐标是否在 116.39-116.42, 39.90-39.92 范围内？
echo 3. 日志中是否有 "Found X pipelines" 消息？
echo.
pause

