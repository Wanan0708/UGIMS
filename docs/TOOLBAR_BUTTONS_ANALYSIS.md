# 工具栏按钮功能分析报告

## 📋 当前状态

### 六个工具栏按钮实现情况

| 按钮 | 当前功能 | 实现状态 | 问题 |
|------|---------|---------|------|
| **新建** | 清空 `currentFile`，设置 `isModified=false` | ⚠️ 不完整 | 没有真正的新建项目功能，不清空场景数据 |
| **打开** | 调用 `onLoadDrawingData()` 从数据库加载用户绘制的数据 | ⚠️ 不完整 | 只加载绘制数据，没有打开项目文件功能 |
| **保存** | 调用 `onSaveAll()` 保存待保存的变更到数据库 | ✅ 完整 | 功能正常，但缺少项目文件保存 |
| **另存为** | 调用 `onSaveDrawingData()` 保存绘制数据到数据库 | ⚠️ 不完整 | 与保存相同，没有文件选择对话框，没有另存为项目文件 |
| **撤销** | 使用 `QUndoStack` 撤销操作 | ✅ 完整 | 功能正常 |
| **重做** | 使用 `QUndoStack` 重做操作 | ✅ 完整 | 功能正常 |

---

## 🔍 详细分析

### 1. 新建按钮 (`handleNewButtonClicked`)

**当前实现：**
```cpp
void MyForm::handleNewButtonClicked()
{
    currentFile.clear();
    updateStatus("New document created");
    isModified = false;
}
```

**问题：**
- ❌ 没有清空地图场景中的绘制数据
- ❌ 没有清空图层状态
- ❌ 没有重置地图视图
- ❌ 没有提示用户保存未保存的变更
- ❌ 没有真正创建新项目

**应该实现：**
- ✅ 检查是否有未保存的变更，提示用户保存
- ✅ 清空所有用户绘制的数据
- ✅ 重置地图视图到默认状态
- ✅ 清空撤销栈
- ✅ 重置项目状态

---

### 2. 打开按钮 (`handleOpenButtonClicked`)

**当前实现：**
```cpp
void MyForm::handleOpenButtonClicked()
{
    onLoadDrawingData();  // 只加载用户绘制的数据
}
```

**问题：**
- ❌ 没有文件选择对话框
- ❌ 只从数据库加载，不支持打开项目文件
- ❌ 没有检查未保存的变更
- ❌ 没有加载项目配置（地图视图、图层状态等）

**应该实现：**
- ✅ 检查未保存的变更，提示用户保存
- ✅ 显示文件选择对话框（支持 `.ugims` 项目文件）
- ✅ 加载项目文件（包含绘制数据、地图视图、图层状态等）
- ✅ 支持打开 JSON 格式的绘制数据文件

---

### 3. 保存按钮 (`handleSaveButtonClicked`)

**当前实现：**
```cpp
void MyForm::handleSaveButtonClicked()
{
    onSaveAll();  // 保存待保存的变更到数据库
}
```

**功能：**
- ✅ 保存所有待保存的变更到数据库
- ✅ 显示保存结果提示

**问题：**
- ⚠️ 只保存到数据库，没有保存项目文件
- ⚠️ 如果 `currentFile` 为空，应该提示另存为

**应该改进：**
- ✅ 如果 `currentFile` 不为空，保存项目文件
- ✅ 如果 `currentFile` 为空，提示另存为
- ✅ 同时保存到数据库和项目文件

---

### 4. 另存为按钮 (`handleSaveAsButtonClicked`)

**当前实现：**
```cpp
void MyForm::handleSaveAsButtonClicked()
{
    onSaveDrawingData();  // 与保存相同，直接保存到数据库
}
```

**问题：**
- ❌ 没有文件选择对话框
- ❌ 与保存功能完全相同，没有"另存为"的概念
- ❌ 没有保存项目文件

**应该实现：**
- ✅ 显示文件选择对话框
- ✅ 保存项目文件（`.ugims` 格式）
- ✅ 更新 `currentFile` 路径
- ✅ 同时保存到数据库

---

### 5. 撤销按钮 (`handleUndoButtonClicked`)

