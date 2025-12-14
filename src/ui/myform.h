#ifndef MYFORM_H
#define MYFORM_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTreeView>
#include <QStandardItemModel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPoint>
#include <QPointF>
#include <QProgressBar>
#include <QTimer>
#include <QPointer>
#include <QScrollBar>
#include <QToolButton>
#include <QGraphicsProxyWidget>
#include <QPropertyAnimation>
#include <QDockWidget>
#include <QStackedWidget>
#include <QUndoStack>  // 撤销栈

// 添加TileMapManager的前置声明
class TileMapManager;
class LayerManager;
class LayerControlPanel;
class DrawingToolPanel;
class MapDrawingManager;
class Pipeline;  // 添加Pipeline前置声明
class Facility;  // 添加Facility前置声明
class QGraphicsEllipseItem;
class QGraphicsTextItem;
class QGraphicsPolygonItem;
struct BurstAnalysisResult;
struct ConnectivityResult;
enum class ConnectivityType;  // 前置声明
class MessageDialog;  // 消息对话框前向声明

namespace Ui {
class MyForm;
}

class MyForm : public QWidget
{
    Q_OBJECT

public:
    explicit MyForm(QWidget *parent = nullptr);
    ~MyForm();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;  // 添加键盘事件
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // 工具栏按钮槽函数
    void handleRefreshButtonClicked();  // 刷新数据（原打开按钮）
    void handleSaveButtonClicked();     // 保存
    void handleUndoButtonClicked();     // 撤销
    void handleRedoButtonClicked();     // 重做
    void handleMessageButtonClicked();  // 消息
    void handleProfileButtonClicked();  // 个人信息
    void onUnreadCountChanged(int count);  // 未读消息数量变化
    void handleUndoForPendingChanges(); // 处理撤销操作对待保存变更列表的影响
    void handleRedoForPendingChanges(); // 处理重做操作对待保存变更列表的影响
    
    // 地图相关槽函数
    void handleLoadMapButtonClicked();
    void handleZoomInButtonClicked();
    void handleZoomOutButtonClicked();
    void handlePanButtonClicked();
    
    // 瓦片地图相关槽函数
    void handleLoadTileMapButtonClicked();
    void handleZoomInTileMapButtonClicked();
    void handleZoomOutTileMapButtonClicked();
    
    // 瓦片下载进度槽函数
    void onTileDownloadProgress(int current, int total);
    void onRegionDownloadProgress(int current, int total, int zoom); // 区域下载进度槽函数

    // Overlay 相关槽
    void onOverlayPanToggled(bool checked);
    
    // 管网数据加载槽函数
    void loadPipelineData();
    void onViewTransformChanged();
    
    // 新功能区按钮槽函数
    // 数据与地图模块
    void onLoadDataButtonClicked();
    void onDownloadMapButtonClicked();
    
    // 空间分析模块
    void onBurstAnalysisButtonClicked();
    void onConnectivityAnalysisButtonClicked();
    void onHealthAssessmentButtonClicked();
    
    // 工单与资产模块
    void onWorkOrderButtonClicked();
    void onAssetManagementButtonClicked();
    
    // 工具模块
    void onDistanceMeasureButtonClicked();
    void onAreaMeasureButtonClicked();
    void onClearMeasureButtonClicked();
    
    // 信息模块
    void onSettingsButtonClicked();
    void onHelpButtonClicked();

    // 设备树相关槽函数
    void onDeviceTreeItemClicked(const QModelIndex &index);
    void onDeviceTreeItemDoubleClicked(const QModelIndex &index);
    void onDeviceSearchTextChanged(const QString &text);  // 搜索框文本变化
    void onDeviceTreeContextMenuRequested(const QPoint &pos);  // 设备树右键菜单
    void onAboutButtonClicked();  // 关于按钮点击
    
    // 设备树右键菜单操作
    void onDeviceTreeRefresh();  // 刷新设备树
    void onDeviceTreeExpandAll();  // 展开全部
    void onDeviceTreeCollapseAll();  // 折叠全部
    void onDeviceTreeExport();  // 导出设备数据
    void onDeviceTreeStatistics();  // 统计信息
    void onDeviceTreeViewInfo();  // 查看设备信息
    void onDeviceTreeEdit();  // 编辑设备
    void onDeviceTreeDelete();  // 删除设备
    void onDeviceTreeCopy();  // 复制设备
    void onDeviceTreeLocateOnMap();  // 在地图上定位
    void onDeviceTreeViewRelated();  // 查看关联设备
    void onDeviceTreeGenerateReport();  // 生成报告
    void onDeviceTreeViewHistory();  // 查看历史记录
    void onDeviceTreePrintLabel();  // 打印标签
    
