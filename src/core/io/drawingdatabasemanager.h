#ifndef DRAWINGDATABASEMANAGER_H
#define DRAWINGDATABASEMANAGER_H

#include <QGraphicsScene>
#include <QHash>
#include "core/models/pipeline.h"
#include "core/common/entitystate.h"  // 引入实体状态枚举

/**
 * @brief 绘制数据数据库管理器
 * 将绘制的管线和设施保存到数据库/从数据库加载
 */
class DrawingDatabaseManager
{
public:
    /**
     * @brief 保存绘制数据到数据库
     * @param scene 场景对象
     * @param pipelineHash 管线哈希表
     * @return 保存成功返回true
     */
    static bool saveToDatabase(QGraphicsScene *scene,
                               const QHash<QGraphicsItem*, Pipeline> &pipelineHash);
    
    /**
     * @brief 从数据库加载绘制数据
     * @param scene 场景对象
     * @param pipelineHash 管线哈希表（输出参数）
     * @param nextId 下一个管线ID（输出参数）
     * @return 加载成功返回true
     */
    static bool loadFromDatabase(QGraphicsScene *scene,
                                 QHash<QGraphicsItem*, Pipeline> &pipelineHash,
                                 int &nextId);
    
    /**
     * @brief 清空数据库中的绘制数据
     * @return 清空成功返回true
     */
    static bool clearDatabase();

private:
    /**
     * @brief 插入管线到数据库 (INSERT)
     * @param pathItem 路径图形项
     * @param pipeline 管线对象
     * @return 成功返回true
     */
    static bool insertPipelineToDatabase(QGraphicsPathItem *pathItem, const Pipeline &pipeline);
    
    /**
     * @brief 更新管线到数据库 (UPDATE)
     * @param pathItem 路径图形项
     * @param pipeline 管线对象
     * @return 成功返回true
     */
    static bool updatePipelineToDatabase(QGraphicsPathItem *pathItem, const Pipeline &pipeline);
    
    /**
     * @brief 从数据库删除管线 (DELETE)
     * @param pipelineId 管线ID
     * @return 成功返回true
     */
    static bool deletePipelineFromDatabase(const QString &pipelineId);
    
    /**
     * @brief 插入设施到数据库 (INSERT)
     * @param ellipseItem 椭圆图形项
     * @return 成功返回true
     */
    static bool insertFacilityToDatabase(QGraphicsEllipseItem *ellipseItem);
    
    /**
     * @brief 更新设施到数据库 (UPDATE)
     * @param ellipseItem 椭圆图形项
     * @return 成功返回true
     */
    static bool updateFacilityToDatabase(QGraphicsEllipseItem *ellipseItem);
    
    /**
     * @brief 从数据库删除设施 (DELETE)
     * @param facilityId 设施ID
     * @return 成功返回true
     */
    static bool deleteFacilityFromDatabase(const QString &facilityId);
    
    /**
     * @brief 从数据库加载管线并创建图形项
     * @param scene 场景对象
     * @param pipelineHash 管线哈希表（输出参数）
     * @return 加载的管线数量
     */
    static int loadPipelinesFromDatabase(QGraphicsScene *scene,
                                        QHash<QGraphicsItem*, Pipeline> &pipelineHash);
    
    /**
     * @brief 从数据库加载设施并创建图形项
     * @param scene 场景对象
     * @return 加载的设施数量
     */
    static int loadFacilitiesFromDatabase(QGraphicsScene *scene);
    
    /**
     * @brief 将QPainterPath转换为WKT格式的LINESTRING
     * @param path 路径对象
     * @return WKT字符串
     */
    static QString painterPathToWkt(const QPainterPath &path);
    
    /**
     * @brief 将WKT格式的LINESTRING转换为QPainterPath
     * @param wkt WKT字符串
     * @return 路径对象
     */
    static QPainterPath wktToPainterPath(const QString &wkt);
    
    /**
     * @brief 将点转换为WKT格式的POINT
     * @param point 点坐标
     * @return WKT字符串
     */
    static QString pointToWkt(const QPointF &point);
    
    /**
     * @brief 从WKT格式的POINT解析点坐标
     * @param wkt WKT字符串
     * @return 点坐标
     */
    static QPointF wktToPoint(const QString &wkt);
};

#endif // DRAWINGDATABASEMANAGER_H
