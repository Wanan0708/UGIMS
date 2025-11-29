#ifndef DRAWINGDATAMANAGER_H
#define DRAWINGDATAMANAGER_H

#include <QString>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QHash>
#include "core/models/pipeline.h"

/**
 * @brief 绘制数据管理器
 * 负责保存和加载绘制的管线和设施数据
 */
class DrawingDataManager
{
public:
    DrawingDataManager();
    ~DrawingDataManager();
    
    /**
     * @brief 保存绘制数据到JSON文件
     * @param filePath 文件路径
     * @param scene 图形场景
     * @param pipelineHash 管线数据哈希表
     * @return 是否成功
     */
    static bool saveToFile(const QString &filePath, 
                          QGraphicsScene *scene,
                          const QHash<QGraphicsItem*, Pipeline> &pipelineHash);
    
    /**
     * @brief 从JSON文件加载绘制数据
     * @param filePath 文件路径
     * @param scene 图形场景
     * @param pipelineHash 管线数据哈希表
     * @param nextId 下一个管线ID（用于自增）
     * @return 是否成功
     */
    static bool loadFromFile(const QString &filePath,
                            QGraphicsScene *scene,
                            QHash<QGraphicsItem*, Pipeline> &pipelineHash,
                            int &nextId);
    
private:
    /**
     * @brief 序列化图形项为JSON
     */
    static QJsonObject serializeGraphicsItem(QGraphicsItem *item);
    
    /**
     * @brief 从JSON反序列化图形项
     */
    static QGraphicsItem* deserializeGraphicsItem(const QJsonObject &json);
};

#endif // DRAWINGDATAMANAGER_H