    // 绘制工具相关槽函数
    void onToggleDrawingTool(bool checked);  // 切换绘制工具显示
    void onStartDrawingPipeline(const QString &pipelineType);  // 开始绘制管线
    void onStartDrawingFacility(const QString &facilityType);  // 开始绘制设施
    
    // 绘制完成槽函数
    void onPipelineDrawingFinished(const QString &pipelineType, const QString &wkt, const QVector<QPointF> &points, const QVector<QString> &connectedFacilityIds = QVector<QString>());
    void onFacilityDrawingFinished(const QString &facilityType, const QString &wkt, const QPointF &point);
    
    // 实体交互槽函数
    void onEntityClicked(QGraphicsItem *item);        // 实体单击
    void onEntityDoubleClicked(QGraphicsItem *item);  // 实体双击
    void onShowContextMenu(const QPoint &pos);        // 显示右键菜单
    void onDeleteSelectedEntity();                    // 删除选中实体
    void onEditSelectedEntity();                      // 编辑选中实体
    void onViewEntityProperties();                    // 查看实体属性
    void onCopyEntity();                              // 复制实体
    void onPasteEntity();                             // 粘贴实体
    void onCopyStyle();                               // 复制样式
    void onPasteStyle();                              // 粘贴样式
    void onDuplicateEntity();                         // 复制实体（原位复制）
    void onBringToFront();                            // 置于顶层
    void onSendToBack();                              // 置于底层
    void clearSelection();                            // 清除选中
    
    // 数据持久化槽函数
    void onSaveDrawingData();                         // 保存绘制数据
    void onLoadDrawingData();                         // 加载绘制数据

private:
    Ui::MyForm *ui;
    QString currentFile;
    bool isModified;
    
    // 地图相关成员
    QGraphicsScene *mapScene;
    QGraphicsPixmapItem *mapItem;
    qreal currentScale;
    int currentZoomLevel;  // 当前瓦片地图缩放层级 (1-10)
    double m_visualScale = 1.0;  // 视觉连续缩放比例（不直接切换瓦片层级）
    QPointF m_lastClickedGeo;    // 最近一次地图单击的地理坐标
    bool m_hasBurstPoint = false; // 是否已有爆管点
    QGraphicsEllipseItem *m_burstMarker = nullptr;
    QGraphicsTextItem *m_burstLabel = nullptr;
    QGraphicsPolygonItem *m_burstAreaItem = nullptr;
    bool m_burstSelectionMode = false; // 是否处于爆管点选模式
    QList<QGraphicsItem*> m_burstHighlights; // 高亮的管线/阀门
    QList<QWidget*> m_disabledDuringBurst;   // 暂时禁用的控件
    
    // 连通性分析相关成员
    bool m_connectivitySelectionMode = false; // 是否处于连通性分析点选模式
    QList<QGraphicsItem*> m_connectivityHighlights; // 高亮的管线
    QList<QWidget*> m_disabledDuringConnectivity;   // 暂时禁用的控件
    int m_currentConnectivityType; // 当前分析类型（存储为int，避免头文件依赖）
    
    // 最短路径分析相关成员
    bool m_shortestPathSelectionMode = false; // 是否处于最短路径点选模式
    QPointF m_shortestPathStartPoint; // 起点
    bool m_hasShortestPathStart = false; // 是否已选择起点
    QGraphicsEllipseItem *m_startMarker = nullptr; // 起点标记
    QGraphicsEllipseItem *m_endMarker = nullptr; // 终点标记
    QList<QWidget*> m_disabledDuringShortestPath; // 暂时禁用的控件
    
    // 距离量算相关成员
    struct DistanceMeasureResult {
        QList<QPointF> points; // 测量点列表
        QList<QGraphicsItem*> markers; // 标记点
        QGraphicsPathItem *line = nullptr; // 距离线
        QGraphicsTextItem *label = nullptr; // 距离标签
        double totalDistance = 0.0; // 总距离（米）
    };
    
    struct AreaMeasureResult {
        QList<QPointF> points; // 测量点列表
        QList<QGraphicsItem*> markers; // 标记点
        QGraphicsPolygonItem *polygon = nullptr; // 面积多边形
        QGraphicsTextItem *label = nullptr; // 面积标签
        double area = 0.0; // 面积（平方米）
    };
    