**当前实现：**
```cpp
void MyForm::handleUndoButtonClicked()
{
    if (m_undoStack && m_undoStack->canUndo()) {
        QString text = m_undoStack->undoText();
        m_undoStack->undo();
        updateStatus("撤销: " + text);
    } else {
        updateStatus("无可撤销的操作");
    }
}
```

**状态：** ✅ 功能完整，无需改进

---

### 6. 重做按钮 (`handleRedoButtonClicked`)

**当前实现：**
```cpp
void MyForm::handleRedoButtonClicked()
{
    if (m_undoStack && m_undoStack->canRedo()) {
        QString text = m_undoStack->redoText();
        m_undoStack->redo();
        updateStatus("重做: " + text);
    } else {
        updateStatus("无可重做的操作");
    }
}
```

**状态：** ✅ 功能完整，无需改进

---

## 💡 改进建议

### 方案一：完善项目文件功能（推荐）

**新增项目文件格式 `.ugims`：**

```json
{
  "version": "1.0",
  "type": "UGIMSProject",
  "metadata": {
    "name": "项目名称",
    "createdAt": "2025-01-27T10:00:00",
    "modifiedAt": "2025-01-27T12:00:00",
    "author": "用户名"
  },
  "mapView": {
    "center": [116.3974, 39.9093],
    "zoom": 12,
    "bounds": {...}
  },
  "layers": {
    "visible": ["water_supply", "sewage"],
    "styles": {...}
  },
  "drawingData": {
    "pipelines": [...],
    "facilities": [...]
  },
  "settings": {
    "coordinateSystem": "WGS84",
    "units": "meters"
  }
}
```

**改进后的功能：**

1. **新建项目**
   - 检查未保存变更
   - 清空所有数据
   - 重置视图
   - 创建新项目

2. **打开项目**
   - 文件选择对话框
   - 加载项目文件
   - 恢复地图视图
   - 恢复图层状态
   - 加载绘制数据

3. **保存项目**
   - 如果已保存过，直接保存
   - 如果未保存，调用另存为

4. **另存为项目**
   - 文件选择对话框
   - 保存完整项目文件
   - 更新当前文件路径

---

### 方案二：保持当前设计，只改进现有功能

**最小改进：**

1. **新建按钮**
   - 添加未保存变更检查
   - 清空绘制数据
   - 重置视图

2. **打开按钮**
   - 添加文件选择对话框
   - 支持打开 JSON 绘制数据文件

3. **另存为按钮**
   - 添加文件选择对话框
   - 保存 JSON 绘制数据文件

---

## 🎯 推荐实施方案

**推荐采用方案一（完善项目文件功能）**，理由：

1. **更符合用户习惯**：用户期望有项目文件的概念
2. **更好的数据管理**：可以保存完整的工作状态
3. **更好的用户体验**：可以保存和恢复工作环境
4. **更专业**：符合专业GIS软件的设计模式

---

## 📝 实施步骤

### 第一步：创建项目文件管理器

1. 创建 `ProjectFileManager` 类
   - `saveProject()` - 保存项目文件
   - `loadProject()` - 加载项目文件
   - `validateProject()` - 验证项目文件格式

### 第二步：改进新建功能

1. 检查未保存变更
2. 清空所有数据
3. 重置视图和状态

### 第三步：改进打开功能

1. 添加文件选择对话框
2. 加载项目文件
3. 恢复所有状态

### 第四步：改进保存功能

1. 检查 `currentFile` 是否为空
2. 如果为空，调用另存为
3. 如果不为空，保存项目文件

### 第五步：改进另存为功能

1. 添加文件选择对话框
2. 保存项目文件
3. 更新 `currentFile`

---

## 🔧 技术实现要点

### 项目文件格式

- **格式**：JSON（易于阅读和调试）
- **扩展名**：`.ugims`
- **版本控制**：支持版本号，便于未来升级

### 数据包含内容

- 地图视图状态（中心点、缩放级别）
- 图层可见性和样式
- 用户绘制的管线和设施
- 项目元数据（名称、创建时间等）
- 用户设置（坐标系、单位等）

### 兼容性处理

- 保持对旧版本 JSON 绘制数据文件的兼容
- 支持从旧格式升级到新格式

---

**最后更新**: 2025-01-27

