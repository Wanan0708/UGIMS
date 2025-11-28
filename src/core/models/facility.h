#ifndef FACILITY_H
#define FACILITY_H

#include <QString>
#include <QDateTime>
#include <QPointF>

/**
 * @brief 设施数据模型
 * 对应数据库 facilities 表
 */
class Facility
{
public:
    Facility();

    // Getter
    int id() const { return m_id; }
    QString facilityId() const { return m_facilityId; }
    QString facilityName() const { return m_facilityName; }
    QString facilityType() const { return m_facilityType; }

    // 几何信息
    QPointF coordinate() const { return m_coordinate; }
    QString geomWkt() const { return m_geomWkt; }
    double elevationM() const { return m_elevationM; }

    // 物理属性
    QString spec() const { return m_spec; }
    QString material() const { return m_material; }
    QString size() const { return m_size; }

    // 关联管线
    QString pipelineId() const { return m_pipelineId; }

    // 建设信息
    QDate buildDate() const { return m_buildDate; }
    QString builder() const { return m_builder; }
    QString owner() const { return m_owner; }
    double assetValue() const { return m_assetValue; }

    // 运维信息
    QString status() const { return m_status; }
    int healthScore() const { return m_healthScore; }
    QDate lastMaintenance() const { return m_lastMaintenance; }
    QDate nextMaintenance() const { return m_nextMaintenance; }
    QString maintenanceUnit() const { return m_maintenanceUnit; }

    // 二维码
    QString qrcodeUrl() const { return m_qrcodeUrl; }

    // 元数据
    QString remarks() const { return m_remarks; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }
    QString createdBy() const { return m_createdBy; }
    QString updatedBy() const { return m_updatedBy; }

    // Setter
    void setId(int id) { m_id = id; }
    void setFacilityId(const QString &id) { m_facilityId = id; }
    void setFacilityName(const QString &name) { m_facilityName = name; }
    void setFacilityType(const QString &type) { m_facilityType = type; }

    // 几何信息
    void setCoordinate(const QPointF &coord) { m_coordinate = coord; }
    void setGeomWkt(const QString &wkt) { m_geomWkt = wkt; }
    void setElevationM(double elevation) { m_elevationM = elevation; }

    // 物理属性
    void setSpec(const QString &spec) { m_spec = spec; }
    void setMaterial(const QString &material) { m_material = material; }
    void setSize(const QString &size) { m_size = size; }

    // 关联管线
    void setPipelineId(const QString &id) { m_pipelineId = id; }

    // 建设信息
    void setBuildDate(const QDate &date) { m_buildDate = date; }
    void setBuilder(const QString &builder) { m_builder = builder; }
    void setOwner(const QString &owner) { m_owner = owner; }
    void setAssetValue(double value) { m_assetValue = value; }

    // 运维信息
    void setStatus(const QString &status) { m_status = status; }
    void setHealthScore(int score) { m_healthScore = score; }
    void setLastMaintenance(const QDate &date) { m_lastMaintenance = date; }
    void setNextMaintenance(const QDate &date) { m_nextMaintenance = date; }
    void setMaintenanceUnit(const QString &unit) { m_maintenanceUnit = unit; }

    // 二维码
    void setQrcodeUrl(const QString &url) { m_qrcodeUrl = url; }

    // 元数据
    void setRemarks(const QString &remarks) { m_remarks = remarks; }
    void setCreatedAt(const QDateTime &time) { m_createdAt = time; }
    void setUpdatedAt(const QDateTime &time) { m_updatedAt = time; }
    void setCreatedBy(const QString &user) { m_createdBy = user; }
    void setUpdatedBy(const QString &user) { m_updatedBy = user; }

    // 工具方法
    bool isValid() const;
    QString getDisplayName() const;
    QString getTypeDisplayName() const;

private:
    int m_id;
    QString m_facilityId;
    QString m_facilityName;
    QString m_facilityType;

    // 几何信息
    QPointF m_coordinate;
    QString m_geomWkt;
    double m_elevationM;

    // 物理属性
    QString m_spec;
    QString m_material;
    QString m_size;

    // 关联管线
    QString m_pipelineId;

    // 建设信息
    QDate m_buildDate;
    QString m_builder;
    QString m_owner;
    double m_assetValue;

    // 运维信息
    QString m_status;
    int m_healthScore;
    QDate m_lastMaintenance;
    QDate m_nextMaintenance;
    QString m_maintenanceUnit;

    // 二维码
    QString m_qrcodeUrl;

    // 元数据
    QString m_remarks;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QString m_createdBy;
    QString m_updatedBy;
};

#endif // FACILITY_H