    bool m_distanceMeasureMode = false; // 是否处于距离量算模式
    QList<QPointF> m_currentDistancePoints; // 当前正在绘制的点列表
    QList<QGraphicsItem*> m_currentDistanceMarkers; // 当前正在绘制的标记
    QGraphicsPathItem *m_currentDistanceLine = nullptr; // 当前正在绘制的距离线（临时）
    QGraphicsPathItem *m_distancePreviewLine = nullptr; // 预览线（从最后一点到鼠标位置）
    QList<DistanceMeasureResult> m_distanceMeasureResults; // 所有距离测量结果
    
    bool m_areaMeasureMode = false; // 是否处于面积量算模式
    QList<QPointF> m_currentAreaPoints; // 当前正在绘制的点列表
    QList<QGraphicsItem*> m_currentAreaMarkers; // 当前正在绘制的标记
    QGraphicsPolygonItem *m_currentAreaPolygon = nullptr; // 当前正在绘制的面积多边形（临时）
    QGraphicsPathItem *m_areaPreviewLine = nullptr; // 预览线（从最后一点到鼠标位置，用于闭合）
    QList<AreaMeasureResult> m_areaMeasureResults; // 所有面积测量结果
    
    // 右键拖拽相关成员
    bool isRightClickDragging;
    QPoint lastRightClickPos;
    QPointF lastRightClickScenePos;
    
    // 缩放限制
    static constexpr int MIN_ZOOM_LEVEL = 3;   // 最小缩放层级（限制为3-10）
    static constexpr int MAX_ZOOM_LEVEL = 10;  // 最大缩放层级
    
    // 瓦片地图管理器
    TileMapManager *tileMapManager;
    
    // 图层管理器（管网可视化）
    LayerManager *m_layerManager;
    
    // 进度条相关
    QProgressBar *progressBar;
    bool isDownloading;
    
    // 拖动更新相关
    QTimer *viewUpdateTimer;  // 延迟更新定时器
    // 滚动条悬浮延迟展开
    QTimer *hScrollHoverTimer = nullptr;
    QTimer *vScrollHoverTimer = nullptr;
    bool hScrollExpanded = false;
    bool vScrollExpanded = false;
    QPointer<QScrollBar> hScrollTarget;
    QPointer<QScrollBar> vScrollTarget;

    // GraphicsView 右上角浮动工具（两种模式：viewport-widget 或 scene-proxy）
    QWidget *gvOverlay = nullptr;               // 直接贴在 viewport 上
    QGraphicsProxyWidget *overlayProxy = nullptr; // 作为场景项添加
    QWidget *overlayPanel = nullptr;            // 场景代理承载的面板
    QToolButton *btnZoomIn = nullptr;
    QToolButton *btnZoomOut = nullptr;
    QToolButton *btnPanToggle = nullptr;
    
    // 浮动状态栏相关
    QWidget *floatingStatusBar = nullptr;             // 浮动状态栏widget
    QLabel *floatingStatusLabel = nullptr;            // 浮动状态标签
    QProgressBar *floatingProgressBar = nullptr;      // 浮动进度条
    QTimer *statusBarFadeTimer = nullptr;             // 状态栏消失定时器
    QPropertyAnimation *statusBarOpacityAnim = nullptr; // 状态栏淡出动画
    void updateVisibleTiles();  // 更新可见瓦片
    void logMessage(const QString &message);
    
    // 设备树相关成员
    QStandardItemModel *deviceTreeModel;  // 设备树模型
    QModelIndex m_currentDeviceTreeIndex;  // 当前右键菜单选中的索引
    bool m_deviceTreeMenuActive;  // 右键菜单是否正在显示
    bool m_deviceTreeDialogActive;  // 设备详情对话框是否正在显示
    
    // 绘制工具相关成员
    QWidget *m_drawingToolContainer;       // 绘制工具容器（右侧滑出面板）
    DrawingToolPanel *m_drawingToolPanel;  // 绘制工具面板
    QPushButton *m_drawingToolToggleBtn;   // 浮动切换按钮（已废弃，改用底部按钮）
    MapDrawingManager *m_drawingManager;   // 地图绘制管理器
    
    // 图层控制面板相关成员
    QWidget *m_layerControlContainer;      // 图层控制容器（右侧滑出面板）
    LayerControlPanel *m_layerControlPanel; // 图层控制面板
    QPushButton *m_layerControlToggleBtn;  // 浮动切换按钮（已废弃，改用底部按钮）
    
