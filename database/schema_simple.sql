-- ==========================================
-- UGIMS 简化数据库架构（不依赖 PostGIS）
-- 临时方案：使用 TEXT 存储 GeoJSON，等安装 PostGIS 后再升级
-- ==========================================

-- 创建扩展
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ==========================================
-- 管线基础表（简化版）
-- ==========================================
CREATE TABLE IF NOT EXISTS pipelines (
    id SERIAL PRIMARY KEY,
    pipeline_id VARCHAR(50) UNIQUE NOT NULL,
    pipeline_name VARCHAR(200),
    pipeline_type VARCHAR(50) NOT NULL,
    
    -- 几何信息（临时使用 TEXT 存储 GeoJSON）
    geom_json TEXT NOT NULL,
    length_m DOUBLE PRECISION,
    depth_m DOUBLE PRECISION,
    
    -- 物理属性
    diameter_mm INTEGER,
    material VARCHAR(50),
    pressure_class VARCHAR(50),
    
    -- 建设信息
    build_date DATE,
    builder VARCHAR(200),
    owner VARCHAR(200),
    construction_cost DECIMAL(15,2),
    
    -- 运维信息
    status VARCHAR(50) DEFAULT 'active',
    health_score INTEGER DEFAULT 100 CHECK (health_score >= 0 AND health_score <= 100),
    last_inspection DATE,
    maintenance_unit VARCHAR(200),
    inspection_cycle INTEGER DEFAULT 365,
    
    -- 元数据
    remarks TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100),
    updated_by VARCHAR(100)
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_pipelines_type ON pipelines(pipeline_type);
CREATE INDEX IF NOT EXISTS idx_pipelines_status ON pipelines(status);
CREATE INDEX IF NOT EXISTS idx_pipelines_build_date ON pipelines(build_date);

-- ==========================================
-- 设施表（简化版）
-- ==========================================
CREATE TABLE IF NOT EXISTS facilities (
    id SERIAL PRIMARY KEY,
    facility_id VARCHAR(50) UNIQUE NOT NULL,
    facility_name VARCHAR(200),
    facility_type VARCHAR(50) NOT NULL,
    
    -- 几何信息（临时使用经纬度）
    longitude DOUBLE PRECISION NOT NULL,
    latitude DOUBLE PRECISION NOT NULL,
    elevation_m DOUBLE PRECISION,
    
    -- 关联信息
    pipeline_id VARCHAR(50),
    
    -- 物理属性
    spec VARCHAR(100),
    material VARCHAR(50),
    
    -- 建设信息
    build_date DATE,
    builder VARCHAR(200),
    owner VARCHAR(200),
    
    -- 运维信息
    status VARCHAR(50) DEFAULT 'active',
    health_score INTEGER DEFAULT 100 CHECK (health_score >= 0 AND health_score <= 100),
    last_inspection DATE,
    maintenance_unit VARCHAR(200),
    
    -- 元数据
    remarks TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100),
    updated_by VARCHAR(100)
);

-- 创建索引
CREATE INDEX IF NOT EXISTS idx_facilities_type ON facilities(facility_type);
CREATE INDEX IF NOT EXISTS idx_facilities_status ON facilities(status);
CREATE INDEX IF NOT EXISTS idx_facilities_pipeline ON facilities(pipeline_id);

-- ==========================================
-- 其他必要的表
-- ==========================================

-- 工单表
CREATE TABLE IF NOT EXISTS work_orders (
    id SERIAL PRIMARY KEY,
    order_id VARCHAR(50) UNIQUE NOT NULL,
    order_type VARCHAR(50) NOT NULL,
    title VARCHAR(200) NOT NULL,
    description TEXT,
    priority VARCHAR(20) DEFAULT 'medium',
    status VARCHAR(50) DEFAULT 'pending',
    
    related_pipeline_id VARCHAR(50),
    related_facility_id VARCHAR(50),
    
    location_description TEXT,
    reporter VARCHAR(100),
    reporter_phone VARCHAR(20),
    
    assigned_to VARCHAR(100),
    assigned_team VARCHAR(100),
    
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    scheduled_start TIMESTAMP,
    scheduled_end TIMESTAMP,
    actual_start TIMESTAMP,
    actual_end TIMESTAMP,
    
    estimated_cost DECIMAL(15,2),
    actual_cost DECIMAL(15,2),
    
    remarks TEXT,
    created_by VARCHAR(100),
    updated_by VARCHAR(100),
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_work_orders_status ON work_orders(status);
CREATE INDEX IF NOT EXISTS idx_work_orders_type ON work_orders(order_type);
CREATE INDEX IF NOT EXISTS idx_work_orders_priority ON work_orders(priority);

