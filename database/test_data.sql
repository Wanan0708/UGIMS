-- ==========================================
-- UGIMS 测试数据
-- 用于开发和测试
-- ==========================================

-- 清空现有测试数据（如果需要）
-- TRUNCATE TABLE pipelines CASCADE;
-- TRUNCATE TABLE facilities CASCADE;

-- ==========================================
-- 插入测试管线数据
-- ==========================================

-- 给水管线示例
INSERT INTO pipelines (
    pipeline_id, pipeline_name, pipeline_type,
    geom, length_m, depth_m,
    diameter_mm, material, pressure_class,
    build_date, builder, owner,
    status, health_score,
    remarks, created_by
) VALUES (
    'WL-2024-001',
    '天安门广场给水主干管',
    'water_supply',
    ST_GeomFromText('LINESTRING(116.397 39.904, 116.400 39.906, 116.403 39.908)', 4326),
    500.0,
    1.5,
    400,
    'ductile_iron',
    '1.0MPa',
    '2020-05-15',
    'XX建设集团',
    '北京市水务局',
    'active',
    85,
    '状态良好，定期巡检',
    'admin'
);

INSERT INTO pipelines (
    pipeline_id, pipeline_name, pipeline_type,
    geom, length_m, depth_m,
    diameter_mm, material, pressure_class,
    build_date, builder, owner,
    status, health_score,
    remarks, created_by
) VALUES (
    'WL-2024-002',
    '长安街给水支线',
    'water_supply',
    ST_GeomFromText('LINESTRING(116.403 39.908, 116.406 39.910, 116.409 39.912)', 4326),
    400.0,
    1.2,
    200,
    'pe',
    '0.6MPa',
    '2022-08-20',
    'YY水利工程公司',
    '北京市水务局',
    'active',
    95,
    '新建管线',
    'admin'
);

-- 排水管线示例
INSERT INTO pipelines (
    pipeline_id, pipeline_name, pipeline_type,
    geom, length_m, depth_m,
    diameter_mm, material, pressure_class,
    build_date, builder, owner,
    status, health_score,
    remarks, created_by
) VALUES (
    'SW-2024-001',
    '故宫雨水管道',
    'sewage',
    ST_GeomFromText('LINESTRING(116.395 39.915, 116.398 39.917, 116.401 39.919)', 4326),
    450.0,
    2.0,
    600,
    'concrete',
    NULL,
    '2018-03-10',
    'ZZ市政工程公司',
    '北京市排水集团',
    'active',
    70,
    '管道老化，需要更新',
    'admin'
);

-- 燃气管线示例
INSERT INTO pipelines (
    pipeline_id, pipeline_name, pipeline_type,
    geom, length_m, depth_m,
    diameter_mm, material, pressure_class,
    build_date, builder, owner,
    status, health_score,
    remarks, created_by
) VALUES (
    'GAS-2024-001',
    '西单燃气中压管',
    'gas',
    ST_GeomFromText('LINESTRING(116.365 39.912, 116.368 39.914, 116.371 39.916)', 4326),
    380.0,
    1.8,
    300,
    'steel',
    '0.4MPa',
    '2019-11-05',
    'AA燃气工程公司',
    '北京市燃气集团',
    'active',
    80,
    '定期检测',
    'admin'
);

-- 电力电缆示例
INSERT INTO pipelines (
    pipeline_id, pipeline_name, pipeline_type,
    geom, length_m, depth_m,
    diameter_mm, material, pressure_class,
    build_date, builder, owner,
    status, health_score,
    remarks, created_by
) VALUES (
    'EL-2024-001',
    '王府井电力主干线',
    'electric',
    ST_GeomFromText('LINESTRING(116.405 39.908, 116.408 39.910, 116.411 39.912)', 4326),
    420.0,
    1.0,
    NULL,
    'copper',
    '10kV',
    '2021-06-18',
    'BB电力公司',
    '国家电网北京公司',
    'active',
    90,
    '高压线路',
    'admin'
);

-- ==========================================
-- 插入测试设施数据
-- ==========================================

