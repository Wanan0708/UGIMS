#include "idgenerator.h"
#include <QRegularExpression>
#include <QDebug>
#include <functional>
#include <QMap>

QString IdGenerator::getPipelineTypePrefix(const QString &pipelineType)
{
    static QMap<QString, QString> typeMap = {
        {"water_supply", "WS"},   // Water Supply - 给水
        {"sewage", "SS"},         // Sewage System - 排水
        {"gas", "GS"},            // Gas - 燃气
        {"electric", "EL"},       // Electric - 电力
        {"telecom", "TC"},        // Telecom - 通信
        {"heat", "HT"}            // Heat - 供热
    };
    
    return typeMap.value(pipelineType, "PL");  // 默认 PL (Pipeline)
}

QString IdGenerator::getFacilityTypePrefix(const QString &facilityType)
{
    static QMap<QString, QString> typeMap = {
        {"valve", "VALVE"},           // 阀门
        {"manhole", "MH"},            // Manhole - 井盖
        {"pump_station", "PS"},       // Pump Station - 泵站
        {"pressure_station", "PRS"},  // Pressure Station - 调压站
        {"transformer", "TR"},        // Transformer - 变压器
        {"junction_box", "JB"}        // Junction Box - 接线盒
    };
    
    return typeMap.value(facilityType, "FAC");  // 默认 FAC (Facility)
}

QString IdGenerator::generatePipelineId(const QString &pipelineType, const QDate &date)
{
    QString prefix = getPipelineTypePrefix(pipelineType);
    QString dateStr = date.toString("yyyyMMdd");
    QString baseId = QString("%1-%2").arg(prefix).arg(dateStr);
    
    // 检查编号是否存在，如果存在则生成下一个可用编号
    // 这里需要一个检查函数，但为了避免循环依赖，我们在调用时传入
    // 暂时先返回基础编号，序号部分由调用者处理
    return QString("%1-001").arg(baseId);
}

QString IdGenerator::generateFacilityId(const QString &facilityType, const QDate &date)
{
    QString prefix = getFacilityTypePrefix(facilityType);
    QString dateStr = date.toString("yyyyMMdd");
    QString baseId = QString("%1-%2").arg(prefix).arg(dateStr);
    
    return QString("%1-001").arg(baseId);
}

QString IdGenerator::generateWorkOrderId(const QDate &date)
{
    QString dateStr = date.toString("yyyyMMdd");
    QString baseId = QString("WO-%1").arg(dateStr);
    
    return QString("%1-001").arg(baseId);
}

bool IdGenerator::parseId(const QString &id, QString &type, QString &date, int &sequence)
{
    // 格式：TYPE-YYYYMMDD-NNN
    // 例如：GS-20251212-001
    QRegularExpression regex(R"(^([A-Z]+)-(\d{8})-(\d+)$)");
    QRegularExpressionMatch match = regex.match(id);
    
    if (match.hasMatch()) {
        type = match.captured(1);
        date = match.captured(2);
        sequence = match.captured(3).toInt();
        return true;
    }
    
    return false;
}

int IdGenerator::getNextSequence(const QString &prefix, std::function<bool(const QString&)> checkExists)
{
    int sequence = 1;
    QString testId;
    
    do {
        testId = QString("%1-%2").arg(prefix).arg(sequence, 3, 10, QChar('0'));
        if (!checkExists(testId)) {
            return sequence;
        }
        sequence++;
        
        // 防止无限循环，最多尝试10000次
        if (sequence > 10000) {
            qWarning() << "[IdGenerator] Too many attempts to generate unique ID";
            break;
        }
    } while (true);
    
    return sequence;
}

QString IdGenerator::generateNextAvailableId(const QString &baseId, 
                                            std::function<bool(const QString&)> checkExists)
{
    // 解析基础编号
    QString type, date;
    int sequence;
    
    if (parseId(baseId, type, date, sequence)) {
        // 如果基础编号已存在，生成下一个
        QString prefix = QString("%1-%2").arg(type).arg(date);
        int nextSeq = getNextSequence(prefix, checkExists);
        return QString("%1-%2").arg(prefix).arg(nextSeq, 3, 10, QChar('0'));
    } else {
        // 如果无法解析，尝试在末尾添加序号
        QString prefix = baseId;
        int nextSeq = getNextSequence(prefix, checkExists);
        return QString("%1-%2").arg(prefix).arg(nextSeq, 3, 10, QChar('0'));
    }
}

