# 🔧 脚本工具说明

本目录包含了常用的开发和维护脚本。

## 📋 脚本列表

### 🔨 编译脚本

#### `build_release.bat`
**功能**: 编译Release版本

**用法**:
```bash
cd E:\Project\CursorProject\UGIMS
.\scripts\build_release.bat
```

**步骤**:
1. 清理旧的构建文件
2. 运行qmake生成Makefile
3. 使用mingw32-make编译
4. 输出到`release/`目录

---

### ▶️ 运行脚本

#### `run.bat`
**功能**: 快速运行程序（自动选择可用版本）

**用法**:
```bash
.\scripts\run.bat
```

**逻辑**:
- 优先运行Release版本
- 如果Release不存在，运行Debug版本
- 都不存在则提示先编译

---

#### `run_debug.bat`
**功能**: 运行Debug版本

**用法**:
```bash
.\scripts\run_debug.bat
```

**特点**:
- 包含调试符号
- 可以attach调试器
- 控制台输出详细

---

#### `run_release.bat`
**功能**: 运行Release版本

**用法**:
```bash
.\scripts\run_release.bat
```

**特点**:
- 经过优化
- 启动速度快
- 体积较小

---

### 🔍 诊断脚本

#### `diagnose.bat`
**功能**: 诊断程序运行环境和常见问题

**用法**:
```bash
.\scripts\diagnose.bat
```

**检查项**:
1. ✅ 检查可执行文件是否存在
2. ✅ 运行windeployqt收集依赖DLL
3. ✅ 检查Qt插件目录
4. ✅ 尝试启动程序
5. ✅ 检查日志文件
6. ✅ 检查配置文件

**输出示例**:
```
========================================
UGIMS 诊断工具
========================================

[1/6] 检查可执行文件...
✅ release\UGIMS.exe 存在

[2/6] 收集Qt依赖...
✅ DLL收集完成

[3/6] 检查配置文件...
✅ config\app.ini 存在
✅ config\database.ini 存在

[4/6] 测试启动...
✅ 程序启动成功

[5/6] 检查日志...
最近10条日志:
...

========================================
诊断完成
========================================
```

---

#### `test_startup.bat`
**功能**: 测试程序启动并输出详细控制台信息

**用法**:
```bash
.\scripts\test_startup.bat
```

**特点**:
- 显示启动过程每一步
- 输出`[Pipeline]`调试信息
- 保持控制台窗口打开
- 便于排查启动问题

**输出示例**:
```
[Pipeline] Step 1: Initialize Logger
[Pipeline] ✅ Logger initialized
[Pipeline] Step 2: Load Config
[Pipeline] ✅ App config loaded
[Pipeline] Step 3: Load Database Config
[Pipeline] ✅ Database config loaded
[Pipeline] Step 4: Connect to Database
[Pipeline] Connecting to database (timeout: 5s)...
[Pipeline] ✅ Database connected
```

---

#### `test_minimal.bat`
**功能**: 编译并运行最小化测试程序

**用法**:
```bash
.\scripts\test_minimal.bat
```

**用途**:
- 测试Qt环境是否正常
- 验证编译器和链接器
- 不依赖项目其他模块
- 快速验证环境

**测试程序**:
- 创建简单的QLabel窗口
- 显示"Hello Qt!"
- 无任何依赖

---

## 🛠️ 常见使用场景

### 场景1: 首次编译项目

```bash
# 1. 编译
.\scripts\build_release.bat

# 2. 收集依赖
cd release
windeployqt UGIMS.exe
cd ..

# 3. 运行
.\scripts\run.bat
```

---

### 场景2: 程序无法启动

```bash
# 1. 运行诊断
.\scripts\diagnose.bat

# 2. 查看详细启动过程
.\scripts\test_startup.bat

# 3. 测试Qt环境
.\scripts\test_minimal.bat

# 4. 查看日志
type logs\app.log
```

---

### 场景3: 开发调试

```bash
# 1. 编译Debug版本
qmake
mingw32-make

# 2. 运行Debug版本
.\scripts\run_debug.bat

# 3. 查看控制台输出
# （自动显示qDebug()、LOG_INFO等信息）
```

---

### 场景4: 发布版本

```bash
# 1. 清理并编译Release
.\scripts\build_release.bat

# 2. 进入release目录
cd release

# 3. 收集所有依赖
windeployqt UGIMS.exe

# 4. 复制配置和数据
xcopy ..\config .\config\ /E /I
xcopy ..\database .\database\ /E /I
xcopy ..\tilemap .\tilemap\ /E /I

# 5. 打包
# 将release目录打包为zip或安装包
```

---

## 📝 脚本修改指南

### 修改编译选项

编辑 `build_release.bat`:

```batch
@echo off
echo Building release version...

REM 清理
mingw32-make clean

REM 生成Makefile (可添加自定义选项)
qmake UGIMS.pro -spec win32-g++ "CONFIG+=release" "DEFINES+=MY_CUSTOM_FLAG"

REM 编译 (调整并行数)
mingw32-make -j8

echo Build complete!
pause
```

---

### 添加自动测试

创建 `test_auto.bat`:

```batch
@echo off
echo Running automated tests...

REM 编译测试版本
qmake UGIMS.pro "CONFIG+=test"
mingw32-make

REM 运行测试
.\test\test_runner.exe

pause
```

---

## ⚠️ 注意事项

1. **路径问题**
   - 脚本假设在项目根目录运行
   - 如果路径不对，手动`cd`到正确位置

2. **权限问题**
   - 某些操作可能需要管理员权限
   - 右键→"以管理员身份运行"

3. **环境变量**
   - 确保Qt、MinGW在系统PATH中
   - 或在脚本中设置完整路径

4. **中文路径**
   - 避免项目路径包含中文或特殊字符
   - 可能导致编译或运行问题

---

## 🔗 相关文档

- [QUICK_START.md](../docs/QUICK_START.md) - 快速开始指南
- [PROJECT_STRUCTURE.md](../docs/PROJECT_STRUCTURE.md) - 项目结构说明
- [DATABASE_SETUP.md](../docs/DATABASE_SETUP.md) - 数据库设置

---

**脚本使用愉快！** 🚀

