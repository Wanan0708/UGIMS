#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <QString>
#include <QDate>
#include <functional>

/**
 * @brief 自动编号生成器
 * 
 * 用于生成唯一的业务编号，格式：类型前缀-日期-序号
 * 例如：GS-20251212-001, WS-20251212-001
 */
class IdGenerator
{
public:
    /**
     * @brief 生成管线编号
     * @param pipelineType 管线类型（water_supply, gas, sewage等）
     * @param date 日期（默认为当前日期）
     * @return 生成的编号，例如：GS-20251212-001
     */
    static QString generatePipelineId(const QString &pipelineType, const QDate &date = QDate::currentDate());
    
    /**
     * @brief 生成设施编号
     * @param facilityType 设施类型（valve, manhole等）
     * @param date 日期（默认为当前日期）
     * @return 生成的编号，例如：VALVE-20251212-001
     */
    static QString generateFacilityId(const QString &facilityType, const QDate &date = QDate::currentDate());
    
    /**
     * @brief 生成工单编号
     * @param date 日期（默认为当前日期）
     * @return 生成的编号，例如：WO-20251212-001
     */
    static QString generateWorkOrderId(const QDate &date = QDate::currentDate());
    
    /**
     * @brief 获取类型前缀
     * @param pipelineType 管线类型
     * @return 类型前缀，例如：GS, WS, SS等
     */
    static QString getPipelineTypePrefix(const QString &pipelineType);
    
    /**
     * @brief 获取设施类型前缀
     * @param facilityType 设施类型
     * @return 类型前缀，例如：VALVE, MH, PS等
     */
    static QString getFacilityTypePrefix(const QString &facilityType);
    
    /**
     * @brief 检查编号是否存在，如果存在则生成下一个可用编号
     * @param baseId 基础编号（不含序号部分）
     * @param checkExists 检查函数，返回true表示编号已存在
     * @return 可用的编号
     */
    static QString generateNextAvailableId(const QString &baseId, 
                                          std::function<bool(const QString&)> checkExists);
    
    /**
     * @brief 解析编号，提取类型、日期和序号
     * @param id 编号
     * @param type 输出的类型前缀
     * @param date 输出的日期字符串
     * @param sequence 输出的序号
     * @return 是否解析成功
     */
    static bool parseId(const QString &id, QString &type, QString &date, int &sequence);

private:
    /**
     * @brief 生成下一个序号
     * @param prefix 编号前缀（类型-日期）
     * @param checkExists 检查函数
     * @return 下一个可用的序号（从1开始）
     */
    static int getNextSequence(const QString &prefix, std::function<bool(const QString&)> checkExists);
};

#endif // IDGENERATOR_H

