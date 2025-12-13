# 撤销/重做功能完整实现

## ✅ 已实现的功能

### 1. 绘制管线/设施 - 撤销/重做支持

**实现位置：**
- `src/ui/myform.cpp::onPipelineDrawingFinished()` (第4104行)
- `src/ui/myform.cpp::onFacilityDrawingFinished()` (第4443行)

**实现方式：**
- 使用 `AddEntityCommand` 命令类
- 在绘制完成后立即创建命令并推入撤销栈
- 支持撤销：删除刚绘制的实体
- 支持重做：重新添加被撤销的实体

**代码示例：**
```cpp
// 绘制管线
AddEntityCommand *cmd = new AddEntityCommand(
    mapScene,
    item,
    &m_drawnPipelines,
    pipeline
);
if (m_undoStack) {
    m_undoStack->push(cmd);
}

// 绘制设施
AddEntityCommand *cmd = new AddEntityCommand(
    mapScene,
    ellipseItem,
    nullptr,  // 设施不使用 pipelineHash
    Pipeline()  // 设施不使用 Pipeline 对象
);
cmd->setText("添加设施");
if (m_undoStack) {
    m_undoStack->push(cmd);
}
```

---

### 2. 修改实体属性 - 撤销/重做支持

**实现位置：**
- `src/ui/myform.cpp::onEditSelectedEntity()` (第4852行)
- `src/core/commands/drawcommand.cpp::ChangePropertyCommand` (新增)

**实现方式：**
- 创建 `ChangePropertyCommand` 命令类
- 在属性对话框保存时，比较旧值和新值
- 如果属性发生变化，创建命令并推入撤销栈
- 支持撤销：恢复旧属性值
- 支持重做：重新应用新属性值

**支持修改的属性：**
- 管线：名称、类型、管径
- 设施：名称

**代码示例：**
```cpp
// 修改管线名称
if (oldName != newName) {
    ChangePropertyCommand *cmd = new ChangePropertyCommand(
        m_selectedItem,
        "名称",
        oldName,
        newName,
        &m_drawnPipelines
    );
    if (m_undoStack) {
        m_undoStack->push(cmd);
    }
}
```

---

### 3. 移动实体 - 撤销/重做支持

**实现位置：**
- `src/ui/myform.cpp::selectItem()` (第5099行)
- `src/ui/myform.cpp::clearSelection()` (第5090行)
- `src/ui/myform.cpp::eventFilter()` (第1300行)

**实现方式：**
- 使用 `MoveEntityCommand` 命令类（已存在）
- 在选中实体时记录开始位置
- 在取消选中或鼠标释放时检查位置是否变化
- 如果位置变化，创建命令并推入撤销栈
- 支持撤销：恢复原位置
- 支持重做：重新移动到新位置
- `MoveEntityCommand` 支持合并连续移动，避免撤销栈过大

**实现细节：**
- 仅对设施（`facility`）启用移动功能
- 选中时设置 `ItemIsMovable` 标志
- 取消选中时恢复不可移动状态
- 在鼠标释放和取消选中时检查位置变化

**代码示例：**
```cpp
// 选中实体时记录位置
m_selectedItemStartPos = item->pos();
if (entityType == "facility") {
    item->setFlag(QGraphicsItem::ItemIsMovable, true);
}

// 取消选中时检查位置变化
QPointF currentPos = m_selectedItem->pos();
if (currentPos != m_selectedItemStartPos) {
    MoveEntityCommand *cmd = new MoveEntityCommand(
        m_selectedItem,
        m_selectedItemStartPos,
        currentPos
    );
    if (m_undoStack) {
        m_undoStack->push(cmd);
    }
}
```

---

## 📊 完整功能列表

| 操作 | 支持撤销/重做 | 命令类 | 实现状态 |
|------|--------------|--------|---------|
| **删除实体** | ✅ | `DeleteEntityCommand` | ✅ 已实现 |
| **粘贴样式** | ✅ | `ChangeStyleCommand` | ✅ 已实现 |
| **绘制管线** | ✅ | `AddEntityCommand` | ✅ **新实现** |
| **绘制设施** | ✅ | `AddEntityCommand` | ✅ **新实现** |
| **移动实体** | ✅ | `MoveEntityCommand` | ✅ **新实现** |
| **修改属性** | ✅ | `ChangePropertyCommand` | ✅ **新实现** |

---

## 🔧 新增命令类

### ChangePropertyCommand

**文件：**
- `src/core/commands/drawcommand.h` (第113-134行)
- `src/core/commands/drawcommand.cpp` (第210-277行)

**功能：**
- 支持修改实体的属性（名称、类型等）
- 自动更新工具提示
- 支持管线和设施

**构造函数：**
```cpp
ChangePropertyCommand(
    QGraphicsItem *item,
    const QString &propertyName,
    const QVariant &oldValue,
    const QVariant &newValue,
    QHash<QGraphicsItem*, Pipeline> *pipelineHash = nullptr,
    QUndoCommand *parent = nullptr
);
```

---

## 💡 使用说明

### 撤销/重做操作

1. **撤销**：按 `Ctrl+Z` 或点击撤销按钮
2. **重做**：按 `Ctrl+Y` 或点击重做按钮
3. **撤销栈限制**：最多支持50步撤销

### 支持的操作

- ✅ 绘制新实体后可以撤销
- ✅ 删除实体后可以撤销
- ✅ 修改属性后可以撤销
- ✅ 移动实体后可以撤销
- ✅ 粘贴样式后可以撤销

### 注意事项

1. **移动实体**：仅设施支持移动，管线不支持（因为管线由路径定义）
2. **属性修改**：目前支持修改名称和类型，其他属性需要扩展 `ChangePropertyCommand`
3. **撤销栈**：撤销栈在刷新数据时会清空

---

## 🚀 改进效果

### 用户体验提升

1. **更安全的操作**：所有操作都可以撤销，减少误操作风险
2. **更流畅的工作流**：可以随时撤销和重做，提高工作效率
3. **更符合用户习惯**：与主流软件（如AutoCAD、QGIS）保持一致

### 技术优势

1. **命令模式**：使用命令模式实现，代码结构清晰
2. **可扩展性**：易于添加新的可撤销操作
3. **性能优化**：移动命令支持合并，避免撤销栈过大

---

**最后更新**: 2025-01-27

