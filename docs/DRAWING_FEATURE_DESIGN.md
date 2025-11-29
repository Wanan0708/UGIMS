# 管网绘制功能设计文档

## 1. 功能概述

实现在地图上交互式绘制管网数据的功能,包括管线(线状要素)和设施(点状要素)两大类。

## 2. 组件设计

### 2.1 DrawingToolPanel (绘制工具面板)

**位置**: 左侧设备树下方
**功能**: 提供管网类型选择工具

#### 管线类型 (6种)
- 💧 给水管 (water_supply)
- 🚰 排水管 (sewage)
- 🔥 燃气管 (gas)
- ⚡ 电力电缆 (electric)
- 📡 通信光缆 (telecom)
- 🌡️ 供热管 (heat)

#### 设施类型 (6种)
- 🔵 阀门 (valve)
- 🟢 井盖 (manhole)
- 🏗️ 泵站 (pump_station)
- 🔌 变压器 (transformer)
- ⚙️ 调压站 (regulator)
- 📦 接线盒 (junction_box)

## 3. 交互流程

### 3.1 绘制管线流程
```
1. 用户点击管线类型按钮
   ↓
2. 发送 startDrawingPipeline(type) 信号
   ↓
3. 地图进入折线绘制模式
   ↓
4. 用户在地图上点击添加节点
   - 左键点击: 添加节点
   - 右键/双击/Enter: 完成绘制
   - ESC: 取消绘制
   ↓
5. 完成后弹出属性编辑对话框
   ↓
6. 保存到数据库并渲染到地图
```

### 3.2 绘制设施流程
```
1. 用户点击设施类型按钮
   ↓
2. 发送 startDrawingFacility(type) 信号
   ↓
3. 地图进入点绘制模式
   ↓
4. 用户在地图上点击放置设施
   - 左键点击: 放置设施
   - ESC: 取消绘制
   ↓
5. 弹出属性编辑对话框
   ↓
6. 保存到数据库并渲染到地图
```

## 4. 后续需要实现的组件

### 4.1 MapDrawingManager (地图绘制管理器)
**职责**: 
- 管理地图上的绘制交互
- 处理鼠标事件
- 绘制临时图形
- 生成几何数据

**主要方法**:
```cpp
class MapDrawingManager : public QObject {
    // 开始绘制管线
    void startDrawingPipeline(const QString &type);
    
    // 开始绘制设施
    void startDrawingFacility(const QString &type);
    
    // 取消绘制
    void cancelDrawing();
    
    // 处理地图点击
    void handleMapClick(const QPointF &coordinate);
    
    // 处理鼠标移动(实时预览)
    void handleMouseMove(const QPointF &coordinate);
    
signals:
    // 绘制完成信号
    void pipelineDrawn(const QVector<QPointF> &coordinates);
    void facilityDrawn(const QPointF &coordinate);
};
```

### 4.2 PipelineEditDialog (管线属性编辑对话框)
**职责**: 编辑管线详细属性

**字段分组**:
- 基础信息: 编号、名称、类型
- 几何信息: 长度、埋深
- 物理属性: 管径、材质、压力等级
- 建设信息: 建设时间、建设单位、造价
- 运维信息: 状态、健康度、养护单位

### 4.3 FacilityEditDialog (设施属性编辑对话框)
**职责**: 编辑设施详细属性

**字段分组**:
- 基础信息: 编号、名称、类型
- 位置信息: 坐标、高程
- 物理属性: 规格型号、材质、尺寸
- 关联管线: 所属管线
- 运维信息: 状态、健康度、维护记录

### 4.4 集成到主窗口
在 `myform.ui` 的左侧面板中添加 DrawingToolPanel:

```xml
<widget class="QWidget" name="leftPanel">
  <layout class="QVBoxLayout">
    <!-- 设备树搜索框 -->
    <widget class="QLineEdit" name="deviceSearchBox"/>
    
    <!-- 设备树 -->
    <widget class="QTreeView" name="deviceTreeView"/>
    
    <!-- 绘制工具面板 (新增) -->
    <widget class="DrawingToolPanel" name="drawingToolPanel"/>
  </layout>
</widget>
```

## 5. 数据流

```
DrawingToolPanel (工具选择)
    ↓ 信号
MyForm (主窗口)
    ↓ 调用
MapDrawingManager (绘制管理)
    ↓ 交互
User (用户绘制)
    ↓ 完成
PipelineEditDialog / FacilityEditDialog (属性编辑)
    ↓ 保存
PipelineDAO / FacilityDAO (数据访问)
    ↓ 存储
Database (数据库)
    ↓ 读取
PipelineRenderer / FacilityRenderer (渲染)
    ↓ 显示
Map (地图)
```

## 6. 技术要点

### 6.1 坐标系统
- 地图坐标: 经纬度 (WGS84)
- 屏幕坐标: 像素坐标
- 需要实现坐标转换方法

### 6.2 几何数据格式
- 管线: WKT LINESTRING
  ```
  LINESTRING(116.404 39.915, 116.405 39.916, ...)
  ```
- 设施: WKT POINT
  ```
  POINT(116.404 39.915)
  ```

### 6.3 绘制状态管理
```cpp
enum DrawingMode {
    NoDrawing,          // 无绘制
    DrawingPolyline,    // 绘制折线(管线)
    DrawingPoint        // 绘制点(设施)
};
```

### 6.4 临时图形渲染
- 使用 QPainter 在地图上绘制临时图形
- 实时显示鼠标跟随的预览线段/点
- 已绘制的节点用小圆点标记

## 7. UI 样式设计

### 7.1 工具按钮样式
- 默认: 白色背景, 灰色边框
- 悬停: 浅蓝色背景, 蓝色边框
- 选中: 蓝色背景, 白色文字, 加粗

### 7.2 面板布局
- 固定宽度, 自适应高度
- 分组框使用图标+文字
- 按钮左对齐, 带Emoji图标

## 8. 下一步实施计划

1. ✅ **Phase 1**: 创建 DrawingToolPanel 组件 (已完成)
2. ⏳ **Phase 2**: 实现 MapDrawingManager 绘制管理器
3. ⏳ **Phase 3**: 创建属性编辑对话框
4. ⏳ **Phase 4**: 集成到主窗口
5. ⏳ **Phase 5**: 测试和优化

## 9. 相关文件

### 已创建
- `src/widgets/drawingtoolpanel.h` - 工具面板头文件
- `src/widgets/drawingtoolpanel.cpp` - 工具面板实现

### 需要创建
- `src/map/mapdrawingmanager.h` - 绘制管理器头文件
- `src/map/mapdrawingmanager.cpp` - 绘制管理器实现
- `src/widgets/pipelineeditdialog.h` - 管线编辑对话框
- `src/widgets/pipelineeditdialog.cpp` - 管线编辑对话框实现
- `src/widgets/facilityeditdialog.h` - 设施编辑对话框
- `src/widgets/facilityeditdialog.cpp` - 设施编辑对话框实现

### 需要修改
- `src/ui/myform.ui` - 添加工具面板到界面
- `src/ui/myform.h` - 添加信号槽连接
- `src/ui/myform.cpp` - 实现绘制逻辑
- `CustomTitleBarApp.pro` - 添加新文件到编译
