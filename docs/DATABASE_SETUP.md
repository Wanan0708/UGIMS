# 🗄️ 数据库设置完整指南

## 📋 目录
1. [安装PostgreSQL](#1-安装postgresql)
2. [创建数据库](#2-创建数据库)
3. [配置连接](#3-配置连接)
4. [导入数据](#4-导入数据)
5. [测试连接](#5-测试连接)
6. [故障排除](#6-故障排除)

---

## 1. 安装PostgreSQL

### Windows

#### 方法A: 官方安装包（推荐）

1. **下载PostgreSQL**
   - 访问：https://www.postgresql.org/download/windows/
   - 下载最新版本（推荐16.x或15.x）

2. **运行安装程序**
   ```
   postgresql-16.x-windows-x64.exe
   ```

3. **安装选项**
   - 安装目录：`C:\Program Files\PostgreSQL\16`
   - 端口：`5432`（默认）
   - 密码：**请记住这个密码！**
   - Locale：`Chinese (Simplified), China`

4. **组件选择**
   - ✅ PostgreSQL Server
   - ✅ pgAdmin 4
   - ✅ Stack Builder
   - ✅ Command Line Tools

5. **安装PostGIS（必需）**
   - 安装完成后，运行 Stack Builder
   - 选择：Spatial Extensions → PostGIS
   - 安装 PostGIS 3.x

#### 方法B: 使用包管理器

```powershell
# 使用 Chocolatey
choco install postgresql16 postgis

# 或使用 Scoop
scoop install postgresql
```

### Linux

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install postgresql postgresql-contrib postgis

# CentOS/RHEL
sudo yum install postgresql-server postgresql-contrib postgis

# Arch Linux
sudo pacman -S postgresql postgis
```

### macOS

```bash
# 使用 Homebrew
brew install postgresql@16 postgis
brew services start postgresql@16
```

---

## 2. 创建数据库

### 步骤1: 连接到PostgreSQL

```bash
# Windows (以管理员身份运行 PowerShell)
cd "C:\Program Files\PostgreSQL\16\bin"
.\psql.exe -U postgres

# Linux/macOS
psql -U postgres
```

**输入密码**：安装时设置的密码

### 步骤2: 创建数据库和用户

```sql
-- 创建数据库
CREATE DATABASE ugims
    ENCODING 'UTF8'
    LC_COLLATE = 'Chinese (Simplified)_China.936'
    LC_CTYPE = 'Chinese (Simplified)_China.936';

-- 连接到新数据库
\c ugims

-- 启用PostGIS扩展
CREATE EXTENSION IF NOT EXISTS postgis;
CREATE EXTENSION IF NOT EXISTS postgis_topology;

-- 验证PostGIS
SELECT PostGIS_Version();

-- 应该看到类似：3.4.0 r21927
```

### 步骤3: 创建专用用户（可选，推荐）

```sql
-- 创建用户
CREATE USER ugims_user WITH PASSWORD 'your_secure_password';

-- 授权
GRANT ALL PRIVILEGES ON DATABASE ugims TO ugims_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ugims_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ugims_user;

-- 退出
\q
```

---

## 3. 配置连接

### 编辑 `config/database.ini`

```ini
[database]
type=postgresql
host=localhost
port=5432
dbname=ugims
username=postgres
password=你的密码  # ← 修改这里！

[connection]
pool_size=5
max_idle=3
timeout=30

[ssl]
mode=disable

[postgis]
srid=4326
```

### 安全建议

⚠️ **不要**将密码提交到Git仓库！

创建 `.env` 文件（已在.gitignore中）：
```env
DB_PASSWORD=your_actual_password
```

或使用环境变量：
```bash
# Windows PowerShell
$env:UGIMS_DB_PASSWORD = "your_password"

# Linux/macOS
export UGIMS_DB_PASSWORD="your_password"
```

---

## 4. 导入数据

### 步骤1: 创建数据库结构

```bash
# 在项目根目录
cd E:\Project\CursorProject\UGIMS

# 导入schema
psql -U postgres -d ugims -f database/schema.sql
```

**预期输出**：
```
CREATE TABLE
CREATE INDEX
CREATE TRIGGER
...
```

### 步骤2: 导入测试数据

```bash
psql -U postgres -d ugims -f database/test_data.sql
```

**预期输出**：
```
INSERT 0 1
INSERT 0 1
...
```

### 步骤3: 验证数据

```sql
-- 连接数据库
psql -U postgres -d ugims

-- 检查表
\dt

-- 应该看到：
-- pipelines
-- facilities
-- work_orders
-- ...

-- 检查数据量
SELECT 'pipelines' as table_name, COUNT(*) FROM pipelines
UNION ALL
SELECT 'facilities', COUNT(*) FROM facilities;

-- 应该看到：
--  table_name | count
-- ------------+-------
--  pipelines  |     5
--  facilities |     4
```

---

## 5. 测试连接

### 方法1: 使用pgAdmin

1. 打开 pgAdmin 4
2. 右键 "Servers" → Create → Server
3. 填写：
   - Name: UGIMS Local
   - Host: localhost
   - Port: 5432
   - Database: ugims
   - Username: postgres
   - Password: (保存密码)
4. 点击 "Save"
5. 展开 ugims → Schemas → public → Tables

### 方法2: 使用命令行

```bash
# 测试连接
psql -U postgres -d ugims -c "SELECT version();"

# 测试PostGIS
psql -U postgres -d ugims -c "SELECT PostGIS_Version();"

# 测试数据
psql -U postgres -d ugims -c "SELECT COUNT(*) FROM pipelines;"
```

### 方法3: 使用程序自带工具

```bash
# 运行程序，查看日志
.\release\CustomTitleBarApp.exe

# 检查日志
type logs\app.log | findstr /C:"Database"
```

**预期日志**：
```
[INFO] Database configured: PostgreSQL at localhost:5432/ugims
[INFO] Attempting to connect to database...
[INFO] Successfully connected to database
[INFO] PostGIS version: 3.4.0
```

---

## 6. 故障排除

### 问题1: 连接超时

**症状**：
```
[ERROR] Failed to connect to database: timeout
```

**检查清单**：
```bash
# 1. PostgreSQL服务是否运行？
# Windows
sc query postgresql-x64-16

# Linux/macOS
sudo systemctl status postgresql

# 2. 端口是否被占用？
netstat -an | findstr 5432

# 3. 防火墙是否阻止？
# Windows：检查 Windows Defender 防火墙
# Linux：sudo ufw status
```

**解决方案**：
```bash
# Windows - 启动服务
net start postgresql-x64-16

# Linux
sudo systemctl start postgresql

# macOS
brew services start postgresql@16
```

---

### 问题2: 密码错误

**症状**：
```
[ERROR] Failed to connect to database: password authentication failed
```

**解决方案**：
```bash
# 重置密码
# Windows（以管理员运行）
cd "C:\Program Files\PostgreSQL\16\bin"
.\psql.exe -U postgres

# 在psql中
ALTER USER postgres WITH PASSWORD 'new_password';

# 然后修改 config/database.ini
```

---

### 问题3: 数据库不存在

**症状**：
```
[ERROR] database "ugims" does not exist
```

**解决方案**：
```sql
-- 创建数据库
createdb -U postgres ugims

-- 或在psql中
CREATE DATABASE ugims;
```

---

### 问题4: PostGIS不可用

**症状**：
```
[WARNING] PostGIS extension not available
```

**解决方案**：
```sql
-- 连接到数据库
psql -U postgres -d ugims

-- 启用PostGIS
CREATE EXTENSION IF NOT EXISTS postgis;

-- 验证
SELECT PostGIS_Version();
```

---

### 问题5: 权限不足

**症状**：
```
ERROR: permission denied for table pipelines
```

**解决方案**：
```sql
-- 授予权限
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO postgres;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO postgres;
```

---

## 📊 数据库状态检查脚本

创建 `check_db.bat`：

```batch
@echo off
echo ========================================
echo UGIMS 数据库状态检查
echo ========================================
echo.

echo [1/5] 检查PostgreSQL服务...
sc query postgresql-x64-16 | findstr STATE
echo.

echo [2/5] 测试连接...
psql -U postgres -d ugims -c "SELECT version();" 2>nul
if errorlevel 1 (
    echo ❌ 连接失败
) else (
    echo ✅ 连接成功
)
echo.

echo [3/5] 检查PostGIS...
psql -U postgres -d ugims -c "SELECT PostGIS_Version();" 2>nul
if errorlevel 1 (
    echo ❌ PostGIS不可用
) else (
    echo ✅ PostGIS可用
)
echo.

echo [4/5] 检查表...
psql -U postgres -d ugims -c "\dt" 2>nul
echo.

echo [5/5] 检查数据...
psql -U postgres -d ugims -c "SELECT 'pipelines' as tbl, COUNT(*) FROM pipelines UNION ALL SELECT 'facilities', COUNT(*) FROM facilities;" 2>nul
echo.

echo ========================================
echo 检查完成
echo ========================================
pause
```

---

## 🎯 快速开始流程

### 全新安装（首次）

```bash
# 1. 安装PostgreSQL + PostGIS
# 下载并安装

# 2. 创建数据库
createdb -U postgres ugims

# 3. 启用PostGIS
psql -U postgres -d ugims -c "CREATE EXTENSION postgis;"

# 4. 导入数据
psql -U postgres -d ugims -f database/schema.sql
psql -U postgres -d ugims -f database/test_data.sql

# 5. 配置连接
notepad config\database.ini
# 修改password

# 6. 运行程序
.\release\CustomTitleBarApp.exe
```

### 已有数据库

```bash
# 1. 确认服务运行
sc query postgresql-x64-16

# 2. 测试连接
psql -U postgres -d ugims -c "SELECT 1"

# 3. 检查配置
notepad config\database.ini

# 4. 运行程序
.\release\CustomTitleBarApp.exe
```

---

## 📝 配置模板

### 本地开发（默认）

```ini
[database]
host=localhost
port=5432
dbname=ugims
username=postgres
password=your_password
```

### 远程服务器

```ini
[database]
host=192.168.1.100
port=5432
dbname=ugims
username=ugims_user
password=secure_password

[ssl]
mode=require
cert=/path/to/client-cert.pem
key=/path/to/client-key.pem
root_cert=/path/to/ca-cert.pem
```

### Docker容器

```ini
[database]
host=localhost
port=5432
dbname=ugims
username=postgres
password=postgres
```

---

## 🔒 安全最佳实践

1. **不要使用默认密码**
   ```sql
   ALTER USER postgres WITH PASSWORD 'strong_random_password';
   ```

2. **创建专用用户**
   ```sql
   CREATE USER ugims_app WITH PASSWORD 'app_password';
   GRANT CONNECT ON DATABASE ugims TO ugims_app;
   ```

3. **限制连接来源**
   编辑 `pg_hba.conf`：
   ```
   # 只允许本地连接
   host ugims ugims_app 127.0.0.1/32 scram-sha-256
   ```

4. **启用SSL**
   ```ini
   [ssl]
   mode=require
   ```

5. **定期备份**
   ```bash
   pg_dump -U postgres ugims > backup_$(date +%Y%m%d).sql
   ```

---

## 📞 获取帮助

### 有用的命令

```sql
-- 查看所有数据库
\l

-- 查看所有表
\dt

-- 查看表结构
\d pipelines

-- 查看索引
\di

-- 查看用户
\du

-- 退出
\q
```

### 日志位置

```
Windows: C:\Program Files\PostgreSQL\16\data\log\
Linux:   /var/log/postgresql/
macOS:   /usr/local/var/log/postgresql@16/
```

---

**数据库设置完成！现在可以启用管网可视化功能了。** 🎉

