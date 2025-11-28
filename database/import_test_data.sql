-- UGIMS Test Data (UTF-8 Encoding)
-- Test pipeline and facility data for Beijing Tiananmen area

-- Clear existing data
TRUNCATE TABLE pipelines CASCADE;
TRUNCATE TABLE facilities CASCADE;

-- Test pipelines (Beijing Tiananmen area: Lon 116.39-116.42, Lat 39.90-39.92)

-- 1. Water supply pipeline (East side)
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'WS-BJ-001',
    'Tiananmen East Water Supply',
    'water_supply',
    ST_GeomFromText('LINESTRING(116.400 39.905, 116.405 39.905)', 4326),
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

-- 2. Sewage pipeline (Changan Street)
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'SW-BJ-001',
    'Changan Street Sewage',
    'sewage',
    ST_GeomFromText('LINESTRING(116.395 39.908, 116.400 39.908)', 4326),
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

-- 3. Gas pipeline (South side)
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'GAS-BJ-001',
    'Tiananmen South Gas',
    'gas',
    ST_GeomFromText('LINESTRING(116.395 39.902, 116.400 39.902)', 4326),
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

-- 4. Electric cable (East Changan)
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'ELC-BJ-001',
    'East Changan Power Cable',
    'electric',
    ST_GeomFromText('LINESTRING(116.402 39.910, 116.407 39.910)', 4326),
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

-- 5. Telecom cable (Qianmen)
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'TEL-BJ-001',
    'Qianmen Telecom Cable',
    'telecom',
    ST_GeomFromText('LINESTRING(116.398 39.900, 116.403 39.900)', 4326),
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

-- 6. Heating pipeline (West Changan)
INSERT INTO pipelines (pipeline_id, pipeline_name, pipeline_type, geom, length_m, depth_m, diameter_mm, material, pressure_class, build_date, owner, status, health_score)
VALUES (
    'HT-BJ-001',
    'West Changan Heating',
    'heat',
    ST_GeomFromText('LINESTRING(116.390 39.906, 116.395 39.906)', 4326),
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

-- Test facilities

-- 1. Valve (Tiananmen East)
INSERT INTO facilities (facility_id, facility_name, facility_type, geom, elevation_m, pipeline_id, spec, material, build_date, owner, status, health_score)
VALUES (
    'VLV-BJ-001',
    'Tiananmen East Valve',
    'valve',
    ST_GeomFromText('POINT(116.402 39.905)', 4326),
    45.5,
    'WS-BJ-001',
    'DN800',
    'ductile_iron',
    '2015-06-01',
    'Beijing Water Group',
    'normal',
    94
);

-- 2. Manhole (Changan Street)
INSERT INTO facilities (facility_id, facility_name, facility_type, geom, elevation_m, pipeline_id, spec, material, build_date, owner, status, health_score)
VALUES (
    'MH-BJ-001',
    'Changan Street Manhole',
    'manhole',
    ST_GeomFromText('POINT(116.398 39.908)', 4326),
    44.8,
    'SW-BJ-001',
    '1200x800',
    'concrete',
    '2010-03-15',
    'Beijing Drainage Group',
    'normal',
    90
);

-- 3. Gas valve (Qianmen)
INSERT INTO facilities (facility_id, facility_name, facility_type, geom, elevation_m, pipeline_id, spec, material, build_date, owner, status, health_score)
VALUES (
    'VLV-BJ-002',
    'Qianmen Gas Valve',
    'valve',
    ST_GeomFromText('POINT(116.400 39.902)', 4326),
    45.2,
    'GAS-BJ-001',
    'DN400',
    'steel',
    '2012-09-20',
    'Beijing Gas Group',
    'normal',
    93
);

-- Verify data
SELECT COUNT(*) as pipeline_count FROM pipelines;
SELECT COUNT(*) as facility_count FROM facilities;
SELECT pipeline_type, COUNT(*) as count FROM pipelines GROUP BY pipeline_type ORDER BY pipeline_type;

