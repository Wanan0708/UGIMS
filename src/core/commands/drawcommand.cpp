#include "core/commands/drawcommand.h"
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>

// ==========================================
// AddEntityCommand 实现
// ==========================================

AddEntityCommand::AddEntityCommand(QGraphicsScene *scene, 
                                 QGraphicsItem *item,
                                 QHash<QGraphicsItem*, Pipeline> *pipelineHash,
                                 const Pipeline &pipeline,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_item(item)
    , m_pipelineHash(pipelineHash)
    , m_pipeline(pipeline)
    , m_isFirstRedo(true)
{
    setText("添加实体");
}

void AddEntityCommand::undo()
{
    // 从场景中移除
    if (m_scene && m_item) {
        m_scene->removeItem(m_item);
    }
    
    // 从哈希表中移除
    if (m_pipelineHash && m_pipelineHash->contains(m_item)) {
        m_pipelineHash->remove(m_item);
    }
}

void AddEntityCommand::redo()
{
    if (m_isFirstRedo) {
        // 第一次执行时，实体已经添加到场景，不需要重复添加
        m_isFirstRedo = false;
        
        // 但需要添加到哈希表
        if (m_pipelineHash && m_pipeline.id() > 0) {
            (*m_pipelineHash)[m_item] = m_pipeline;
        }
    } else {
        // 撤销后重做：重新添加到场景
        if (m_scene && m_item) {
            m_scene->addItem(m_item);
        }
        
        // 重新添加到哈希表
        if (m_pipelineHash && m_pipeline.id() > 0) {
            (*m_pipelineHash)[m_item] = m_pipeline;
        }
    }
}

// ==========================================
// DeleteEntityCommand 实现
// ==========================================

DeleteEntityCommand::DeleteEntityCommand(QGraphicsScene *scene,
                                        QGraphicsItem *item,
                                        QHash<QGraphicsItem*, Pipeline> *pipelineHash,
                                        QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_item(item)
    , m_pipelineHash(pipelineHash)
    , m_hasPipeline(false)
{
    setText("删除实体");
    
    // 保存管线数据
    if (m_pipelineHash && m_pipelineHash->contains(item)) {
        m_pipeline = (*m_pipelineHash)[item];
        m_hasPipeline = true;
    }
}

void DeleteEntityCommand::undo()
{
    // 重新添加到场景
    if (m_scene && m_item) {
        m_scene->addItem(m_item);
    }
    
    // 重新添加到哈希表
    if (m_pipelineHash && m_hasPipeline) {
        (*m_pipelineHash)[m_item] = m_pipeline;
    }
}

void DeleteEntityCommand::redo()
{
    // 从场景中移除
    if (m_scene && m_item) {
        m_scene->removeItem(m_item);
    }
    
    // 从哈希表中移除
    if (m_pipelineHash && m_pipelineHash->contains(m_item)) {
        m_pipelineHash->remove(m_item);
    }
}

// ==========================================
// ChangeStyleCommand 实现
// ==========================================

ChangeStyleCommand::ChangeStyleCommand(QGraphicsItem *item,
                                      const QColor &oldColor,
                                      int oldWidth,
                                      const QColor &newColor,
                                      int newWidth,
                                      QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_item(item)
    , m_oldColor(oldColor)
    , m_oldWidth(oldWidth)
    , m_newColor(newColor)
    , m_newWidth(newWidth)
{
    setText("修改样式");
}

void ChangeStyleCommand::undo()
{
    applyStyle(m_oldColor, m_oldWidth);
}

void ChangeStyleCommand::redo()
{
    applyStyle(m_newColor, m_newWidth);
}

void ChangeStyleCommand::applyStyle(const QColor &color, int width)
{
    if (!m_item) {
        return;
    }
    
    // 保存到data
    m_item->setData(3, color);
    m_item->setData(4, width);
    
    // 应用到图形
    if (auto pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(m_item)) {
        QPen pen = pathItem->pen();
        pen.setColor(color);
        pen.setWidth(width);
        pathItem->setPen(pen);
    } else if (auto ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(m_item)) {
        ellipseItem->setBrush(QBrush(color));
        QPen pen = ellipseItem->pen();
        pen.setColor(color.darker(120));
        ellipseItem->setPen(pen);
    }
}

// ==========================================
// MoveEntityCommand 实现
// ==========================================

MoveEntityCommand::MoveEntityCommand(QGraphicsItem *item,
                                    const QPointF &oldPos,
                                    const QPointF &newPos,
                                    QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_item(item)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
    setText("移动实体");
}

void MoveEntityCommand::undo()
{
    if (m_item) {
        m_item->setPos(m_oldPos);
    }
}

void MoveEntityCommand::redo()
{
    if (m_item) {
        m_item->setPos(m_newPos);
    }
}

bool MoveEntityCommand::mergeWith(const QUndoCommand *other)
{
    // 只合并相同实体的移动命令
    const MoveEntityCommand *moveCommand = static_cast<const MoveEntityCommand*>(other);
    
    if (moveCommand->m_item != m_item) {
        return false;
    }
    
    // 更新新位置
    m_newPos = moveCommand->m_newPos;
    return true;
}

// ==========================================
// ChangePropertyCommand 实现
// ==========================================

ChangePropertyCommand::ChangePropertyCommand(QGraphicsItem *item,
                                             const QString &propertyName,
                                             const QVariant &oldValue,
                                             const QVariant &newValue,
                                             QHash<QGraphicsItem*, Pipeline> *pipelineHash,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_item(item)
    , m_propertyName(propertyName)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_pipelineHash(pipelineHash)
{
    setText(QString("修改%1").arg(propertyName));
}

void ChangePropertyCommand::undo()
{
    applyProperty(m_oldValue);
}

void ChangePropertyCommand::redo()
{
    applyProperty(m_newValue);
}

void ChangePropertyCommand::applyProperty(const QVariant &value)
{
    if (!m_item) {
        return;
    }
    
    QString entityType = m_item->data(0).toString();
    
    // 如果是管线，更新 pipelineHash 中的对象
    if (entityType == "pipeline" && m_pipelineHash && m_pipelineHash->contains(m_item)) {
        Pipeline &pipeline = (*m_pipelineHash)[m_item];
        
        if (m_propertyName == "名称" || m_propertyName == "pipelineName") {
            pipeline.setPipelineName(value.toString());
            m_item->setToolTip(QString("%1\n类型: %2\n管径: DN%3")
                              .arg(pipeline.pipelineName())
                              .arg(m_item->data(2).toString())
                              .arg(pipeline.diameterMm()));
        } else if (m_propertyName == "类型" || m_propertyName == "pipelineType") {
            pipeline.setPipelineType(value.toString());
        } else if (m_propertyName == "管径" || m_propertyName == "diameterMm") {
            pipeline.setDiameterMm(value.toInt());
            m_item->setToolTip(QString("%1\n类型: %2\n管径: DN%3")
                              .arg(pipeline.pipelineName())
                              .arg(m_item->data(2).toString())
                              .arg(pipeline.diameterMm()));
        }
    }
    
    // 如果是设施，更新工具提示
    if (entityType == "facility") {
        if (m_propertyName == "名称" || m_propertyName == "facilityName") {
            QString typeName = m_item->data(2).toString();
            m_item->setToolTip(QString("%1\n类型: %2")
                              .arg(value.toString())
                              .arg(typeName));
        }
    }
}

bool ChangePropertyCommand::mergeWith(const QUndoCommand *other)
{
    // 只合并相同实体和相同属性的连续修改命令
    const ChangePropertyCommand *propertyCommand = static_cast<const ChangePropertyCommand*>(other);
    
    if (propertyCommand->m_item != m_item || 
        propertyCommand->m_propertyName != m_propertyName) {
        return false;
    }
    
    // 更新新值（保留第一次的旧值，使用最后一次的新值）
    m_newValue = propertyCommand->m_newValue;
    return true;
}