    // 右侧工具栏和面板系统
    QWidget *m_panelSwitcher;              // 右侧工具栏（按钮容器）
    QPushButton *m_drawingToolBtn;         // 绘制工具按钮（右侧）
    QPushButton *m_layerControlBtn;        // 图层管理按钮（右侧）
    
    QWidget *m_panelContainer;             // 面板总容器（包含StackWidget+底部按钮）
    QStackedWidget *m_panelStack;          // 面板堆栈（切换绘制/图层面板）
    QPushButton *m_panelDrawingBtn;        // 面板内的绘制按钮
    QPushButton *m_panelLayerBtn;          // 面板内的图层按钮
    QPushButton *m_panelCloseBtn;          // 面板关闭按钮
    QString m_currentPanel;                // 当前显示的面板（"drawing" / "layer" / ""）
    
    // 消息和个人信息对话框
    MessageDialog *m_messageDialog;        // 消息对话框（单例，延迟创建）
    
    // 实体选中管理
    QGraphicsItem *m_selectedItem;         // 当前选中的图形项
    QPen m_originalPen;                    // 选中前的原始画笔（用于恢复）
    QBrush m_originalBrush;               // 选中前的原始画刷（用于恢复设施）
    QPointF m_selectedItemStartPos;        // 选中项的开始位置（用于移动撤销）
    QHash<QGraphicsItem*, Pipeline> m_drawnPipelines;  // 已绘制的管线数据（用于编辑）
    int m_nextPipelineId;                  // 下一个管线ID（自增）
    
    // 复制/粘贴功能
    QGraphicsItem *m_copiedItem;           // 复制的图形项（用于粘贴）
    QColor m_copiedColor;                  // 复制的颜色（样式复制）
    int m_copiedLineWidth;                 // 复制的线宽（样式复制）
    bool m_hasStyleCopied;                 // 是否复制了样式
    
    // 撤销/重做功能
    QUndoStack *m_undoStack;               // 撤销栈
    
    // 变更跟踪系统
    enum ChangeType {
        ChangeAdded,      // 新增
        ChangeModified,   // 修改
        ChangeDeleted     // 删除
    };
    
    struct PendingChange {
        ChangeType type;
        QString entityType;  // "pipeline" 或 "facility" 或 "workorder"
        QVariant data;       // Pipeline, Facility 或 WorkOrder 对象
        int originalId;       // 原始ID（用于修改和删除）
        QGraphicsItem *graphicsItem;  // 关联的图形项（用于管线/设施）
    };
    
    QList<PendingChange> m_pendingChanges;  // 待保存的变更列表
    bool m_hasUnsavedChanges;               // 是否有未保存的变更
    
    // 保存相关槽函数
    void onSaveAll();                       // 保存所有变更
    void onSaveAllTriggered();              // 保存按钮触发
    bool savePendingChanges();              // 执行保存操作
    void markAsModified();                  // 标记为已修改
    void clearPendingChanges();             // 清空待保存变更
    
    void setupFunctionalArea();
    void setupDeviceTree();  // 设置设备树
    QString formatPipelineDisplayName(const Pipeline &pipeline);  // 格式化管线显示名称
    QString formatFacilityDisplayName(const Facility &facility);  // 格式化设施显示名称
    QString getStatusIcon(const QString &status, int healthScore);  // 获取状态图标
    
    // 设备树辅助函数
    void expandAllChildren(const QModelIndex &index);  // 递归展开所有子节点
    void collapseAllChildren(const QModelIndex &index);  // 递归折叠所有子节点
    void countDevices(QStandardItem *item, int &pipelineCount, int &facilityCount);  // 统计设备数量
    void collectDevicesFromItem(QStandardItem *item, QList<Pipeline> &pipelines, QList<Facility> &facilities);  // 收集设备数据
    QPointF calculateCenter(const QVector<QPointF> &coordinates);  // 计算管线中心点
    
