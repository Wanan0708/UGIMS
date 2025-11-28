#include "core/models/facility.h"

Facility::Facility()
    : m_id(0)
    , m_elevationM(0.0)
    , m_assetValue(0.0)
    , m_healthScore(100)
{
}

bool Facility::isValid() const
{
    return m_id > 0 && !m_facilityId.isEmpty() && !m_facilityType.isEmpty();
}

QString Facility::getDisplayName() const
{
    if (!m_facilityName.isEmpty()) {
        return m_facilityName;
    }
    return m_facilityId;
}

QString Facility::getTypeDisplayName() const
{
    static QMap<QString, QString> typeMap = {
        {"valve", "阀门"},
        {"manhole", "井盖"},
        {"pump_station", "泵站"},
        {"transformer", "变压器"},
        {"regulator", "调压站"},
        {"junction_box", "接线盒"}
    };

    return typeMap.value(m_facilityType, m_facilityType);
}

