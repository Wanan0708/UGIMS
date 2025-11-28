-- ==========================================
-- UGIMS 数据库架构
-- 城市地下管网智能管理系统
-- PostgreSQL + PostGIS
-- ==========================================

-- 创建 PostGIS 扩展
CREATE EXTENSION IF NOT EXISTS postgis;
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ==========================================
-- 管线基础表
-- ==========================================
CREATE TABLE pipelines (
    id SERIAL PRIMARY KEY,
    pipeline_id VARCHAR(50) UNIQUE NOT NULL,
    pipeline_name VARCHAR(200),
    pipeline_type VARCHAR(50) NOT NULL,  -- water_supply, sewage, gas, electric, telecom, heat
    
    -- 几何信息
    geom GEOMETRY(LINESTRING, 4326) NOT NULL,
    length_m DOUBLE PRECISION,
    depth_m DOUBLE PRECISION,
    
    -- 物理属性
    diameter_mm INTEGER,
    material VARCHAR(50),  -- ductile_iron, steel, pe, pvc, concrete
    pressure_class VARCHAR(50),
    
    -- 建设信息
    build_date DATE,
    builder VARCHAR(200),
    owner VARCHAR(200),
    construction_cost DECIMAL(15,2),
    
    -- 运维信息
    status VARCHAR(50) DEFAULT 'active',  -- active, inactive, repairing, abandoned
    health_score INTEGER DEFAULT 100 CHECK (health_score >= 0 AND health_score <= 100),
    last_inspection DATE,
    maintenance_unit VARCHAR(200),
    inspection_cycle INTEGER DEFAULT 365,  -- 天数
    
    -- 元数据
    remarks TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100),
    updated_by VARCHAR(100)
);

-- 创建空间索引
CREATE INDEX idx_pipelines_geom ON pipelines USING GIST(geom);
CREATE INDEX idx_pipelines_type ON pipelines(pipeline_type);
CREATE INDEX idx_pipelines_status ON pipelines(status);
CREATE INDEX idx_pipelines_build_date ON pipelines(build_date);

-- 添加注释
COMMENT ON TABLE pipelines IS '管线基础信息表';
COMMENT ON COLUMN pipelines.pipeline_type IS '管线类型：water_supply-给水, sewage-排水, gas-燃气, electric-电力, telecom-通信, heat-供热';

-- ==========================================
-- 设施基础表
-- ==========================================
CREATE TABLE facilities (
    id SERIAL PRIMARY KEY,
    facility_id VARCHAR(50) UNIQUE NOT NULL,
    facility_name VARCHAR(200),
    facility_type VARCHAR(50) NOT NULL,  -- valve, manhole, pump_station, transformer, etc.
    
    -- 几何信息
    geom GEOMETRY(POINT, 4326) NOT NULL,
    elevation_m DOUBLE PRECISION,
    
    -- 物理属性
    spec VARCHAR(100),  -- 规格型号
    material VARCHAR(50),
    size VARCHAR(50),
    
    -- 关联管线
    pipeline_id VARCHAR(50),
    
    -- 建设信息
    build_date DATE,
    builder VARCHAR(200),
    owner VARCHAR(200),
    asset_value DECIMAL(15,2),
    
    -- 运维信息
    status VARCHAR(50) DEFAULT 'normal',  -- normal, warning, fault, repairing
    health_score INTEGER DEFAULT 100 CHECK (health_score >= 0 AND health_score <= 100),
    last_maintenance DATE,
    next_maintenance DATE,
    maintenance_unit VARCHAR(200),
    
    -- 二维码
    qrcode_url VARCHAR(500),
    
    -- 元数据
    remarks TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100),
    updated_by VARCHAR(100)
);

-- 创建空间索引
CREATE INDEX idx_facilities_geom ON facilities USING GIST(geom);
CREATE INDEX idx_facilities_type ON facilities(facility_type);
CREATE INDEX idx_facilities_status ON facilities(status);
CREATE INDEX idx_facilities_pipeline ON facilities(pipeline_id);

COMMENT ON TABLE facilities IS '设施设备信息表';

