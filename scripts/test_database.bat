@echo off
chcp 65001 >nul
echo ========================================
echo PostgreSQL 数据库环境测试
echo ========================================
echo.

echo [1/5] 检查PostgreSQL服务...
sc query | findstr /C:"postgresql" >nul
if errorlevel 1 (
    echo ❌ PostgreSQL服务未找到
    echo.
    echo 请确认PostgreSQL已安装
    echo 常见服务名：
    echo   - postgresql-x64-16
    echo   - postgresql-x64-15
    echo   - PostgreSQL
    pause
    exit /b 1
) else (
    echo ✅ PostgreSQL服务已找到
    sc query | findstr /C:"postgresql"
)
echo.

echo [2/5] 检查psql命令...
where psql >nul 2>&1
if errorlevel 1 (
    echo ⚠️  psql命令不在PATH中
    echo 尝试常见安装路径...
    if exist "C:\Program Files\PostgreSQL\16\bin\psql.exe" (
        set "PSQL_PATH=C:\Program Files\PostgreSQL\16\bin\psql.exe"
        echo ✅ 找到: !PSQL_PATH!
    ) else if exist "C:\Program Files\PostgreSQL\15\bin\psql.exe" (
        set "PSQL_PATH=C:\Program Files\PostgreSQL\15\bin\psql.exe"
        echo ✅ 找到: !PSQL_PATH!
    ) else (
        echo ❌ 未找到psql
        pause
        exit /b 1
    )
) else (
    echo ✅ psql命令可用
    psql --version
)
echo.

echo [3/5] 检查配置文件...
if exist "config\database.ini" (
    echo ✅ config\database.ini 存在
    echo.
    echo 配置内容:
    type config\database.ini | findstr /V "^#" | findstr /V "^$"
) else (
    echo ❌ config\database.ini 不存在
    pause
    exit /b 1
)
echo.

echo [4/5] 测试数据库连接...
echo 请输入PostgreSQL密码（如果提示）
echo.

REM 尝试连接数据库
psql -U postgres -d postgres -c "SELECT version();" 2>nul
if errorlevel 1 (
    echo ❌ 无法连接到PostgreSQL
    echo.
    echo 请检查：
    echo   1. PostgreSQL服务是否运行
    echo   2. 密码是否正确
    echo   3. 端口是否正确（默认5432）
    pause
    exit /b 1
) else (
    echo ✅ 数据库连接成功
)
echo.

echo [5/5] 检查ugims数据库...
psql -U postgres -d postgres -c "\l" 2>nul | findstr "ugims" >nul
if errorlevel 1 (
    echo ⚠️  ugims数据库不存在
    echo.
    set /p CREATE_DB="是否创建数据库? (Y/N): "
    if /i "!CREATE_DB!"=="Y" (
        echo 创建数据库中...
        psql -U postgres -c "CREATE DATABASE ugims ENCODING 'UTF8';"
        if errorlevel 1 (
            echo ❌ 创建失败
        ) else (
            echo ✅ 数据库创建成功
            
            echo 启用PostGIS扩展...
            psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS postgis;"
            
            echo 导入数据库结构...
            if exist "database\schema.sql" (
                psql -U postgres -d ugims -f database\schema.sql
                echo ✅ 结构导入完成
            )
            
            echo 导入测试数据...
            if exist "database\test_data.sql" (
                psql -U postgres -d ugims -f database\test_data.sql
                echo ✅ 测试数据导入完成
            )
        )
    )
) else (
    echo ✅ ugims数据库存在
    
    echo 检查PostGIS扩展...
    psql -U postgres -d ugims -c "SELECT PostGIS_Version();" 2>nul | findstr "POSTGIS" >nul
    if errorlevel 1 (
        echo ⚠️  PostGIS未启用
        echo 启用PostGIS...
        psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS postgis;"
    ) else (
        echo ✅ PostGIS已启用
    )
    
    echo 检查数据表...
    psql -U postgres -d ugims -c "\dt" 2>nul | findstr "pipelines" >nul
    if errorlevel 1 (
        echo ⚠️  数据表不存在
        echo 导入数据库结构...
        if exist "database\schema.sql" (
            psql -U postgres -d ugims -f database\schema.sql
            echo ✅ 结构导入完成
        )
        if exist "database\test_data.sql" (
            psql -U postgres -d ugims -f database\test_data.sql
            echo ✅ 测试数据导入完成
        )
    ) else (
        echo ✅ 数据表存在
        
        echo 检查数据...
        echo.
        echo === 管线数据 ===
        psql -U postgres -d ugims -c "SELECT pipeline_type, COUNT(*) FROM pipelines GROUP BY pipeline_type;"
        echo.
        echo === 设施数据 ===
        psql -U postgres -d ugims -c "SELECT facility_type, COUNT(*) FROM facilities GROUP BY facility_type;"
    )
)
echo.

echo ========================================
echo 数据库环境检查完成
echo ========================================
echo.
echo 如果所有检查都通过，可以启动程序测试管网可视化
echo.
pause

