#ifndef DRAWCOMMAND_H
#define DRAWCOMMAND_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QHash>
#include "core/models/pipeline.h"

/**
 * @brief 绘制命令基类
 * 用于实现撤销/重做功能
 */

// ==========================================
// 添加实体命令
// ==========================================
class AddEntityCommand : public QUndoCommand
{
public:
    AddEntityCommand(QGraphicsScene *scene, 
                     QGraphicsItem *item,
                     QHash<QGraphicsItem*, Pipeline> *pipelineHash = nullptr,
                     const Pipeline &pipeline = Pipeline(),
                     QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *m_scene;
    QGraphicsItem *m_item;
    QHash<QGraphicsItem*, Pipeline> *m_pipelineHash;
    Pipeline m_pipeline;
    bool m_isFirstRedo;
};

// ==========================================
// 删除实体命令
// ==========================================
class DeleteEntityCommand : public QUndoCommand
{
public:
    DeleteEntityCommand(QGraphicsScene *scene,
                        QGraphicsItem *item,
                        QHash<QGraphicsItem*, Pipeline> *pipelineHash = nullptr,
                        QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;

private:
    QGraphicsScene *m_scene;
    QGraphicsItem *m_item;
    QHash<QGraphicsItem*, Pipeline> *m_pipelineHash;
    Pipeline m_pipeline;
    bool m_hasPipeline;
};

// ==========================================
// 修改样式命令
// ==========================================
class ChangeStyleCommand : public QUndoCommand
{
public:
    ChangeStyleCommand(QGraphicsItem *item,
                       const QColor &oldColor,
                       int oldWidth,
                       const QColor &newColor,
                       int newWidth,
                       QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;

private:
    QGraphicsItem *m_item;
    QColor m_oldColor;
    int m_oldWidth;
    QColor m_newColor;
    int m_newWidth;
    
    void applyStyle(const QColor &color, int width);
};

// ==========================================
// 移动实体命令
// ==========================================
class MoveEntityCommand : public QUndoCommand
{
public:
    MoveEntityCommand(QGraphicsItem *item,
                      const QPointF &oldPos,
                      const QPointF &newPos,
                      QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;
    
    // 合并相同实体的连续移动
    bool mergeWith(const QUndoCommand *other) override;
    int id() const override { return 1; }

private:
    QGraphicsItem *m_item;
    QPointF m_oldPos;
    QPointF m_newPos;
};

// ==========================================
// 修改属性命令
// ==========================================
class ChangePropertyCommand : public QUndoCommand
{
public:
    ChangePropertyCommand(QGraphicsItem *item,
                          const QString &propertyName,
                          const QVariant &oldValue,
                          const QVariant &newValue,
                          QHash<QGraphicsItem*, Pipeline> *pipelineHash = nullptr,
                          QUndoCommand *parent = nullptr);
    
    void undo() override;
    void redo() override;
    
    // 合并相同实体的连续属性修改
    bool mergeWith(const QUndoCommand *other) override;
    int id() const override { return 2; }

private:
    QGraphicsItem *m_item;
    QString m_propertyName;
    QVariant m_oldValue;
    QVariant m_newValue;
    QHash<QGraphicsItem*, Pipeline> *m_pipelineHash;
    
    void applyProperty(const QVariant &value);
};

#endif // DRAWCOMMAND_H
