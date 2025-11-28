-- ==========================================
-- UGIMS 测试数据（简化版，不依赖 PostGIS）
-- 北京天安门区域测试数据
-- ==========================================

-- 清空现有数据
TRUNCATE TABLE pipelines CASCADE;
TRUNCATE TABLE facilities CASCADE;

-- ==========================================
-- 测试管线数据
-- ==========================================

-- 1. 给水管线（天安门东侧）
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom_json, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'WS-BJ-001',
    'Tiananmen East Water Supply Main',
    'water_supply',
    '{"type":"LineString","coordinates":[[116.400,39.905],[116.405,39.905]]}',
    500.0,
    1.5,
    800,
    'ductile_iron',
    'PN16',
    '2015-06-01',
    'Beijing Water Group',
    'active',
    95
);

-- 2. 排水管线（长安街段）
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom_json, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'SW-BJ-001',
    'Changan Street Sewage Line',
    'sewage',
    '{"type":"LineString","coordinates":[[116.395,39.908],[116.400,39.908]]}',
    480.0,
    2.5,
    1000,
    'concrete',
    'gravity',
    '2010-03-15',
    'Beijing Drainage Group',
    'active',
    88
);

-- 3. 燃气管线（天安门南）
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom_json, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'GAS-BJ-001',
    'Tiananmen South Gas Line',
    'gas',
    '{"type":"LineString","coordinates":[[116.395,39.902],[116.400,39.902]]}',
    450.0,
    1.8,
    400,
    'steel',
    'HP-4.0',
    '2012-09-20',
    'Beijing Gas Group',
    'active',
    92
);

-- 4. 电力电缆（东长安街）
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom_json, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'ELC-BJ-001',
    'East Changan Street Power Cable',
    'electric',
    '{"type":"LineString","coordinates":[[116.402,39.910],[116.407,39.910]]}',
    520.0,
    1.2,
    150,
    'copper',
    '10kV',
    '2018-04-10',
    'State Grid Beijing',
    'active',
    98
);

-- 5. 通信光缆（前门段）
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom_json, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'TEL-BJ-001',
    'Qianmen Telecom Cable',
    'telecom',
    '{"type":"LineString","coordinates":[[116.398,39.900],[116.403,39.900]]}',
    480.0,
    0.8,
    100,
    'fiber',
    'standard',
    '2019-11-05',
    'China Telecom',
    'active',
    99
);

-- 6. 供热管线（西长安街）
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom_json, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'HT-BJ-001',
    'West Changan Street Heating Line',
    'heat',
    '{"type":"LineString","coordinates":[[116.390,39.906],[116.395,39.906]]}',
    490.0,
    2.0,
    600,
    'steel',
    'PN25',
    '2008-10-20',
    'Beijing Heat Group',
    'active',
    85
);

-- ==========================================
-- 测试设施数据
-- ==========================================

-- 1. 阀门（天安门东路口）
INSERT INTO facilities (facility_id, facility_name, facility_type, longitude, latitude, elevation_m, pipeline_id, spec, material, build_date, owner, status, health_score)
VALUES (
    'VLV-BJ-001',
    'Tiananmen East Valve',
    'valve',
    116.402,
    39.905,
    45.5,
    'WS-BJ-001',
    'DN800',
    'ductile_iron',
    '2015-06-01',
    'Beijing Water Group',
    'active',
    94
);

-- 2. 检查井（长安街）
INSERT INTO facilities (facility_id, facility_name, facility_type, longitude, latitude, elevation_m, pipeline_id, spec, material, build_date, owner, status, health_score)
VALUES (
    'MH-BJ-001',
    'Changan Street Manhole',
    'manhole',
    116.398,
    39.908,
    44.8,
    'SW-BJ-001',
    '1200x800',
    'concrete',
    '2010-03-15',
    'Beijing Drainage Group',
    'active',
    90
);

-- 3. 阀门井（前门）
INSERT INTO facilities (facility_id, facility_name, facility_type, longitude, latitude, elevation_m, pipeline_id, spec, material, build_date, owner, status, health_score)
VALUES (
    'VLV-BJ-002',
    'Qianmen Gas Valve',
    'valve',
    16.400,
    39.902,
    45.2,
    'GAS-BJ-001',
    'DN400',
    'steel',
    '2012-09-20',
    'Beijing Gas Group',
    'active',
    93
);

-- ==========================================
-- 验证数据
-- ==========================================
SELECT COUNT(*) as total_pipelines FROM pipelines;
SELECT COUNT(*) as total_facilities FROM facilities;
SELECT pipeline_type, COUNT(*) as count FROM pipelines GROUP BY pipeline_type ORDER BY pipeline_type;