-- 阀门示例
INSERT INTO facilities (
    facility_id, facility_name, facility_type,
    geom, elevation_m,
    spec, material, size,
    pipeline_id,
    build_date, builder, owner,
    status, health_score,
    qrcode_url, remarks, created_by
) VALUES (
    'VALVE-001',
    '天安门1号阀门井',
    'valve',
    ST_SetSRID(ST_MakePoint(116.400, 39.906), 4326),
    45.0,
    'DN400闸阀',
    'ductile_iron',
    'DN400',
    'WL-2024-001',
    '2020-05-15',
    'XX建设集团',
    '北京市水务局',
    'normal',
    85,
    'https://qr.ugims.com/VALVE-001',
    '正常运行',
    'admin'
);

INSERT INTO facilities (
    facility_id, facility_name, facility_type,
    geom, elevation_m,
    spec, material, size,
    pipeline_id,
    build_date, builder, owner,
    status, health_score,
    qrcode_url, remarks, created_by
) VALUES (
    'VALVE-002',
    '天安门2号阀门井',
    'valve',
    ST_SetSRID(ST_MakePoint(116.403, 39.908), 4326),
    45.0,
    'DN400蝶阀',
    'ductile_iron',
    'DN400',
    'WL-2024-001',
    '2020-05-15',
    'XX建设集团',
    '北京市水务局',
    'normal',
    90,
    'https://qr.ugims.com/VALVE-002',
    '正常运行',
    'admin'
);

-- 检查井示例
INSERT INTO facilities (
    facility_id, facility_name, facility_type,
    geom, elevation_m,
    spec, material, size,
    pipeline_id,
    build_date, builder, owner,
    status, health_score,
    remarks, created_by
) VALUES (
    'MANHOLE-001',
    '故宫雨水检查井1号',
    'manhole',
    ST_SetSRID(ST_MakePoint(116.398, 39.917), 4326),
    44.0,
    '圆形检查井',
    'concrete',
    '直径1.2m',
    'SW-2024-001',
    '2018-03-10',
    'ZZ市政工程公司',
    '北京市排水集团',
    'normal',
    75,
    '井盖完好',
    'admin'
);

-- 泵站示例
INSERT INTO facilities (
    facility_id, facility_name, facility_type,
    geom, elevation_m,
    spec, material, size,
    pipeline_id,
    build_date, builder, owner,
    status, health_score,
    asset_value,
    remarks, created_by
) VALUES (
    'PUMP-001',
    '长安街加压泵站',
    'pump_station',
    ST_SetSRID(ST_MakePoint(116.406, 39.910), 4326),
    46.0,
    '一体化预制泵站',
    'stainless_steel',
    '5m³',
    'WL-2024-002',
    '2022-08-20',
    'YY水利工程公司',
    '北京市水务局',
    'normal',
    95,
    850000.00,
    '全自动控制系统',
    'admin'
);

-- ==========================================
-- 验证数据插入
-- ==========================================

-- 查询管线数量
SELECT pipeline_type, COUNT(*) as count
FROM pipelines
GROUP BY pipeline_type;

-- 查询设施数量
SELECT facility_type, COUNT(*) as count
FROM facilities
GROUP BY facility_type;

-- 测试空间查询：查找天安门广场附近的管线
SELECT 
    pipeline_id,
    pipeline_name,
    pipeline_type,
    ST_AsText(geom) as geometry,
    ST_Distance(
        geom::geography,
        ST_SetSRID(ST_MakePoint(116.400, 39.906), 4326)::geography
    ) as distance_meters
FROM pipelines
WHERE ST_DWithin(
    geom::geography,
    ST_SetSRID(ST_MakePoint(116.400, 39.906), 4326)::geography,
    1000
)
ORDER BY distance_meters;

-- 测试空间查询：查找指定范围内的设施
SELECT 
    facility_id,
    facility_name,
    facility_type,
    ST_AsText(geom) as geometry
FROM facilities
WHERE ST_Within(
    geom,
    ST_MakeEnvelope(116.395, 39.905, 116.410, 39.920, 4326)
);

