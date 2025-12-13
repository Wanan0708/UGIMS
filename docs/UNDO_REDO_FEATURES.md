# 撤销和重做功能说明

## 📋 当前支持撤销/重做的操作

### ✅ 已实现的功能

#### 1. **删除实体** (`DeleteEntityCommand`)
- **操作**：删除选中的管线或设施
- **触发方式**：
  - 右键菜单 → "删除"
  - 选中实体后按 `Delete` 键
- **撤销/重做**：✅ 支持
- **实现位置**：`src/ui/myform.cpp::onDeleteSelectedEntity()`

**功能说明：**
- 删除操作会立即从地图上移除实体
- 支持撤销，撤销后实体会重新显示在地图上
- 支持重做，重做后实体会再次被删除
- 删除操作会记录到待保存变更列表（`m_pendingChanges`）

---

#### 2. **粘贴样式** (`ChangeStyleCommand`)
- **操作**：将复制的样式应用到选中的实体
- **触发方式**：
  - 右键菜单 → "粘贴样式"
  - 需要先复制一个实体的样式（复制样式功能）
- **撤销/重做**：✅ 支持
- **实现位置**：`src/ui/myform.cpp::onPasteStyle()`

**功能说明：**
- 可以复制一个实体的颜色和线宽样式
- 粘贴到另一个实体时，会保存旧样式以便撤销
- 支持撤销，撤销后恢复原来的样式
- 支持重做，重做后重新应用粘贴的样式

---

### ❌ 未实现撤销/重做的操作

#### 1. **绘制管线/设施**
- **操作**：在地图上绘制新的管线或设施
- **当前状态**：❌ 不支持撤销/重做
- **原因**：绘制完成后直接添加到场景，没有使用 `AddEntityCommand`

**建议改进：**
- 在 `onPipelineDrawingFinished()` 和 `onFacilityDrawingFinished()` 中使用 `AddEntityCommand`
- 这样绘制完成后可以撤销，删除刚绘制的实体

---

#### 2. **移动实体**
- **操作**：拖拽移动实体位置
- **当前状态**：❌ 不支持撤销/重做
- **原因**：虽然定义了 `MoveEntityCommand`，但代码中未使用

**建议改进：**
- 在实体移动时（`itemChange` 事件）使用 `MoveEntityCommand`
- `MoveEntityCommand` 支持合并连续移动，避免撤销栈过大

---

#### 3. **修改实体属性**
- **操作**：通过属性对话框修改实体的属性（名称、类型等）
- **当前状态**：❌ 不支持撤销/重做
- **原因**：属性修改直接保存，没有使用命令模式

**建议改进：**
- 创建 `ChangePropertyCommand` 类
- 在属性对话框确认时使用该命令

---

## 🔧 撤销栈配置

### 配置信息
```cpp
m_undoStack = new QUndoStack(this);
m_undoStack->setUndoLimit(50);  // 最多支持50步撤销
```

### 快捷键
- **撤销**：`Ctrl+Z`
- **重做**：`Ctrl+Y`

### 按钮
- **撤销按钮**：工具栏左上角，图标为左箭头
- **重做按钮**：工具栏左上角，图标为右箭头

---

## 📊 功能对比表

| 操作 | 支持撤销/重做 | 命令类 | 实现状态 |
|------|--------------|--------|---------|
| 删除实体 | ✅ | `DeleteEntityCommand` | ✅ 已实现 |
| 粘贴样式 | ✅ | `ChangeStyleCommand` | ✅ 已实现 |
| 绘制管线 | ❌ | `AddEntityCommand` | ⚠️ 已定义但未使用 |
| 绘制设施 | ❌ | `AddEntityCommand` | ⚠️ 已定义但未使用 |
| 移动实体 | ❌ | `MoveEntityCommand` | ⚠️ 已定义但未使用 |
| 修改属性 | ❌ | 无 | ❌ 未实现 |

---

## 💡 使用示例

### 删除实体并撤销
1. 选中一个管线或设施
2. 按 `Delete` 键或右键菜单选择"删除"
3. 实体从地图上消失
4. 按 `Ctrl+Z` 或点击撤销按钮
5. 实体重新显示在地图上

### 粘贴样式并撤销
1. 选中一个实体，复制其样式（如果有复制样式功能）
2. 选中另一个实体
3. 右键菜单选择"粘贴样式"
4. 样式被应用
5. 按 `Ctrl+Z` 撤销，样式恢复原样

---

## 🚀 改进建议

### 优先级高
1. **为绘制操作添加撤销支持**
   - 在 `onPipelineDrawingFinished()` 中使用 `AddEntityCommand`
   - 在 `onFacilityDrawingFinished()` 中使用 `AddEntityCommand`
   - 这样用户可以撤销刚绘制的实体

### 优先级中
2. **为移动操作添加撤销支持**
   - 监听实体的 `itemChange` 事件
   - 使用 `MoveEntityCommand` 记录移动
   - `MoveEntityCommand` 已支持合并连续移动

### 优先级低
3. **为属性修改添加撤销支持**
   - 创建 `ChangePropertyCommand` 类
   - 在属性对话框确认时使用

---

## 📝 相关文件

- **命令类定义**：`src/core/commands/drawcommand.h`
- **命令类实现**：`src/core/commands/drawcommand.cpp`
- **撤销栈使用**：`src/ui/myform.cpp`
- **撤销栈初始化**：`src/ui/myform.cpp::MyForm()` (第133行)

---

**最后更新**: 2025-01-27

