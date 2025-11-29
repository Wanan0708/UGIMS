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
