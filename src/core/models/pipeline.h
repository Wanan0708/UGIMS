#ifndef PIPELINE_H
#define PIPELINE_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QPointF>

/**
 * @brief 管线数据模型
 * 对应数据库 pipelines 表
 */
class Pipeline
{
public:
    Pipeline();

    // Getter
    int id() const { return m_id; }
    QString pipelineId() const { return m_pipelineId; }
    QString pipelineName() const { return m_pipelineName; }
    QString pipelineType() const { return m_pipelineType; }

    // 几何信息
    QVector<QPointF> coordinates() const { return m_coordinates; }
    QString geomWkt() const { return m_geomWkt; }
    double lengthM() const { return m_lengthM; }
    double depthM() const { return m_depthM; }

    // 物理属性
    int diameterMm() const { return m_diameterMm; }
    QString material() const { return m_material; }
    QString pressureClass() const { return m_pressureClass; }

    // 建设信息
    QDate buildDate() const { return m_buildDate; }
    QString builder() const { return m_builder; }
    QString owner() const { return m_owner; }
    double constructionCost() const { return m_constructionCost; }

    // 运维信息
    QString status() const { return m_status; }
    int healthScore() const { return m_healthScore; }
    QDate lastInspection() const { return m_lastInspection; }
    QString maintenanceUnit() const { return m_maintenanceUnit; }
    int inspectionCycle() const { return m_inspectionCycle; }

    // 元数据
    QString remarks() const { return m_remarks; }
    QDateTime createdAt() const { return m_createdAt; }
    QDateTime updatedAt() const { return m_updatedAt; }
    QString createdBy() const { return m_createdBy; }
    QString updatedBy() const { return m_updatedBy; }

    // Setter
    void setId(int id) { m_id = id; }
    void setPipelineId(const QString &id) { m_pipelineId = id; }
    void setPipelineName(const QString &name) { m_pipelineName = name; }
    void setPipelineType(const QString &type) { m_pipelineType = type; }

    // 几何信息
    void setCoordinates(const QVector<QPointF> &coords) { m_coordinates = coords; }
    void setGeomWkt(const QString &wkt) { m_geomWkt = wkt; }
    void setLengthM(double length) { m_lengthM = length; }
    void setDepthM(double depth) { m_depthM = depth; }

    // 物理属性
    void setDiameterMm(int diameter) { m_diameterMm = diameter; }
    void setMaterial(const QString &material) { m_material = material; }
    void setPressureClass(const QString &pressureClass) { m_pressureClass = pressureClass; }

    // 建设信息
    void setBuildDate(const QDate &date) { m_buildDate = date; }
    void setBuilder(const QString &builder) { m_builder = builder; }
    void setOwner(const QString &owner) { m_owner = owner; }
    void setConstructionCost(double cost) { m_constructionCost = cost; }

    // 运维信息
    void setStatus(const QString &status) { m_status = status; }
    void setHealthScore(int score) { m_healthScore = score; }
    void setLastInspection(const QDate &date) { m_lastInspection = date; }
    void setMaintenanceUnit(const QString &unit) { m_maintenanceUnit = unit; }
    void setInspectionCycle(int cycle) { m_inspectionCycle = cycle; }

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
    // 基础信息
    int m_id;
    QString m_pipelineId;
    QString m_pipelineName;
    QString m_pipelineType;

    // 几何信息
    QVector<QPointF> m_coordinates;  // 经纬度坐标列表
    QString m_geomWkt;               // WKT格式几何数据
    double m_lengthM;
    double m_depthM;

    // 物理属性
    int m_diameterMm;
    QString m_material;
    QString m_pressureClass;

    // 建设信息
    QDate m_buildDate;
    QString m_builder;
    QString m_owner;
    double m_constructionCost;

    // 运维信息
    QString m_status;
    int m_healthScore;
    QDate m_lastInspection;
    QString m_maintenanceUnit;
    int m_inspectionCycle;

    // 元数据
    QString m_remarks;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QString m_createdBy;
    QString m_updatedBy;
};

#endif // PIPELINE_H