    void setupDrawingToolPanel();  // 设置绘制工具面板（右侧滑出）
    void positionDrawingToolPanel();  // 定位绘制工具面板
    void setupLayerControlPanel();  // 设置图层控制面板（右侧滑出）
    void positionLayerControlPanel();  // 定位图层控制面板
    void setupPanelSwitcher();  // 设置底部面板切换器
    void positionPanelSwitcher();  // 定位底部面板切换器
    void switchToPanel(const QString &panelName);  // 切换到指定面板
    void filterDeviceTree(const QString &searchText);  // 过滤设备树
    void setItemVisibility(QStandardItem *item, bool visible);  // 设置节点可见性
    bool filterItem(QStandardItem *item, const QString &searchText);  // 递归过滤节点
    void setupMapArea();
    void setupSplitter();
    void updateStatus(const QString &message);
    void updateUndoRedoButtonStates();    // 更新撤销/重做按钮状态
    void updateMessageButtonIcon(bool hasMessage);  // 更新消息按钮图标（有消息/无消息）
    void createFloatingStatusBar();       // 创建浮动状态栏
    void positionFloatingStatusBar();      // 定位浮动状态栏
    void updateFloatingProgressBar(int current, int total); // 更新浮动进度条
    void loadMap(const QString &mapPath);
    void createGraphicsOverlay();               // viewport 方案
    void positionGraphicsOverlay();
    void createGraphicsOverlayScene();          // scene+proxy 方案（兜底）
    void positionGraphicsOverlayScene();
    
    // 管网可视化初始化
    void initializePipelineVisualization();
    void checkPipelineRenderResult();
    QString resolveProjectPath(const QString &relativePath) const;
    bool runDatabaseInitScript();
    void promptDatabaseSetup(const QString &reason);
    void openConfigDirectory();
    void renderBurstPoint(const QPointF &geoLonLat);
    void renderBurstResult(const BurstAnalysisResult &result);
    void renderBurstHighlights(const BurstAnalysisResult &result);
    void clearBurstHighlights();
    void highlightPipelineById(const QString &pipelineId);
    void highlightFacilityById(const QString &facilityId);
    void setUiEnabledDuringBurst(bool enabled);
    void startBurstSelectionMode();
    void cancelBurstSelectionMode();
    void performBurstAnalysis(const QPointF &geoLonLat);
    
    // 连通性分析相关函数
    void clearConnectivityHighlights();
    void renderConnectivityResult(const ConnectivityResult &result);
    void highlightPipelineByIdForConnectivity(const QString &pipelineId, const QColor &color);
    void setUiEnabledDuringConnectivity(bool enabled);
    void startConnectivitySelectionMode(ConnectivityType type);
    void cancelConnectivitySelectionMode();
    void performConnectivityAnalysis(const QPointF &geoLonLat);
    
    // 最短路径分析相关函数
    void clearShortestPathMarkers();
    void renderShortestPathResult(const ConnectivityResult &result);
    void setUiEnabledDuringShortestPath(bool enabled);
    void startShortestPathSelectionMode();
    void cancelShortestPathSelectionMode();
    void handleShortestPathPointSelection(const QPointF &geoLonLat);
    void performShortestPathAnalysis(const QPointF &startPoint, const QPointF &endPoint);
    
    // 距离量算相关函数
    void startDistanceMeasureMode();
    void cancelDistanceMeasureMode();
    void handleDistanceMeasurePoint(const QPointF &geoLonLat);
    void finishCurrentDistanceMeasure(); // 完成当前距离测量
    void updateCurrentDistanceLine(); // 更新当前正在绘制的距离线
    void updateDistancePreview(const QPointF &mouseGeoPos); // 更新预览线
    void clearDistancePreview(); // 清除预览线
    void clearAllDistanceMeasures(); // 清除所有距离测量结果
    void deleteDistanceMeasure(QGraphicsItem *item); // 删除单条距离测量结果
    double calculateDistance(const QPointF &p1, const QPointF &p2); // 计算两点间距离（米）
    
    // 面积量算相关函数
    void startAreaMeasureMode();
    void cancelAreaMeasureMode();
    void handleAreaMeasurePoint(const QPointF &geoLonLat);
    void finishCurrentAreaMeasure(); // 完成当前面积测量
    void updateCurrentAreaPolygon(); // 更新当前正在绘制的面积多边形
    void updateAreaPreview(const QPointF &mouseGeoPos); // 更新预览线
    void clearAreaPreview(); // 清除预览线
    void clearAllAreaMeasures(); // 清除所有面积测量结果
    void deleteAreaMeasure(QGraphicsItem *item); // 删除单条面积测量结果
    double calculateArea(const QList<QPointF> &points); // 计算多边形面积（平方米）
    
    // 实体选中辅助方法
    void selectItem(QGraphicsItem *item);     // 选中项
    void highlightItem(QGraphicsItem *item);  // 高亮显示
    void unhighlightItem(QGraphicsItem *item);// 取消高亮
    bool isEntityItem(QGraphicsItem *item);   // 判断是否为实体项
    
    // 添加公共方法来触发区域下载
public:
    void startRegionDownload(); // 公共方法来触发区域下载
};

#endif // MYFORM_H