-- ==========================================
-- 工单管理表
-- ==========================================
CREATE TABLE work_orders (
    id SERIAL PRIMARY KEY,
    order_id VARCHAR(50) UNIQUE NOT NULL,
    order_title VARCHAR(200) NOT NULL,
    order_type VARCHAR(50) NOT NULL,  -- inspection, maintenance, emergency, renovation
    priority VARCHAR(20) DEFAULT 'normal',  -- low, normal, high, urgent
    
    -- 关联资产
    asset_type VARCHAR(20),  -- pipeline, facility
    asset_id VARCHAR(50),
    location GEOMETRY(POINT, 4326),
    
    -- 工单内容
    description TEXT,
    required_actions TEXT,
    
    -- 人员分配
    assigned_to VARCHAR(100),
    assigned_at TIMESTAMP,
    assigned_by VARCHAR(100),
    
    -- 状态流转
    status VARCHAR(50) DEFAULT 'pending',  -- pending, assigned, in_progress, completed, cancelled
    
    -- 时间节点
    plan_start_time TIMESTAMP,
    plan_end_time TIMESTAMP,
    actual_start_time TIMESTAMP,
    actual_end_time TIMESTAMP,
    
    -- 完成情况
    completion_rate INTEGER DEFAULT 0 CHECK (completion_rate >= 0 AND completion_rate <= 100),
    work_result TEXT,
    photos TEXT[],  -- 照片文件路径数组
    
    -- 审核
    reviewed_by VARCHAR(100),
    reviewed_at TIMESTAMP,
    review_result VARCHAR(20),  -- approved, rejected
    review_comments TEXT,
    
    -- 元数据
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100)
);

CREATE INDEX idx_work_orders_type ON work_orders(order_type);
CREATE INDEX idx_work_orders_status ON work_orders(status);
CREATE INDEX idx_work_orders_assigned ON work_orders(assigned_to);
CREATE INDEX idx_work_orders_created ON work_orders(created_at);
CREATE INDEX idx_work_orders_asset ON work_orders(asset_type, asset_id);

COMMENT ON TABLE work_orders IS '工单管理表';

