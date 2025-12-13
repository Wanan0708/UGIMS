# 工具栏按钮功能改进建议（重新评估）

## 🤔 是否需要项目文件功能？

### 项目特点分析

**UGIMS 是一个数据管理系统，不是绘图软件：**
- ✅ 主要数据存储在数据库中（SQLite/PostgreSQL）
- ✅ 多用户共享同一数据库
- ✅ 数据是持久化的，不需要"项目文件"来保存
- ✅ 系统启动时自动加载数据库中的数据

### 结论：**不需要完整的项目文件功能**

**理由：**
1. **数据统一管理**：所有管网数据应该存储在数据库中，而不是分散在项目文件中
2. **多用户协作**：多个用户应该访问同一数据库，而不是各自的项目文件
3. **数据持久化**：数据已经持久化在数据库中，不需要额外的项目文件
4. **系统架构**：这是一个企业级管理系统，不是个人绘图工具

---

## ✅ 推荐的改进方案

### 方案：改进现有功能，不引入项目文件

#### 1. **新建按钮** - 清空工作区

**功能：**
- 检查是否有未保存的变更，提示用户保存
- 清空用户绘制的数据（`m_drawnPipelines`）
- 重置地图视图到默认位置
- 清空撤销栈
- **不**清空数据库中的数据（数据库数据是持久化的）

**实现：**
```cpp
void MyForm::handleNewButtonClicked()
{
    // 检查未保存的变更
    if (m_hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "未保存的变更",
            "有未保存的变更，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        if (reply == QMessageBox::Save) {
            onSaveAll();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    // 清空用户绘制的数据
    clearUserDrawingData();
    
    // 重置视图
    resetMapView();
    
    // 清空撤销栈
    if (m_undoStack) {
        m_undoStack->clear();
    }
    
    currentFile.clear();
    m_hasUnsavedChanges = false;
    updateStatus("已清空工作区");
}
```

---

#### 2. **打开按钮** - 导入绘制数据

**功能：**
- 显示文件选择对话框
- 支持打开 JSON 格式的绘制数据文件（已有 `DrawingDataManager`）
- 导入到当前工作区（追加或替换）
- **不**加载项目文件，只导入绘制数据

**实现：**
```cpp
void MyForm::handleOpenButtonClicked()
{
    // 检查未保存的变更
    if (m_hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "未保存的变更",
            "有未保存的变更，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
        );
        if (reply == QMessageBox::Save) {
            onSaveAll();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    // 文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开绘制数据",
        "",
        "JSON文件 (*.json);;所有文件 (*.*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 加载JSON文件
    DrawingDataManager::loadFromFile(
        fileName,
        mapScene,
        m_drawnPipelines,
        m_nextPipelineId
    );
    
    currentFile = fileName;
    updateStatus("已加载绘制数据: " + QFileInfo(fileName).fileName());
}
```

---

#### 3. **保存按钮** - 保存到数据库

**功能：**
- 保存所有待保存的变更到数据库（已有功能）
- 如果 `currentFile` 不为空，同时保存到JSON文件（可选）
- **主要保存到数据库**，这是数据管理系统的主要存储方式

**当前实现已经很好，只需小改进：**
```cpp
void MyForm::handleSaveButtonClicked()
{
    // 保存到数据库（主要方式）
    onSaveAll();
    
    // 如果打开了JSON文件，也保存到文件（可选）
    if (!currentFile.isEmpty() && QFileInfo(currentFile).suffix() == "json") {
        DrawingDataManager::saveToFile(
            currentFile,
            mapScene,
            m_drawnPipelines
        );
    }
}
```

---

#### 4. **另存为按钮** - 导出绘制数据

**功能：**
- 显示文件选择对话框
- 保存用户绘制的数据到JSON文件（用于数据交换、备份）
- **不**保存项目文件，只保存绘制数据
- 更新 `currentFile`

**实现：**
```cpp
void MyForm::handleSaveAsButtonClicked()
{
    // 先保存到数据库
    onSaveAll();
    
    // 文件选择对话框
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出绘制数据",
        "",
        "JSON文件 (*.json);;所有文件 (*.*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 确保扩展名
    if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".json";
    }
    
    // 保存到文件
    bool success = DrawingDataManager::saveToFile(
        fileName,
        mapScene,
        m_drawnPipelines
    );
    
    if (success) {
        currentFile = fileName;
        updateStatus("已导出绘制数据: " + QFileInfo(fileName).fileName());
    }
}
```

---

#### 5. **撤销/重做按钮** - 保持不变

**当前实现已经很好，无需改进**

---

## 📊 功能对比

| 功能 | 项目文件方案 | 推荐方案（数据库驱动） |
|------|------------|---------------------|
| **数据存储** | 项目文件 + 数据库 | 主要：数据库，辅助：JSON文件（数据交换） |
| **多用户协作** | 每个用户有自己的项目文件 | 所有用户共享数据库 |
| **数据持久化** | 需要保存项目文件 | 数据自动持久化在数据库 |
| **数据备份** | 备份项目文件 | 备份数据库 |
| **复杂度** | 高（需要管理项目文件） | 低（数据库是单一数据源） |
| **符合系统架构** | ❌ 不符合（这是数据管理系统） | ✅ 符合（数据库驱动） |

---

## 🎯 最终建议

### ✅ 推荐：**不创建项目文件功能**

**原因：**
1. **系统定位**：这是数据管理系统，不是绘图软件
2. **数据架构**：数据应该统一存储在数据库中
3. **用户场景**：多用户协作，共享数据库
4. **维护成本**：项目文件会增加系统复杂度

### ✅ 推荐：**改进现有按钮功能**

**改进内容：**
1. **新建**：清空工作区，检查未保存变更
2. **打开**：添加文件选择对话框，支持导入JSON绘制数据
3. **保存**：保持当前功能（保存到数据库），可选保存到JSON文件
4. **另存为**：添加文件选择对话框，导出JSON绘制数据（用于数据交换）

### 📝 保留的功能

- ✅ JSON绘制数据文件的导入/导出（用于数据交换、备份）
- ✅ 数据库存储（主要数据存储方式）
- ✅ 撤销/重做功能（保持不变）

---

## 🔧 实施优先级

### 高优先级
1. **另存为按钮** - 添加文件选择对话框（用户最需要的功能）
2. **打开按钮** - 添加文件选择对话框（完善现有功能）

### 中优先级
3. **新建按钮** - 添加未保存变更检查（提升用户体验）

### 低优先级
4. **保存按钮** - 可选保存到JSON文件（锦上添花）

---

**结论：不需要项目文件功能，只需改进现有按钮，使其更符合数据库驱动的管理系统。**

**最后更新**: 2025-01-27

