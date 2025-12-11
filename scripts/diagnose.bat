@echo off
chcp 65001 >nul
echo ========================================
echo UGIMS 启动问题诊断工具
echo ========================================
echo.

echo [1/6] 检查可执行文件...
set EXE_DEBUG=build\Desktop_Qt_6_8_1_MinGW_64_bit-Debug\debug\UGIMS.exe
set EXE_RELEASE=release\UGIMS.exe

if exist "%EXE_DEBUG%" (
    echo ✅ Debug版本存在: %EXE_DEBUG%
    for %%F in ("%EXE_DEBUG%") do echo    大小: %%~zF 字节
) else (
    echo ❌ Debug版本不存在: %EXE_DEBUG%
)

if exist "%EXE_RELEASE%" (
    echo ✅ Release版本存在: %EXE_RELEASE%
    for %%F in ("%EXE_RELEASE%") do echo    大小: %%~zF 字节
) else (
    echo ❌ Release版本不存在: %EXE_RELEASE%
)

echo.
echo [2/6] 检查Qt DLL...
if exist "%EXE_DEBUG%" (
    if exist "build\Desktop_Qt_6_8_1_MinGW_64_bit-Debug\debug\Qt6Core.dll" (
        echo ✅ Debug版本Qt依赖已部署
    ) else (
        echo ❌ Debug版本缺少Qt DLL
        echo    运行: cd build\Desktop_Qt_6_8_1_MinGW_64_bit-Debug\debug
        echo          windeployqt UGIMS.exe
    )
)

if exist "%EXE_RELEASE%" (
    if exist "release\Qt6Core.dll" (
        echo ✅ Release版本Qt依赖已部署
    ) else (
        echo ❌ Release版本缺少Qt DLL
        echo    运行: cd release
        echo          windeployqt UGIMS.exe
    )
)

echo.
echo [3/6] 检查配置文件...
if exist "config\app.ini" (
    echo ✅ config\app.ini 存在
) else (
    echo ❌ config\app.ini 不存在
)

if exist "config\database.ini" (
    echo ✅ config\database.ini 存在
) else (
    echo ⚠️  config\database.ini 不存在（可选）
)

if exist "style.qss" (
    echo ✅ style.qss 存在
) else (
    echo ❌ style.qss 不存在
)

echo.
echo [4/6] 尝试运行程序（查看错误）...
echo ========================================

if exist "%EXE_RELEASE%" (
    echo 测试 Release 版本...
    echo.
    "%EXE_RELEASE%" 2>&1
    echo.
    echo 退出代码: %ERRORLEVEL%
) else if exist "%EXE_DEBUG%" (
    echo 测试 Debug 版本...
    echo.
    "%EXE_DEBUG%" 2>&1
    echo.
    echo 退出代码: %ERRORLEVEL%
) else (
    echo ❌ 没有可执行文件可测试
)

echo ========================================
echo.

echo [5/6] 检查日志文件...
if exist "logs\app.log" (
    echo ✅ logs\app.log 存在
    for %%F in ("logs\app.log") do (
        if %%~zF GTR 0 (
            echo    大小: %%~zF 字节
            echo.
            echo === 日志内容 ===
            type "logs\app.log"
            echo === 日志结束 ===
        ) else (
            echo    ❌ 文件为空（程序未启动到Logger初始化）
        )
    )
) else (
    echo ❌ 日志文件不存在
)

echo.
echo [6/6] 检查依赖DLL（使用where命令）...
where Qt6Core.dll 2>nul
if errorlevel 1 (
    echo ⚠️  Qt6Core.dll 不在系统PATH中
    echo    这是正常的，程序应从本地目录加载
) else (
    echo ✅ 找到Qt6Core.dll在PATH中
)

echo.
echo ========================================
echo 诊断完成
echo ========================================
echo.
echo 💡 常见问题解决:
echo.
echo 问题1: 程序无响应且没有错误
echo   → 缺少Qt DLL
echo   → 解决: 运行 windeployqt
echo.
echo 问题2: 日志文件为空
echo   → 程序在Logger初始化前崩溃
echo   → 检查config文件是否存在
echo   → 检查是否缺少PostgreSQL DLL (libpq.dll)
echo.
echo 问题3: Debug版本编译失败
echo   → 在Qt Creator中查看编译错误
echo   → 或运行: mingw32-make clean
echo            mingw32-make 2^>^&1 ^| more
echo.
pause

