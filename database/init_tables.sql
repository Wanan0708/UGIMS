-- UGIMS Database Schema (UTF-8 Encoding)
-- Core tables for pipeline management system

-- Enable extensions
CREATE EXTENSION IF NOT EXISTS postgis;
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Drop existing tables if any
DROP TABLE IF EXISTS pipelines CASCADE;
DROP TABLE IF EXISTS facilities CASCADE;

-- Pipelines table
CREATE TABLE pipelines (
    id SERIAL PRIMARY KEY,
    pipeline_id VARCHAR(50) UNIQUE NOT NULL,
    pipeline_name VARCHAR(200),
    pipeline_type VARCHAR(50) NOT NULL,
    
    -- Geometry
    geom GEOMETRY(LINESTRING, 4326) NOT NULL,
    length_m DOUBLE PRECISION,
    depth_m DOUBLE PRECISION,
    
    -- Physical properties
    diameter_mm INTEGER,
    material VARCHAR(50),
    pressure_class VARCHAR(50),
    
    -- Construction info
    build_date DATE,
    builder VARCHAR(200),
    owner VARCHAR(200),
    construction_cost DECIMAL(15,2),
    
    -- Operation info
    status VARCHAR(50) DEFAULT 'active',
    health_score INTEGER DEFAULT 100 CHECK (health_score >= 0 AND health_score <= 100),
    last_inspection DATE,
    maintenance_unit VARCHAR(200),
    inspection_cycle INTEGER DEFAULT 365,
    
    -- Metadata
    remarks TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100),
    updated_by VARCHAR(100)
);

-- Spatial index for pipelines
CREATE INDEX idx_pipelines_geom ON pipelines USING GIST(geom);
CREATE INDEX idx_pipelines_type ON pipelines(pipeline_type);
CREATE INDEX idx_pipelines_status ON pipelines(status);

-- Facilities table
CREATE TABLE facilities (
    id SERIAL PRIMARY KEY,
    facility_id VARCHAR(50) UNIQUE NOT NULL,
    facility_name VARCHAR(200),
    facility_type VARCHAR(50) NOT NULL,
    
    -- Geometry
    geom GEOMETRY(POINT, 4326) NOT NULL,
    elevation_m DOUBLE PRECISION,
    
    -- Physical properties
    spec VARCHAR(100),
    material VARCHAR(50),
    size VARCHAR(50),
    
    -- Related pipeline
    pipeline_id VARCHAR(50),
    
    -- Construction info
    build_date DATE,
    builder VARCHAR(200),
    owner VARCHAR(200),
    asset_value DECIMAL(15,2),
    
    -- Operation info
    status VARCHAR(50) DEFAULT 'normal',
    health_score INTEGER DEFAULT 100 CHECK (health_score >= 0 AND health_score <= 100),
    last_maintenance DATE,
    next_maintenance DATE,
    maintenance_unit VARCHAR(200),
    
    -- QR code
    qrcode_url VARCHAR(500),
    
    -- Metadata
    remarks TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_by VARCHAR(100),
    updated_by VARCHAR(100)
);

-- Spatial index for facilities
CREATE INDEX idx_facilities_geom ON facilities USING GIST(geom);
CREATE INDEX idx_facilities_type ON facilities(facility_type);
CREATE INDEX idx_facilities_status ON facilities(status);
CREATE INDEX idx_facilities_pipeline ON facilities(pipeline_id);

-- Success message
SELECT 'Tables created successfully!' AS result;

