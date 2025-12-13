-- ==========================================
-- 插入默认用户数据
-- UGIMS 管网管理系统
-- ==========================================

-- 注意：此脚本使用预计算的SHA-256哈希值，不需要pgcrypto扩展
-- 如果需要使用digest函数，请先执行: CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- 重要提示：执行此脚本前，请确保已经执行了 schema.sql 创建了 users 表
-- 如果 users 表不存在，请先执行以下命令创建表：
-- 
-- 方法1：执行完整的 schema.sql
-- psql -U your_username -d ugims -f database/schema.sql
--
-- 方法2：只创建 users 表（见下面的 CREATE TABLE 语句）

-- ==========================================
-- 如果 users 表不存在，先创建表（可选）
-- ==========================================
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    real_name VARCHAR(100),
    email VARCHAR(100),
    phone VARCHAR(20),
    role VARCHAR(50) NOT NULL,
    permissions TEXT[],
    department VARCHAR(100),
    organization VARCHAR(200),
    status VARCHAR(20) DEFAULT 'active',
    last_login TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 创建索引（如果不存在）
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_users_role ON users(role);

-- 删除已存在的默认用户（如果存在）
DELETE FROM users WHERE username IN ('admin', 'manager', 'inspector', 'viewer');

-- ==========================================
-- 1. 超级管理员 (admin)
-- ==========================================
INSERT INTO users (
    username,
    password_hash,
    real_name,
    email,
    phone,
    role,
    permissions,
    department,
    organization,
    status,
    created_at,
    updated_at
) VALUES (
    'admin',
    '240be518fabd2724ddb6f04eeb1da5967448d7e831c08c8fa822809f74c720a9',  -- 密码: admin123 (SHA-256)
    '超级管理员',
    'admin@ugims.com',
    '13800000001',
    'admin',
    ARRAY['view_map', 'view_pipeline', 'view_facility', 
          'create_pipeline', 'edit_pipeline', 'delete_pipeline',
          'create_facility', 'edit_facility', 'delete_facility',
          'create_workorder', 'edit_workorder', 'delete_workorder',
          'assign_workorder', 'start_workorder', 'complete_workorder',
          'view_statistics', 'export_data',
          'manage_users', 'system_settings'],
    '系统管理部',
    'UGIMS系统',
    'active',
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);

-- ==========================================
-- 2. 管理员 (manager)
-- ==========================================
INSERT INTO users (
    username,
    password_hash,
    real_name,
    email,
    phone,
    role,
    permissions,
    department,
    organization,
    status,
    created_at,
    updated_at
) VALUES (
    'manager',
    '866485796cfa8d7c0cf7111640205b83076433547577511d81f8030ae99ecea5',  -- 密码: manager123 (SHA-256)
    '管理员',
    'manager@ugims.com',
    '13800000002',
    'manager',
    ARRAY['view_map', 'view_pipeline', 'view_facility',
          'create_pipeline', 'edit_pipeline',
          'create_facility', 'edit_facility',
          'create_workorder', 'edit_workorder',
          'assign_workorder', 'start_workorder', 'complete_workorder',
          'view_statistics', 'export_data'],
    '运营管理部',
    'UGIMS系统',
    'active',
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);

-- ==========================================
-- 3. 巡检员 (inspector)
-- ==========================================
INSERT INTO users (
    username,
    password_hash,
    real_name,
    email,
    phone,
    role,
    permissions,
    department,
    organization,
    status,
    created_at,
    updated_at
) VALUES (
    'inspector',
    'd899f47283d49dce931d95081a21c20051d0f70109fd9ae6d6774601274190da',  -- 密码: inspector123 (SHA-256)
    '巡检员',
    'inspector@ugims.com',
    '13800000003',
    'inspector',
    ARRAY['view_map', 'view_pipeline', 'view_facility',
          'create_workorder',
          'start_workorder', 'complete_workorder'],
    '巡检部',
    'UGIMS系统',
    'active',
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);

-- ==========================================
-- 4. 查看者 (viewer)
-- ==========================================
INSERT INTO users (
    username,
    password_hash,
    real_name,
    email,
    phone,
    role,
    permissions,
    department,
    organization,
    status,
    created_at,
    updated_at
) VALUES (
    'viewer',
    '65375049b9e4d7cad6c9ba286fdeb9394b28135a3e84136404cfccfdcc438894',  -- 密码: viewer123 (SHA-256)
    '查看者',
    'viewer@ugims.com',
    '13800000004',
    'viewer',
    ARRAY['view_map', 'view_pipeline', 'view_facility'],  -- 只有查看权限
    '信息部',
    'UGIMS系统',
    'active',
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);

-- ==========================================
-- 验证插入结果
-- ==========================================
SELECT 
    id,
    username,
    real_name,
    role,
    status,
    created_at
FROM users
WHERE username IN ('admin', 'manager', 'inspector', 'viewer')
ORDER BY id;

-- ==========================================
-- 用户账号和密码说明
-- ==========================================
-- 超级管理员:
--   用户名: admin
--   密码: admin123
--   权限: 所有权限
--
-- 管理员:
--   用户名: manager
--   密码: manager123
--   权限: 除用户管理和系统设置外的所有权限
--
-- 巡检员:
--   用户名: inspector
--   密码: inspector123
--   权限: 查看、创建工单、开始和完成工单
--
-- 查看者:
--   用户名: viewer
--   密码: viewer123
--   权限: 仅查看权限
-- ==========================================

