-- ==========================================
-- 检查 users 表是否存在
-- ==========================================

-- 方法1：查询所有表，查找 users 表
SELECT 
    schemaname,
    tablename,
    tableowner
FROM pg_tables
WHERE tablename = 'users'
ORDER BY schemaname;

-- 方法2：查询 users 表的所有列
SELECT 
    column_name,
    data_type,
    character_maximum_length,
    is_nullable,
    column_default
FROM information_schema.columns
WHERE table_name = 'users'
ORDER BY ordinal_position;

-- 方法3：查询 users 表中的数据
SELECT 
    id,
    username,
    real_name,
    role,
    status,
    created_at
FROM users
ORDER BY id;

-- 方法4：统计 users 表的记录数
SELECT COUNT(*) as total_users FROM users;

-- 方法5：查看表所在的 schema
SELECT 
    table_schema,
    table_name,
    table_type
FROM information_schema.tables
WHERE table_name = 'users';

