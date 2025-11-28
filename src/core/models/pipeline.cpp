#include "core/models/pipeline.h"

Pipeline::Pipeline()
    : m_id(0)
    , m_lengthM(0.0)
    , m_depthM(0.0)
    , m_diameterMm(0)
    , m_constructionCost(0.0)
    , m_healthScore(100)
    , m_inspectionCycle(365)
{
}

bool Pipeline::isValid() const
{
    return m_id > 0 && !m_pipelineId.isEmpty() && !m_pipelineType.isEmpty();
}

QString Pipeline::getDisplayName() const
{
    if (!m_pipelineName.isEmpty()) {
        return m_pipelineName;
    }
    return m_pipelineId;
}

QString Pipeline::getTypeDisplayName() const
{
    static QMap<QString, QString> typeMap = {
        {"water_supply", "给水管"},
        {"sewage", "排水管"},
        {"gas", "燃气管"},
        {"electric", "电力电缆"},
        {"telecom", "通信光缆"},
        {"heat", "供热管"}
    };

    return typeMap.value(m_pipelineType, m_pipelineType);
}

