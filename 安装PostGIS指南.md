# 🗺️ PostGIS 安装指南

**问题**: `extension "postgis" is not available`  
**原因**: PostgreSQL 没有安装 PostGIS 扩展  
**影响**: 无法存储和查询地理空间数据（管线、设施位置）

---

## 🚀 **快速安装（推荐）**

### 方法 1: 使用 Stack Builder（最简单）

1. **打开 Stack Builder**
   - 开始菜单 → PostgreSQL → Application Stack Builder

2. **选择 PostgreSQL 实例**
   - 选择你安装的 PostgreSQL 版本（例如 PostgreSQL 16）

3. **选择 PostGIS**
   - 展开 "Spatial Extensions"
   - 勾选 "PostGIS X.X Bundle for PostgreSQL"
   - 点击 Next

4. **下载并安装**
   - 选择下载目录
   - 等待下载完成
   - 按照安装向导完成安装

5. **验证安装**
   ```bash
   psql -U postgres -d ugims -c "CREATE EXTENSION postgis;"
   ```
   
   如果成功，会显示：`CREATE EXTENSION`

---

### 方法 2: 手动下载安装包

1. **下载 PostGIS**
   - 访问：https://postgis.net/windows_downloads/
   - 或直接下载：https://download.osgeo.org/postgis/windows/
   - 选择与你的 PostgreSQL 版本匹配的安装包

2. **运行安装程序**
   - 双击下载的 .exe 文件
   - 选择 PostgreSQL 安装目录
   - 完成安装

3. **验证安装**
   ```bash
   psql -U postgres -d ugims -c "CREATE EXTENSION postgis;"
   ```

---

### 方法 3: 使用包管理器（如果你有 Chocolatey）

```bash
choco install postgis
```

---

## ✅ **安装完成后**

### 1. 启用 PostGIS 扩展

```bash
psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS postgis;"
psql -U postgres -d ugims -c "CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\";"
```

### 2. 导入正确的表结构和数据

```bash
# 删除旧的简化表（如果有）
psql -U postgres -d ugims -c "DROP TABLE IF EXISTS pipelines CASCADE;"
psql -U postgres -d ugims -c "DROP TABLE IF EXISTS facilities CASCADE;"

# 导入完整的表结构
psql -U postgres -d ugims -f database\schema.sql

# 导入测试数据
psql -U postgres -d ugims -f database\test_data.sql
```

### 3. 验证数据

```bash
psql -U postgres -d ugims -c "SELECT COUNT(*) FROM pipelines;"
psql -U postgres -d ugims -c "SELECT COUNT(*) FROM facilities;"
```

应该看到：
- 管线数量：6
- 设施数量：3

---

## 🔧 **如果暂时无法安装 PostGIS**

### 临时方案：使用简化数据结构

如果现在无法安装 PostGIS，可以使用简化版：

```bash
.\导入简化数据.bat
```

**注意**：
- ⚠️ 简化版使用 TEXT 和经纬度字段，不支持空间索引
- ⚠️ 需要修改 DAO 代码以适配简化结构
- ⚠️ 性能和功能会受限

**强烈建议安装 PostGIS 以获得完整功能！**

---

## 📌 **推荐做法**

1. ✅ **先安装 PostGIS**（10分钟内可完成）
2. ✅ **使用完整的 schema.sql**
3. ✅ **享受完整的空间分析功能**

而不是：
- ❌ 使用简化版（功能受限）
- ❌ 手动修改大量代码

---

## 🎯 **下一步**

### 选项 A: 安装 PostGIS（推荐）⭐

1. 打开 Stack Builder
2. 安装 PostGIS
3. 运行 `.\导入测试数据.bat`
4. 启动程序

### 选项 B: 临时使用简化版

1. 运行 `.\导入简化数据.bat`
2. 我会修改 DAO 代码以适配简化结构
3. 后续还是建议安装 PostGIS

---

**请告诉我你选择哪个方案？** 

- **方案 A**：我等你安装好 PostGIS（推荐）
- **方案 B**：我现在修改代码以适配简化数据结构

🎯

