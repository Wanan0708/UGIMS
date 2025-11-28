@echo off
chcp 65001 >nul
echo ==========================================
echo UGIMS Database Initialization
echo ==========================================
echo.

echo [1/2] Creating tables...
psql -U postgres -d ugims -f "database\init_tables.sql"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo X Table creation failed
    pause
    exit /b 1
)
echo.
echo [OK] Tables created successfully
echo.

echo [2/2] Importing test data...
psql -U postgres -d ugims -f "database\import_test_data.sql"
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo X Test data import failed
    pause
    exit /b 1
)
echo.

echo ==========================================
echo [OK] Database initialized successfully!
echo ==========================================
echo.
echo Now you can start the application:
echo   .\release\CustomTitleBarApp.exe
echo.
pause