-- ==========================================
-- 资产变更历史表
-- ==========================================
CREATE TABLE asset_changes (
    id SERIAL PRIMARY KEY,
    asset_type VARCHAR(20) NOT NULL,  -- pipeline, facility
    asset_id VARCHAR(50) NOT NULL,
    change_type VARCHAR(50) NOT NULL,  -- create, update, delete, status_change
    
    -- 变更内容
    field_name VARCHAR(100),
    old_value TEXT,
    new_value TEXT,
    
    -- 变更信息
    change_reason TEXT,
    changed_by VARCHAR(100),
    changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_asset_changes_asset ON asset_changes(asset_type, asset_id);
CREATE INDEX idx_asset_changes_time ON asset_changes(changed_at);

COMMENT ON TABLE asset_changes IS '资产变更历史记录表';

-- ==========================================
-- 巡检记录表
-- ==========================================
CREATE TABLE inspection_records (
    id SERIAL PRIMARY KEY,
    record_id VARCHAR(50) UNIQUE NOT NULL,
    work_order_id VARCHAR(50) REFERENCES work_orders(order_id),
    
    -- 巡检对象
    asset_type VARCHAR(20) NOT NULL,
    asset_id VARCHAR(50) NOT NULL,
    
    -- 巡检信息
    inspection_date TIMESTAMP NOT NULL,
    inspector VARCHAR(100),
    
    -- 巡检结果
    inspection_result VARCHAR(50),  -- normal, warning, fault
    health_score INTEGER CHECK (health_score >= 0 AND health_score <= 100),
    
    -- 发现的问题
    issues TEXT[],
    suggestions TEXT,
    
    -- 现场记录
    photos TEXT[],
    gps_location GEOMETRY(POINT, 4326),
    
    -- 元数据
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_inspection_asset ON inspection_records(asset_type, asset_id);
CREATE INDEX idx_inspection_date ON inspection_records(inspection_date);
CREATE INDEX idx_inspection_order ON inspection_records(work_order_id);

COMMENT ON TABLE inspection_records IS '巡检记录表';

-- ==========================================
-- 故障报修记录表
-- ==========================================
CREATE TABLE fault_reports (
    id SERIAL PRIMARY KEY,
    report_id VARCHAR(50) UNIQUE NOT NULL,
    
    -- 报修信息
    fault_type VARCHAR(50) NOT NULL,  -- leak, burst, blockage, malfunction
    fault_description TEXT NOT NULL,
    location GEOMETRY(POINT, 4326),
    address VARCHAR(500),
    
    -- 关联资产
    asset_type VARCHAR(20),
    asset_id VARCHAR(50),
    
    -- 影响范围
    severity VARCHAR(20) DEFAULT 'medium',  -- low, medium, high, critical
    affected_users INTEGER,
    
    -- 报修人信息
    reporter VARCHAR(100),
    reporter_phone VARCHAR(20),
    report_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    
    -- 处理流程
    work_order_id VARCHAR(50) REFERENCES work_orders(order_id),
    status VARCHAR(50) DEFAULT 'reported',  -- reported, confirmed, dispatched, repairing, resolved
    
    -- 处理结果
    repair_start_time TIMESTAMP,
    repair_end_time TIMESTAMP,
    repair_description TEXT,
    photos TEXT[],
    
    -- 元数据
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_fault_reports_type ON fault_reports(fault_type);
CREATE INDEX idx_fault_reports_status ON fault_reports(status);
CREATE INDEX idx_fault_reports_time ON fault_reports(report_time);
CREATE INDEX idx_fault_reports_geom ON fault_reports USING GIST(location);

COMMENT ON TABLE fault_reports IS '故障报修记录表';

-- ==========================================
-- 用户权限表
-- ==========================================
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    real_name VARCHAR(100),
    
    -- 联系方式
    email VARCHAR(100),
    phone VARCHAR(20),
    
    -- 角色和权限
    role VARCHAR(50) NOT NULL,  -- admin, manager, inspector, viewer
    permissions TEXT[],
    
    -- 所属单位
    department VARCHAR(100),
    organization VARCHAR(200),
    
    -- 账户状态
    status VARCHAR(20) DEFAULT 'active',  -- active, inactive, locked
    
    -- 时间戳
    last_login TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE UNIQUE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_role ON users(role);

COMMENT ON TABLE users IS '用户账户表';

-- ==========================================
-- 系统日志表
-- ==========================================
CREATE TABLE system_logs (
    id SERIAL PRIMARY KEY,
    log_type VARCHAR(50) NOT NULL,  -- login, operation, error, security
    log_level VARCHAR(20) NOT NULL,  -- debug, info, warning, error, critical
    
    -- 操作信息
    username VARCHAR(50),
    operation VARCHAR(100),
    module VARCHAR(50),
    description TEXT,
    
    -- 请求信息
    ip_address VARCHAR(50),
    user_agent TEXT,
    
    -- 结果
    status VARCHAR(20),  -- success, failure
    error_message TEXT,
    
    -- 时间戳
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_system_logs_type ON system_logs(log_type);
CREATE INDEX idx_system_logs_username ON system_logs(username);
CREATE INDEX idx_system_logs_time ON system_logs(created_at);

COMMENT ON TABLE system_logs IS '系统操作日志表';

-- ==========================================
-- 附件文件表
-- ==========================================
CREATE TABLE attachments (
    id SERIAL PRIMARY KEY,
    file_id VARCHAR(50) UNIQUE NOT NULL,
    
    -- 关联对象
    related_type VARCHAR(50) NOT NULL,  -- pipeline, facility, work_order, inspection
    related_id VARCHAR(50) NOT NULL,
    
    -- 文件信息
    file_name VARCHAR(255) NOT NULL,
    file_path VARCHAR(500) NOT NULL,
    file_type VARCHAR(50),  -- photo, document, cad, video
    file_size BIGINT,
    mime_type VARCHAR(100),
    
    -- 描述
    description TEXT,
    
    -- 上传信息
    uploaded_by VARCHAR(100),
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_attachments_related ON attachments(related_type, related_id);
CREATE INDEX idx_attachments_type ON attachments(file_type);

COMMENT ON TABLE attachments IS '附件文件表';

-- ==========================================
-- 触发器：自动更新 updated_at 字段
-- ==========================================
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- 为各表添加更新触发器
CREATE TRIGGER update_pipelines_updated_at BEFORE UPDATE ON pipelines
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_facilities_updated_at BEFORE UPDATE ON facilities
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_work_orders_updated_at BEFORE UPDATE ON work_orders
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- ==========================================
-- 视图：管线统计
-- ==========================================
CREATE VIEW v_pipeline_statistics AS
SELECT 
    pipeline_type,
    COUNT(*) as total_count,
    SUM(length_m) as total_length_m,
    AVG(health_score) as avg_health_score,
    COUNT(CASE WHEN status = 'active' THEN 1 END) as active_count,
    COUNT(CASE WHEN health_score < 60 THEN 1 END) as low_health_count
FROM pipelines
GROUP BY pipeline_type;

COMMENT ON VIEW v_pipeline_statistics IS '管线统计视图';

-- ==========================================
-- 视图：工单统计
-- ==========================================
CREATE VIEW v_workorder_statistics AS
SELECT 
    order_type,
    status,
    COUNT(*) as order_count,
    AVG(completion_rate) as avg_completion_rate
FROM work_orders
WHERE created_at >= CURRENT_DATE - INTERVAL '30 days'
GROUP BY order_type, status;

COMMENT ON VIEW v_workorder_statistics IS '工单统计视图（近30天）';

-- ==========================================
-- 初始化管理员账户
-- ==========================================
-- 密码：admin123（实际使用时应使用加密哈希）
INSERT INTO users (username, password_hash, real_name, role, status)
VALUES ('admin', 'admin123_hash_placeholder', '系统管理员', 'admin', 'active');

-- ==========================================
-- 完成
-- ==========================================
COMMENT ON DATABASE ugims IS 'UGIMS - 城市地下管网智能管理系统数据库';

