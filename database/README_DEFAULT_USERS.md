# 默认用户账号和密码

## 使用方法

### 前提条件

**重要**：执行此脚本前，请确保 `users` 表已经创建。

如果 `users` 表不存在，有两种方式创建：

#### 方法1：执行完整的数据库架构（推荐）

```bash
psql -U your_username -d ugims -f database/schema.sql
```

这会创建所有表，包括 `users` 表。

#### 方法2：脚本会自动创建表（如果不存在）

更新后的 `insert_default_users.sql` 脚本已经包含了 `CREATE TABLE IF NOT EXISTS` 语句，如果表不存在会自动创建。

### 执行插入用户脚本

执行以下SQL脚本来插入默认用户：

```bash
psql -U your_username -d ugims -f database/insert_default_users.sql
```

或者在PostgreSQL客户端中：
1. 连接到 `ugims` 数据库
2. 打开 `insert_default_users.sql` 文件
3. 执行脚本内容

## 默认用户账号

### 1. 超级管理员 (admin)
- **用户名**: `admin`
- **密码**: `admin123`
- **角色**: admin
- **权限**: 所有权限（包括用户管理、系统设置等）
- **用途**: 系统管理员，拥有最高权限

### 2. 管理员 (manager)
- **用户名**: `manager`
- **密码**: `manager123`
- **角色**: manager
- **权限**: 
  - 查看地图、管线、设施
  - 创建/编辑管线、设施
  - 创建/编辑工单
  - 派发、开始、完成工单
  - 查看统计、导出数据
  - **不包含**: 用户管理、系统设置、删除操作
- **用途**: 日常运营管理

### 3. 巡检员 (inspector)
- **用户名**: `inspector`
- **密码**: `inspector123`
- **角色**: inspector
- **权限**: 
  - 查看地图、管线、设施
  - 创建工单
  - 开始、完成工单
- **用途**: 现场巡检人员

### 4. 查看者 (viewer)
- **用户名**: `viewer`
- **密码**: `viewer123`
- **角色**: viewer
- **权限**: 
  - 仅查看权限（地图、管线、设施）
  - **不包含**: 任何编辑、创建、删除操作
- **用途**: 只读用户，用于查看数据

## 安全提示

⚠️ **重要**: 这些是默认测试账号，密码强度较低。在生产环境中：

1. **立即修改所有默认密码**
2. **使用强密码策略**（至少8位，包含大小写字母、数字和特殊字符）
3. **定期更换密码**
4. **根据实际需求调整用户权限**

## 密码哈希说明

所有密码使用 SHA-256 算法进行哈希存储，原始密码不会保存在数据库中。

如果需要修改密码，可以通过用户管理界面进行，或者使用以下SQL（需要先计算新密码的SHA-256哈希值）：

```sql
UPDATE users 
SET password_hash = '新密码的SHA-256哈希值',
    updated_at = CURRENT_TIMESTAMP
WHERE username = '用户名';
```

## 验证用户

执行脚本后，可以使用以下SQL验证用户是否创建成功：

```sql
SELECT username, real_name, role, status, created_at 
FROM users 
WHERE username IN ('admin', 'manager', 'inspector', 'viewer')
ORDER BY id;
```

