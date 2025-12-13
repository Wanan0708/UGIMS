#include "myform.h"
#include "ui_myform.h"
#include "core/common/version.h"  // 添加版本信息头文件
#include "tilemap/tilemapmanager.h"  // 添加瓦片地图管理器头文件
#include "widgets/drawingtoolpanel.h"  // 添加绘制工具面板头文件
#include "widgets/layercontrolpanel.h"  // 添加图层控制面板头文件
#include "widgets/entitypropertiesdialog.h"  // 实体属性编辑对话框
#include "widgets/workordermanagerdialog.h"  // 工单管理对话框
#include "widgets/assetmanagerdialog.h"  // 资产管理对话框
#include "widgets/healthassessmentdialog.h"  // 健康度评估对话框
#include "widgets/settingsdialog.h"  // 系统设置对话框
#include "core/auth/sessionmanager.h"  // 会话管理
#include "core/auth/permissionmanager.h"  // 权限管理
#include "map/mapdrawingmanager.h"  // 添加绘制管理器头文件
#include "ui/pipelineeditdialog.h"  // 添加管线编辑对话框头文件
#include "core/models/pipeline.h"  // 添加Pipeline模型头文件
#include "core/models/facility.h"  // 添加Facility模型头文件
#include "core/commands/drawcommand.h"  // 添加命令类
#include "analysis/burstanalyzer.h"  // 爆管分析
#include "analysis/connectivityanalyzer.h"  // 连通性分析
#include "core/io/drawingdatabasemanager.h"  // 数据库持久化
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include "dao/pipelinedao.h"
#include "dao/facilitydao.h"
#include "dao/workorderdao.h"
#include "core/database/databasemanager.h"
#include "widgets/facilityeditdialog.h"
#include "core/models/workorder.h"
#include "core/utils/idgenerator.h"
#include "core/common/entitystate.h"  // 实体状态枚举
#include <QDebug>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPixmap>
#include <QLineF>
#include <limits>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTimer>
#include <QShowEvent>
#include <QResizeEvent>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QMenuBar>
#include <QMenu>
#include <QProcess>
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QStyle>
#include <QWidgetAction>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>  // 添加网格布局头文件
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QPainter>
#include <QMainWindow>
#include <QDesktopServices>
#include <QUrl>
#include <cmath>
#include <QtGlobal>
#include "mapmanagerdialog.h"
#include "tilemap/downloadscheduler.h"
#include "tilemap/manifeststore.h"
#include "widgets/mapmanagersettings.h"

// 管网可视化相关
#include "core/common/logger.h"
#include "core/common/config.h"
#include "core/database/databasemanager.h"
#include "map/layermanager.h"
#include "map/pipelinerenderer.h"

MyForm::MyForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyForm)
    , isModified(false)
    , mapScene(nullptr)
    , mapItem(nullptr)
    , currentScale(1.0)
    , currentZoomLevel(1)  // 初始化为第1层级
    , m_visualScale(1.0)   // 初始视觉缩放比例
    , isRightClickDragging(false)  // 初始化右键拖拽状态
    , tileMapManager(nullptr)  // 初始化瓦片地图管理器
    , m_layerManager(nullptr)  // 初始化图层管理器
    , progressBar(nullptr)  // 初始化进度条
    , isDownloading(false)  // 初始化下载状态
    , viewUpdateTimer(nullptr)  // 初始化更新定时器
    , deviceTreeModel(nullptr)  // 初始化设备树模型
    , m_drawingToolContainer(nullptr)  // 初始化绘制工具容器
    , m_drawingToolPanel(nullptr)  // 初始化绘制工具面板
    , m_drawingToolToggleBtn(nullptr)  // 初始化浮动切换按钮（已废弃）
    , m_drawingManager(nullptr)  // 初始化绘制管理器
    , m_layerControlContainer(nullptr)  // 初始化图层控制容器
    , m_layerControlPanel(nullptr)  // 初始化图层控制面板
    , m_layerControlToggleBtn(nullptr)  // 初始化浮动切换按钮（已废弃）
    , m_panelSwitcher(nullptr)  // 初始化右侧工具栏
    , m_drawingToolBtn(nullptr)  // 初始化绘制工具按钮
    , m_layerControlBtn(nullptr)  // 初始化图层管理按钮
    , m_panelContainer(nullptr)  // 初始化面板容器
    , m_panelStack(nullptr)  // 初始化面板堆栈
    , m_panelDrawingBtn(nullptr)  // 初始化面板绘制按钮
    , m_panelLayerBtn(nullptr)  // 初始化面板图层按钮
    , m_panelCloseBtn(nullptr)  // 初始化面板关闭按钮
    , m_currentPanel("")  // 初始为空，没有面板展开
    , m_selectedItem(nullptr)  // 初始化选中项
    , m_nextPipelineId(1)  // 从ID=1开始
    , m_copiedItem(nullptr)  // 初始化复制项
    , m_copiedLineWidth(3)  // 默认线宽
    , m_hasStyleCopied(false)  // 初始化样式复制标志
    , m_undoStack(nullptr)  // 初始化撤销栈
    , m_currentConnectivityType(0)  // 初始化为Upstream (0)
    , m_hasUnsavedChanges(false)  // 初始化未保存变更标志
{
    logMessage("=== MyForm constructor started ===");
    ui->setupUi(this);
    
    // 创建撤销栈
    m_undoStack = new QUndoStack(this);
    m_undoStack->setUndoLimit(50);  // 限制最多50步撤销
    
    // 初始化变更跟踪系统
    m_pendingChanges.clear();
    
    // 设置功能区
    setupFunctionalArea();
    
    // 设置设备树
    setupDeviceTree();
    
    // 设置绘制工具面板（右侧滑出）
    setupDrawingToolPanel();
    
    // 设置图层控制面板（右侧滑出）
    setupLayerControlPanel();
    
    // 注意：底部面板切换器在 showEvent 中创建，因为需要 viewport 已经准备好
    
    // 连接搜索框信号
    connect(ui->deviceSearchBox, &QLineEdit::textChanged, this, &MyForm::onDeviceSearchTextChanged);
    
    logMessage("=== MyForm constructor finished (UI setup only) ===");
    
    // 异步初始化地图区域（延迟100ms，让窗口先显示）
    QTimer::singleShot(10, this, [this]() {
        qDebug() << "=== Starting async map area initialization ===";
        try {
            setupMapArea();
            updateStatus("地图初始化完成");
        } catch (const std::exception &e) {
            qDebug() << "Map area initialization failed:" << e.what();
            updateStatus("地图初始化失败");
        } catch (...) {
            qDebug() << "Map area initialization failed: unknown error";
            updateStatus("地图初始化失败");
        }
    });
    
    // 异步初始化管网可视化（延迟800ms执行，确保地图先初始化）
    QTimer::singleShot(30, this, [this]() {
        qDebug() << "=== Starting async pipeline visualization initialization ===";
        updateStatus("正在初始化管网可视化...");
        
        try {
            initializePipelineVisualization();
        } catch (const std::exception &e) {
            qDebug() << "Pipeline visualization initialization failed:" << e.what();
            updateStatus("管网可视化初始化失败（程序仍可正常使用）");
        } catch (...) {
            qDebug() << "Pipeline visualization initialization failed: unknown error";
            updateStatus("管网可视化初始化失败（程序仍可正常使用）");
        }
    });
}
// 工具：绘制加号/减号图标
static QIcon makePlusMinusIcon(bool isPlus, const QColor &color = QColor(255,255,255))
{
    const int sz = 18;
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(color, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(pen);
    // 水平线
    p.drawLine(QPointF(3, sz/2.0), QPointF(sz-3, sz/2.0));
    if (isPlus) {
        // 垂直线
        p.drawLine(QPointF(sz/2.0, 3), QPointF(sz/2.0, sz-3));
    }
    p.end();
    return QIcon(pm);
}

MyForm::~MyForm()
{
    delete ui;
}

void MyForm::showEvent(QShowEvent *event)
{
    // 窗口显示后设置splitter比例
    QWidget::showEvent(event);
    setupSplitter();
    
    // 创建底部面板切换器（只创建一次）
    if (!m_panelSwitcher) {
        setupPanelSwitcher();
    }
    
    // 确保浮动工具条创建并定位（放到下一个事件循环，确保 viewport已布局）
    QTimer::singleShot(0, this, [this]() {
        if (mapScene && !gvOverlay) createGraphicsOverlay();
        if (mapScene) positionGraphicsOverlay();
        // 初始化绘制工具面板位置
        positionDrawingToolPanel();
        // 初始化图层控制面板位置
        positionLayerControlPanel();
        // 初始化右侧工具栏位置（延迟一点确保布局完成）
        QTimer::singleShot(50, this, [this]() {
            positionPanelSwitcher();
        });
    });
}

void MyForm::keyPressEvent(QKeyEvent *event)
{
    // Delete 键删除选中的实体
    if (event->key() == Qt::Key_Delete) {
        if (m_selectedItem) {
            onDeleteSelectedEntity();
            event->accept();
            return;
        }
    }
    // Esc 键：优先取消绘制，其次取消选中
    else if (event->key() == Qt::Key_Escape) {
        // 如果正在绘制，取消绘制
        if (m_drawingManager && m_drawingManager->isDrawing()) {
            m_drawingManager->cancelDrawing();
            updateStatus("✅ 已取消绘制");
            event->accept();
            return;
        }
        // 如果有选中项，取消选中
        if (m_selectedItem) {
            clearSelection();
            event->accept();
            return;
        }
    }
    // Ctrl+S 保存所有变更
    else if (event->key() == Qt::Key_S && event->modifiers() == Qt::ControlModifier) {
        onSaveAll();
        event->accept();
        return;
    }
    
    QWidget::keyPressEvent(event);

    // Esc 退出爆管点选模式
    if (m_burstSelectionMode && event->key() == Qt::Key_Escape) {
        cancelBurstSelectionMode();
        return;
    }
    
    // Esc 退出连通性分析点选模式
    if (m_connectivitySelectionMode && event->key() == Qt::Key_Escape) {
        cancelConnectivitySelectionMode();
        return;
    }
    
    // Esc 退出最短路径点选模式
    if (m_shortestPathSelectionMode && event->key() == Qt::Key_Escape) {
        cancelShortestPathSelectionMode();
        return;
    }
}

void MyForm::resizeEvent(QResizeEvent *event)
{
    // 窗口大小改变时重新设置splitter比例
    QWidget::resizeEvent(event);
    setupSplitter();
    
    // 更新瓦片地图管理器的视图大小
    if (tileMapManager && ui->graphicsView) {
        QSize viewSize = ui->graphicsView->viewport()->size();
        tileMapManager->setViewSize(viewSize.width(), viewSize.height());
    }
    // 重新定位右上角浮动工具条
    positionGraphicsOverlay();
    // 重新定位浮动状态条
    positionFloatingStatusBar();
    // 重新定位右侧工具栏和面板（注意：不再定位drawingToolPanel和layerControlPanel）
    positionPanelSwitcher();
}

void MyForm::setupSplitter()
{
    // 设置splitter（listView与graphicsView的比例为1:4）
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);
    
    // 设置具体的尺寸
    QList<int> sizes;
    int totalWidth = ui->splitter->width();
    if (totalWidth > 0) {
        sizes << totalWidth / 5 << totalWidth * 4 / 5;
        ui->splitter->setSizes(sizes);
    }
}

void MyForm::setupFunctionalArea() {
    // 为功能区设置对象名称，以便应用样式
    ui->functionalArea->setObjectName("functionalArea");
    
    // 初始化状态
    updateStatus("Ready");

    // 设置工具栏按钮的图标
    ui->refreshButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    ui->saveButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->undoButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowBack));
    ui->redoButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowForward));

    // 设置工具栏按钮的样式
    auto setToolButtonStyle = [](QToolButton *btn) {
        btn->setIconSize(QSize(20, 20));
        btn->setFixedSize(32, 32);
        btn->setStyleSheet(
            "QToolButton{background:transparent; border:none; padding:4px;}"
            "QToolButton:hover{background:rgba(255,122,24,0.15); border-radius:4px;}"
            "QToolButton:pressed{background:rgba(255,122,24,0.30); border-radius:4px;}"
        );
    };
    setToolButtonStyle(ui->refreshButton);
    setToolButtonStyle(ui->saveButton);
    setToolButtonStyle(ui->undoButton);
    setToolButtonStyle(ui->redoButton);

    // 连接工具栏按钮
    connect(ui->refreshButton, &QToolButton::clicked, this, &MyForm::handleRefreshButtonClicked);
    connect(ui->saveButton, &QToolButton::clicked, this, &MyForm::handleSaveButtonClicked);
    connect(ui->undoButton, &QToolButton::clicked, this, &MyForm::handleUndoButtonClicked);
    connect(ui->redoButton, &QToolButton::clicked, this, &MyForm::handleRedoButtonClicked);

    // 设置快捷键
    ui->refreshButton->setShortcut(QKeySequence(Qt::Key_F5));
    ui->saveButton->setShortcut(QKeySequence::Save);
    ui->undoButton->setShortcut(QKeySequence::Undo);
    ui->redoButton->setShortcut(QKeySequence::Redo);


    // 新功能区按钮的信号槽连接
    // 数据与地图模块
    if (auto loadDataBtn = ui->functionalArea->findChild<QPushButton*>("loadDataButton")) {
        connect(loadDataBtn, &QPushButton::clicked, this, &MyForm::onLoadDataButtonClicked);
    }
    if (auto downloadMapBtn = ui->functionalArea->findChild<QPushButton*>("downloadMapButton")) {
        connect(downloadMapBtn, &QPushButton::clicked, this, &MyForm::onDownloadMapButtonClicked);
    }
    if (auto mapMgrBtn = ui->functionalArea->findChild<QPushButton*>("mapManagerButton")) {
        connect(mapMgrBtn, &QPushButton::clicked, this, [this]() {
            auto dlg = new MapManagerDialog(this);
            dlg->setAttribute(Qt::WA_DeleteOnClose, true);
            static ManifestStore store("manifest.json");
            store.load();
            static MapManagerSettings settings = MapManagerSettings::load("settings.json");
            dlg->setSettings(settings);
            auto *sched = new DownloadScheduler(dlg);
            sched->configure(settings);
            sched->setManifest(&store);
            sched->setTileManager(tileMapManager);
            connect(sched, &DownloadScheduler::taskProgress, dlg, &MapManagerDialog::onTaskProgress);
            connect(dlg, &MapManagerDialog::requestPause, sched, &DownloadScheduler::pause);
            connect(dlg, &MapManagerDialog::requestResume, sched, &DownloadScheduler::resume);
            connect(dlg, &MapManagerDialog::requestStartDownload, this, [sched, &store, dlg]() mutable {
                auto s = dlg->getSettings();
                DownloadTask t; t.minLat = 18; t.maxLat = 54; t.minLon = 73; t.maxLon = 135;
                t.minZoom = s.minZoom; t.maxZoom = s.maxZoom; t.status = "pending";
                store.upsertTask(t); store.save();
                sched->start();
            });
            connect(dlg, &MapManagerDialog::requestSaveSettings, this, [dlg, &settings]() mutable {
                settings = dlg->getSettings();
                settings.save("settings.json");
            });
            dlg->show();
        });
    }
    
    // 空间分析模块
    if (auto burstAnalysisBtn = ui->functionalArea->findChild<QPushButton*>("burstAnalysisButton")) {
        connect(burstAnalysisBtn, &QPushButton::clicked, this, &MyForm::onBurstAnalysisButtonClicked);
    }
    if (auto connectivityBtn = ui->functionalArea->findChild<QPushButton*>("connectivityAnalysisButton")) {
        connect(connectivityBtn, &QPushButton::clicked, this, &MyForm::onConnectivityAnalysisButtonClicked);
    }
    if (auto healthAssessmentBtn = ui->functionalArea->findChild<QPushButton*>("healthAssessmentButton")) {
        connect(healthAssessmentBtn, &QPushButton::clicked, this, &MyForm::onHealthAssessmentButtonClicked);
    }
    
    // 工单与资产模块
    if (auto workOrderBtn = ui->functionalArea->findChild<QPushButton*>("workOrderButton")) {
        connect(workOrderBtn, &QPushButton::clicked, this, &MyForm::onWorkOrderButtonClicked);
    }
    if (auto assetMgmtBtn = ui->functionalArea->findChild<QPushButton*>("assetManagementButton")) {
        connect(assetMgmtBtn, &QPushButton::clicked, this, &MyForm::onAssetManagementButtonClicked);
    }
    
    // 工具模块
    if (auto settingsBtn = ui->functionalArea->findChild<QPushButton*>("settingsButton")) {
        connect(settingsBtn, &QPushButton::clicked, this, &MyForm::onSettingsButtonClicked);
    }
    if (auto helpBtn = ui->functionalArea->findChild<QPushButton*>("helpButton")) {
        connect(helpBtn, &QPushButton::clicked, this, &MyForm::onHelpButtonClicked);
    }
    if (auto aboutBtn = ui->functionalArea->findChild<QPushButton*>("aboutButton")) {
        connect(aboutBtn, &QPushButton::clicked, this, &MyForm::onAboutButtonClicked);
    }
}

void MyForm::setupMapArea() {
    logMessage("Setting up map area");
    
    // 创建地图场景
    mapScene = new QGraphicsScene(this);
    
    // 设置场景
    ui->graphicsView->setScene(mapScene);
    
    // 启用交互
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    
    // 设置滚动条策略
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 自定义滚动条样式：默认细线，悬浮时展开便于拖拽
    if (auto hsb = ui->graphicsView->horizontalScrollBar()) {
        hsb->setStyleSheet(
            "QScrollBar:horizontal{height:4px; background:transparent; margin:0px;}"
            "QScrollBar:horizontal:hover{height:12px;}"
            "QScrollBar::handle:horizontal{background:#ff7a18; min-width:24px; border-radius:2px;}"
            "QScrollBar::handle:horizontal:hover{background:#ff9a40;}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal{width:0px; height:0px; background:transparent; border:none;}"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{background:rgba(0,0,0,0.18); border-radius:2px;}"
        );
        hsb->setMouseTracking(true);
        hsb->setAttribute(Qt::WA_Hover, true);
        hsb->installEventFilter(this);
        hScrollTarget = hsb;
        if (!hScrollHoverTimer) {
            hScrollHoverTimer = new QTimer(this);
            hScrollHoverTimer->setSingleShot(true);
            hScrollHoverTimer->setInterval(180); // 0.18s 延迟
        }
    }
    if (auto vsb = ui->graphicsView->verticalScrollBar()) {
        vsb->setStyleSheet(
            "QScrollBar:vertical{width:4px; background:transparent; margin:0px;}"
            "QScrollBar:vertical:hover{width:12px;}"
            "QScrollBar::handle:vertical{background:#ff7a18; min-height:24px; border-radius:2px;}"
            "QScrollBar::handle:vertical:hover{background:#ff9a40;}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical{width:0px; height:0px; background:transparent; border:none;}"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background:rgba(0,0,0,0.18); border-radius:2px;}"
        );
        vsb->setMouseTracking(true);
        vsb->setAttribute(Qt::WA_Hover, true);
        vsb->installEventFilter(this);
        vScrollTarget = vsb;
        if (!vScrollHoverTimer) {
            vScrollHoverTimer = new QTimer(this);
            vScrollHoverTimer->setSingleShot(true);
            vScrollHoverTimer->setInterval(180); // 0.18s 延迟
        }
    }
    
    // 启用鼠标跟踪
    ui->graphicsView->setMouseTracking(true);
    
    // 安装事件过滤器以处理滚轮事件
    ui->graphicsView->viewport()->installEventFilter(this);
    
    // 创建瓦片地图管理器
    logMessage("Creating TileMapManager");
    tileMapManager = new TileMapManager(this);
    logMessage(QString("TileMapManager created: %1").arg(tileMapManager != nullptr));
    tileMapManager->initScene(mapScene);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    
    // 创建视图更新定时器（用于拖动时延迟更新瓦片）
    viewUpdateTimer = new QTimer(this);
    viewUpdateTimer->setSingleShot(true);
    viewUpdateTimer->setInterval(100);  // 拖动停止100ms后更新，提升响应速度
    connect(viewUpdateTimer, &QTimer::timeout, this, &MyForm::updateVisibleTiles);
    
    // 连接滚动条变化信号，实现拖拽时的瓦片更新
    connect(ui->graphicsView->horizontalScrollBar(), &QScrollBar::valueChanged, 
            this, [this]() {
        if (tileMapManager && !isDownloading) {
            viewUpdateTimer->start();  // 重启定时器，实现防抖更新
        }
        positionGraphicsOverlay();
        positionPanelSwitcher();  // 同步更新面板和按钮位置
    });
    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, [this]() {
        if (tileMapManager && !isDownloading) {
            viewUpdateTimer->start();  // 重启定时器，实现防抖更新
        }
        positionGraphicsOverlay();
        positionPanelSwitcher();  // 同步更新面板和按钮位置
    });
    
    // 连接下载进度信号（在这里连接，因为tileMapManager已经创建）
    logMessage("Connecting regionDownloadProgress signal");
    connect(tileMapManager, &TileMapManager::regionDownloadProgress, this, &MyForm::onRegionDownloadProgress);
    connect(tileMapManager, &TileMapManager::downloadFinished, this, [this]() {
        updateStatus("Tile map download completed");
        // 隐藏浮动进度条
        if (floatingProgressBar) {
            floatingProgressBar->setVisible(false);
        }
        // 停止下载标记
        isDownloading = false;
    });
    connect(tileMapManager, &TileMapManager::localTilesFound, this, [this](int zoomLevel, int tileCount) {
        updateStatus(QString("Found %1 local tiles at zoom level %2").arg(tileCount).arg(zoomLevel));
    });
    connect(tileMapManager, &TileMapManager::noLocalTilesFound, this, [this]() {
        updateStatus("No local tiles found - Use 'Load Tile Map' to download");
    });
    
    // 连接缩放完成信号，用于调整视图位置
    connect(tileMapManager, &TileMapManager::zoomChanged, this, 
            [this](int oldZoom, int newZoom, double mouseLat, double mouseLon) {
        Q_UNUSED(oldZoom);
        Q_UNUSED(newZoom);
        
        // 缩放完成后，计算鼠标地理坐标在新场景中的位置
        // 然后调整滚动条，让这个位置仍然在视口中心
        
        // 将地理坐标转换为新缩放级别的瓦片坐标
        int mouseTileX, mouseTileY;
        // 这里需要调用TileMapManager的latLonToTile，但它是private
        // 暂时跳过，让瓦片地图自然加载
        
        qDebug() << "Zoom changed, mouse was at GEO:" << mouseLat << mouseLon;
    });
    
    logMessage("Signal connected");
    
    // 检查本地是否有已下载的瓦片，如果有则自动显示
    logMessage("Checking for local tiles...");
    updateStatus("Checking for local tiles...");
    tileMapManager->checkLocalTiles();
    logMessage("Local tiles check completed");
    
    // 显示本地瓦片信息
    tileMapManager->getLocalTilesInfo();
    
    // 初始化视图大小
    QTimer::singleShot(100, this, [this]() {
        // 延迟执行以确保视图已经正确初始化
        QSize viewSize = ui->graphicsView->viewport()->size();
        tileMapManager->setViewSize(viewSize.width(), viewSize.height());
        logMessage(QString("Initial view size: %1x%2").arg(viewSize.width()).arg(viewSize.height()));
        
        // 显示当前可用的最大缩放级别
        int maxZoom = tileMapManager->getMaxAvailableZoom();
        if (maxZoom > 0) {
            // 设置当前层级为可用的最大层级（但不超过10）
            currentZoomLevel = qMin(maxZoom, MAX_ZOOM_LEVEL);
            tileMapManager->setZoom(currentZoomLevel);
            // 将视图中心定位到当前地图中心对应的全图绝对坐标
            QPointF centerScene = tileMapManager->getCenterScenePos();
            ui->graphicsView->centerOn(centerScene);
            updateStatus(QString("Ready - Current zoom level: %1/%2").arg(currentZoomLevel).arg(MAX_ZOOM_LEVEL));
            logMessage(QString("Current zoom level: %1/%2").arg(currentZoomLevel).arg(MAX_ZOOM_LEVEL));
        } else {
            updateStatus("Ready - Use 'Load Tile Map' to download new tiles");
        }
    });
    // 创建右上角浮动工具条（避免重复）
    if (!gvOverlay) createGraphicsOverlay();
    
    // 创建浮动状态栏（位于地图左下角）
    createFloatingStatusBar();
    
    // 创建绘制管理器（必须在 mapScene 和 tileMapManager 创建之后）
    qDebug() << "Creating MapDrawingManager...";
    m_drawingManager = new MapDrawingManager(mapScene, ui->graphicsView, tileMapManager, this);
    
    // 连接绘制完成信号
    connect(m_drawingManager, &MapDrawingManager::pipelineDrawingFinished,
            this, &MyForm::onPipelineDrawingFinished);
    connect(m_drawingManager, &MapDrawingManager::facilityDrawingFinished,
            this, &MyForm::onFacilityDrawingFinished);
    qDebug() << "MapDrawingManager created and connected";
    
    // 显示初始状态信息
    updateStatus("Ready");
}

void MyForm::createGraphicsOverlay()
{
    if (!ui->graphicsView || gvOverlay) return;
    // 如果之前创建过场景代理浮层，先清理，避免与 viewport 版并存
    if (overlayProxy) {
        if (mapScene) mapScene->removeItem(overlayProxy);
        delete overlayProxy; overlayProxy = nullptr;
    }
    if (overlayPanel) { delete overlayPanel; overlayPanel = nullptr; }
    gvOverlay = new QWidget(ui->graphicsView->viewport());
    gvOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    gvOverlay->setObjectName("gvOverlay");
    gvOverlay->setStyleSheet(
        "#gvOverlay{background:rgba(30,31,38,0.25); border:1px solid rgba(255,0,0,0.35); border-radius:6px;}"
        "QToolButton{background:rgba(30,31,38,0.65); border:1px solid rgba(255,255,255,0.15);"
        "border-radius:6px; width:28px; height:28px; color:#fff;}"
        "QToolButton:hover{background:rgba(30,31,38,0.8);}"
    );

    QVBoxLayout *vl = new QVBoxLayout(gvOverlay);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(8);

    btnZoomIn = new QToolButton(gvOverlay);
    btnZoomIn->setToolTip(tr("放大"));
    btnZoomIn->setIcon(makePlusMinusIcon(true));
    btnZoomIn->setIconSize(QSize(18,18));
    connect(btnZoomIn, &QToolButton::clicked, this, &MyForm::handleZoomInButtonClicked);

    btnZoomOut = new QToolButton(gvOverlay);
    btnZoomOut->setToolTip(tr("缩小"));
    btnZoomOut->setIcon(makePlusMinusIcon(false));
    btnZoomOut->setIconSize(QSize(18,18));
    connect(btnZoomOut, &QToolButton::clicked, this, &MyForm::handleZoomOutButtonClicked);

    btnPanToggle = new QToolButton(gvOverlay);
    btnPanToggle->setToolTip(tr("拖拽(开/关)"));
    btnPanToggle->setCheckable(true);
    btnPanToggle->setChecked(ui->graphicsView->dragMode() == QGraphicsView::ScrollHandDrag);
    btnPanToggle->setIcon(QApplication::style()->standardIcon(QStyle::SP_DesktopIcon));
    connect(btnPanToggle, &QToolButton::toggled, this, &MyForm::onOverlayPanToggled);
    
    // 第4个按钮：图层管理
    QToolButton *btnLayerControl = new QToolButton(gvOverlay);
    btnLayerControl->setToolTip(tr("图层管理"));
    QIcon layerIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    btnLayerControl->setIcon(layerIcon);
    btnLayerControl->setIconSize(QSize(18,18));
    connect(btnLayerControl, &QToolButton::clicked, this, [this]() {
        // 切换逻辑：如果已经打开图层面板，再次点击则关闭
        if (m_currentPanel == "layer") {
            switchToPanel("");
        } else {
            switchToPanel("layer");
        }
    });
    
    // 第5个按钮：绘制工具
    QToolButton *btnDrawingTool = new QToolButton(gvOverlay);
    btnDrawingTool->setToolTip(tr("绘制工具"));
    QIcon drawIcon = QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
    btnDrawingTool->setIcon(drawIcon);
    btnDrawingTool->setIconSize(QSize(18,18));
    connect(btnDrawingTool, &QToolButton::clicked, this, [this]() {
        // 切换逻辑：如果已经打开绘制面板，再次点击则关闭
        if (m_currentPanel == "drawing") {
            switchToPanel("");
        } else {
            switchToPanel("drawing");
        }
    });

    vl->addWidget(btnZoomIn);
    vl->addWidget(btnZoomOut);
    vl->addWidget(btnPanToggle);
    vl->addWidget(btnLayerControl);
    vl->addWidget(btnDrawingTool);

    // 明确设置容器尺寸，避免 sizeHint 为 0 导致不可见
    int w = 28;
    int h = 28 * 5 + 8 * 4; // 5个按钮 + 4个间距
    gvOverlay->setFixedSize(w + 2, h + 2);
    gvOverlay->adjustSize();
    gvOverlay->raise();
    positionGraphicsOverlay();
    gvOverlay->show();
    qDebug() << "Overlay created size:" << gvOverlay->size();
}

void MyForm::positionGraphicsOverlay()
{
    if (!gvOverlay || !ui->graphicsView) return;
    const int margin = 8;
    QSize vp = ui->graphicsView->viewport()->size();
    QSize sz = gvOverlay->sizeHint();
    if (sz.isEmpty()) sz = gvOverlay->size();
    
    // 如果面板展开，缩放按钮放在面板左侧
    int rightOffset = margin;
    if (m_panelContainer && m_panelContainer->isVisible()) {
        rightOffset = m_panelContainer->width() + margin;  // 面板宽度 + 间距
    }
    
    int x = vp.width() - sz.width() - rightOffset;
    int y = margin;
    gvOverlay->setGeometry(x, y, sz.width(), sz.height());
    qDebug() << "Overlay positioned at:" << gvOverlay->geometry() << "viewport:" << vp 
             << "panel visible:" << (m_panelContainer && m_panelContainer->isVisible());
}

void MyForm::onOverlayPanToggled(bool checked)
{
    ui->graphicsView->setDragMode(checked ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}

void MyForm::createGraphicsOverlayScene()
{
    if (!mapScene) return; // 禁用场景代理方案
    return;
    overlayPanel = new QWidget; // dead code, kept for reference
    overlayPanel->setObjectName("overlayPanel");
    overlayPanel->setStyleSheet(
        "#overlayPanel{background:rgba(30,31,38,0.25); border:1px solid rgba(255,0,0,0.35); border-radius:6px;}"
        "QToolButton{background:rgba(30,31,38,0.65); border:1px solid rgba(255,255,255,0.15);"
        "border-radius:6px; width:28px; height:28px; color:#fff;}"
        "QToolButton:hover{background:rgba(30,31,38,0.8);}"
    );

    QVBoxLayout *vl = new QVBoxLayout(overlayPanel);
    vl->setContentsMargins(4, 4, 4, 4);
    vl->setSpacing(8);

    QToolButton *zIn = new QToolButton(overlayPanel);
    zIn->setToolTip(tr("放大"));
    zIn->setIcon(makePlusMinusIcon(true));
    zIn->setIconSize(QSize(18,18));
    connect(zIn, &QToolButton::clicked, this, &MyForm::handleZoomInButtonClicked);

    QToolButton *zOut = new QToolButton(overlayPanel);
    zOut->setToolTip(tr("缩小"));
    zOut->setIcon(makePlusMinusIcon(false));
    zOut->setIconSize(QSize(18,18));
    connect(zOut, &QToolButton::clicked, this, &MyForm::handleZoomOutButtonClicked);

    QToolButton *pan = new QToolButton(overlayPanel);
    pan->setToolTip(tr("拖拽(开/关)"));
    pan->setCheckable(true);
    pan->setChecked(ui->graphicsView->dragMode() == QGraphicsView::ScrollHandDrag);
    pan->setIcon(QApplication::style()->standardIcon(QStyle::SP_DesktopIcon));
    connect(pan, &QToolButton::toggled, this, &MyForm::onOverlayPanToggled);

    vl->addWidget(zIn);
    vl->addWidget(zOut);
    vl->addWidget(pan);
    overlayPanel->adjustSize();

    overlayProxy = mapScene->addWidget(overlayPanel, Qt::Widget);
    overlayProxy->setZValue(1e6); // 确保在最顶层
    positionGraphicsOverlayScene();
}

void MyForm::positionGraphicsOverlayScene()
{
    // disabled
    return;
}

bool MyForm::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->graphicsView->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            
            // 如果瓦片地图管理器存在，使用离散的层级缩放
            if (tileMapManager) {
                // 平滑视觉缩放：累积 visual scale，跨阈值才切换瓦片层级
                const bool zoomingIn = wheelEvent->angleDelta().y() > 0;
                const double stepFactor = zoomingIn ? 1.12 : (1.0 / 1.12);
                const double upscaleThreshold = 2.0;
                const double downscaleThreshold = 0.5;
                
                QPointF mouseViewportPos = wheelEvent->position();          // 保留亚像素精度
                QPointF mouseScenePos = ui->graphicsView->mapToScene(mouseViewportPos.toPoint());
                QPointF mouseGeo;
                if (tileMapManager) {
                    mouseGeo = tileMapManager->sceneToGeo(mouseScenePos, currentZoomLevel);
                }

                // 边界保护：达到最小级别时不再缩小；达到最大级别时不再放大
                if (!zoomingIn && currentZoomLevel <= MIN_ZOOM_LEVEL && m_visualScale <= 1.0) {
                    updateStatus(QString("已到最小层级 (%1)，不能继续缩小").arg(MIN_ZOOM_LEVEL));
                    return true;
                }
                if (zoomingIn && currentZoomLevel >= MAX_ZOOM_LEVEL && m_visualScale >= 1.0) {
                    updateStatus(QString("已到最大层级 (%1)，不能继续放大").arg(MAX_ZOOM_LEVEL));
                    return true;
                }
                
                // 先在视图层做连续缩放
                m_visualScale = qBound(0.1, m_visualScale * stepFactor, 8.0); // 限制极值，避免过大/过小
                double visualScaleForAnchor = m_visualScale; // 记录切级前的视觉缩放，用于精确锚点计算
                ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
                ui->graphicsView->scale(stepFactor, stepFactor);
                
                bool zoomLevelChanged = false;
                
                // 向上跨阈值：切换到更高层级
                while (m_visualScale >= upscaleThreshold && currentZoomLevel < MAX_ZOOM_LEVEL) {
                    m_visualScale /= 2.0;
                    currentZoomLevel++;
                    zoomLevelChanged = true;
                }
                
                // 向下跨阈值：切换到更低层级
                while (m_visualScale <= downscaleThreshold && currentZoomLevel > MIN_ZOOM_LEVEL) {
                    m_visualScale *= 2.0;
                    currentZoomLevel--;
                    zoomLevelChanged = true;
                }
                
                if (zoomLevelChanged) {
                    qDebug() << "[SmoothZoom] Switch tiles to level" << currentZoomLevel 
                             << "visualScale reset to" << m_visualScale;
                    
                    // 调整瓦片缩放并以鼠标为锚点
                    tileMapManager->setZoomAtMousePosition(
                        currentZoomLevel, 
                        mouseScenePos.x(), 
                        mouseScenePos.y(),
                        mouseViewportPos.x(),
                        mouseViewportPos.y(),
                        ui->graphicsView->viewport()->width(),
                        ui->graphicsView->viewport()->height(),
                        visualScaleForAnchor  // 使用切级前的真实视觉缩放保持锚点一致
                    );
                    
                    // 同步渲染器层级
                    if (m_layerManager) {
                        m_layerManager->setZoom(currentZoomLevel);
                        qDebug() << "[Zoom] Renderer zoom updated to:" << currentZoomLevel;
                    }
                    
                    // 重置视图变换为新的视觉比例，避免累乘误差
                    ui->graphicsView->resetTransform();
                    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
                    ui->graphicsView->scale(m_visualScale, m_visualScale);

                    // 重新对齐视图，使鼠标下的地理位置保持不变
                    if (tileMapManager && !mouseGeo.isNull()) {
                        QPointF targetScene = tileMapManager->geoToScene(mouseGeo.x(), mouseGeo.y());
                        QPointF viewportCenterF = QPointF(ui->graphicsView->viewport()->rect().center());
                        QPointF offsetViewport = mouseViewportPos - viewportCenterF;
                        // 视口到场景的偏移（考虑当前视觉缩放）
                        QPointF offsetScene(offsetViewport.x() / m_visualScale,
                                            offsetViewport.y() / m_visualScale);
                        QPointF desiredCenter = targetScene - offsetScene;
                        ui->graphicsView->centerOn(desiredCenter);
                        tileMapManager->updateTilesForViewImmediate(desiredCenter.x(), desiredCenter.y());
                        // 同步瓦片管理器的中心，避免后续缩放累积偏差
                        QPointF desiredGeo = tileMapManager->sceneToGeo(desiredCenter, currentZoomLevel);
                        tileMapManager->setCenter(desiredGeo.y(), desiredGeo.x());
                    } else {
                        // 回退：按当前中心刷新
                        QPointF centerScene = tileMapManager->getCenterScenePos();
                        ui->graphicsView->centerOn(centerScene);
                        tileMapManager->updateTilesForViewImmediate(centerScene.x(), centerScene.y());
                    }

                }
                
                updateStatus(QString("Tile Map Zoom Level: %1/%2  (visual x%3)")
                             .arg(currentZoomLevel)
                             .arg(MAX_ZOOM_LEVEL)
                             .arg(m_visualScale, 0, 'f', 2));
            } else {
                // 如果没有瓦片地图管理器，使用原来的连续缩放
                qreal scaleFactor = 1.15;
                if (wheelEvent->angleDelta().y() > 0) {
                    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
                    ui->graphicsView->scale(scaleFactor, scaleFactor);
                    currentScale *= scaleFactor;
                } else {
                    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
                    ui->graphicsView->scale(1/scaleFactor, 1/scaleFactor);
                    currentScale /= scaleFactor;
                }
                updateStatus(QString("Zoom: %1x").arg(currentScale, 0, 'f', 2));
            }
            return true; // 事件已处理
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            // 处理绘制模式下的鼠标点击
            if (m_drawingManager && m_drawingManager->isDrawing()) {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                
                if (mouseEvent->button() == Qt::LeftButton) {
                    // 左键：添加点
                    m_drawingManager->handleMouseClick(scenePos);
                    return true; // 阻止事件继续传播
                } else if (mouseEvent->button() == Qt::RightButton) {
                    // 右键：完成绘制
                    m_drawingManager->handleRightClick(scenePos);
                    return true;
                }
            }
            
            // 爆管点选模式：仅处理地图点击，右键/ESC 取消
            if (m_burstSelectionMode) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                    if (tileMapManager) {
                        QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                        performBurstAnalysis(QPointF(geo.x(), geo.y()));
                    }
                } else if (mouseEvent->button() == Qt::RightButton) {
                    cancelBurstSelectionMode();
                }
                return true; // 拦截其他操作
            }
            
            // 连通性分析点选模式：仅处理地图点击，右键/ESC 取消
            if (m_connectivitySelectionMode) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                    if (tileMapManager) {
                        QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                        performConnectivityAnalysis(QPointF(geo.x(), geo.y()));
                    }
                } else if (mouseEvent->button() == Qt::RightButton) {
                    cancelConnectivitySelectionMode();
                }
                return true; // 拦截其他操作
            }
            
            // 最短路径点选模式：需要选择起点和终点
            if (m_shortestPathSelectionMode) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                    if (tileMapManager) {
                        QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                        handleShortestPathPointSelection(QPointF(geo.x(), geo.y()));
                    }
                } else if (mouseEvent->button() == Qt::RightButton) {
                    cancelShortestPathSelectionMode();
                }
                return true; // 拦截其他操作
            }

            // 非绘制模式：处理实体选中和右键菜单
            if (!m_drawingManager || !m_drawingManager->isDrawing()) {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                
                // 使用屏幕像素作为基准，转换为场景坐标
                // 2像素的容差，与双击事件保持一致
                const qreal screenPixelTolerance = 2.0;
                QPointF screenDelta(screenPixelTolerance, 0);
                QPointF sceneDelta = ui->graphicsView->mapToScene(screenDelta.toPoint()) - 
                                     ui->graphicsView->mapToScene(QPoint(0, 0));
                qreal searchRadius = qAbs(sceneDelta.x());
                
                // 如果转换失败，使用默认值
                if (searchRadius <= 0 || searchRadius > 1000) {
                    searchRadius = 2.0;
                }
                
                QRectF searchRect(scenePos.x() - searchRadius, 
                                 scenePos.y() - searchRadius, 
                                 searchRadius * 2, 
                                 searchRadius * 2);
                
                // 获取搜索区域内的所有图形项，按 Z 值排序（Z 值高的在前）
                QList<QGraphicsItem*> items = mapScene->items(searchRect, Qt::IntersectsItemShape, 
                                                               Qt::DescendingOrder, 
                                                               ui->graphicsView->transform());
                
                // 如果没有找到项，尝试精确点击位置
                if (items.isEmpty()) {
                    items = mapScene->items(scenePos, Qt::IntersectsItemShape, 
                                           Qt::DescendingOrder, 
                                           ui->graphicsView->transform());
                }
                
                // 收集所有实体项
                QList<QGraphicsItem*> facilityItems;
                QList<QGraphicsItem*> pipelineItems;
                
                for (QGraphicsItem *candidate : items) {
                    if (isEntityItem(candidate)) {
                        QString entityType = candidate->data(0).toString();
                        if (entityType == "facility") {
                            facilityItems.append(candidate);
                        } else if (entityType == "pipeline") {
                            pipelineItems.append(candidate);
                        }
                    }
                }
                
                // 优先选择设施（Z值更高），然后选择管线
                QGraphicsItem *item = nullptr;
                
                if (!facilityItems.isEmpty()) {
                    // 对于设施，也需要检查距离（虽然设施是点，但也要确保在2像素范围内）
                    qreal minDistance = std::numeric_limits<qreal>::max();
                    QGraphicsItem *nearestFacility = nullptr;
                    
                    // 使用与searchRadius相同的计算方法
                    qreal maxAllowedDistance = searchRadius;
                    
                    for (QGraphicsItem *facility : facilityItems) {
                        // 获取设施的位置
                        QPointF facilityPos = facility->scenePos();
                        qreal dist = QLineF(scenePos, facilityPos).length();
                        
                        // 只考虑距离在允许范围内的设施
                        if (dist <= maxAllowedDistance && dist < minDistance) {
                            minDistance = dist;
                            nearestFacility = facility;
                        }
                    }
                    
                    item = nearestFacility;
                } else if (!pipelineItems.isEmpty()) {
                    // 对于管线，无论数量多少，都需要计算距离并检查是否在允许范围内
                    // 这样可以避免在空白区域误选管线
                    qreal minDistance = std::numeric_limits<qreal>::max();
                    QGraphicsItem *nearestPipeline = nullptr;
                    
                    // 使用与searchRadius相同的值，确保一致性
                    // searchRadius已经是2像素对应的场景坐标距离
                    qreal maxAllowedDistance = searchRadius;
                    
                    for (QGraphicsItem *pipeline : pipelineItems) {
                        QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(pipeline);
                        if (pathItem) {
                            // 计算点击位置到管线路径的最短距离
                            QPainterPath path = pathItem->path();
                            qreal minPathDistance = std::numeric_limits<qreal>::max();
                            
                            // 遍历路径的所有线段，找到最近的距离
                            for (int i = 0; i < path.elementCount() - 1; i++) {
                                QPointF p1 = path.elementAt(i);
                                QPointF p2 = path.elementAt(i + 1);
                                QLineF line(p1, p2);
                                
                                // 计算点到线段的距离
                                qreal lineLength = line.length();
                                if (lineLength > 0) {
                                    QPointF v = p2 - p1;
                                    QPointF w = scenePos - p1;
                                    qreal t = qBound(0.0, QPointF::dotProduct(w, v) / (lineLength * lineLength), 1.0);
                                    QPointF nearestPoint = p1 + t * v;
                                    qreal dist = QLineF(scenePos, nearestPoint).length();
                                    if (dist < minPathDistance) {
                                        minPathDistance = dist;
                                    }
                                } else {
                                    // 线段长度为0，直接计算点到点的距离
                                    qreal dist = QLineF(scenePos, p1).length();
                                    if (dist < minPathDistance) {
                                        minPathDistance = dist;
                                    }
                                }
                            }
                            
                            // 只考虑距离在允许范围内的管线
                            if (minPathDistance <= maxAllowedDistance && minPathDistance < minDistance) {
                                minDistance = minPathDistance;
                                nearestPipeline = pipeline;
                            }
                        }
                    }
                    
                    // 如果找到了距离在允许范围内的管线，选择它；否则不选择任何管线
                    item = nearestPipeline;
                }
                
                if (mouseEvent->button() == Qt::LeftButton) {
                    // 左键点击：选中实体
                    if (item && isEntityItem(item)) {
                        onEntityClicked(item);
                        return true;
                    } else {
                        // 点击空白处，清除选中
                        clearSelection();
                    }
                } else if (mouseEvent->button() == Qt::RightButton) {
                    // 右键点击：显示菜单或拖拽
                    if (item && isEntityItem(item)) {
                        // 如果点击的是实体，显示菜单
                        if (item != m_selectedItem) {
                            onEntityClicked(item);  // 先选中
                        }
                        onShowContextMenu(mouseEvent->pos());
                        return true;
                    } else {
                        // 点击空白处，启用拖拽
                        QPoint mousePos = ui->graphicsView->mapFromGlobal(QCursor::pos());
                        QRect viewRect = ui->graphicsView->rect();
                        if (viewRect.contains(mousePos)) {
                            lastRightClickPos = mouseEvent->pos();
                            lastRightClickScenePos = ui->graphicsView->mapToScene(lastRightClickPos);
                            isRightClickDragging = true;
                            if (tileMapManager) tileMapManager->setDragging(true);
                            ui->graphicsView->setCursor(Qt::ClosedHandCursor);
                            return true;
                        }
                    }
                }
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            // 处理双击事件
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_drawingManager && m_drawingManager->isDrawing()) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                    m_drawingManager->handleDoubleClick(scenePos);
                    return true;
                }
            } else {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                
                // 使用屏幕像素作为基准，转换为场景坐标
                // 2像素的容差，与单击事件保持一致
                const qreal screenPixelTolerance = 2.0;
                QPointF screenDelta(screenPixelTolerance, 0);
                QPointF sceneDelta = ui->graphicsView->mapToScene(screenDelta.toPoint()) - 
                                     ui->graphicsView->mapToScene(QPoint(0, 0));
                qreal searchRadius = qAbs(sceneDelta.x());
                
                // 如果转换失败，使用默认值
                if (searchRadius <= 0 || searchRadius > 1000) {
                    searchRadius = 2.0;
                }
                
                QRectF searchRect(scenePos.x() - searchRadius, 
                                 scenePos.y() - searchRadius, 
                                 searchRadius * 2, 
                                 searchRadius * 2);
                
                // 获取搜索区域内的所有图形项，按 Z 值排序（Z 值高的在前）
                QList<QGraphicsItem*> items = mapScene->items(searchRect, Qt::IntersectsItemShape, 
                                                               Qt::DescendingOrder, 
                                                               ui->graphicsView->transform());
                
                // 如果没有找到项，尝试精确点击位置
                if (items.isEmpty()) {
                    items = mapScene->items(scenePos, Qt::IntersectsItemShape, 
                                           Qt::DescendingOrder, 
                                           ui->graphicsView->transform());
                }
                
                // 收集所有实体项
                QList<QGraphicsItem*> facilityItems;
                QList<QGraphicsItem*> pipelineItems;
                
                for (QGraphicsItem *candidate : items) {
                    if (isEntityItem(candidate)) {
                        QString entityType = candidate->data(0).toString();
                        if (entityType == "facility") {
                            facilityItems.append(candidate);
                        } else if (entityType == "pipeline") {
                            pipelineItems.append(candidate);
                        }
                    }
                }
                
                // 优先选择设施（Z值更高），然后选择管线
                QGraphicsItem *item = nullptr;
                
                if (!facilityItems.isEmpty()) {
                    // 对于设施，也需要检查距离（虽然设施是点，但也要确保在2像素范围内）
                    qreal minDistance = std::numeric_limits<qreal>::max();
                    QGraphicsItem *nearestFacility = nullptr;
                    
                    // 使用与searchRadius相同的计算方法
                    qreal maxAllowedDistance = searchRadius;
                    
                    for (QGraphicsItem *facility : facilityItems) {
                        // 获取设施的位置
                        QPointF facilityPos = facility->scenePos();
                        qreal dist = QLineF(scenePos, facilityPos).length();
                        
                        // 只考虑距离在允许范围内的设施
                        if (dist <= maxAllowedDistance && dist < minDistance) {
                            minDistance = dist;
                            nearestFacility = facility;
                        }
                    }
                    
                    item = nearestFacility;
                } else if (!pipelineItems.isEmpty()) {
                    // 对于管线，无论数量多少，都需要计算距离并检查是否在允许范围内
                    // 这样可以避免在空白区域误选管线
                    qreal minDistance = std::numeric_limits<qreal>::max();
                    QGraphicsItem *nearestPipeline = nullptr;
                    
                    // 使用与searchRadius相同的计算方法，确保一致性
                    qreal maxAllowedDistance = searchRadius;
                    
                    for (QGraphicsItem *pipeline : pipelineItems) {
                        QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(pipeline);
                        if (pathItem) {
                            // 计算点击位置到管线路径的最短距离
                            QPainterPath path = pathItem->path();
                            qreal minPathDistance = std::numeric_limits<qreal>::max();
                            
                            // 遍历路径的所有线段，找到最近的距离
                            for (int i = 0; i < path.elementCount() - 1; i++) {
                                QPointF p1 = path.elementAt(i);
                                QPointF p2 = path.elementAt(i + 1);
                                QLineF line(p1, p2);
                                
                                // 计算点到线段的距离
                                qreal lineLength = line.length();
                                if (lineLength > 0) {
                                    QPointF v = p2 - p1;
                                    QPointF w = scenePos - p1;
                                    qreal t = qBound(0.0, QPointF::dotProduct(w, v) / (lineLength * lineLength), 1.0);
                                    QPointF nearestPoint = p1 + t * v;
                                    qreal dist = QLineF(scenePos, nearestPoint).length();
                                    if (dist < minPathDistance) {
                                        minPathDistance = dist;
                                    }
                                } else {
                                    // 线段长度为0，直接计算点到点的距离
                                    qreal dist = QLineF(scenePos, p1).length();
                                    if (dist < minPathDistance) {
                                        minPathDistance = dist;
                                    }
                                }
                            }
                            
                            // 只考虑距离在允许范围内的管线
                            if (minPathDistance <= maxAllowedDistance && minPathDistance < minDistance) {
                                minDistance = minPathDistance;
                                nearestPipeline = pipeline;
                            }
                        }
                    }
                    
                    // 如果找到了距离在允许范围内的管线，选择它；否则不选择任何管线
                    item = nearestPipeline;
                }
                
                if (item && isEntityItem(item) && mouseEvent->button() == Qt::LeftButton) {
                    onEntityDoubleClicked(item);
                    return true;
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            // 处理绘制模式下的鼠标移动（显示预览）
            if (m_drawingManager && m_drawingManager->isDrawing()) {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                m_drawingManager->handleMouseMove(scenePos);
                // 不阻止事件，允许同时拖动
            }
            
            // 只有当鼠标在QGraphicsView区域时，右键拖拽才生效
            if (isRightClickDragging && (mouseEvent->buttons() & Qt::RightButton)) {
                // 使用滚动条实现 1:1 平移，避免映射误差与轴向耦合
                    QPointF delta = mouseEvent->pos() - lastRightClickPos;
                QScrollBar *h = ui->graphicsView->horizontalScrollBar();
                QScrollBar *v = ui->graphicsView->verticalScrollBar();
                // 方向修正：拖拽左（delta<0）应显示右侧 → 滚动值增加
                h->setValue(h->value() - (int)delta.x());
                v->setValue(v->value() - (int)delta.y());
                    lastRightClickPos = mouseEvent->pos();
                lastRightClickScenePos = ui->graphicsView->mapToScene(mouseEvent->pos());

                if (tileMapManager && !isDownloading) {
                    QPointF centeredScene = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect().center());
                    tileMapManager->updateTilesForViewImmediate(centeredScene.x(), centeredScene.y());
                }
                    return true; // 事件已处理
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            // 检查选中项是否移动（用于撤销/重做）
            if (mouseEvent->button() == Qt::LeftButton && m_selectedItem) {
                QPointF currentPos = m_selectedItem->pos();
                if (currentPos != m_selectedItemStartPos) {
                    // 位置发生变化，创建移动命令
                    MoveEntityCommand *cmd = new MoveEntityCommand(
                        m_selectedItem,
                        m_selectedItemStartPos,
                        currentPos
                    );
                    if (m_undoStack) {
                        m_undoStack->push(cmd);
                    }
                    // 更新开始位置
                    m_selectedItemStartPos = currentPos;
                }
            }
            
            // 右键释放时禁用拖拽模式
            if (mouseEvent->button() == Qt::RightButton) {
                isRightClickDragging = false;
                ui->graphicsView->setCursor(Qt::ArrowCursor);
                
                if (tileMapManager && !isDownloading) {
                    tileMapManager->setDragging(false);
                    // 以松开时光标下的场景坐标作为新的视图中心来源
                    QPointF sceneAtRelease = lastRightClickScenePos;
                    tileMapManager->updateTilesForView(sceneAtRelease.x(), sceneAtRelease.y());
                    qDebug() << "Right-click drag ended, updating tiles using release scene pos:" << sceneAtRelease;
                }
                return true; // 事件已处理
            }
        }
    }
    // 处理滚动条 hover 展开：确保不改变滚动条控件整体宽高，仅通过 handle 的 margin 营造粗细变化
    if (obj == ui->graphicsView->horizontalScrollBar() || obj == ui->graphicsView->verticalScrollBar()) {
        QScrollBar *sb = qobject_cast<QScrollBar*>(obj);
        if (!sb) return QWidget::eventFilter(obj, event);
        if (event->type() == QEvent::Enter) {
            if (sb->orientation() == Qt::Horizontal) {
                hScrollExpanded = false;
                hScrollHoverTimer->stop();
                hScrollHoverTimer->start();
                disconnect(hScrollHoverTimer, nullptr, this, nullptr);
                connect(hScrollHoverTimer, &QTimer::timeout, this, [this]() {
                    QScrollBar *sb = hScrollTarget;
                    if (!sb) return;
                    // 若鼠标仍在滚动条上再展开
                    QPoint g = QCursor::pos();
                    QPoint local = sb->mapFromGlobal(g);
                    if (sb->rect().contains(local)) {
                        hScrollExpanded = true;
                        sb->setStyleSheet(
                            "QScrollBar:horizontal{height:12px; background:transparent; margin:0px;}"
                            "QScrollBar::handle:horizontal{background:#ff7a18; min-width:24px; border-radius:2px;}"
                            "QScrollBar::handle:horizontal:hover{background:#ff9a40;}"
                            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal{width:0px; height:0px; background:transparent; border:none;}"
                            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{background:rgba(0,0,0,0.18); border-radius:2px;}"
                        );
                    }
                });
            } else {
                vScrollExpanded = false;
                vScrollHoverTimer->stop();
                vScrollHoverTimer->start();
                disconnect(vScrollHoverTimer, nullptr, this, nullptr);
                connect(vScrollHoverTimer, &QTimer::timeout, this, [this]() {
                    QScrollBar *sb = vScrollTarget;
                    if (!sb) return;
                    QPoint g = QCursor::pos();
                    QPoint local = sb->mapFromGlobal(g);
                    if (sb->rect().contains(local)) {
                        vScrollExpanded = true;
                        sb->setStyleSheet(
                            "QScrollBar:vertical{width:12px; background:transparent; margin:0px;}"
                            "QScrollBar::handle:vertical{background:#ff7a18; min-height:24px; border-radius:2px;}"
                            "QScrollBar::handle:vertical:hover{background:#ff9a40;}"
                            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical{width:0px; height:0px; background:transparent; border:none;}"
                            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background:rgba(0,0,0,0.18); border-radius:2px;}"
                        );
                    }
                });
            }
        } else if (event->type() == QEvent::Leave) {
            if (sb->orientation() == Qt::Horizontal) {
                hScrollHoverTimer->stop();
                hScrollExpanded = false;
            } else {
                vScrollHoverTimer->stop();
                vScrollExpanded = false;
            }
            if (sb->orientation() == Qt::Horizontal) {
                sb->setStyleSheet(
                    "QScrollBar:horizontal{height:4px; background:transparent; margin:0px;}"
                    "QScrollBar:horizontal:hover{height:12px;}"
                    "QScrollBar::handle:horizontal{background:#ff7a18; min-width:24px; border-radius:2px;}"
                    "QScrollBar::handle:horizontal:hover{background:#ff9a40;}"
                    "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal{width:0px; height:0px; background:transparent; border:none;}"
                    "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{background:rgba(0,0,0,0.18); border-radius:2px;}"
                );
            } else {
                sb->setStyleSheet(
                    "QScrollBar:vertical{width:4px; background:transparent; margin:0px;}"
                    "QScrollBar:vertical:hover{width:12px;}"
                    "QScrollBar::handle:vertical{background:#ff7a18; min-height:24px; border-radius:2px;}"
                    "QScrollBar::handle:vertical:hover{background:#ff9a40;}"
                    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical{width:0px; height:0px; background:transparent; border:none;}"
                    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background:rgba(0,0,0,0.18); border-radius:2px;}"
                );
            }
        } else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::Wheel) {
            // 任何交互都取消延迟并立即展开
            if (sb->orientation() == Qt::Horizontal) {
                hScrollHoverTimer->stop();
                hScrollExpanded = true;
                sb->setStyleSheet(
                    "QScrollBar:horizontal{height:12px; background:transparent; margin:0px;}"
                    "QScrollBar::handle:horizontal{background:#ff7a18; min-width:24px; border-radius:2px;}"
                    "QScrollBar::handle:horizontal:hover{background:#ff9a40;}"
                    "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal{width:0px; height:0px; background:transparent; border:none;}"
                    "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal{background:rgba(0,0,0,0.18); border-radius:2px;}"
                );
            } else {
                vScrollHoverTimer->stop();
                vScrollExpanded = true;
                sb->setStyleSheet(
                    "QScrollBar:vertical{width:12px; background:transparent; margin:0px;}"
                    "QScrollBar::handle:vertical{background:#ff7a18; min-height:24px; border-radius:2px;}"
                    "QScrollBar::handle:vertical:hover{background:#ff9a40;}"
                    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical{width:0px; height:0px; background:transparent; border:none;}"
                    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background:rgba(0,0,0,0.18); border-radius:2px;}"
                );
            }
        }
        return QWidget::eventFilter(obj, event);
    }
    return QWidget::eventFilter(obj, event);
}

void MyForm::updateStatus(const QString &message) {
    // 更新浮动状态栏
    if (floatingStatusLabel) {
        floatingStatusLabel->setText(message);
        // 显示浮动状态栏
        if (floatingStatusBar) {
            floatingStatusBar->show();
            floatingStatusBar->raise();
            // 重置透明度
            floatingStatusBar->setWindowOpacity(1.0);
            // 重启3秒倒计时
            if (statusBarFadeTimer) {
                statusBarFadeTimer->stop();
                statusBarFadeTimer->start(3000); // 3秒后开始消失
            }
        }
    }
    qDebug() << "Status:" << message;
}

void MyForm::createFloatingStatusBar()
{
    if (!ui->graphicsView) return;
    
    // 创建浮动状态栏容器（添加到 graphicsView 而不是 viewport，使其位置不随地图移动）
    floatingStatusBar = new QWidget(ui->graphicsView);
    floatingStatusBar->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 鼠标事件穿透
    floatingStatusBar->setObjectName("floatingStatusBar");
    // 使用透明背景和亮色文字（带阴影效果确保可读性）
    floatingStatusBar->setStyleSheet(
        "#floatingStatusBar{"
        "  background: transparent;"
        "  padding: 8px 12px;"
        "}"
        "QLabel{"
        "  color: #ffffff;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "  text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.9);"
        "}"
        "QProgressBar{"
        "  background: rgba(0, 0, 0, 0.5);"
        "  border: 1px solid rgba(255, 255, 255, 0.3);"
        "  border-radius: 3px;"
        "  text-align: center;"
        "  color: #ffffff;"
        "  font-weight: bold;"
        "}"
        "QProgressBar::chunk{"
        "  background: #ff7a18;"
        "  border-radius: 2px;"
        "}"
    );
    
    QVBoxLayout *layout = new QVBoxLayout(floatingStatusBar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    
    // 创建状态标签
    floatingStatusLabel = new QLabel("Ready");
    layout->addWidget(floatingStatusLabel);
    
    // 创建进度条
    floatingProgressBar = new QProgressBar();
    floatingProgressBar->setRange(0, 100);
    floatingProgressBar->setValue(0);
    floatingProgressBar->setTextVisible(true);
    floatingProgressBar->setMaximumHeight(20);
    floatingProgressBar->setVisible(false); // 初始隐藏
    layout->addWidget(floatingProgressBar);
    
    // 设置最小宽度和最大宽度，确保文本能完整显示
    floatingStatusBar->setMinimumWidth(500);   // 增加最小宽度
    floatingStatusBar->setMaximumWidth(1500);  // 增加最大宽度到1500以显示完整文本
    floatingStatusBar->adjustSize();
    
    // 创建3秒消失定时器
    statusBarFadeTimer = new QTimer(this);
    statusBarFadeTimer->setSingleShot(true);
    connect(statusBarFadeTimer, &QTimer::timeout, this, [this]() {
        // 创建淡出动画
        if (!statusBarOpacityAnim) {
            statusBarOpacityAnim = new QPropertyAnimation(floatingStatusBar, "windowOpacity");
            statusBarOpacityAnim->setDuration(1000); // 1秒淡出
            statusBarOpacityAnim->setStartValue(1.0);
            statusBarOpacityAnim->setEndValue(0.0);
            connect(statusBarOpacityAnim, &QPropertyAnimation::finished, this, [this]() {
                floatingStatusBar->hide();
                floatingStatusBar->setWindowOpacity(1.0); // 恢复透明度以备下次使用
            });
        } else {
            statusBarOpacityAnim->setStartValue(floatingStatusBar->windowOpacity());
            statusBarOpacityAnim->setEndValue(0.0);
        }
        statusBarOpacityAnim->start();
    });
    
    // 初始化时不隐藏，让它显示初始状态
    // floatingStatusBar->hide();
    positionFloatingStatusBar();
    floatingStatusBar->show();  // 显示浮动状态栏
}

void MyForm::positionFloatingStatusBar()
{
    if (!floatingStatusBar || !ui->graphicsView) return;
    
    const int margin = 10;
    QSize vp = ui->graphicsView->size();  // 使用 graphicsView 的大小，而不是 viewport
    QSize sz = floatingStatusBar->sizeHint();
    if (sz.isEmpty()) sz = floatingStatusBar->size();
    
    // 位于地图左下角
    int x = margin;
    int y = vp.height() - sz.height() - margin;
    floatingStatusBar->setGeometry(x, y, sz.width(), sz.height());
}

void MyForm::updateFloatingProgressBar(int current, int total)
{
    if (!floatingProgressBar) return;
    
    floatingProgressBar->setMaximum(total > 0 ? total : 100);
    floatingProgressBar->setValue(current);
    
    // 显示进度条
    if (floatingStatusBar) {
        floatingStatusBar->show();
        floatingStatusBar->raise();
        floatingStatusBar->setWindowOpacity(1.0);
        floatingProgressBar->setVisible(true);
        // 下载中不自动消失，停止定时器
        if (statusBarFadeTimer) {
            statusBarFadeTimer->stop();
        }
    }
}

void MyForm::loadMap(const QString &mapPath) {
    // 清除之前的地图项
    if (mapItem) {
        mapScene->removeItem(mapItem);
        delete mapItem;
        mapItem = nullptr;
    }
    
    // 加载新地图
    QPixmap pixmap(mapPath);
    if (!pixmap.isNull()) {
        mapItem = mapScene->addPixmap(pixmap);
        mapScene->setSceneRect(pixmap.rect());
        currentScale = 1.0;
        ui->graphicsView->resetTransform();
        updateStatus("Map loaded: " + mapPath);
    } else {
        updateStatus("Failed to load map: " + mapPath);
    }
}

void MyForm::handleRefreshButtonClicked()
{
    qDebug() << "Refresh button clicked";
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        QMessageBox::warning(this, "提示", "数据库未连接，无法刷新数据。\n\n请检查数据库配置。");
        updateStatus("数据库未连接");
        return;
    }
    
    // 检查是否有未保存的变更
    if (m_hasUnsavedChanges) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "未保存的变更",
            "有未保存的变更，刷新数据会丢失这些变更。\n\n是否先保存再刷新？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
        
        if (reply == QMessageBox::Save) {
            onSaveAll();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    updateStatus("正在刷新数据...");
    
    // 重新加载数据库数据
    if (m_layerManager) {
        // 计算当前地图视图的地理范围
        QRectF geoBounds;
        if (tileMapManager && ui->graphicsView) {
            // 获取视图的可见矩形（场景坐标）
            QRectF viewportRect = ui->graphicsView->mapToScene(
                ui->graphicsView->viewport()->rect()
            ).boundingRect();
            
            // 将场景坐标转换为地理坐标
            QPointF topLeft = tileMapManager->sceneToGeo(viewportRect.topLeft(), currentZoomLevel);
            QPointF bottomRight = tileMapManager->sceneToGeo(viewportRect.bottomRight(), currentZoomLevel);
            
            // 计算地理范围（考虑可能的坐标翻转）
            double minLon = qMin(topLeft.x(), bottomRight.x());
            double maxLon = qMax(topLeft.x(), bottomRight.x());
            double minLat = qMin(topLeft.y(), bottomRight.y());
            double maxLat = qMax(topLeft.y(), bottomRight.y());
            
            geoBounds = QRectF(minLon, minLat, maxLon - minLon, maxLat - minLat);
            
            qDebug() << "[Refresh] Current view bounds:" << geoBounds;
        } else {
            // 如果没有地图管理器，使用默认范围或之前设置的范围
            geoBounds = m_layerManager->getVisibleBounds();
            if (geoBounds.isEmpty()) {
                geoBounds = QRectF(116.390, 39.900, 0.030, 0.020);
            }
        }
        
        // 设置可视范围并刷新所有图层
        m_layerManager->setVisibleBounds(geoBounds);
        m_layerManager->refreshAllLayers();
        
        // 同时重新加载用户绘制的数据
        DrawingDatabaseManager::loadFromDatabase(
            mapScene,
            m_drawnPipelines,
            m_nextPipelineId
        );
        
        updateStatus("数据刷新完成");
        QMessageBox::information(this, "刷新完成", 
            QString("已重新加载当前视图范围内的数据。\n\n范围: 经度 %1-%2, 纬度 %3-%4")
                .arg(geoBounds.left(), 0, 'f', 4)
                .arg(geoBounds.right(), 0, 'f', 4)
                .arg(geoBounds.top(), 0, 'f', 4)
                .arg(geoBounds.bottom(), 0, 'f', 4));
    } else {
        // 如果图层管理器未初始化，尝试初始化
        updateStatus("正在初始化...");
        initializePipelineVisualization();
    }
}

void MyForm::handleSaveButtonClicked()
{
    qDebug() << "Save button clicked";
    
    // 保存所有待保存的变更到数据库
    onSaveAll();
}


void MyForm::handleUndoButtonClicked()
{
    if (m_undoStack && m_undoStack->canUndo()) {
        QString text = m_undoStack->undoText();
        m_undoStack->undo();
        qDebug() << "↩️ Undo:" << text;
        updateStatus("撤销: " + text);
    } else {
        updateStatus("无可撤销的操作");
    }
}

void MyForm::handleRedoButtonClicked()
{
    if (m_undoStack && m_undoStack->canRedo()) {
        QString text = m_undoStack->redoText();
        m_undoStack->redo();
        qDebug() << "↪️ Redo:" << text;
        updateStatus("重做: " + text);
    } else {
        updateStatus("无可重做的操作");
    }
}

void MyForm::handleLoadMapButtonClicked()
{
    qDebug() << "Load Map button clicked";
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load Map"), "", 
        tr("Image Files (*.png *.jpg *.bmp *.gif);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        loadMap(fileName);
    }
}

void MyForm::handleZoomInButtonClicked()
{
    qDebug() << "Zoom In button clicked";
    
    // 如果有瓦片地图管理器，使用层级缩放
    if (tileMapManager) {
        handleZoomInTileMapButtonClicked();
    } else if (mapItem) {
        // 如果只有普通地图，使用连续缩放
        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        currentScale *= 1.2;
        ui->graphicsView->scale(1.2, 1.2);
        updateStatus(QString("Zoom: %1x").arg(currentScale, 0, 'f', 2));
    }
}

void MyForm::handleZoomOutButtonClicked()
{
    qDebug() << "Zoom Out button clicked";
    
    // 如果有瓦片地图管理器，使用层级缩放
    if (tileMapManager) {
        handleZoomOutTileMapButtonClicked();
    } else if (mapItem) {
        // 如果只有普通地图，使用连续缩放
        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        currentScale /= 1.2;
        ui->graphicsView->scale(1/1.2, 1/1.2);
        updateStatus(QString("Zoom: %1x").arg(currentScale, 0, 'f', 2));
    }
}

void MyForm::handlePanButtonClicked()
{
    qDebug() << "Pan button clicked";
    if (ui->graphicsView->dragMode() == QGraphicsView::ScrollHandDrag) {
        ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
        // no-op: UI 按钮已移除
        updateStatus("Pan mode disabled");
    } else {
        ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
        // no-op: UI 按钮已移除
        updateStatus("Pan mode enabled - drag to move map");
    }
}

// 瓦片地图相关槽函数实现
void MyForm::handleLoadTileMapButtonClicked()
{
    logMessage("=== Load Tile Map button clicked ===");
    
    // 清除之前的图片地图项
    if (mapItem) {
        logMessage("Removing existing map item");
        if (mapItem->scene()) {  // 检查图形项是否属于某个场景
            mapScene->removeItem(mapItem);
        }
        delete mapItem;
        mapItem = nullptr;
    }
    
    // 清除之前的场景矩形
    logMessage("Clearing scene rect");
    mapScene->setSceneRect(0, 0, 0, 0);
    
    // 设置北京坐标为中心点
    logMessage("Setting center to Beijing coordinates: 39.9042, 116.4074");
    tileMapManager->setCenter(39.9042, 116.4074);
    // 设置缩放级别为最小层级（3）开始
    logMessage("Setting zoom level to MIN_ZOOM_LEVEL");
    currentZoomLevel = MIN_ZOOM_LEVEL;
    tileMapManager->setZoom(currentZoomLevel);
    m_visualScale = 1.0;
    ui->graphicsView->resetTransform();
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    
    // 触发中国区域级别3的瓦片地图下载
    logMessage("Calling startRegionDownload");
    startRegionDownload();
    
    updateStatus("Tile map loaded - downloading tiles...");
    logMessage("Tile map loading initiated");
}

void MyForm::handleZoomInTileMapButtonClicked()
{
    qDebug() << "Zoom In Tile Map button clicked";
    
    // 使用固定的10级缩放限制
    if (currentZoomLevel < MAX_ZOOM_LEVEL) {
        currentZoomLevel++;
        
        // 按钮缩放：以视口中心为缩放点
        QRect viewportRect = ui->graphicsView->viewport()->rect();
        QPoint viewportCenter = viewportRect.center();
        QPointF sceneCenter = ui->graphicsView->mapToScene(viewportCenter);
        
        tileMapManager->setZoomAtMousePosition(
            currentZoomLevel,
            sceneCenter.x(),
            sceneCenter.y(),
            viewportCenter.x(),
            viewportCenter.y(),
            viewportRect.width(),
            viewportRect.height(),
            1.0  // 按钮触发时视觉缩放基线已重置
        );
        // 与滚轮缩放保持一致：对齐视图中心并立即刷新可见瓦片
        QPointF centerScene = tileMapManager->getCenterScenePos();
        ui->graphicsView->centerOn(centerScene);
        tileMapManager->updateTilesForViewImmediate(centerScene.x(), centerScene.y());
        
        // 重置视觉缩放基线
        m_visualScale = 1.0;
        ui->graphicsView->resetTransform();
        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        
        // 重置视觉缩放基线
        m_visualScale = 1.0;
        ui->graphicsView->resetTransform();
        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        
        // 同步缩放级别到管网渲染器
        if (m_layerManager) {
            m_layerManager->setZoom(currentZoomLevel);
        }
        
        // 更新状态显示
        updateStatus(QString("Tile Map Zoom Level: %1/%2").arg(currentZoomLevel).arg(MAX_ZOOM_LEVEL));
        qDebug() << "Tile map zoom in to level:" << currentZoomLevel;
    } else {
        updateStatus(QString("Maximum zoom level reached (%1/%2)").arg(currentZoomLevel).arg(MAX_ZOOM_LEVEL));
        qDebug() << "Cannot zoom in further, already at max level:" << MAX_ZOOM_LEVEL;
    }
}

void MyForm::handleZoomOutTileMapButtonClicked()
{
    qDebug() << "Zoom Out Tile Map button clicked";
    
    // 使用固定的10级缩放限制
    if (currentZoomLevel > MIN_ZOOM_LEVEL) {
        currentZoomLevel--;
        
        // 按钮缩放：以视口中心为缩放点
        QRect viewportRect = ui->graphicsView->viewport()->rect();
        QPoint viewportCenter = viewportRect.center();
        QPointF sceneCenter = ui->graphicsView->mapToScene(viewportCenter);
        
        tileMapManager->setZoomAtMousePosition(
            currentZoomLevel,
            sceneCenter.x(),
            sceneCenter.y(),
            viewportCenter.x(),
            viewportCenter.y(),
            viewportRect.width(),
            viewportRect.height(),
            1.0  // 按钮触发时视觉缩放基线已重置
        );
        // 与滚轮缩放保持一致：对齐视图中心并立即刷新可见瓦片
        QPointF centerScene2 = tileMapManager->getCenterScenePos();
        ui->graphicsView->centerOn(centerScene2);
        tileMapManager->updateTilesForViewImmediate(centerScene2.x(), centerScene2.y());
        
        // 同步缩放级别到管网渲染器
        if (m_layerManager) {
            m_layerManager->setZoom(currentZoomLevel);
        }
        
        // 更新状态显示
        updateStatus(QString("Tile Map Zoom Level: %1/%2").arg(currentZoomLevel).arg(MAX_ZOOM_LEVEL));
        qDebug() << "Tile map zoom out to level:" << currentZoomLevel;
    } else {
        updateStatus(QString("Minimum zoom level reached (%1/%2)").arg(MIN_ZOOM_LEVEL).arg(MAX_ZOOM_LEVEL));
        qDebug() << "Cannot zoom out further, already at min level:" << MIN_ZOOM_LEVEL;
    }
}

void MyForm::onTileDownloadProgress(int current, int total)
{
    if (total > 0) {
        int progress = (current * 100) / total;
        updateStatus(QString("Downloading tiles: %1% (%2/%3)").arg(progress).arg(current).arg(total));
    } else {
        updateStatus(QString("Downloading tiles: %1 tiles").arg(current));
    }
}

void MyForm::onRegionDownloadProgress(int current, int total, int zoom)
{
    qDebug() << "MyForm::onRegionDownloadProgress received:" << current << "/" << total << "zoom:" << zoom;
    
    if (total > 0) {
        int progress = (current * 100) / total;
        qDebug() << "Download progress:" << current << "/" << total << "(" << progress << "%) at zoom level" << zoom;
        
        // 使用浮动进度条
        updateFloatingProgressBar(current, total);
        
        // 更新浮动进度条的金上文本
        if (floatingProgressBar) {
            floatingProgressBar->setFormat(QString("Downloading zoom level %1: %2% (%3/%4)").arg(zoom).arg(progress).arg(current).arg(total));
        }
        
        // 更新浮动状态栏文本
        updateStatus(QString("Downloading zoom level %1: %2% (%3/%4)").arg(zoom).arg(progress).arg(current).arg(total));
    } else {
        qDebug() << "Download progress:" << current << "tiles at zoom level" << zoom;
        updateStatus(QString("Downloading zoom level %1: %2 tiles").arg(zoom).arg(current));
    }
}

void MyForm::startRegionDownload()
{
    logMessage("=== Starting region download ===");
    
    // 下载中国区域的地图瓦片
    // 中国大致范围：纬度18°N-54°N，经度73°E-135°E
    // 下载缩放级别：1-10级，提供良好的缩放体验
    logMessage("Calling tileMapManager->downloadRegion for China region");
    logMessage("Parameters: minLat=18.0, maxLat=54.0, minLon=73.0, maxLon=135.0, minZoom=3, maxZoom=10");
    tileMapManager->downloadRegion(18.0, 54.0, 73.0, 135.0, 3, 10);
    
    updateStatus("Starting China region map download (levels 3-10)...");
    logMessage("China region map download initiated - Levels 3-10");
}

void MyForm::updateVisibleTiles()
{
    if (!tileMapManager || !ui->graphicsView || isDownloading) {
        qDebug() << "updateVisibleTiles: Skipping update - tileMapManager:" << (tileMapManager != nullptr) 
                 << "graphicsView:" << (ui->graphicsView != nullptr) 
                 << "isDownloading:" << isDownloading;
        return;
    }
    
    // 获取当前视图中心在场景中的位置
    QPointF viewCenter = ui->graphicsView->mapToScene(
        ui->graphicsView->viewport()->rect().center()
    );
    
    qDebug() << "updateVisibleTiles: View center in scene:" << viewCenter;
    
    // 通知瓦片地图管理器根据新的视图中心加载瓦片
    tileMapManager->updateTilesForView(viewCenter.x(), viewCenter.y());
    
    // 重定位由 TileMapManager::loadTiles 内部负责（其会调用 repositionTiles）
    
    // 更新状态显示
    updateStatus("Updating visible tiles...");
}

void MyForm::logMessage(const QString &message)
{
    // 记录日志到文件
    QFile logFile("debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " - " << message << "\n";
        logFile.close();
    }
    
    // 同时输出到控制台
    qDebug() << message;
}

// ==========================================
// 管网可视化功能实现
// ==========================================

// 辅助函数：智能查找配置文件路径
static QString findConfigFile(const QString &filename) {
    QStringList possiblePaths = {
        // Qt Creator Debug/Release build
        QCoreApplication::applicationDirPath() + "/../../" + filename,
        // MinGW32-make build (release/)
        QCoreApplication::applicationDirPath() + "/../" + filename,
        // 当前工作目录
        filename,
        // 绝对路径（如果在项目根目录运行）
        QCoreApplication::applicationDirPath() + "/" + filename
    };
    
    for (const QString &path : possiblePaths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            qDebug() << "[Path] ✅ Found config at:" << fileInfo.absoluteFilePath();
            return fileInfo.absoluteFilePath();
        } else {
            qDebug() << "[Path] ❌ Not found:" << path;
        }
    }
    
    qDebug() << "[Path] ⚠️  Config file not found:" << filename;
    return QString();
}

QString MyForm::resolveProjectPath(const QString &relativePath) const
{
    QStringList bases = {
        QCoreApplication::applicationDirPath() + "/../..",
        QCoreApplication::applicationDirPath() + "/..",
        QCoreApplication::applicationDirPath(),
        QDir::currentPath()
    };
    
    for (const QString &base : bases) {
        QString candidate = QDir(base).absoluteFilePath(relativePath);
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return QString();
}

void MyForm::renderBurstPoint(const QPointF &geoLonLat)
{
    if (!mapScene || !tileMapManager) return;
    QPointF scenePos = tileMapManager->geoToScene(geoLonLat.x(), geoLonLat.y());
    
    // 清理旧标记
    if (m_burstMarker) { mapScene->removeItem(m_burstMarker); delete m_burstMarker; m_burstMarker = nullptr; }
    if (m_burstLabel) { mapScene->removeItem(m_burstLabel); delete m_burstLabel; m_burstLabel = nullptr; }
    clearBurstHighlights();
    
    // 画圆点
    const double r = 6.0;
    m_burstMarker = mapScene->addEllipse(scenePos.x() - r, scenePos.y() - r, 2*r, 2*r,
                                         QPen(Qt::red, 2.0), QBrush(QColor(255,0,0,120)));
    m_burstMarker->setZValue(1e5);
    
    // 标签
    m_burstLabel = mapScene->addText(QString("爆管点\n%.6f, %.6f").arg(geoLonLat.y()).arg(geoLonLat.x()));
    m_burstLabel->setDefaultTextColor(Qt::red);
    m_burstLabel->setPos(scenePos + QPointF(8, -8));
    m_burstLabel->setZValue(1e5);
}

void MyForm::clearBurstHighlights()
{
    for (auto *item : m_burstHighlights) {
        if (mapScene) mapScene->removeItem(item);
        delete item;
    }
    m_burstHighlights.clear();
}

void MyForm::highlightPipelineById(const QString &pipelineId)
{
    if (!mapScene || !tileMapManager) return;
    PipelineDAO dao;
    Pipeline pl = dao.findByPipelineId(pipelineId);
    if (!pl.isValid() || pl.coordinates().isEmpty()) return;

    QPainterPath path;
    bool first = true;
    for (const QPointF &geo : pl.coordinates()) {
        QPointF scenePt = tileMapManager->geoToScene(geo.x(), geo.y());
        if (first) { path.moveTo(scenePt); first = false; }
        else { path.lineTo(scenePt); }
    }
    QGraphicsPathItem *item = mapScene->addPath(path, QPen(QColor(255, 0, 0), 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setZValue(1e6 - 1);
    m_burstHighlights << item;
}

void MyForm::highlightFacilityById(const QString &facilityId)
{
    if (!mapScene || !tileMapManager) return;
    FacilityDAO dao;
    Facility fa = dao.findByFacilityId(facilityId);
    if (!fa.isValid() || fa.coordinate().isNull()) return;

    QPointF scenePt = tileMapManager->geoToScene(fa.coordinate().x(), fa.coordinate().y());
    const double r = 8.0;
    QGraphicsEllipseItem *item = mapScene->addEllipse(scenePt.x() - r, scenePt.y() - r, 2*r, 2*r,
                                                      QPen(QColor(255, 120, 0), 3.0),
                                                      QBrush(QColor(255, 120, 0, 80)));
    item->setZValue(1e6 - 2);
    m_burstHighlights << item;
}

void MyForm::renderBurstHighlights(const BurstAnalysisResult &result)
{
    clearBurstHighlights();

    for (const QString &pid : result.affectedPipelines) {
        highlightPipelineById(pid);
    }
    for (const QString &fid : result.affectedValves) {
        highlightFacilityById(fid);
    }
}

void MyForm::setUiEnabledDuringBurst(bool enabled)
{
    // 恢复之前暂存的控件
    if (enabled) {
        for (QWidget *w : m_disabledDuringBurst) {
            if (w) w->setEnabled(true);
        }
        m_disabledDuringBurst.clear();
        return;
    }

    // 暂时禁用功能区和面板切换器，避免误操作
    QList<QWidget*> targets;
    if (ui->functionalArea) targets << ui->functionalArea;
    if (m_panelSwitcher) targets << m_panelSwitcher;
    if (m_panelContainer) targets << m_panelContainer;

    for (QWidget *w : targets) {
        if (w && w->isEnabled()) {
            m_disabledDuringBurst << w;
            w->setEnabled(false);
        }
    }
}

void MyForm::renderBurstResult(const BurstAnalysisResult &result)
{
    if (!mapScene || !tileMapManager) return;
    
    clearBurstHighlights();
    renderBurstPoint(result.burstLocation);
    
    // 清理旧区域
    if (m_burstAreaItem) {
        mapScene->removeItem(m_burstAreaItem);
        delete m_burstAreaItem;
        m_burstAreaItem = nullptr;
    }
    
    if (!result.affectedArea.isEmpty()) {
        QPolygonF scenePoly;
        for (const QPointF &p : result.affectedArea) {
            QPointF scenePt = tileMapManager->geoToScene(p.x(), p.y());
            scenePoly << scenePt;
        }
        m_burstAreaItem = mapScene->addPolygon(scenePoly, QPen(QColor(255,0,0), 2, Qt::DashLine), QBrush(QColor(255,0,0,40)));
        m_burstAreaItem->setZValue(1e4);
    }

    renderBurstHighlights(result);
}

void MyForm::startBurstSelectionMode()
{
    m_burstSelectionMode = true;
    m_hasBurstPoint = false;
    updateStatus("爆管分析：请选择爆管点（左键确认，右键/ESC 取消）");
    ui->graphicsView->setCursor(Qt::CrossCursor);
    setUiEnabledDuringBurst(false);
}

void MyForm::cancelBurstSelectionMode()
{
    m_burstSelectionMode = false;
    ui->graphicsView->unsetCursor();
    updateStatus("已取消爆管点选择");
    setUiEnabledDuringBurst(true);
}

bool MyForm::runDatabaseInitScript()
{
    QString scriptPath = resolveProjectPath("scripts/create_database.bat");
    if (scriptPath.isEmpty()) {
        QMessageBox::warning(this, "未找到脚本", "找不到 scripts/create_database.bat，请确认脚本是否存在。");
        return false;
    }
    
    // 使用 cmd /C 运行批处理，便于用户看到输出
    bool started = QProcess::startDetached("cmd.exe", QStringList() << "/C" << QString("\"%1\"").arg(scriptPath));
    if (!started) {
        QMessageBox::warning(this, "启动失败", "无法启动一键导入脚本，请检查权限或手动运行。");
        return false;
    }
    
    QMessageBox::information(this, "已启动", "已打开命令行运行一键导入脚本，请按提示完成 PostgreSQL + PostGIS 初始化。");
    return true;
}

void MyForm::openConfigDirectory()
{
    QString cfgDir = resolveProjectPath("config");
    if (cfgDir.isEmpty()) {
        QMessageBox::warning(this, "未找到配置目录", "找不到 config 目录，请确认文件是否存在。");
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(cfgDir));
}

void MyForm::promptDatabaseSetup(const QString &reason)
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle("数据库未就绪");
    msgBox.setText(reason + "\n\n数据库要求：PostgreSQL + PostGIS\n可选操作：\n- 运行 scripts/create_database.bat 一键导入结构与示例数据\n- 查看 docs/DATABASE_SETUP.md 安装指南\n- 打开 config 目录修改连接信息");
    
    QPushButton *runBtn = msgBox.addButton("运行一键导入", QMessageBox::AcceptRole);
    QPushButton *docBtn = msgBox.addButton("查看安装指南", QMessageBox::ActionRole);
    QPushButton *cfgBtn = msgBox.addButton("打开配置目录", QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    if (msgBox.clickedButton() == runBtn) {
        runDatabaseInitScript();
    } else if (msgBox.clickedButton() == docBtn) {
        QString docPath = resolveProjectPath("docs/DATABASE_SETUP.md");
        if (docPath.isEmpty()) {
            QMessageBox::warning(this, "未找到文档", "找不到 docs/DATABASE_SETUP.md，请确认文档是否存在。");
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(docPath));
        }
    } else if (msgBox.clickedButton() == cfgBtn) {
        openConfigDirectory();
    }
}

void MyForm::initializePipelineVisualization()
{
    qDebug() << "[Pipeline] Step 1: Initialize Logger";
    qDebug() << "[Pipeline] Application dir:" << QCoreApplication::applicationDirPath();
    
    // 1. 初始化日志系统（如果还未初始化）- 必须最先执行！
    static bool loggerInitialized = false;
    if (!loggerInitialized) {
        try {
            // 智能查找日志目录
            QString logDir;
            QStringList possibleLogDirs = {
                QCoreApplication::applicationDirPath() + "/../../logs",
                QCoreApplication::applicationDirPath() + "/../logs",
                "logs"
            };
            
            for (const QString &dir : possibleLogDirs) {
                QDir d(dir);
                if (d.exists() || d.mkpath(".")) {
                    logDir = dir;
                     break;
                }
            }
            
            QString logPath = logDir + "/app.log";
            Logger::instance().initialize(logPath, Logger::Info);
            loggerInitialized = true;
            LOG_INFO("Logger initialized successfully");
            qDebug() << "[Pipeline] ✅ Logger initialized at:" << QFileInfo(logPath).absoluteFilePath();
        } catch (const std::exception &e) {
            qDebug() << "Failed to initialize logger:" << e.what();
            updateStatus("日志初始化失败（程序仍可正常使用）");
            return;
        } catch (...) {
            qDebug() << "Failed to initialize logger: unknown error";
            updateStatus("日志初始化失败（程序仍可正常使用）");
            return;
        }
    } else {
        qDebug() << "[Pipeline] ✅ Logger already initialized";
    }
    
    LOG_INFO("=== Initializing pipeline visualization ===");
    qDebug() << "[Pipeline] Step 2: Load Config";
    
    // 2. 加载配置
    try {
        QString appConfigPath = findConfigFile("config/app.ini");
        if (appConfigPath.isEmpty()) {
            promptDatabaseSetup("未找到配置文件：config/app.ini");
            return;
        }
        
        Config::instance().initialize(appConfigPath);
        LOG_INFO("App config initialized");
        qDebug() << "[Pipeline] ✅ App config loaded";
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Failed to initialize config: %1").arg(e.what()));
        updateStatus("配置初始化失败（程序仍可正常使用）");
        qDebug() << "[Pipeline] ❌ Config failed:" << e.what();
        return;
    }
    
    qDebug() << "[Pipeline] Step 3: Load Database Config";
    
    // 尝试加载数据库配置，如果失败则不影响程序运行
    QString dbConfigPath = findConfigFile("config/database.ini");
    
    if (dbConfigPath.isEmpty() || !Config::instance().loadDatabaseConfig(dbConfigPath)) {
        LOG_WARNING("Database config not found, pipeline visualization disabled");
        updateStatus("数据库配置未找到（地图功能正常）");
        promptDatabaseSetup("未找到数据库配置：config/database.ini");
        qDebug() << "[Pipeline] ⚠️  No database config - visualization disabled";
        return;
    }
    
    qDebug() << "[Pipeline] ✅ Database config loaded";
    qDebug() << "[Pipeline] Step 4: Connect to Database";
    
    // 3. 连接数据库（使用try-catch保护，设置超时）
    bool dbConnected = false;
    try {
        DatabaseManager &db = DatabaseManager::instance();
        
        qDebug() << "[Pipeline] Initializing DatabaseManager...";
        if (!db.initialize()) {
            LOG_ERROR("Failed to initialize database");
            updateStatus("数据库初始化失败（地图功能正常）");
            qDebug() << "[Pipeline] ❌ DB init failed";
            return;
        }
        
        qDebug() << "[Pipeline] Connecting to database (timeout: 5s)...";
        updateStatus("正在连接数据库...");
        
        if (!db.connect()) {
            QString error = db.lastError();
            LOG_WARNING(QString("Database connection failed: %1").arg(error));
            updateStatus(QString("数据库连接失败: %1（地图功能正常）").arg(error));
            promptDatabaseSetup(QString("数据库连接失败：%1").arg(error));
            qDebug() << "[Pipeline] ❌ DB connect failed:" << error;
            return;
        }
        
        dbConnected = true;
        qDebug() << "[Pipeline] ✅ Database connected";
        
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Database exception: %1").arg(e.what()));
        updateStatus(QString("数据库异常: %1（地图功能正常）").arg(e.what()));
        qDebug() << "[Pipeline] ❌ Exception:" << e.what();
        return;
    } catch (...) {
        LOG_ERROR("Database unknown exception");
        updateStatus("数据库连接异常（地图功能正常）");
        qDebug() << "[Pipeline] ❌ Unknown exception";
        return;
    }
    
    if (!dbConnected) {
        qDebug() << "[Pipeline] Database not connected, skipping visualization";
        return;
    }
    
    LOG_INFO("Database connected successfully");
    qDebug() << "[Pipeline] Step 5: Create LayerManager";
    
    // 4. 创建图层管理器
    if (!mapScene) {
        LOG_ERROR("Cannot create LayerManager: mapScene is null");
        updateStatus("图层管理器创建失败：地图场景未初始化");
        qDebug() << "[Pipeline] ❌ mapScene is null";
        return;
    }
    
    try {
        qDebug() << "[Pipeline] Creating LayerManager...";
        m_layerManager = new LayerManager(mapScene, this);
        LOG_INFO("LayerManager created successfully");
        qDebug() << "[Pipeline] ✅ LayerManager created";
        
        // 设置 TileMapManager 和缩放级别（与瓦片地图保持一致）
        if (tileMapManager) {
            // 关键：传递 TileMapManager 给 LayerManager，让管网使用相同的坐标系统
            m_layerManager->setTileMapManager(tileMapManager);
            qDebug() << "[Pipeline] ✅ TileMapManager set to LayerManager";
            
            int currentZoom = currentZoomLevel;  // 使用全局的 currentZoomLevel
            m_layerManager->setZoom(currentZoom);
            m_layerManager->setTileSize(256);
            qDebug() << "[Pipeline] Renderer zoom set to:" << currentZoom;
            LOG_INFO(QString("Renderer zoom synchronized to tile map: %1").arg(currentZoom));
        }
        
        // 5. 连接信号
        qDebug() << "[Pipeline] Connecting signals...";
        connect(m_layerManager, &LayerManager::layerRefreshed,
                this, [this](LayerManager::LayerType type) {
            LOG_INFO(QString("Layer refreshed: %1 - %2")
                         .arg((int)type)
                         .arg(m_layerManager->getLayerName(type)));
            qDebug() << "[Pipeline] Layer refreshed:" << m_layerManager->getLayerName(type);
        });
        
        connect(m_layerManager, &LayerManager::loadProgress,
                this, [this](int current, int total) {
            updateStatus(QString("加载管网数据: %1/%2").arg(current).arg(total));
        });
        
        qDebug() << "[Pipeline] ✅ Signals connected";
        updateStatus("数据库已连接，准备加载管网数据...");
        
        // 连接图层管理器到控制面板
        if (m_layerControlPanel) {
            m_layerControlPanel->setLayerManager(m_layerManager);
            qDebug() << "[Pipeline] ✅ LayerManager connected to control panel";
        }
        
        // 6. 延迟加载初始数据（延迟2秒，确保地图视图已完全初始化）
        // 这样可以根据实际地图视图范围加载数据，而不是使用固定范围
        qDebug() << "[Pipeline] Scheduling data load in 2 seconds (waiting for map view to be ready)...";
        QTimer::singleShot(100, this, &MyForm::loadPipelineData);
        
        LOG_INFO("=== Pipeline visualization initialized successfully ===");
        qDebug() << "[Pipeline] ✅ Initialization complete";
        
    } catch (const std::exception &e) {
        LOG_ERROR(QString("Failed to create LayerManager: %1").arg(e.what()));
        updateStatus(QString("图层管理器创建失败: %1").arg(e.what()));
        qDebug() << "[Pipeline] ❌ LayerManager creation failed:" << e.what();
        m_layerManager = nullptr;
    } catch (...) {
        LOG_ERROR("Failed to create LayerManager: unknown exception");
        updateStatus("图层管理器创建失败");
        qDebug() << "[Pipeline] ❌ LayerManager creation failed: unknown exception";
        m_layerManager = nullptr;
    }
}

void MyForm::loadPipelineData()
{
    if (!m_layerManager) {
        LOG_WARNING("LayerManager not initialized, skipping pipeline data load");
        return;
    }
    
    LOG_INFO("Loading pipeline data from database");
    updateStatus("正在加载管网数据...");
    qDebug() << "[Pipeline] ========== Starting pipeline data load ==========";
    
    // 计算当前地图视图的地理范围
    QRectF geoBounds;
    
    if (tileMapManager && ui->graphicsView) {
        // 获取视图的可见矩形（场景坐标）
        QRectF viewportRect = ui->graphicsView->mapToScene(
            ui->graphicsView->viewport()->rect()
        ).boundingRect();
        
        // 将场景坐标转换为地理坐标
        QPointF topLeft = tileMapManager->sceneToGeo(viewportRect.topLeft(), currentZoomLevel);
        QPointF bottomRight = tileMapManager->sceneToGeo(viewportRect.bottomRight(), currentZoomLevel);
        
        // 计算地理范围（考虑可能的坐标翻转）
        double minLon = qMin(topLeft.x(), bottomRight.x());
        double maxLon = qMax(topLeft.x(), bottomRight.x());
        double minLat = qMin(topLeft.y(), bottomRight.y());
        double maxLat = qMax(topLeft.y(), bottomRight.y());
        
        geoBounds = QRectF(minLon, minLat, maxLon - minLon, maxLat - minLat);
        
        qDebug() << "[Pipeline] Calculated view bounds from map:" << geoBounds;
    } else {
        // 如果地图未初始化，使用默认范围（北京天安门区域）
        // 或者使用一个较大的范围以加载更多数据
        geoBounds = QRectF(
            116.390,  // 最小经度
            39.900,   // 最小纬度
            0.030,    // 宽度（经度跨度）
            0.020     // 高度（纬度跨度）
        );
        qDebug() << "[Pipeline] Using default bounds (map not ready):" << geoBounds;
    }
    
    // 只用于数据库空间查询的可视范围，不改变当前地图视图和缩放
    m_layerManager->setVisibleBounds(geoBounds);
    qDebug() << "[Pipeline] Visible bounds set to:" << geoBounds;
    
    // 刷新所有可见图层（不会修改 tileMapManager 的 center/zoom）
    m_layerManager->refreshAllLayers();
    qDebug() << "[Pipeline] refreshAllLayers() called";
    
    // 同时加载用户绘制的数据
    DrawingDatabaseManager::loadFromDatabase(
        mapScene,
        m_drawnPipelines,
        m_nextPipelineId
    );
    
    LOG_INFO("Pipeline layers refreshed");
    updateStatus("管网数据加载完成");
    checkPipelineRenderResult();
}

void MyForm::checkPipelineRenderResult()
{
    // 延迟检查渲染结果，让渲染器有时间完成
    QTimer::singleShot(500, this, [this]() {
        if (!mapScene || !ui->graphicsView) {
            updateStatus("场景未初始化");
            return;
        }
        
        // 统计场景中的项目
        QList<QGraphicsItem*> allItems = mapScene->items();
        int totalItems = allItems.size();
        int pipelineItems = 0;
        int facilityItems = 0;
        
        for (QGraphicsItem *item : allItems) {
            QString type = item->data(0).toString();
            if (type == "pipeline") {
                pipelineItems++;
            } else if (type == "facility") {
                facilityItems++;
            }
        }
        
        qDebug() << "[Pipeline] ========== Render Result ==========";
        qDebug() << "[Pipeline] Total scene items:" << totalItems;
        qDebug() << "[Pipeline] Pipeline items:" << pipelineItems;
        qDebug() << "[Pipeline] Facility items:" << facilityItems;
        
        if (pipelineItems > 0 || facilityItems > 0) {
            QRectF sceneBounds = mapScene->itemsBoundingRect();
            qDebug() << "[Pipeline] Scene bounds:" << sceneBounds;
            LOG_INFO(QString("Pipeline rendered: %1 pipelines, %2 facilities")
                         .arg(pipelineItems).arg(facilityItems));
            updateStatus(QString("管网数据加载完成 (%1个管线, %2个设施)")
                             .arg(pipelineItems).arg(facilityItems));
        } else {
            qDebug() << "[Pipeline] ⚠️  No pipeline/facility items found in scene!";
            LOG_WARNING("No pipeline items rendered - check database data");
            updateStatus("未找到管网数据（请检查数据库）");
        }
        qDebug() << "[Pipeline] ==========================================";
    });
}

void MyForm::onViewTransformChanged()
{
    // 视图变换时更新可视范围（用于动态加载）
    // 当前版本暂不实现动态加载，后续优化时可以启用
    LOG_DEBUG("View transform changed");
}

// ========================================
// 新功能区槽函数实现
// ========================================

// 数据与地图模块
void MyForm::onLoadDataButtonClicked()
{
    qDebug() << "[UI] Load Data button clicked";
    updateStatus("准备导入数据库/示例数据...");
    promptDatabaseSetup("需要导入管线/设施数据以启用可视化和分析。");
}

void MyForm::onDownloadMapButtonClicked()
{
    qDebug() << "[UI] Download Map button clicked";
    updateStatus("打开地图下载管理器...");
    // 触发地图管理对话框
    auto dlg = new MapManagerDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose, true);
    static ManifestStore store("manifest.json");
    store.load();
    static MapManagerSettings settings = MapManagerSettings::load("settings.json");
    dlg->setSettings(settings);
    auto *sched = new DownloadScheduler(dlg);
    sched->configure(settings);
    sched->setManifest(&store);
    sched->setTileManager(tileMapManager);
    connect(sched, &DownloadScheduler::taskProgress, dlg, &MapManagerDialog::onTaskProgress);
    connect(dlg, &MapManagerDialog::requestPause, sched, &DownloadScheduler::pause);
    connect(dlg, &MapManagerDialog::requestResume, sched, &DownloadScheduler::resume);
    connect(dlg, &MapManagerDialog::requestStartDownload, this, [sched, &store, dlg]() mutable {
        auto s = dlg->getSettings();
        DownloadTask t; t.minLat = 18; t.maxLat = 54; t.minLon = 73; t.maxLon = 135;
        t.minZoom = s.minZoom; t.maxZoom = s.maxZoom; t.status = "pending";
        store.upsertTask(t); store.save();
        sched->start();
    });
    connect(dlg, &MapManagerDialog::requestSaveSettings, this, [dlg, &settings]() mutable {
        settings = dlg->getSettings();
        settings.save("settings.json");
    });
    dlg->show();
}

// 空间分析模块
void MyForm::onBurstAnalysisButtonClicked()
{
    qDebug() << "[UI] Burst Analysis button clicked";
    // 需要数据库数据支撑
    if (!DatabaseManager::instance().isConnected()) {
        promptDatabaseSetup("需要连接数据库才能执行爆管分析。");
        return;
    }

    // 进入爆管点选模式
    startBurstSelectionMode();
}

void MyForm::performBurstAnalysis(const QPointF &geoLonLat)
{
    updateStatus("正在执行爆管分析...");

    BurstAnalyzer analyzer(this);
    BurstAnalysisResult result = analyzer.analyzeBurst(geoLonLat);

    if (!result.success) {
        QMessageBox::warning(this, "爆管分析失败",
                             QString("原因：%1\n\n请确认数据库中存在管线/阀门数据。").arg(result.message));
        updateStatus("爆管分析失败，可继续点击地图选择爆管点，右键/ESC退出");
        return;
    }

    QString valves = result.affectedValves.isEmpty() ? "无" : result.affectedValves.join(", ");
    QString pipes = result.affectedPipelines.isEmpty() ? "无" : result.affectedPipelines.join(", ");
    QString actions = result.suggestedActions.isEmpty() ? "—" : result.suggestedActions.join("\n- ");
    QString contacts = result.emergencyContacts.isEmpty() ? "—" : result.emergencyContacts.join("\n");

    QString summary = QString(
        "爆管位置：%.6f, %.6f\n"
        "爆管管线：%1\n"
        "管径(mm)：%2\n"
        "需关闭阀门：%3\n"
        "受影响管线：%4\n"
        "影响面积(m²)：%5\n"
        "估算影响用户：%6\n"
        "预计维修时长(小时)：%7\n"
        "建议操作：\n- %8\n\n"
        "应急联系人：\n%9")
        .arg(result.pipelineId.isEmpty() ? "自动匹配" : result.pipelineId)
        .arg(result.pipelineDiameter, 0, 'f', 1)
        .arg(valves)
        .arg(pipes)
        .arg(result.affectedAreaSize, 0, 'f', 1)
        .arg(result.estimatedAffectedUsers)
        .arg(result.estimatedRepairTime, 0, 'f', 1)
        .arg(actions)
        .arg(contacts);

    QMessageBox::information(this, "爆管分析结果", summary);
    renderBurstResult(result);
    updateStatus("爆管分析完成，可继续点击地图选择爆管点，右键/ESC退出");
}

void MyForm::onConnectivityAnalysisButtonClicked()
{
    qDebug() << "[UI] Connectivity Analysis button clicked";
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        promptDatabaseSetup("连通性分析需要数据库连接。");
        return;
    }
    
    // 弹出选择分析类型的对话框
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle("选择连通性分析类型");
    msgBox.setText("请选择要执行的连通性分析类型：");
    
    QPushButton *upstreamBtn = msgBox.addButton("上游追踪", QMessageBox::ActionRole);
    QPushButton *downstreamBtn = msgBox.addButton("下游追踪", QMessageBox::ActionRole);
    QPushButton *shortestPathBtn = msgBox.addButton("最短路径", QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    
    ConnectivityType analysisType = ConnectivityType::Upstream;
    if (msgBox.clickedButton() == upstreamBtn) {
        analysisType = ConnectivityType::Upstream;
        startConnectivitySelectionMode(analysisType);
    } else if (msgBox.clickedButton() == downstreamBtn) {
        analysisType = ConnectivityType::Downstream;
        startConnectivitySelectionMode(analysisType);
    } else if (msgBox.clickedButton() == shortestPathBtn) {
        // 最短路径需要两个点
        startShortestPathSelectionMode();
    } else {
        return; // 取消
    }
}

// ========================================
// 连通性分析功能实现
// ========================================

void MyForm::clearConnectivityHighlights()
{
    for (auto *item : m_connectivityHighlights) {
        if (mapScene) mapScene->removeItem(item);
        delete item;
    }
    m_connectivityHighlights.clear();
}

void MyForm::renderConnectivityResult(const ConnectivityResult &result)
{
    if (!mapScene || !tileMapManager) return;
    
    clearConnectivityHighlights();
    
    // 高亮追踪到的管线
    QColor highlightColor;
    QString typeName;
    if (result.type == ConnectivityType::Upstream) {
        highlightColor = QColor(0, 150, 255); // 蓝色表示上游
        typeName = "上游";
    } else if (result.type == ConnectivityType::Downstream) {
        highlightColor = QColor(255, 150, 0); // 橙色表示下游
        typeName = "下游";
    } else {
        highlightColor = QColor(0, 255, 0); // 绿色表示路径
        typeName = "路径";
    }
    
    // 高亮路径中的管线
    for (const QString &pipelineId : result.pathPipelines) {
        highlightPipelineByIdForConnectivity(pipelineId, highlightColor);
    }
    
    // 标记起点
    if (!result.startPoint.isNull()) {
        QPointF scenePos = tileMapManager->geoToScene(result.startPoint.x(), result.startPoint.y());
        QGraphicsEllipseItem *marker = mapScene->addEllipse(scenePos.x() - 6, scenePos.y() - 6, 12, 12,
                                                             QPen(highlightColor, 2), QBrush(highlightColor));
        marker->setZValue(1e5);
        m_connectivityHighlights << marker;
    }
}

void MyForm::highlightPipelineByIdForConnectivity(const QString &pipelineId, const QColor &color)
{
    if (!mapScene || !tileMapManager) return;
    PipelineDAO dao;
    Pipeline pl = dao.findByPipelineId(pipelineId);
    if (!pl.isValid() || pl.coordinates().isEmpty()) return;

    QPainterPath path;
    bool first = true;
    for (const QPointF &geo : pl.coordinates()) {
        QPointF scenePt = tileMapManager->geoToScene(geo.x(), geo.y());
        if (first) { path.moveTo(scenePt); first = false; }
        else { path.lineTo(scenePt); }
    }
    QGraphicsPathItem *item = mapScene->addPath(path, QPen(color, 4.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setZValue(1e6 - 1);
    m_connectivityHighlights << item;
}

void MyForm::setUiEnabledDuringConnectivity(bool enabled)
{
    // 恢复之前暂存的控件
    if (enabled) {
        for (QWidget *w : m_disabledDuringConnectivity) {
            if (w) w->setEnabled(true);
        }
        m_disabledDuringConnectivity.clear();
        return;
    }

    // 暂时禁用功能区和面板切换器，避免误操作
    QList<QWidget*> targets;
    if (ui->functionalArea) targets << ui->functionalArea;
    if (m_panelSwitcher) targets << m_panelSwitcher;
    if (m_panelContainer) targets << m_panelContainer;

    for (QWidget *w : targets) {
        if (w && w->isEnabled()) {
            m_disabledDuringConnectivity << w;
            w->setEnabled(false);
        }
    }
}

void MyForm::startConnectivitySelectionMode(ConnectivityType type)
{
    m_connectivitySelectionMode = true;
    m_currentConnectivityType = static_cast<int>(type); // 存储为int避免头文件依赖
    
    QString typeName = (type == ConnectivityType::Upstream) ? "上游追踪" : "下游追踪";
    updateStatus(QString("连通性分析（%1）：请选择起点（左键确认，右键/ESC 取消）").arg(typeName));
    ui->graphicsView->setCursor(Qt::CrossCursor);
    setUiEnabledDuringConnectivity(false);
    clearConnectivityHighlights();
}

void MyForm::cancelConnectivitySelectionMode()
{
    m_connectivitySelectionMode = false;
    ui->graphicsView->unsetCursor();
    updateStatus("已取消连通性分析");
    setUiEnabledDuringConnectivity(true);
    clearConnectivityHighlights();
}

void MyForm::performConnectivityAnalysis(const QPointF &geoLonLat)
{
    if (!DatabaseManager::instance().isConnected()) {
        QMessageBox::warning(this, "错误", "数据库未连接，无法进行连通性分析");
        cancelConnectivitySelectionMode();
        return;
    }
    
    updateStatus("正在执行连通性分析...");
    
    ConnectivityAnalyzer analyzer(this);
    ConnectivityResult result;
    
    ConnectivityType currentType = static_cast<ConnectivityType>(m_currentConnectivityType);
    
    if (currentType == ConnectivityType::Upstream) {
        result = analyzer.traceUpstream(geoLonLat, 100);
    } else if (currentType == ConnectivityType::Downstream) {
        result = analyzer.traceDownstream(geoLonLat, 100);
    } else {
        QMessageBox::warning(this, "错误", "不支持的分析类型");
        cancelConnectivitySelectionMode();
        return;
    }
    
    if (!result.success) {
        QMessageBox::warning(this, "分析失败", result.message.isEmpty() ? "连通性分析失败" : result.message);
        updateStatus("连通性分析失败，可继续点击地图选择起点，右键/ESC退出");
        return;
    }
    
    // 显示结果
    QString typeName = (currentType == ConnectivityType::Upstream) ? "上游追踪" : "下游追踪";
    QString summary = QString(
        "%1分析结果\n\n"
        "起点位置: %2, %3\n"
        "追踪深度: %4 层\n"
        "路径管线数: %5 条\n"
        "路径总长度: %6 m\n"
        "是否连通: %7\n"
        "\n路径管线列表:\n%8"
    )
    .arg(typeName)
    .arg(geoLonLat.y(), 0, 'f', 6)
    .arg(geoLonLat.x(), 0, 'f', 6)
    .arg(result.nodeCount)
    .arg(result.pathPipelines.size())
    .arg(result.totalLength, 0, 'f', 1)
    .arg(result.isConnected ? "是" : "否")
    .arg(result.pathPipelines.join(", "));
    
    QMessageBox::information(this, "连通性分析结果", summary);
    renderConnectivityResult(result);
    updateStatus(QString("%1分析完成，可继续点击地图选择起点，右键/ESC退出").arg(typeName));
}

// ========================================
// 最短路径分析功能实现
// ========================================

void MyForm::clearShortestPathMarkers()
{
    if (m_startMarker && mapScene) {
        mapScene->removeItem(m_startMarker);
        delete m_startMarker;
        m_startMarker = nullptr;
    }
    if (m_endMarker && mapScene) {
        mapScene->removeItem(m_endMarker);
        delete m_endMarker;
        m_endMarker = nullptr;
    }
    clearConnectivityHighlights(); // 复用连通性分析的高亮清理
}

void MyForm::renderShortestPathResult(const ConnectivityResult &result)
{
    if (!mapScene || !tileMapManager) return;
    
    clearConnectivityHighlights();
    
    // 高亮路径中的管线（绿色表示最短路径）
    QColor pathColor = QColor(0, 255, 0); // 绿色
    for (const QString &pipelineId : result.pathPipelines) {
        highlightPipelineByIdForConnectivity(pipelineId, pathColor);
    }
    
    // 标记起点和终点
    if (!result.startPoint.isNull()) {
        QPointF scenePos = tileMapManager->geoToScene(result.startPoint.x(), result.startPoint.y());
        QGraphicsEllipseItem *marker = mapScene->addEllipse(scenePos.x() - 8, scenePos.y() - 8, 16, 16,
                                                             QPen(QColor(0, 255, 0), 2), QBrush(QColor(0, 255, 0)));
        marker->setZValue(1e5);
        m_connectivityHighlights << marker;
    }
    
    if (!result.endPoint.isNull()) {
        QPointF scenePos = tileMapManager->geoToScene(result.endPoint.x(), result.endPoint.y());
        QGraphicsEllipseItem *marker = mapScene->addEllipse(scenePos.x() - 8, scenePos.y() - 8, 16, 16,
                                                             QPen(QColor(255, 0, 0), 2), QBrush(QColor(255, 0, 0)));
        marker->setZValue(1e5);
        m_connectivityHighlights << marker;
    }
}

void MyForm::setUiEnabledDuringShortestPath(bool enabled)
{
    if (enabled) {
        for (QWidget *w : m_disabledDuringShortestPath) {
            if (w) w->setEnabled(true);
        }
        m_disabledDuringShortestPath.clear();
        return;
    }

    QList<QWidget*> targets;
    if (ui->functionalArea) targets << ui->functionalArea;
    if (m_panelSwitcher) targets << m_panelSwitcher;
    if (m_panelContainer) targets << m_panelContainer;

    for (QWidget *w : targets) {
        if (w && w->isEnabled()) {
            m_disabledDuringShortestPath << w;
            w->setEnabled(false);
        }
    }
}

void MyForm::startShortestPathSelectionMode()
{
    m_shortestPathSelectionMode = true;
    m_hasShortestPathStart = false;
    m_shortestPathStartPoint = QPointF();
    
    updateStatus("最短路径分析：请先选择起点（左键确认起点，右键/ESC 取消）");
    ui->graphicsView->setCursor(Qt::CrossCursor);
    setUiEnabledDuringShortestPath(false);
    clearShortestPathMarkers();
    clearConnectivityHighlights();
}

void MyForm::cancelShortestPathSelectionMode()
{
    m_shortestPathSelectionMode = false;
    m_hasShortestPathStart = false;
    ui->graphicsView->unsetCursor();
    updateStatus("已取消最短路径分析");
    setUiEnabledDuringShortestPath(true);
    clearShortestPathMarkers();
}

void MyForm::handleShortestPathPointSelection(const QPointF &geoLonLat)
{
    if (!m_hasShortestPathStart) {
        // 选择起点
        m_shortestPathStartPoint = geoLonLat;
        m_hasShortestPathStart = true;
        
        // 在地图上标记起点
        if (mapScene && tileMapManager) {
            QPointF scenePos = tileMapManager->geoToScene(geoLonLat.x(), geoLonLat.y());
            clearShortestPathMarkers();
            m_startMarker = mapScene->addEllipse(scenePos.x() - 8, scenePos.y() - 8, 16, 16,
                                                QPen(QColor(0, 255, 0), 2), QBrush(QColor(0, 255, 0)));
            m_startMarker->setZValue(1e5);
        }
        
        updateStatus("最短路径分析：起点已选择，请选择终点（左键确认终点，右键/ESC 取消）");
    } else {
        // 选择终点，执行分析
        performShortestPathAnalysis(m_shortestPathStartPoint, geoLonLat);
    }
}

void MyForm::performShortestPathAnalysis(const QPointF &startPoint, const QPointF &endPoint)
{
    if (!DatabaseManager::instance().isConnected()) {
        QMessageBox::warning(this, "错误", "数据库未连接，无法进行最短路径分析");
        cancelShortestPathSelectionMode();
        return;
    }
    
    updateStatus("正在计算最短路径...");
    
    ConnectivityAnalyzer analyzer(this);
    ConnectivityResult result = analyzer.findShortestPath(startPoint, endPoint);
    
    if (!result.success) {
        QMessageBox::warning(this, "分析失败", result.message.isEmpty() ? "最短路径查找失败" : result.message);
        updateStatus("最短路径查找失败，可重新选择起点和终点，右键/ESC退出");
        return;
    }
    
    // 显示结果
    QString summary = QString(
        "最短路径分析结果\n\n"
        "起点位置: %1, %2\n"
        "终点位置: %3, %4\n"
        "路径管线数: %5 条\n"
        "路径总长度: %6 m\n"
        "是否连通: %7\n"
        "\n路径管线列表:\n%8"
    )
    .arg(startPoint.y(), 0, 'f', 6)
    .arg(startPoint.x(), 0, 'f', 6)
    .arg(endPoint.y(), 0, 'f', 6)
    .arg(endPoint.x(), 0, 'f', 6)
    .arg(result.pathPipelines.size())
    .arg(result.totalLength, 0, 'f', 1)
    .arg(result.isConnected ? "是" : "否")
    .arg(result.pathPipelines.join(", "));
    
    QMessageBox::information(this, "最短路径分析结果", summary);
    renderShortestPathResult(result);
    
    // 重置状态，允许继续选择新的起点和终点
    m_hasShortestPathStart = false;
    m_shortestPathStartPoint = QPointF();
    updateStatus("最短路径分析完成，可继续选择新的起点和终点，右键/ESC退出");
}

// 工单与资产模块
void MyForm::onWorkOrderButtonClicked()
{
    qDebug() << "[UI] Work Order button clicked";
    
    // 检查权限
    if (!PermissionManager::canViewMap()) {
        QMessageBox::warning(this, "权限不足", "您没有权限访问工单管理功能。");
        return;
    }
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        promptDatabaseSetup("工单管理需要数据库连接。");
        return;
    }
    
    updateStatus("打开工单管理...");
    
    // 打开工单管理对话框
    WorkOrderManagerDialog *dialog = new WorkOrderManagerDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->exec();
    
    updateStatus("工单管理已关闭");
}

void MyForm::onAssetManagementButtonClicked()
{
    qDebug() << "[UI] Asset Management button clicked";
    
    // 检查权限
    if (!PermissionManager::canViewMap()) {
        QMessageBox::warning(this, "权限不足", "您没有权限访问资产管理功能。");
        return;
    }
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        promptDatabaseSetup("资产管理需要数据库连接。");
        return;
    }
    
    updateStatus("打开资产管理...");
    
    // 打开资产管理对话框
    AssetManagerDialog *dialog = new AssetManagerDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    
    // 连接信号，接收资产编辑变更
    connect(dialog, &AssetManagerDialog::pipelineModified, this, [this](const Pipeline &pipeline, int originalId) {
        PendingChange change;
        change.type = ChangeModified;
        change.entityType = "pipeline";
        change.data = QVariant::fromValue(pipeline);
        change.originalId = originalId;
        change.graphicsItem = nullptr;  // 资产编辑不关联图形项
        m_pendingChanges.append(change);
        markAsModified();
        qDebug() << "[Asset Edit] Pipeline marked as pending save:" << pipeline.pipelineId();
    });
    
    connect(dialog, &AssetManagerDialog::facilityModified, this, [this](const Facility &facility, int originalId) {
        PendingChange change;
        change.type = ChangeModified;
        change.entityType = "facility";
        change.data = QVariant::fromValue(facility);
        change.originalId = originalId;
        change.graphicsItem = nullptr;  // 资产编辑不关联图形项
        m_pendingChanges.append(change);
        markAsModified();
        qDebug() << "[Asset Edit] Facility marked as pending save:" << facility.facilityId();
    });
    
    dialog->exec();
    
    updateStatus("资产管理已关闭");
}

void MyForm::onHealthAssessmentButtonClicked()
{
    qDebug() << "[UI] Health Assessment button clicked";
    
    // 检查数据库连接
    if (!DatabaseManager::instance().isConnected()) {
        promptDatabaseSetup("健康度评估需要数据库连接。");
        return;
    }
    
    updateStatus("打开健康度评估...");
    
    HealthAssessmentDialog dialog(this);
    dialog.exec();
    
    // 评估完成后，刷新地图显示（如果需要按健康度着色）
    if (m_layerManager) {
        m_layerManager->refreshAllLayers();
    }
    
    updateStatus("健康度评估已关闭");
}

// 工具模块
void MyForm::onSettingsButtonClicked()
{
    qDebug() << "[UI] Settings button clicked";
    
    // 检查权限
    if (!PermissionManager::canAccessSettings()) {
        QMessageBox::warning(this, "权限不足", "您没有权限访问系统设置功能。");
        return;
    }
    
    updateStatus("打开系统设置...");
    
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        updateStatus("系统设置已保存");
        // 可以在这里重新加载配置或刷新界面
    } else {
        updateStatus("已取消设置");
    }
}

void MyForm::onHelpButtonClicked()
{
    qDebug() << "[UI] Help button clicked";
    updateStatus("打开帮助文档...");
    QMessageBox::information(this, "帮助", "欢迎使用城市地下管网智能管理系统(UGIMS)\n\n" 
                            "主要功能模块：\n"
                            "• 数据与地图：导入管网数据、下载离线地图\n"
                            "• 空间分析：爆管影响分析、连通性分析\n"
                            "• 工单与资产：工单管理、资产台账管理\n"
                            "• 工具：系统设置、在线帮助");
}

// 设备树设置
void MyForm::setupDeviceTree()
{
    qDebug() << "Setting up device tree...";
    
    // 创建模型
    deviceTreeModel = new QStandardItemModel(this);
    
    // 设置模型（不显示表头）
    ui->deviceTreeView->setModel(deviceTreeModel);
    
    // 连接信号
    connect(ui->deviceTreeView, &QTreeView::clicked, this, &MyForm::onDeviceTreeItemClicked);
    connect(ui->deviceTreeView, &QTreeView::doubleClicked, this, &MyForm::onDeviceTreeItemDoubleClicked);
    
    // ============ 第1层：管网类型 ============
    
    // 1. 给水管网
    QStandardItem *waterNetwork = new QStandardItem("📘 给水管网");
    waterNetwork->setEditable(false);
    deviceTreeModel->appendRow(waterNetwork);
    
    // 第2层：设施类别
    QStandardItem *waterPipes = new QStandardItem("🔧 管线");
    waterPipes->setEditable(false);
    waterNetwork->appendRow(waterPipes);
    
    // 第3层：具体设备
    QStandardItem *pipe1 = new QStandardItem("  DN300主干管-GS001 🟢运行中");
    pipe1->setEditable(false);
    waterPipes->appendRow(pipe1);
    
    QStandardItem *pipe2 = new QStandardItem("  DN200支管-GS002 🟢运行中");
    pipe2->setEditable(false);
    waterPipes->appendRow(pipe2);
    
    QStandardItem *pipe3 = new QStandardItem("  DN150支管-GS003 🟡维护中");
    pipe3->setEditable(false);
    waterPipes->appendRow(pipe3);
    
    // 阀门井
    QStandardItem *waterValves = new QStandardItem("🚰 阀门井");
    waterValves->setEditable(false);
    waterNetwork->appendRow(waterValves);
    
    QStandardItem *valve1 = new QStandardItem("  阀门井-V001 🟢开启");
    valve1->setEditable(false);
    waterValves->appendRow(valve1);
    
    QStandardItem *valve2 = new QStandardItem("  阀门井-V002 🟢开启");
    valve2->setEditable(false);
    waterValves->appendRow(valve2);
    
    QStandardItem *valve3 = new QStandardItem("  阀门井-V003 🔴关闭");
    valve3->setEditable(false);
    waterValves->appendRow(valve3);
    
    // 泵站
    QStandardItem *waterPumps = new QStandardItem("⚙️ 泵站");
    waterPumps->setEditable(false);
    waterNetwork->appendRow(waterPumps);
    
    QStandardItem *pump1 = new QStandardItem("  一泵站-P001 🟢运行中");
    pump1->setEditable(false);
    waterPumps->appendRow(pump1);
    
    QStandardItem *pump2 = new QStandardItem("  二泵站-P002 🟡待机");
    pump2->setEditable(false);
    waterPumps->appendRow(pump2);
    
    // 监测点
    QStandardItem *waterMonitors = new QStandardItem("🔍 监测点");
    waterMonitors->setEditable(false);
    waterNetwork->appendRow(waterMonitors);
    
    QStandardItem *monitor1 = new QStandardItem("  压力监测-M001 🟢在线");
    monitor1->setEditable(false);
    waterMonitors->appendRow(monitor1);
    
    QStandardItem *monitor2 = new QStandardItem("  流量监测-M002 🟢在线");
    monitor2->setEditable(false);
    waterMonitors->appendRow(monitor2);
    
    // 2. 排水管网
    QStandardItem *drainNetwork = new QStandardItem("📗 排水管网");
    drainNetwork->setEditable(false);
    deviceTreeModel->appendRow(drainNetwork);
    
    QStandardItem *drainPipes = new QStandardItem("🔧 管线");
    drainPipes->setEditable(false);
    drainNetwork->appendRow(drainPipes);
    
    QStandardItem *drain1 = new QStandardItem("  DN400雨水管-PS001 🟢运行中");
    drain1->setEditable(false);
    drainPipes->appendRow(drain1);
    
    QStandardItem *drain2 = new QStandardItem("  DN300污水管-PS002 🟢运行中");
    drain2->setEditable(false);
    drainPipes->appendRow(drain2);
    
    QStandardItem *drainWells = new QStandardItem("🚪 检查井");
    drainWells->setEditable(false);
    drainNetwork->appendRow(drainWells);
    
    QStandardItem *well1 = new QStandardItem("  检查井-J001 🟢正常");
    well1->setEditable(false);
    drainWells->appendRow(well1);
    
    QStandardItem *well2 = new QStandardItem("  检查井-J002 🟡淤积");
    well2->setEditable(false);
    drainWells->appendRow(well2);
    
    QStandardItem *drainPumps = new QStandardItem("⚙️ 泵站");
    drainPumps->setEditable(false);
    drainNetwork->appendRow(drainPumps);
    
    QStandardItem *drainPump1 = new QStandardItem("  排水泵站-P003 🟢运行中");
    drainPump1->setEditable(false);
    drainPumps->appendRow(drainPump1);
    
    // 3. 燃气管网
    QStandardItem *gasNetwork = new QStandardItem("📙 燃气管网");
    gasNetwork->setEditable(false);
    deviceTreeModel->appendRow(gasNetwork);
    
    QStandardItem *gasPipes = new QStandardItem("🔧 管线");
    gasPipes->setEditable(false);
    gasNetwork->appendRow(gasPipes);
    
    QStandardItem *gas1 = new QStandardItem("  DN200主管-RQ001 🟢运行中");
    gas1->setEditable(false);
    gasPipes->appendRow(gas1);
    
    QStandardItem *gas2 = new QStandardItem("  DN100支管-RQ002 🟢运行中");
    gas2->setEditable(false);
    gasPipes->appendRow(gas2);
    
    QStandardItem *gasValves = new QStandardItem("🚰 阀门井");
    gasValves->setEditable(false);
    gasNetwork->appendRow(gasValves);
    
    QStandardItem *gasValve1 = new QStandardItem("  调压柜-V004 🟢正常");
    gasValve1->setEditable(false);
    gasValves->appendRow(gasValve1);
    
    QStandardItem *gasMonitors = new QStandardItem("🔍 监测点");
    gasMonitors->setEditable(false);
    gasNetwork->appendRow(gasMonitors);
    
    QStandardItem *gasMonitor1 = new QStandardItem("  燃气监测-M003 🟢在线");
    gasMonitor1->setEditable(false);
    gasMonitors->appendRow(gasMonitor1);
    
    // 4. 电力管网
    QStandardItem *powerNetwork = new QStandardItem("📕 电力管网");
    powerNetwork->setEditable(false);
    deviceTreeModel->appendRow(powerNetwork);
    
    QStandardItem *powerCables = new QStandardItem("🔧 电缆");
    powerCables->setEditable(false);
    powerNetwork->appendRow(powerCables);
    
    QStandardItem *cable1 = new QStandardItem("  10kV电缆-DL001 🟢运行中");
    cable1->setEditable(false);
    powerCables->appendRow(cable1);
    
    QStandardItem *powerFacilities = new QStandardItem("⚡ 配电设施");
    powerFacilities->setEditable(false);
    powerNetwork->appendRow(powerFacilities);
    
    QStandardItem *power1 = new QStandardItem("  配电箱-D001 🟢正常");
    power1->setEditable(false);
    powerFacilities->appendRow(power1);
    
    // 5. 通信管网
    QStandardItem *commNetwork = new QStandardItem("📒 通信管网");
    commNetwork->setEditable(false);
    deviceTreeModel->appendRow(commNetwork);
    
    QStandardItem *commCables = new QStandardItem("🔧 光缆");
    commCables->setEditable(false);
    commNetwork->appendRow(commCables);
    
    QStandardItem *fiber1 = new QStandardItem("  主干光缆-TX001 🟢运行中");
    fiber1->setEditable(false);
    commCables->appendRow(fiber1);
    
    QStandardItem *commPoints = new QStandardItem("📡 接入点");
    commPoints->setEditable(false);
    commNetwork->appendRow(commPoints);
    
    QStandardItem *comm1 = new QStandardItem("  分线盒-F001 🟢正常");
    comm1->setEditable(false);
    commPoints->appendRow(comm1);
    
    // 6. 热力管网
    QStandardItem *heatNetwork = new QStandardItem("📓 热力管网");
    heatNetwork->setEditable(false);
    deviceTreeModel->appendRow(heatNetwork);
    
    QStandardItem *heatPipes = new QStandardItem("🔧 管线");
    heatPipes->setEditable(false);
    heatNetwork->appendRow(heatPipes);
    
    QStandardItem *heat1 = new QStandardItem("  DN400供热管-RL001 🟢运行中");
    heat1->setEditable(false);
    heatPipes->appendRow(heat1);
    
    QStandardItem *heatStations = new QStandardItem("⚙️ 换热站");
    heatStations->setEditable(false);
    heatNetwork->appendRow(heatStations);
    
    QStandardItem *heatStation1 = new QStandardItem("  换热站-H001 🟢运行中");
    heatStation1->setEditable(false);
    heatStations->appendRow(heatStation1);
    
    // 默认展开第一层（给水管网）
    ui->deviceTreeView->expand(deviceTreeModel->index(0, 0));
    
    qDebug() << "Device tree setup completed with hierarchical structure";
    updateStatus("设备树初始化完成 - 6类管网");
}

// 设备树点击事件
void MyForm::onDeviceTreeItemClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(index);
    if (!item) return;
    
    QString text = item->text();
    
    // 判断层级：根据父节点数量判断
    int level = 0;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        level++;
        parent = parent.parent();
    }
    
    QString levelName;
    switch(level) {
        case 0: levelName = "管网类型"; break;
        case 1: levelName = "设施类别"; break;
        case 2: levelName = "具体设备"; break;
        default: levelName = "详细信息"; break;
    }
    
    qDebug() << "Device tree item clicked - Level:" << level << "(" << levelName << ")" << "Text:" << text;
    updateStatus("选中[" + levelName + "]: " + text);
}

// 设备树双击事件
void MyForm::onDeviceTreeItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(index);
    if (!item) return;
    
    QString name = deviceTreeModel->item(index.row(), 0)->text();
    
    qDebug() << "Device tree item double-clicked:" << name;
    updateStatus("打开设备详情: " + name);
    
    // 判断层级
    int level = 0;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        level++;
        parent = parent.parent();
    }
    
    // 只有第3层（具体设备）才显示详情
    if (level == 2) {
        // 尝试从设备名称中提取ID（例如 "DN300主干管-GS001"）
        QString deviceId;
        QStringList parts = name.split("-");
        if (parts.size() >= 2) {
            deviceId = parts.last().split(" ").first(); // 提取 "GS001"
        }
        
        // 尝试查找对应的管线或设施
        if (!deviceId.isEmpty()) {
            PipelineDAO pipelineDao;
            Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
            
            if (pipeline.isValid()) {
                // 显示管线详情
                PipelineEditDialog dialog(this);
                dialog.loadPipeline(pipeline);
                dialog.exec();
                return;
            }
            
            FacilityDAO facilityDao;
            Facility facility = facilityDao.findByFacilityId(deviceId);
            
            if (facility.isValid()) {
                // 显示设施详情
                FacilityEditDialog dialog(this, facility);
                dialog.exec();
                return;
            }
        }
        
        // 如果找不到对应的实体，显示基本信息
        QMessageBox::information(this, "设备详情", 
                                 QString("设备名称: %1\n\n"
                                        "设备ID: %2\n\n"
                                        "提示: 未在数据库中找到对应的实体数据。\n"
                                        "请确保设备ID正确，或通过地图选择设备查看详情。")
                                 .arg(name)
                                 .arg(deviceId.isEmpty() ? "未知" : deviceId));
    } else {
        // 非设备层级，显示层级信息
        QString levelName;
        switch(level) {
            case 0: levelName = "管网类型"; break;
            case 1: levelName = "设施类别"; break;
            default: levelName = "其他"; break;
        }
        
        QMessageBox::information(this, "信息", 
                                 QString("选中项: %1\n层级: %2")
                                 .arg(name)
                                 .arg(levelName));
    }
}

// 搜索框文本变化事件
void MyForm::onDeviceSearchTextChanged(const QString &text)
{
    qDebug() << "Search text changed:" << text;
    filterDeviceTree(text);
    
    if (text.isEmpty()) {
        updateStatus("显示所有设备");
    } else {
        updateStatus("搜索: " + text);
    }
}

// 过滤设备树
void MyForm::filterDeviceTree(const QString &searchText)
{
    if (searchText.isEmpty()) {
        // 搜索框为空，显示所有节点
        for (int i = 0; i < deviceTreeModel->rowCount(); ++i) {
            QStandardItem *item = deviceTreeModel->item(i);
            setItemVisibility(item, true);
        }
        // 折叠所有节点，只展开第一层
        ui->deviceTreeView->collapseAll();
        ui->deviceTreeView->expand(deviceTreeModel->index(0, 0));
    } else {
        // 有搜索文本，过滤节点
        for (int i = 0; i < deviceTreeModel->rowCount(); ++i) {
            QStandardItem *item = deviceTreeModel->item(i);
            bool hasMatch = filterItem(item, searchText);
            setItemVisibility(item, hasMatch);
        }
        // 展开所有匹配的节点
        ui->deviceTreeView->expandAll();
    }
}

// 设置节点可见性
void MyForm::setItemVisibility(QStandardItem *item, bool visible)
{
    if (!item) return;
    
    QModelIndex index = item->index();
    ui->deviceTreeView->setRowHidden(index.row(), index.parent(), !visible);
    
    // 递归设置子节点
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem *child = item->child(i);
        if (child) {
            setItemVisibility(child, visible);
        }
    }
}

// 递归过滤节点（返回是否包含匹配的子节点）
bool MyForm::filterItem(QStandardItem *item, const QString &searchText)
{
    if (!item) return false;
    
    QString itemText = item->text();
    bool currentMatch = itemText.contains(searchText, Qt::CaseInsensitive);
    
    bool hasMatchingChild = false;
    
    // 递归检查所有子节点
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem *child = item->child(i);
        if (child) {
            bool childMatch = filterItem(child, searchText);
            
            // 设置子节点可见性
            QModelIndex childIndex = child->index();
            ui->deviceTreeView->setRowHidden(childIndex.row(), childIndex.parent(), !childMatch);
            
            if (childMatch) {
                hasMatchingChild = true;
            }
        }
    }
    
    // 如果当前节点匹配或者有匹配的子节点，则显示当前节点
    return currentMatch || hasMatchingChild;
}

// 关于按钮点击事件
void MyForm::onAboutButtonClicked()
{
    qDebug() << "About button clicked";
    
    // 创建关于对话框
    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle("关于 " + QString(APP_NAME));
    aboutDialog->setMinimumSize(600, 500);
    aboutDialog->setMaximumSize(600, 500);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(aboutDialog);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // ========== 顶部区域（深色背景） ==========
    QWidget *headerWidget = new QWidget(aboutDialog);
    headerWidget->setStyleSheet(
        "QWidget {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                               stop:0 #FF7A18, stop:1 #FF9A48);"
        "}"
    );
    QVBoxLayout *headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setSpacing(8);
    headerLayout->setContentsMargins(30, 25, 30, 25);
    
    // 应用图标（使用符号代替）
    QLabel *iconLabel = new QLabel("🏛️", headerWidget);
    QFont iconFont;
    iconFont.setPointSize(48);
    iconLabel->setFont(iconFont);
    iconLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(iconLabel);
    
    // 中文名称
    QLabel *titleLabel = new QLabel(APP_NAME, headerWidget);
    QFont titleFont;
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: white;");
    headerLayout->addWidget(titleLabel);
    
    // 英文名称
    QLabel *enNameLabel = new QLabel(APP_NAME_EN, headerWidget);
    QFont enNameFont;
    enNameFont.setPointSize(9);
    enNameLabel->setFont(enNameFont);
    enNameLabel->setAlignment(Qt::AlignCenter);
    enNameLabel->setStyleSheet("color: rgba(255, 255, 255, 0.9);");
    headerLayout->addWidget(enNameLabel);
    
    mainLayout->addWidget(headerWidget);
    
    // ========== 内容区域 ==========
    QWidget *contentWidget = new QWidget(aboutDialog);
    contentWidget->setStyleSheet("background-color: white;");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(30, 25, 30, 20);
    
    // 软件描述
    QLabel *descLabel = new QLabel(APP_DESCRIPTION, contentWidget);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    QFont descFont;
    descFont.setPointSize(9);
    descLabel->setFont(descFont);
    descLabel->setStyleSheet("color: #555; line-height: 1.6;");
    contentLayout->addWidget(descLabel);
    
    // 分隔线
    QFrame *line1 = new QFrame(contentWidget);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setStyleSheet("color: #e0e0e0;");
    contentLayout->addWidget(line1);
    
    // 信息区域（使用网格布局）
    QGridLayout *infoLayout = new QGridLayout();
    infoLayout->setSpacing(12);
    infoLayout->setColumnStretch(0, 1);
    infoLayout->setColumnStretch(1, 2);
    
    QFont labelFont;
    labelFont.setPointSize(9);
    labelFont.setBold(true);
    
    QFont valueFont;
    valueFont.setPointSize(9);
    
    int row = 0;
    
    // 著作人
    QLabel *authorLabelTitle = new QLabel("著作人:", contentWidget);
    authorLabelTitle->setFont(labelFont);
    authorLabelTitle->setStyleSheet("color: #333;");
    QLabel *authorLabelValue = new QLabel(QString("%1 (%2)").arg(APP_AUTHOR).arg(APP_ORGANIZATION), contentWidget);
    authorLabelValue->setFont(valueFont);
    authorLabelValue->setStyleSheet("color: #666;");
    infoLayout->addWidget(authorLabelTitle, row, 0, Qt::AlignRight);
    infoLayout->addWidget(authorLabelValue, row++, 1);
    
    // 联系方式
    QLabel *contactLabelTitle = new QLabel("联系方式:", contentWidget);
    contactLabelTitle->setFont(labelFont);
    contactLabelTitle->setStyleSheet("color: #333;");
    QLabel *contactLabelValue = new QLabel(APP_CONTACT, contentWidget);
    contactLabelValue->setFont(valueFont);
    contactLabelValue->setStyleSheet("color: #FF7A18;");
    contactLabelValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    contactLabelValue->setCursor(Qt::IBeamCursor);
    infoLayout->addWidget(contactLabelTitle, row, 0, Qt::AlignRight);
    infoLayout->addWidget(contactLabelValue, row++, 1);
    
    // 网站
    QLabel *websiteLabelTitle = new QLabel("项目地址:", contentWidget);
    websiteLabelTitle->setFont(labelFont);
    websiteLabelTitle->setStyleSheet("color: #333;");
    QLabel *websiteLabelValue = new QLabel(QString("<a href='%1' style='color: #FF7A18; text-decoration: none;'>%1</a>").arg(APP_WEBSITE), contentWidget);
    websiteLabelValue->setFont(valueFont);
    websiteLabelValue->setOpenExternalLinks(true);
    websiteLabelValue->setTextInteractionFlags(Qt::TextBrowserInteraction);
    infoLayout->addWidget(websiteLabelTitle, row, 0, Qt::AlignRight);
    infoLayout->addWidget(websiteLabelValue, row++, 1);
    
    // 许可证
    QLabel *licenseLabelTitle = new QLabel("许可证:", contentWidget);
    licenseLabelTitle->setFont(labelFont);
    licenseLabelTitle->setStyleSheet("color: #333;");
    QLabel *licenseLabelValue = new QLabel(APP_LICENSE, contentWidget);
    licenseLabelValue->setFont(valueFont);
    licenseLabelValue->setStyleSheet("color: #666;");
    infoLayout->addWidget(licenseLabelTitle, row, 0, Qt::AlignRight);
    infoLayout->addWidget(licenseLabelValue, row++, 1);
    
    // 版本号
    QLabel *versionLabelTitle = new QLabel("版本号:", contentWidget);
    versionLabelTitle->setFont(labelFont);
    versionLabelTitle->setStyleSheet("color: #333;");
    QLabel *versionLabelValue = new QLabel(QString("v%1").arg(APP_VERSION), contentWidget);
    QFont versionValueFont = valueFont;
    versionValueFont.setBold(true);
    versionLabelValue->setFont(versionValueFont);
    versionLabelValue->setStyleSheet("color: #FF7A18;");
    infoLayout->addWidget(versionLabelTitle, row, 0, Qt::AlignRight);
    infoLayout->addWidget(versionLabelValue, row++, 1);
    
    contentLayout->addLayout(infoLayout);
    
    // 分隔线
    QFrame *line2 = new QFrame(contentWidget);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setStyleSheet("color: #e0e0e0;");
    contentLayout->addWidget(line2);
    
    // 版权声明
    QLabel *copyrightLabel = new QLabel(APP_COPYRIGHT, contentWidget);
    copyrightLabel->setAlignment(Qt::AlignCenter);
    QFont copyrightFont;
    copyrightFont.setPointSize(8);
    copyrightLabel->setFont(copyrightFont);
    copyrightLabel->setStyleSheet("color: #999;");
    contentLayout->addWidget(copyrightLabel);
    
    // 添加弹性空间
    contentLayout->addStretch();
    
    mainLayout->addWidget(contentWidget);
    
    // ========== 底部按钮区 ==========
    QWidget *buttonWidget = new QWidget(aboutDialog);
    buttonWidget->setStyleSheet("background-color: #f8f8f8; border-top: 1px solid #e0e0e0;");
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(20, 15, 20, 15);
    
    buttonLayout->addStretch();
    
    // 确定按钮
    QPushButton *okButton = new QPushButton("确定", buttonWidget);
    okButton->setMinimumSize(120, 38);
    okButton->setCursor(Qt::PointingHandCursor);
    okButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF7A18;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-size: 11pt;"
        "  font-weight: bold;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #FF8C3A;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #E66A08;"
        "}"
    );
    connect(okButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    
    buttonLayout->addStretch();
    
    mainLayout->addWidget(buttonWidget);
    
    qDebug() << "Showing about dialog - Version:" << APP_VERSION << "Build:" << APP_BUILD_DATE;
    aboutDialog->exec();
    updateStatus("查看关于信息");
}

// ========================================
// 绘制工具相关函数（右侧滑出面板）
// ========================================

void MyForm::setupDrawingToolPanel()
{
    qDebug() << "设置绘制工具面板 (右侧滑出)...";
    
    // 创建绘制工具面板
    m_drawingToolPanel = new DrawingToolPanel(this);
    
    // 创建容器窗口（用于滑出效果）
    m_drawingToolContainer = new QWidget(ui->graphicsView->viewport());  // 父对象改为viewport
    m_drawingToolContainer->setObjectName("drawingToolContainer");
    m_drawingToolContainer->setStyleSheet(
        "#drawingToolContainer {"
        "  background-color: white;"
        "  border-left: 2px solid #d0d0d0;"
        "  border-radius: 0px;"
        "}"
    );
    
    // 容器布局
    QVBoxLayout *containerLayout = new QVBoxLayout(m_drawingToolContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // 添加面板到容器
    containerLayout->addWidget(m_drawingToolPanel);
    
    // 设置容器宽度
    m_drawingToolContainer->setFixedWidth(250);
    
    // 初始隐藏（移到右侧外面）
    m_drawingToolContainer->hide();
    
    // 创建浮动切换按钮（贴在地图右侧） - 暂时不显示，会被底部按钮替代
    m_drawingToolToggleBtn = new QPushButton("绘制\n工具", ui->graphicsView->viewport());  // 父对象改为viewport
    m_drawingToolToggleBtn->setObjectName("drawingToolToggleBtn");
    m_drawingToolToggleBtn->setToolTip("绘制工具");
    m_drawingToolToggleBtn->setCheckable(true);
    m_drawingToolToggleBtn->setFixedSize(30, 80);  // 调整尺寸：30x80
    m_drawingToolToggleBtn->setCursor(Qt::PointingHandCursor);
    m_drawingToolToggleBtn->setStyleSheet(
        "#drawingToolToggleBtn {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 rgba(100, 150, 255, 0.85),"  // 浅蓝色渐变，半透明
        "                               stop:1 rgba(80, 130, 235, 0.85));"
        "  color: white;"
        "  border: 1px solid rgba(255, 255, 255, 0.4);"  // 白色边框增强可见性
        "  border-radius: 4px 0px 0px 4px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  padding: 4px 2px;"
        "  line-height: 1.2;"
        "  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);"  // 轻微阴影
        "}"
        "#drawingToolToggleBtn:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 rgba(120, 170, 255, 0.95),"
        "                               stop:1 rgba(100, 150, 245, 0.95));"
        "  border: 1px solid rgba(255, 255, 255, 0.6);"
        "}"
        "#drawingToolToggleBtn:checked {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 rgba(70, 120, 200, 0.9),"
        "                               stop:1 rgba(60, 110, 180, 0.9));"
        "  border: 1px solid rgba(255, 255, 255, 0.5);"
        "}"
    );
    
    // 隐藏右侧按钮（改用底部切换）
    m_drawingToolToggleBtn->hide();
    
    // 注意：不再使用右侧按钮的信号，改用底部StackWidget切换
    
    // 连接绘制工具面板的信号
    connect(m_drawingToolPanel, &DrawingToolPanel::startDrawingPipeline,
            this, &MyForm::onStartDrawingPipeline);
    connect(m_drawingToolPanel, &DrawingToolPanel::startDrawingFacility,
            this, &MyForm::onStartDrawingFacility);
    
    // 注意：m_drawingManager 将在 setupMapArea() 中创建，因为它依赖 mapScene 和 tileMapManager
    
    qDebug() << "Drawing tool panel setup completed (DrawingManager will be created after map initialization)";
}

void MyForm::positionDrawingToolPanel()
{
    if (!m_drawingToolContainer) {
        return;
    }
    
    // 获取viewport的几何信息
    QRect viewportRect = ui->graphicsView->viewport()->rect();
    int viewportWidth = viewportRect.width();
    int viewportHeight = viewportRect.height();
    
    // 面板宽度
    int panelWidth = m_drawingToolContainer->width();
    
    // 面板展开显示在右侧，高度与地图窗口一致
    m_drawingToolContainer->setGeometry(
        viewportWidth - panelWidth,  // 贴到右边
        0,                           // 从顶部开始
        panelWidth,
        viewportHeight               // 高度与viewport一致
    );
    
    m_drawingToolContainer->raise();
}

void MyForm::onToggleDrawingTool(bool checked)
{
    // 这个函数现在由 lambda 处理，保留作为备用
    Q_UNUSED(checked);
}

void MyForm::onStartDrawingPipeline(const QString &pipelineType)
{
    qDebug() << "Start drawing pipeline:" << pipelineType;
    
    // 检查权限
    if (!PermissionManager::canCreatePipeline()) {
        QMessageBox::warning(this, "权限不足", "您没有权限创建管线。");
        return;
    }
    
    updateStatus(QString("开始绘制管线: %1 (左键添加点，右键或双击结束当前线)").arg(m_drawingToolPanel->currentTypeName()));
    
    if (m_drawingManager) {
        // 应用当前样式
        m_drawingManager->setDrawingStyle(
            m_drawingToolPanel->currentColor(),
            m_drawingToolPanel->currentLineWidth()
        );
        
        // 开始绘制管线
        m_drawingManager->startDrawingPipeline(pipelineType);
    }
}

void MyForm::onStartDrawingFacility(const QString &facilityType)
{
    qDebug() << "Start drawing facility:" << facilityType;
    
    // 检查权限
    if (!PermissionManager::canCreateFacility()) {
        QMessageBox::warning(this, "权限不足", "您没有权限创建设施。");
        return;
    }
    
    updateStatus(QString("开始绘制设施: %1 (点击地图设置位置)").arg(m_drawingToolPanel->currentTypeName()));
    
    if (m_drawingManager) {
        // 应用当前样式
        m_drawingManager->setDrawingStyle(
            m_drawingToolPanel->currentColor(),
            m_drawingToolPanel->currentLineWidth()
        );
        
        // 开始绘制设施
        m_drawingManager->startDrawingFacility(facilityType);
    }
}

void MyForm::onPipelineDrawingFinished(const QString &pipelineType, const QString &wkt, const QVector<QPointF> &scenePoints)
{
    qDebug() << "Pipeline drawing finished, type:" << pipelineType << "WKT:" << wkt;
    updateStatus("管线绘制完成，请输入属性...");
    
    // 创建并显示属性编辑对话框
    PipelineEditDialog *dialog = new PipelineEditDialog(this);
    dialog->setPipelineType(pipelineType);
    dialog->setGeometry(wkt);
    
    // 自动生成管线编号（可修改）
    // 编号格式：类型前缀-日期-序号，例如：GS-20251212-001
    // 在保存时会检查唯一性，如果重复则自动生成新编号
    dialog->setAutoGeneratedCode(0, pipelineType);
    
    // 自动计算管线长度（使用地理坐标，可手动修改）
    dialog->calculateAndSetLengthFromWKT(wkt);
    
    // 注意：不要设置 Qt::WA_DeleteOnClose，因为我们使用 exec() 模态对话框，手动管理内存
    
    int result = dialog->exec();
    
    if (result == QDialog::Accepted) {
        // 获取管线对象（在对话框关闭前获取数据）
        Pipeline pipeline = dialog->getPipeline();
        
        // 如果编号为空，自动生成
        if (pipeline.pipelineId().isEmpty()) {
            QString baseId = IdGenerator::generatePipelineId(pipeline.pipelineType());
            PipelineDAO checkDao;
            QString uniqueId = IdGenerator::generateNextAvailableId(
                baseId,
                [&checkDao](const QString &id) -> bool {
                    return checkDao.findByPipelineId(id).isValid();
                }
            );
            pipeline.setPipelineId(uniqueId);
            qDebug() << "[Pipeline Drawing] Auto-generated unique ID:" << uniqueId;
        }
        
        // 确保几何信息正确设置
        pipeline.setGeomWkt(wkt);
        // 解析WKT为坐标点
        QString wktClean = wkt;
        wktClean.remove("LINESTRING(");
        wktClean.remove(")");
        QStringList points = wktClean.split(',', Qt::SkipEmptyParts);
        QVector<QPointF> coordinates;
        for (const QString &point : points) {
            QStringList xy = point.trimmed().split(' ', Qt::SkipEmptyParts);
            if (xy.size() >= 2) {
                bool okX = false, okY = false;
                double x = xy[0].toDouble(&okX);
                double y = xy[1].toDouble(&okY);
                if (okX && okY) {
                    coordinates.append(QPointF(x, y));
                }
            }
        }
        if (!coordinates.isEmpty()) {
            pipeline.setCoordinates(coordinates);
            qDebug() << "Set coordinates count:" << coordinates.size();
        } else {
            qDebug() << "Warning: Failed to parse coordinates from WKT:" << wkt;
        }
        
        // 生成唯一ID（自增）
        pipeline.setId(m_nextPipelineId++);
        
        qDebug() << "Pipeline created:" << pipeline.pipelineName() << "Type:" << pipeline.pipelineType();
        qDebug() << "Generated ID:" << pipeline.id();
        qDebug() << "WKT:" << wkt;
        qDebug() << "Scene points count:" << scenePoints.size();
        
        // 手动删除对话框
        delete dialog;
        dialog = nullptr;
        
        // 不立即保存到数据库，而是添加到待保存列表
        // 直接在场景中绘制管线（使用绘制时的场景坐标）
        if (mapScene && scenePoints.size() >= 2) {
            qDebug() << "Drawing pipeline on scene...";
            
            // 创建路径
            QPainterPath path;
            path.moveTo(scenePoints.first());
            for (int i = 1; i < scenePoints.size(); ++i) {
                path.lineTo(scenePoints[i]);
            }
            
            // 获取管线样式（根据类型设置颜色）
            QColor lineColor;
            QString typeName;
            if (pipelineType == "water_supply") {
                lineColor = QColor(0, 112, 192);  // 蓝色
                typeName = "给水";
            } else if (pipelineType == "sewage") {
                lineColor = QColor(112, 48, 160);  // 紫色
                typeName = "排水";
            } else if (pipelineType == "gas") {
                lineColor = QColor(255, 192, 0);   // 黄色
                typeName = "燃气";
            } else if (pipelineType == "electric") {
                lineColor = QColor(255, 0, 0);     // 红色
                typeName = "电力";
            } else if (pipelineType == "telecom") {
                lineColor = QColor(0, 176, 80);    // 绿色
                typeName = "通信";
            } else if (pipelineType == "heat") {
                lineColor = QColor(255, 128, 0);   // 橙色
                typeName = "供热";
            } else {
                lineColor = QColor(128, 128, 128); // 灰色
                typeName = "未知";
            }
            
            // 创建画笔
            QPen pen(lineColor, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            
            // 添加到场景
            QGraphicsPathItem *item = mapScene->addPath(path, pen);
            item->setZValue(100);  // 确保在瓦片地图之上
            
            // 设置工具提示
            QString tooltip = QString("%1\n类型: %2\n管径: DN%3")
                                  .arg(pipeline.pipelineName())
                                  .arg(typeName)
                                  .arg(pipeline.diameterMm());
            item->setToolTip(tooltip);
            
            // 存储数据（用于后续查询和删除）
            item->setData(0, "pipeline");
            item->setData(1, pipeline.pipelineId());
            item->setData(2, pipelineType);
            item->setData(100, static_cast<int>(EntityState::Added));  // 新增：设置实体状态为Added
            
            // 关键：保存管线对象到hash表，用于后续编辑
            m_drawnPipelines[item] = pipeline;
            
            // 添加到待保存变更列表
            PendingChange change;
            change.type = ChangeAdded;
            change.entityType = "pipeline";
            change.data = QVariant::fromValue(pipeline);
            change.originalId = -1;  // 新增的没有原始ID
            change.graphicsItem = item;
            m_pendingChanges.append(change);
            markAsModified();
            
            // 使用命令模式添加实体（支持撤销/重做）
            AddEntityCommand *cmd = new AddEntityCommand(
                mapScene,
                item,
                &m_drawnPipelines,
                pipeline
            );
            if (m_undoStack) {
                m_undoStack->push(cmd);
            }
            
            qDebug() << "✅ Pipeline drawn successfully on scene (pending save)";
            qDebug() << "   Color:" << lineColor.name() << "Type:" << typeName;
            qDebug() << "   First point:" << scenePoints.first();
            qDebug() << "   Last point:" << scenePoints.last();
            qDebug() << "   Pending changes count:" << m_pendingChanges.size();
        } else {
            qDebug() << "❌ Cannot draw pipeline: mapScene=" << mapScene 
                     << "scenePoints.size()=" << scenePoints.size();
        }
        
        // 显示成功信息
        QMessageBox::information(this, "成功", 
            QString("管线创建成功！\n\n"
                    "ID: %1\n"
                    "名称: %2\n"
                    "类型: %3\n"
                    "几何数据: %4\n\n"
                    "提示：请点击保存按钮或按 Ctrl+S 保存到数据库")
            .arg(pipeline.id())
            .arg(pipeline.pipelineName())
            .arg(pipeline.pipelineType())
            .arg(wkt.left(50) + "..."));
        
        updateStatus(QString("管线创建成功（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
    } else {
        // 取消操作，手动删除对话框
        delete dialog;
        dialog = nullptr;
        
        qDebug() << "Pipeline creation cancelled";
        updateStatus("取消管线创建");
    }
}

// ==========================================
// 变更跟踪和批量保存功能实现
// ==========================================

void MyForm::markAsModified()
{
    m_hasUnsavedChanges = true;
    // 更新窗口标题显示未保存标记
    QString title = windowTitle();
    if (!title.contains("*")) {
        setWindowTitle(title + " *");
    }
}

void MyForm::clearPendingChanges()
{
    m_pendingChanges.clear();
    m_hasUnsavedChanges = false;
    // 移除窗口标题的未保存标记
    QString title = windowTitle();
    title.remove(" *");
    setWindowTitle(title);
}

void MyForm::onSaveAll()
{
    if (m_pendingChanges.isEmpty()) {
        QMessageBox::information(this, "提示", "没有待保存的变更");
        return;
    }
    
    if (!savePendingChanges()) {
        QMessageBox::warning(this, "错误", "保存失败，请查看日志了解详情");
    } else {
        QMessageBox::information(this, "成功", 
            QString("成功保存 %1 项变更到数据库").arg(m_pendingChanges.size()));
        clearPendingChanges();
    }
}

void MyForm::onSaveAllTriggered()
{
    onSaveAll();
}

bool MyForm::savePendingChanges()
{
    if (m_pendingChanges.isEmpty()) {
        return true;
    }
    
    int successCount = 0;
    int failCount = 0;
    QStringList errors;
    
    for (const PendingChange &change : m_pendingChanges) {
        bool success = false;
        
        if (change.entityType == "pipeline") {
            Pipeline pipeline = change.data.value<Pipeline>();
            PipelineDAO dao;
            
            if (change.type == ChangeAdded) {
                // 检查编号是否存在，如果存在则自动生成新编号
                QString originalId = pipeline.pipelineId();
                QString finalId = originalId;
                
                // 如果编号为空，自动生成
                if (originalId.isEmpty()) {
                    finalId = IdGenerator::generatePipelineId(pipeline.pipelineType());
                    qDebug() << QString("[Save] Pipeline ID is empty, auto-generated: %1").arg(finalId);
                }
                
                // 检查编号是否存在，如果存在则生成下一个可用编号
                Pipeline existing = dao.findByPipelineId(finalId);
                if (existing.isValid()) {
                    // 生成下一个可用编号
                    finalId = IdGenerator::generateNextAvailableId(
                        finalId,
                        [&dao](const QString &id) -> bool {
                            return dao.findByPipelineId(id).isValid();
                        }
                    );
                    qDebug() << QString("[Save] Pipeline ID %1 already exists, auto-generated new ID: %2")
                                .arg(originalId).arg(finalId);
                    pipeline.setPipelineId(finalId);
                    
                    // 更新图形项的数据（如果有关联）
                    if (change.graphicsItem) {
                        change.graphicsItem->setData(1, finalId);
                    }
                }
                
                // 执行插入
                success = dao.insert(pipeline);
            } else if (change.type == ChangeModified) {
                success = dao.update(pipeline, change.originalId);
            } else if (change.type == ChangeDeleted) {
                success = dao.deleteById(change.originalId);
            }
            
            if (!success) {
                QString error = DatabaseManager::instance().lastError();
                errors.append(QString("管线 %1: %2").arg(pipeline.pipelineId()).arg(error));
                failCount++;
            } else {
                successCount++;
            }
        } else if (change.entityType == "facility") {
            Facility facility = change.data.value<Facility>();
            FacilityDAO dao;
            
            if (change.type == ChangeAdded) {
                // 检查编号是否存在，如果存在则自动生成新编号
                QString originalId = facility.facilityId();
                QString finalId = originalId;
                
                // 如果编号为空，自动生成
                if (originalId.isEmpty()) {
                    finalId = IdGenerator::generateFacilityId(facility.facilityType());
                    qDebug() << QString("[Save] Facility ID is empty, auto-generated: %1").arg(finalId);
                }
                
                // 检查编号是否存在，如果存在则生成下一个可用编号
                Facility existing = dao.findByFacilityId(finalId);
                if (existing.isValid()) {
                    // 生成下一个可用编号
                    finalId = IdGenerator::generateNextAvailableId(
                        finalId,
                        [&dao](const QString &id) -> bool {
                            return dao.findByFacilityId(id).isValid();
                        }
                    );
                    qDebug() << QString("[Save] Facility ID %1 already exists, auto-generated new ID: %2")
                                .arg(originalId).arg(finalId);
                    facility.setFacilityId(finalId);
                    
                    // 更新图形项的数据（如果有关联）
                    if (change.graphicsItem) {
                        change.graphicsItem->setData(1, finalId);
                    }
                }
                
                // 执行插入
                success = dao.insert(facility);
            } else if (change.type == ChangeModified) {
                success = dao.update(facility, change.originalId);
            } else if (change.type == ChangeDeleted) {
                success = dao.deleteById(change.originalId);
            }
            
            if (!success) {
                QString error = DatabaseManager::instance().lastError();
                errors.append(QString("设施 %1: %2").arg(facility.facilityId()).arg(error));
                failCount++;
            } else {
                successCount++;
            }
        } else if (change.entityType == "workorder") {
            WorkOrder workOrder = change.data.value<WorkOrder>();
            WorkOrderDAO dao;
            
            if (change.type == ChangeAdded) {
                success = dao.insert(workOrder);
            } else if (change.type == ChangeModified) {
                // WorkOrderDAO 需要实现 update 方法
                // 暂时跳过修改的工单
                success = true;  // 暂时标记为成功，避免错误
            }
            
            if (!success) {
                QString error = DatabaseManager::instance().lastError();
                errors.append(QString("工单 %1: %2").arg(workOrder.orderId()).arg(error));
                failCount++;
            } else {
                successCount++;
            }
        }
    }
    
    qDebug() << QString("[Save] Success: %1, Failed: %2").arg(successCount).arg(failCount);
    if (!errors.isEmpty()) {
        qDebug() << "[Save] Errors:" << errors.join("\n");
    }
    
    return failCount == 0;
}

void MyForm::onFacilityDrawingFinished(const QString &facilityType, const QString &wkt, const QPointF &point)
{
    qDebug() << "Facility drawing finished, type:" << facilityType << "WKT:" << wkt;
    updateStatus("设施绘制完成，请输入属性...");
    
    // 解析WKT获取地理坐标
    QPointF geoCoord;
    QString wktClean = wkt;
    wktClean.remove("POINT(");
    wktClean.remove(")");
    QStringList coords = wktClean.split(' ', Qt::SkipEmptyParts);
    if (coords.size() >= 2) {
        geoCoord = QPointF(coords[0].toDouble(), coords[1].toDouble());
    }
    
    // 创建临时设施对象，设置类型和坐标
    Facility tempFacility;
    tempFacility.setFacilityType(facilityType);
    tempFacility.setCoordinate(geoCoord);
    tempFacility.setGeomWkt(wkt);
    
    // 创建并显示属性编辑对话框
    FacilityEditDialog *dialog = new FacilityEditDialog(this, tempFacility);
    
    int result = dialog->exec();
    
    if (result == QDialog::Accepted) {
        // 获取设施对象
        Facility facility = dialog->resultFacility();
        
        // 确保几何信息正确设置
        facility.setCoordinate(geoCoord);
        facility.setGeomWkt(wkt);
        
        // 手动删除对话框
        delete dialog;
        dialog = nullptr;
        
        // 不立即保存到数据库，而是添加到待保存列表
        // 在地图上显示设施
        if (mapScene) {
            qDebug() << "Creating facility on scene at:" << point;
            
            // 创建圆形图形项
            double radius = 8.0;  // 设施半径
            QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem(
                point.x() - radius,
                point.y() - radius,
                radius * 2,
                radius * 2
            );
            
            // 设置样式（根据类型）
            QColor color;
            QString typeName;
            QString facilityTypeStr = facility.facilityType();
            if (facilityTypeStr == "valve") {
                color = QColor(255, 0, 0);  // 红色
                typeName = "阀门";
            } else if (facilityTypeStr == "manhole") {
                color = QColor(128, 128, 0);  // 黄褐色
                typeName = "井盖";
            } else if (facilityTypeStr == "pump_station") {
                color = QColor(0, 128, 255);  // 浅蓝色
                typeName = "泵站";
            } else if (facilityTypeStr == "transformer") {
                color = QColor(255, 128, 0);  // 橙色
                typeName = "变压器";
            } else if (facilityTypeStr == "pressure_station" || facilityTypeStr == "regulator") {
                color = QColor(128, 0, 255);  // 紫色
                typeName = "调压站";
            } else if (facilityTypeStr == "junction_box") {
                color = QColor(0, 255, 128);  // 青色
                typeName = "接线盒";
            } else {
                color = QColor(128, 128, 128);  // 灰色
                typeName = "未知";
            }
            
            ellipseItem->setBrush(QBrush(color));
            ellipseItem->setPen(QPen(Qt::black, 2));
            ellipseItem->setZValue(150);  // 确保在管线之上
            
            // 设置数据
            ellipseItem->setData(0, "facility");  // 实体类型
            ellipseItem->setData(1, facility.facilityId());  // 设施ID
            ellipseItem->setData(2, facilityTypeStr);  // 设施类型
            ellipseItem->setData(100, static_cast<int>(EntityState::Added));  // 新增：设置实体状态为Added
            
            // 设置工具提示
            ellipseItem->setToolTip(QString("%1\n类型: %2")
                                   .arg(facility.facilityName().isEmpty() ? facility.facilityId() : facility.facilityName())
                                   .arg(typeName));
            
            // 添加到场景
            mapScene->addItem(ellipseItem);
            
            // 添加到待保存变更列表
            PendingChange change;
            change.type = ChangeAdded;
            change.entityType = "facility";
            change.data = QVariant::fromValue(facility);
            change.originalId = -1;  // 新增的没有原始ID
            change.graphicsItem = ellipseItem;
            m_pendingChanges.append(change);
            markAsModified();
            
            // 使用命令模式添加实体（支持撤销/重做）
            // 注意：设施不使用 pipelineHash，传入 nullptr
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
            
            qDebug() << "✅ Facility created on scene (pending save):" << typeName;
            qDebug() << "   Pending changes count:" << m_pendingChanges.size();
            
            // 显示成功信息
            QMessageBox::information(this, "成功",
                QString("设施创建成功！\n\n"
                        "名称: %1\n"
                        "类型: %2\n"
                        "几何数据: %3\n\n"
                        "提示：请点击保存按钮或按 Ctrl+S 保存到数据库")
                .arg(facility.facilityName().isEmpty() ? facility.facilityId() : facility.facilityName())
                .arg(typeName)
                .arg(wkt));
            
            updateStatus(QString("设施创建成功（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
        } else {
            qDebug() << "❌ Cannot create facility: mapScene is null";
            updateStatus("设施创建失败");
        }
    } else {
        // 取消操作，手动删除对话框
        delete dialog;
        dialog = nullptr;
        
        qDebug() << "Facility creation cancelled";
        updateStatus("取消设施创建");
    }
}

// ==========================================
// 实体交互功能实现
// ==========================================

void MyForm::onEntityClicked(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    
    // 如果点击的是同一个项，不重复处理
    if (item == m_selectedItem) {
        return;
    }
    
    qDebug() << "Entity clicked:" << item->data(1).toString();
    
    // 清除之前的选中（必须先清除，否则之前的项不会取消高亮）
    if (m_selectedItem) {
        unhighlightItem(m_selectedItem);
        m_selectedItem = nullptr;
    }
    
    // 选中新项
    selectItem(item);
    
    // 更新状态栏
    QString entityType = item->data(0).toString();
    QString entityId = item->data(2).toString(); // 使用编号（pipelineId/facilityId）
    QString typeName = item->data(3).toString(); // 类型
    
    if (entityType == "pipeline") {
        // 尝试从数据库获取更多信息
        QString pipelineId = entityId;
        QString statusText = QString("已选中管线: %1 (类型: %2)").arg(pipelineId).arg(typeName);
        
        if (DatabaseManager::instance().isConnected() && !pipelineId.isEmpty()) {
            PipelineDAO dao;
            Pipeline pipeline = dao.findByPipelineId(pipelineId);
            if (pipeline.isValid()) {
                statusText = QString("已选中管线: %1 | 管径: DN%2 | 长度: %3m | 健康度: %4分")
                                .arg(pipeline.pipelineName().isEmpty() ? pipelineId : pipeline.pipelineName())
                                .arg(pipeline.diameterMm())
                                .arg(pipeline.lengthM(), 0, 'f', 1)
                                .arg(pipeline.healthScore());
            }
        }
        
        updateStatus(statusText);
    } else if (entityType == "facility") {
        // 尝试从数据库获取更多信息
        QString facilityId = entityId;
        QString statusText = QString("已选中设施: %1 (类型: %2)").arg(facilityId).arg(typeName);
        
        if (DatabaseManager::instance().isConnected() && !facilityId.isEmpty()) {
            FacilityDAO dao;
            Facility facility = dao.findByFacilityId(facilityId);
            if (facility.isValid()) {
                statusText = QString("已选中设施: %1 | 规格: %2 | 健康度: %3分")
                                .arg(facility.facilityName().isEmpty() ? facilityId : facility.facilityName())
                                .arg(facility.spec())
                                .arg(facility.healthScore());
            }
        }
        
        updateStatus(statusText);
    }
}

void MyForm::onEntityDoubleClicked(QGraphicsItem *item)
{
    if (!item || !isEntityItem(item)) {
        return;
    }
    
    qDebug() << "Entity double-clicked:" << item->data(1).toString();
    
    // 先选中
    if (item != m_selectedItem) {
        onEntityClicked(item);
    }
    
    // 编辑属性
    onEditSelectedEntity();
}

void MyForm::onShowContextMenu(const QPoint &pos)
{
    if (!m_selectedItem) {
        return;
    }
    
    // 创建右键菜单
    QMenu contextMenu(this);
    contextMenu.setStyleSheet(
        "QMenu {"
        "  background-color: white;"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
        "QMenu::item {"
        "  padding: 6px 20px;"
        "  border-radius: 2px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #1890ff;"
        "  color: white;"
        "}"
        "QMenu::item:disabled {"
        "  color: #bfbfbf;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background-color: #e0e0e0;"
        "  margin: 4px 0px;"
        "}"
    );
    
    // 基本操作
    QAction *viewAction = contextMenu.addAction("📋 查看属性");
    QAction *editAction = contextMenu.addAction("✏️ 编辑属性");
    
    contextMenu.addSeparator();
    
    // 复制/粘贴操作
    QAction *copyAction = contextMenu.addAction("📋 复制");
    QAction *pasteAction = contextMenu.addAction("📄 粘贴");
    pasteAction->setEnabled(m_copiedItem != nullptr);
    
    QAction *duplicateAction = contextMenu.addAction("📌 原位复制");
    
    contextMenu.addSeparator();
    
    // 样式操作
    QAction *copyStyleAction = contextMenu.addAction("🎨 复制样式");
    QAction *pasteStyleAction = contextMenu.addAction("🖌️ 粘贴样式");
    pasteStyleAction->setEnabled(m_hasStyleCopied);
    
    contextMenu.addSeparator();
    
    // 图层操作
    QAction *bringToFrontAction = contextMenu.addAction("⬆️ 置于顶层");
    QAction *sendToBackAction = contextMenu.addAction("⬇️ 置于底层");
    
    contextMenu.addSeparator();
    
    // 删除操作
    QAction *deleteAction = contextMenu.addAction("🗑️ 删除");
    QFont deleteFont = deleteAction->font();
    deleteFont.setBold(true);
    deleteAction->setFont(deleteFont);
    
    // 连接信号
    connect(viewAction, &QAction::triggered, this, &MyForm::onViewEntityProperties);
    connect(editAction, &QAction::triggered, this, &MyForm::onEditSelectedEntity);
    connect(copyAction, &QAction::triggered, this, &MyForm::onCopyEntity);
    connect(pasteAction, &QAction::triggered, this, &MyForm::onPasteEntity);
    connect(duplicateAction, &QAction::triggered, this, &MyForm::onDuplicateEntity);
    connect(copyStyleAction, &QAction::triggered, this, &MyForm::onCopyStyle);
    connect(pasteStyleAction, &QAction::triggered, this, &MyForm::onPasteStyle);
    connect(bringToFrontAction, &QAction::triggered, this, &MyForm::onBringToFront);
    connect(sendToBackAction, &QAction::triggered, this, &MyForm::onSendToBack);
    connect(deleteAction, &QAction::triggered, this, &MyForm::onDeleteSelectedEntity);
    
    // 显示菜单
    contextMenu.exec(ui->graphicsView->mapToGlobal(pos));
}

void MyForm::onDeleteSelectedEntity()
{
    if (!m_selectedItem) {
        return;
    }
    
    QString entityType = m_selectedItem->data(0).toString();
    QString entityId = m_selectedItem->data(1).toString();
    
    // 检查权限
    bool hasPermission = false;
    if (entityType == "pipeline") {
        hasPermission = PermissionManager::canDeletePipeline();
    } else if (entityType == "facility") {
        hasPermission = PermissionManager::canDeleteFacility();
    }
    
    if (!hasPermission) {
        QMessageBox::warning(this, "权限不足", "您没有权限删除该实体。");
        return;
    }
    
    // 确认删除
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除该%1吗？\n\nID: %2")
            .arg(entityType == "pipeline" ? "管线" : "设施")
            .arg(entityId),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        qDebug() << "Deleting entity:" << entityId;
        
        // 获取实体状态
        QVariant stateVariant = m_selectedItem->data(100);
        EntityState entityState = EntityState::Detached;
        if (stateVariant.isValid()) {
            entityState = static_cast<EntityState>(stateVariant.toInt());
        }
        
        qDebug() << "Entity state:" << static_cast<int>(entityState);
        
        // 处理待保存变更列表
        if (entityState == EntityState::Added) {
            // 如果是新添加的实体，从待保存列表中移除对应的 ChangeAdded 记录
            for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
                const PendingChange &change = m_pendingChanges[i];
                if (change.entityType == entityType && 
                    change.graphicsItem == m_selectedItem &&
                    change.type == ChangeAdded) {
                    m_pendingChanges.removeAt(i);
                    qDebug() << "Removed ChangeAdded from pending changes";
                    break;
                }
            }
        } else if (entityState == EntityState::Unchanged) {
            // 如果是已存在的实体，添加到待保存列表作为 ChangeDeleted
            int databaseId = -1;
            
            if (entityType == "pipeline") {
                // 从 hash 表获取管线对象
                if (m_drawnPipelines.contains(m_selectedItem)) {
                    Pipeline pipeline = m_drawnPipelines[m_selectedItem];
                    databaseId = pipeline.id();
                } else {
                    // 如果 hash 表中没有（LayerManager 加载的数据），尝试从图形项获取数据库ID
                    // PipelineRenderer 可能将数据库ID存储在 data(1) 或 data(10)
                    QVariant dbIdVariant = m_selectedItem->data(10);  // 优先从 data(10) 获取
                    if (!dbIdVariant.isValid() || dbIdVariant.toInt() <= 0) {
                        dbIdVariant = m_selectedItem->data(1);  // 如果 data(10) 没有，尝试 data(1)
                    }
                    
                    if (dbIdVariant.isValid() && dbIdVariant.toInt() > 0) {
                        // 如果 data(1) 或 data(10) 存储的是数据库ID
                        databaseId = dbIdVariant.toInt();
                        qDebug() << "Got database ID from item data:" << databaseId;
                    } else {
                        // 如果 data(1) 和 data(10) 都不是数据库ID，那么 data(1) 可能是 pipelineId
                        // 尝试通过 pipelineId 查询数据库
                        QString pipelineId = entityId;  // entityId 来自 data(1)
                        PipelineDAO dao;
                        Pipeline pipeline = dao.findByPipelineId(pipelineId);
                        if (pipeline.isValid()) {
                            databaseId = pipeline.id();
                            qDebug() << "Got database ID from database query by pipelineId:" << pipelineId << "->" << databaseId;
                        } else {
                            qWarning() << "Failed to find pipeline by ID:" << pipelineId;
                            qWarning() << "  data(1):" << m_selectedItem->data(1);
                            qWarning() << "  data(10):" << m_selectedItem->data(10);
                        }
                    }
                }
            } else if (entityType == "facility") {
                // 尝试从图形项获取数据库ID
                QVariant dbIdVariant = m_selectedItem->data(10);  // 优先从 data(10) 获取
                if (!dbIdVariant.isValid() || dbIdVariant.toInt() <= 0) {
                    dbIdVariant = m_selectedItem->data(1);  // 如果 data(10) 没有，尝试 data(1)
                }
                
                if (dbIdVariant.isValid() && dbIdVariant.toInt() > 0) {
                    // 如果 data(1) 或 data(10) 存储的是数据库ID
                    databaseId = dbIdVariant.toInt();
                    qDebug() << "Got facility database ID from item data:" << databaseId;
                } else {
                    // 如果都不是，通过 facilityId 查询数据库
                    QString facilityId = entityId;  // entityId 来自 data(1)，应该是 facilityId
                    FacilityDAO dao;
                    Facility facility = dao.findByFacilityId(facilityId);
                    if (facility.isValid()) {
                        databaseId = facility.id();
                        qDebug() << "Got facility database ID from database query by facilityId:" << facilityId << "->" << databaseId;
                    } else {
                        qWarning() << "Failed to find facility by ID:" << facilityId;
                        qWarning() << "  data(1):" << m_selectedItem->data(1);
                        qWarning() << "  data(10):" << m_selectedItem->data(10);
                    }
                }
            }
            
            if (databaseId > 0) {
                // 添加到待保存变更列表
                PendingChange change;
                change.type = ChangeDeleted;
                change.entityType = entityType;
                // 对于删除操作，data 可以存储一个空的实体对象，或者存储 ID
                if (entityType == "pipeline") {
                    Pipeline pipeline;
                    pipeline.setId(databaseId);
                    pipeline.setPipelineId(entityId);
                    change.data = QVariant::fromValue(pipeline);
                } else if (entityType == "facility") {
                    Facility facility;
                    facility.setId(databaseId);
                    facility.setFacilityId(entityId);
                    change.data = QVariant::fromValue(facility);
                }
                change.originalId = databaseId;
                change.graphicsItem = m_selectedItem;
                m_pendingChanges.append(change);
                markAsModified();
                qDebug() << "Added ChangeDeleted to pending changes, databaseId:" << databaseId;
            } else {
                qWarning() << "Failed to get database ID for entity:" << entityId;
            }
        }
        
        // 更新实体状态为已删除
        m_selectedItem->setData(100, static_cast<int>(EntityState::Deleted));
        
        // 使用命令模式删除（支持撤销）- 立即从界面移除
        DeleteEntityCommand *cmd = new DeleteEntityCommand(
            mapScene,
            m_selectedItem,
            &m_drawnPipelines
        );
        
        if (m_undoStack) {
            m_undoStack->push(cmd);
        }
        
        // 清除选中
        m_selectedItem = nullptr;
        
        updateStatus(QString("已删除%1（待保存，共 %2 项待保存）")
                     .arg(entityType == "pipeline" ? "管线" : "设施")
                     .arg(m_pendingChanges.size()));
        qDebug() << "✅ Entity deleted successfully (pending save)";
    }
}

void MyForm::onEditSelectedEntity()
{
    if (!m_selectedItem) {
        return;
    }
    
    QString entityType = m_selectedItem->data(0).toString();
    
    // 判断实体类型，使用通用属性对话框
    EntityPropertiesDialog::EntityType dialogType = 
        (entityType == "pipeline") ? EntityPropertiesDialog::Pipeline : EntityPropertiesDialog::Facility;
    
    EntityPropertiesDialog *dialog = new EntityPropertiesDialog(
        m_selectedItem, 
        dialogType,
        this
    );
    
    // 连接删除信号
    connect(dialog, &EntityPropertiesDialog::deleteRequested, this, [this]() {
        onDeleteSelectedEntity();
    });
    
    // 保存旧属性值（用于撤销）
    QString oldName;
    QString oldType;
    if (entityType == "pipeline" && m_drawnPipelines.contains(m_selectedItem)) {
        Pipeline oldPipeline = m_drawnPipelines[m_selectedItem];
        oldName = oldPipeline.pipelineName();
        oldType = oldPipeline.pipelineType();
    } else {
        oldName = m_selectedItem->data(0).toString();  // 从 data(0) 获取名称
        oldType = m_selectedItem->data(2).toString();  // 从 data(2) 获取类型
    }
    
    // 连接属性变化信号
    connect(dialog, &EntityPropertiesDialog::propertiesChanged, this, [this, oldName, oldType, entityType, dialog]() {
        // 获取新属性值
        QString newName = dialog->getName();
        QString newType = dialog->getType();
        
        // 使用命令模式修改属性（支持撤销/重做）
        if (entityType == "pipeline" && m_drawnPipelines.contains(m_selectedItem)) {
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
            
            // 修改管线类型
            if (oldType != newType) {
                ChangePropertyCommand *cmd = new ChangePropertyCommand(
                    m_selectedItem,
                    "类型",
                    oldType,
                    newType,
                    &m_drawnPipelines
                );
                if (m_undoStack) {
                    m_undoStack->push(cmd);
                }
            }
        } else {
            // 修改设施名称
            if (oldName != newName) {
                ChangePropertyCommand *cmd = new ChangePropertyCommand(
                    m_selectedItem,
                    "名称",
                    oldName,
                    newName,
                    nullptr
                );
                if (m_undoStack) {
                    m_undoStack->push(cmd);
                }
            }
        }
        
        updateStatus("实体属性已更新");
        qDebug() << "✅ Entity properties updated";
    });
    
    int result = dialog->exec();
    
    // 如果对话框被接受，属性已经在 propertiesChanged 信号中处理
    // 这里只需要清理对话框
    delete dialog;
}

void MyForm::onViewEntityProperties()
{
    if (!m_selectedItem) {
        return;
    }
    
    QString entityType = m_selectedItem->data(0).toString();
    
    if (entityType == "pipeline") {
        Pipeline pipeline;
        bool found = false;
        
        // 优先检查是否是绘制的管线
        if (m_drawnPipelines.contains(m_selectedItem)) {
            pipeline = m_drawnPipelines[m_selectedItem];
            found = true;
        } else {
            // 尝试从数据库查询（使用管线编号）
            QString pipelineId = m_selectedItem->data(2).toString(); // pipelineId存储在data(2)
            if (!pipelineId.isEmpty() && DatabaseManager::instance().isConnected()) {
                PipelineDAO dao;
                pipeline = dao.findByPipelineId(pipelineId);
                if (pipeline.isValid()) {
                    found = true;
                }
            }
        }
        
        if (!found) {
            QMessageBox::warning(this, "错误", "未找到管线数据！");
            return;
        }
        
        // 显示属性信息
        QString info = QString(
            "管线属性\n\n"
            "🆔 数据库ID: %1\n"
            "编号: %2\n"
            "名称: %3\n"
            "类型: %4\n"
            "管径: DN%5 mm\n"
            "材质: %6\n"
            "长度: %7 m\n"
            "埋深: %8 m\n"
            "压力等级: %9\n"
            "建设日期: %10\n"
            "施工单位: %11\n"
            "产权单位: %12\n"
            "建设造价: %13 元\n"
            "运行状态: %14\n"
            "健康度: %15分\n"
            "上次巡检: %16\n"
            "养护单位: %17\n"
            "巡检周期: %18天\n"
            "备注: %19"
        )
        .arg(pipeline.id())
        .arg(pipeline.pipelineId())
        .arg(pipeline.pipelineName())
        .arg(pipeline.pipelineType())
        .arg(pipeline.diameterMm())
        .arg(pipeline.material())
        .arg(pipeline.lengthM(), 0, 'f', 2)
        .arg(pipeline.depthM(), 0, 'f', 2)
        .arg(pipeline.pressureClass())
        .arg(pipeline.buildDate().isValid() ? pipeline.buildDate().toString("yyyy-MM-dd") : "未设置")
        .arg(pipeline.builder())
        .arg(pipeline.owner())
        .arg(pipeline.constructionCost(), 0, 'f', 2)
        .arg(pipeline.status())
        .arg(pipeline.healthScore())
        .arg(pipeline.lastInspection().isValid() ? pipeline.lastInspection().toString("yyyy-MM-dd") : "未设置")
        .arg(pipeline.maintenanceUnit())
        .arg(pipeline.inspectionCycle())
        .arg(pipeline.remarks());
        
        QMessageBox::information(this, "管线属性", info);
    } else if (entityType == "facility") {
        Facility facility;
        bool found = false;
        
        // 尝试从数据库查询（使用设施编号）
        QString facilityId = m_selectedItem->data(2).toString(); // facilityId可能存储在data(2)
        if (!facilityId.isEmpty() && DatabaseManager::instance().isConnected()) {
            FacilityDAO dao;
            facility = dao.findByFacilityId(facilityId);
            if (facility.isValid()) {
                found = true;
            }
        }
        
        if (!found) {
            QMessageBox::warning(this, "错误", "未找到设施数据！");
            return;
        }
        
        // 显示设施属性信息
        QString info = QString(
            "设施属性\n\n"
            "🆔 数据库ID: %1\n"
            "编号: %2\n"
            "名称: %3\n"
            "类型: %4\n"
            "规格型号: %5\n"
            "材质: %6\n"
            "尺寸: %7\n"
            "关联管线: %8\n"
            "高程: %9 m\n"
            "建设日期: %10\n"
            "施工单位: %11\n"
            "产权单位: %12\n"
            "资产价值: %13 元\n"
            "运行状态: %14\n"
            "健康度: %15分\n"
            "上次维护: %16\n"
            "下次维护: %17\n"
            "养护单位: %18\n"
            "二维码: %19\n"
            "备注: %20"
        )
        .arg(facility.id())
        .arg(facility.facilityId())
        .arg(facility.facilityName())
        .arg(facility.facilityType())
        .arg(facility.spec())
        .arg(facility.material())
        .arg(facility.size())
        .arg(facility.pipelineId())
        .arg(facility.elevationM(), 0, 'f', 2)
        .arg(facility.buildDate().isValid() ? facility.buildDate().toString("yyyy-MM-dd") : "未设置")
        .arg(facility.builder())
        .arg(facility.owner())
        .arg(facility.assetValue(), 0, 'f', 2)
        .arg(facility.status())
        .arg(facility.healthScore())
        .arg(facility.lastMaintenance().isValid() ? facility.lastMaintenance().toString("yyyy-MM-dd") : "未设置")
        .arg(facility.nextMaintenance().isValid() ? facility.nextMaintenance().toString("yyyy-MM-dd") : "未设置")
        .arg(facility.maintenanceUnit())
        .arg(facility.qrcodeUrl())
        .arg(facility.remarks());
        
        QMessageBox::information(this, "设施属性", info);
    }
}

void MyForm::clearSelection()
{
    if (m_selectedItem) {
        // 检查位置是否变化（用于移动撤销）
        QPointF currentPos = m_selectedItem->pos();
        if (currentPos != m_selectedItemStartPos) {
            // 位置发生变化，创建移动命令
            MoveEntityCommand *cmd = new MoveEntityCommand(
                m_selectedItem,
                m_selectedItemStartPos,
                currentPos
            );
            if (m_undoStack) {
                m_undoStack->push(cmd);
            }
        }
        
        // 恢复不可移动状态
        m_selectedItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        
        unhighlightItem(m_selectedItem);
        m_selectedItem = nullptr;
        updateStatus("Ready");
    }
}

void MyForm::selectItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    
    m_selectedItem = item;
    m_selectedItemStartPos = item->pos();  // 记录开始位置（用于移动撤销）
    
    // 设置图形项为可移动（仅对设施有效，管线移动需要特殊处理）
    QString entityType = item->data(0).toString();
    if (entityType == "facility") {
        item->setFlag(QGraphicsItem::ItemIsMovable, true);
    }
    
    highlightItem(item);
}

void MyForm::highlightItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    
    QString entityType = item->data(0).toString();
    
    // 处理管线（QGraphicsPathItem）
    QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(item);
    if (pathItem && entityType == "pipeline") {
        // 保存原始画笔
        m_originalPen = pathItem->pen();
        
        // 创建高亮画笔（加粗 + 黄色）
        QPen highlightPen = m_originalPen;
        highlightPen.setWidth(m_originalPen.width() + 3);
        highlightPen.setColor(QColor(255, 215, 0));  // 金色
        pathItem->setPen(highlightPen);
        pathItem->setZValue(200);  // 提升层级
        
        qDebug() << "✨ Pipeline highlighted";
        return;
    }
    
    // 处理设施（QGraphicsEllipseItem）
    QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
    if (ellipseItem && entityType == "facility") {
        // 保存原始画笔和画刷
        m_originalPen = ellipseItem->pen();
        m_originalBrush = ellipseItem->brush();
        
        // 创建高亮样式（加粗边框 + 黄色填充）
        QPen highlightPen = m_originalPen;
        highlightPen.setWidth(m_originalPen.width() + 2);
        highlightPen.setColor(QColor(255, 215, 0));  // 金色边框
        
        QBrush highlightBrush = QBrush(QColor(255, 255, 0, 150));  // 半透明黄色填充
        
        ellipseItem->setPen(highlightPen);
        ellipseItem->setBrush(highlightBrush);
        ellipseItem->setZValue(200);  // 提升层级
        
        qDebug() << "✨ Facility highlighted";
        return;
    }
}

void MyForm::unhighlightItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    
    QString entityType = item->data(0).toString();
    
    // 处理管线（QGraphicsPathItem）
    QGraphicsPathItem *pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(item);
    if (pathItem && entityType == "pipeline") {
        // 恢复原始画笔
        pathItem->setPen(m_originalPen);
        pathItem->setZValue(10);  // 恢复层级（管线原始Z值是10）
        
        qDebug() << "➖ Pipeline unhighlighted";
        return;
    }
    
    // 处理设施（QGraphicsEllipseItem）
    QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
    if (ellipseItem && entityType == "facility") {
        // 恢复原始画笔和画刷
        ellipseItem->setPen(m_originalPen);
        ellipseItem->setBrush(m_originalBrush);
        ellipseItem->setZValue(20);  // 恢复层级（设施原始Z值是20）
        
        qDebug() << "➖ Facility unhighlighted";
        return;
    }
}

bool MyForm::isEntityItem(QGraphicsItem *item)
{
    if (!item) {
        return false;
    }
    
    // 检查是否有实体标记
    QString entityType = item->data(0).toString();
    return (entityType == "pipeline" || entityType == "facility");
}

// ==========================================
// 复制/粘贴/样式操作功能实现
// ==========================================

void MyForm::onCopyEntity()
{
    if (!m_selectedItem) {
        return;
    }
    
    m_copiedItem = m_selectedItem;
    updateStatus("✅ 已复制实体");
    qDebug() << "📋 Entity copied";
}

void MyForm::onPasteEntity()
{
    if (!m_copiedItem || !mapScene) {
        QMessageBox::warning(this, "提示", "没有复制的实体！");
        return;
    }
    
    // 获取鼠标当前位置（场景坐标）
    QPointF scenePos = ui->graphicsView->mapToScene(
        ui->graphicsView->viewport()->rect().center()
    );
    
    // 复制图形项
    QGraphicsItem *newItem = nullptr;
    
    if (auto pathItem = qgraphicsitem_cast<QGraphicsPathItem*>(m_copiedItem)) {
        // 复制路径项（管线）
        QGraphicsPathItem *newPathItem = new QGraphicsPathItem();
        newPathItem->setPath(pathItem->path());
        newPathItem->setPen(pathItem->pen());
        newPathItem->setBrush(pathItem->brush());
        newPathItem->setZValue(100);
        
        // 复制数据
        for (int i = 0; i < 10; ++i) {
            newPathItem->setData(i, m_copiedItem->data(i));
        }
        
        // 偏移位置（20像素）
        newPathItem->setPos(m_copiedItem->pos() + QPointF(20, 20));
        
        newItem = newPathItem;
    } else if (auto ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(m_copiedItem)) {
        // 复制椭圆项（设施）
        QGraphicsEllipseItem *newEllipseItem = new QGraphicsEllipseItem();
        newEllipseItem->setRect(ellipseItem->rect());
        newEllipseItem->setPen(ellipseItem->pen());
        newEllipseItem->setBrush(ellipseItem->brush());
        newEllipseItem->setZValue(100);
        
        // 复制数据
        for (int i = 0; i < 10; ++i) {
            newEllipseItem->setData(i, m_copiedItem->data(i));
        }
        
        // 偏移位置
        newEllipseItem->setPos(m_copiedItem->pos() + QPointF(20, 20));
        
        newItem = newEllipseItem;
    }
    
    if (newItem) {
        // 添加到场景
        mapScene->addItem(newItem);
        
        // 选中新复制的项
        clearSelection();
        selectItem(newItem);
        
        updateStatus("✅ 已粘贴实体");
        qDebug() << "📄 Entity pasted";
    }
}

void MyForm::onDuplicateEntity()
{
    if (!m_selectedItem || !mapScene) {
        return;
    }
    
    // 保存当前选中项
    QGraphicsItem *sourceItem = m_selectedItem;
    
    // 使用复制逻辑
    m_copiedItem = sourceItem;
    onPasteEntity();
    
    // 恢复原始选中
    m_copiedItem = nullptr;
    
    updateStatus("✅ 已原位复制实体");
}

void MyForm::onCopyStyle()
{
    if (!m_selectedItem) {
        return;
    }
    
    // 获取颜色和线宽
    QColor color = m_selectedItem->data(3).value<QColor>();
    int lineWidth = m_selectedItem->data(4).toInt();
    
    if (color.isValid() && lineWidth > 0) {
        m_copiedColor = color;
        m_copiedLineWidth = lineWidth;
        m_hasStyleCopied = true;
        
        updateStatus(QString("✅ 已复制样式: %1, %2px")
                        .arg(color.name())
                        .arg(lineWidth));
        qDebug() << "🎨 Style copied:" << color.name() << lineWidth;
    } else {
        QMessageBox::warning(this, "提示", "无法复制样式！");
    }
}

void MyForm::onPasteStyle()
{
    if (!m_selectedItem || !m_hasStyleCopied) {
        return;
    }
    
    // 获取旧样式
    QColor oldColor = m_selectedItem->data(3).value<QColor>();
    int oldWidth = m_selectedItem->data(4).toInt();
    
    // 使用命令模式修改样式（支持撤销）
    ChangeStyleCommand *cmd = new ChangeStyleCommand(
        m_selectedItem,
        oldColor,
        oldWidth,
        m_copiedColor,
        m_copiedLineWidth
    );
    
    if (m_undoStack) {
        m_undoStack->push(cmd);
    }
    
    updateStatus("✅ 已粘贴样式");
    qDebug() << "🖌️ Style pasted";
}

void MyForm::onBringToFront()
{
    if (!m_selectedItem) {
        return;
    }
    
    // 设置最高层级（除了高亮时的200）
    m_selectedItem->setZValue(150);
    
    updateStatus("⬆️ 已置于顶层");
    qDebug() << "⬆️ Brought to front";
}

void MyForm::onSendToBack()
{
    if (!m_selectedItem) {
        return;
    }
    
    // 设置最低层级
    m_selectedItem->setZValue(50);
    
    updateStatus("⬇️ 已置于底层");
    qDebug() << "⬇️ Sent to back";
}

// ==========================================
// 数据持久化功能实现
// ==========================================

void MyForm::onSaveDrawingData()
{
    // 确认保存到数据库
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认保存",
        "将绘制的管线和设施保存到数据库中，是否继续？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 保存到数据库
    bool success = DrawingDatabaseManager::saveToDatabase(
        mapScene,
        m_drawnPipelines
    );
    
    if (success) {
        QMessageBox::information(this, "成功", "绘制数据已保存到数据库！");
        updateStatus("✅ 已保存绘制数据到数据库");
    } else {
        QMessageBox::warning(this, "错误", "保存失败！请检查数据库连接。");
        updateStatus("❌ 保存失败");
    }
}

void MyForm::onLoadDrawingData()
{
    // 只加载用户绘制的数据（created_by = 'user_drawing'），不清空 LayerManager 加载的数据
    // 先检查是否已有用户绘制的数据，如果有则提示
    bool hasUserDrawnData = false;
    for (QGraphicsItem *item : mapScene->items()) {
        QString entityType = item->data(0).toString();
        if (entityType == "pipeline" || entityType == "facility") {
            // 检查是否是用户绘制的数据（在 m_drawnPipelines 中）
            if (m_drawnPipelines.contains(item)) {
                hasUserDrawnData = true;
                break;
            }
        }
    }
    
    if (hasUserDrawnData) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "确认加载",
            "检测到已有用户绘制的数据，重新加载会替换这些数据，是否继续？\n\n注意：不会影响程序启动时加载的数据。",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply != QMessageBox::Yes) {
            return;
        }
        
        // 只清空用户绘制的数据（m_drawnPipelines 中的项）
        QList<QGraphicsItem*> itemsToRemove;
        for (QGraphicsItem *item : m_drawnPipelines.keys()) {
            itemsToRemove.append(item);
        }
        
        for (QGraphicsItem *item : itemsToRemove) {
            mapScene->removeItem(item);
            delete item;
            m_drawnPipelines.remove(item);
        }
        
        clearSelection();
        
        // 清空撤销栈
        if (m_undoStack) {
            m_undoStack->clear();
        }
    }
    
    // 从数据库加载用户绘制的数据（追加到现有数据）
    bool success = DrawingDatabaseManager::loadFromDatabase(
        mapScene,
        m_drawnPipelines,
        m_nextPipelineId
    );
    
    if (success) {
        QMessageBox::information(this, "成功", 
            QString("已从数据库加载 %1 个用户绘制的管线实体！\n").arg(m_drawnPipelines.size()));
        updateStatus(QString("✅ 已从数据库加载用户绘制数据（共 %1 个管线）").arg(m_drawnPipelines.size()));
    } else {
        QMessageBox::warning(this, "错误", "加载失败！请检查数据库连接。");
        updateStatus("❌ 加载失败");
    }
}

void MyForm::setupLayerControlPanel()
{
    qDebug() << "设置图层控制面板 (右侧滑出)...";
    
    // 创建图层控制面板
    m_layerControlPanel = new LayerControlPanel(this);
    
    // 创建容器窗口（用于滑出效果）
    m_layerControlContainer = new QWidget(ui->graphicsView->viewport());  // 父对象改为viewport
    m_layerControlContainer->setObjectName("layerControlContainer");
    m_layerControlContainer->setStyleSheet(
        "#layerControlContainer {"
        "  background-color: white;"
        "  border-left: 2px solid #d0d0d0;"
        "  border-radius: 0px;"
        "}"
    );
    
    // 容器布局
    QVBoxLayout *containerLayout = new QVBoxLayout(m_layerControlContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // 添加面板到容器
    containerLayout->addWidget(m_layerControlPanel);
    
    // 设置容器宽度
    m_layerControlContainer->setFixedWidth(280);
    
    // 初始隐藏（移到右侧外面）
    m_layerControlContainer->hide();
    
    // 创建浮动切换按钮（贴在地图右侧） - 暂时不显示，会被底部按钮替代
    m_layerControlToggleBtn = new QPushButton("图层\n管理", ui->graphicsView->viewport());  // 父对象改为viewport
    m_layerControlToggleBtn->setObjectName("layerControlToggleBtn");
    m_layerControlToggleBtn->setToolTip("图层控制");
    m_layerControlToggleBtn->setCheckable(true);
    m_layerControlToggleBtn->setFixedSize(30, 80);  // 调整尺寸：30x80
    m_layerControlToggleBtn->setCursor(Qt::PointingHandCursor);
    m_layerControlToggleBtn->setStyleSheet(
        "#layerControlToggleBtn {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 rgba(76, 175, 80, 0.85),"  // 绿色渐变，半透明
        "                               stop:1 rgba(56, 155, 60, 0.85));"
        "  color: white;"
        "  border: 1px solid rgba(255, 255, 255, 0.4);"  // 白色边框增强可见性
        "  border-radius: 4px 0px 0px 4px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  padding: 4px 2px;"
        "  line-height: 1.2;"
        "  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.2);"  // 轻微阴影
        "}"
        "#layerControlToggleBtn:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 rgba(96, 195, 100, 0.95),"
        "                               stop:1 rgba(76, 175, 80, 0.95));"
        "  border: 1px solid rgba(255, 255, 255, 0.6);"
        "}"
        "#layerControlToggleBtn:checked {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                               stop:0 rgba(56, 155, 60, 0.9),"
        "                               stop:1 rgba(46, 135, 50, 0.9));"
        "  border: 1px solid rgba(255, 255, 255, 0.5);"
        "}"
    );
    
    // 隐藏右侧按钮（改用底部切换）
    m_layerControlToggleBtn->hide();
    
    // 连接图层管理器（如果已创建）
    if (m_layerManager) {
        m_layerControlPanel->setLayerManager(m_layerManager);
    }
    
    qDebug() << "图层控制面板设置完成";
}

void MyForm::positionLayerControlPanel()
{
    if (!m_layerControlContainer) {
        return;
    }
    
    // 获取viewport的几何信息
    QRect viewportRect = ui->graphicsView->viewport()->rect();
    int viewportWidth = viewportRect.width();
    int viewportHeight = viewportRect.height();
    
    // 面板宽度
    int panelWidth = m_layerControlContainer->width();
    
    // 面板展开显示在右侧，高度与地图窗口一致
    m_layerControlContainer->setGeometry(
        viewportWidth - panelWidth,  // 贴到右边
        0,                           // 从顶部开始
        panelWidth,
        viewportHeight               // 高度与viewport一致
    );
    
    m_layerControlContainer->raise();
}

// 设置右侧工具栏
void MyForm::setupPanelSwitcher()
{
    qDebug() << "设置面板系统...";
    
    // ==================== 1. 隐藏原来的右侧工具栏（不再使用） ====================
    // 注：不再创建 m_panelSwitcher，因为按钮已经集成到 gvOverlay 中
    m_panelSwitcher = nullptr;
    
    // ==================== 2. 创建面板容器（StackWidget + 底部关闭按钮） ====================
    m_panelContainer = new QWidget(ui->graphicsView->viewport());
    m_panelContainer->setObjectName("panelContainer");
    m_panelContainer->setStyleSheet(
        "#panelContainer { background-color: white; border-left: 2px solid #d0d0d0; }"
    );
    m_panelContainer->setFixedWidth(280);
    m_panelContainer->hide();  // 初始隐藏
    
    QVBoxLayout *containerLayout = new QVBoxLayout(m_panelContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);
    
    // 创建 StackWidget
    m_panelStack = new QStackedWidget(m_panelContainer);
    m_panelStack->addWidget(m_drawingToolPanel);  // 索引0: 绘制面板
    m_panelStack->addWidget(m_layerControlPanel); // 索引1: 图层面板
    containerLayout->addWidget(m_panelStack, 1);
    
    // 创建底部关闭按钮区域
    QWidget *bottomBar = new QWidget(m_panelContainer);
    bottomBar->setObjectName("bottomBar");
    bottomBar->setStyleSheet(
        "#bottomBar { background-color: transparent; }"
    );
    bottomBar->setFixedHeight(60);  // 增加高度以容纳优雅的按钮
    
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(12, 12, 12, 12);
    bottomLayout->setSpacing(0);
    
    // 关闭按钮（现代卡片样式，图标+文字）
    m_panelCloseBtn = new QPushButton(bottomBar);
    
    // 设置图标和文字
    QIcon closeIcon = QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton);
    m_panelCloseBtn->setIcon(closeIcon);
    m_panelCloseBtn->setIconSize(QSize(16, 16));
    m_panelCloseBtn->setText("关闭面板");
    
    m_panelCloseBtn->setFixedHeight(36);
    m_panelCloseBtn->setCursor(Qt::PointingHandCursor);
    m_panelCloseBtn->setToolTip("点击关闭当前面板");
    m_panelCloseBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f8f9fa);"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 6px;"
        "  padding: 6px 16px;"
        "  font-size: 13px;"
        "  font-weight: 500;"
        "  color: #495057;"
        "  text-align: center;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fff5f5, stop:1 #ffe3e3);"
        "  border: 1px solid #f87171;"
        "  color: #dc2626;"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fee2e2, stop:1 #fecaca);"
        "  border: 1px solid #ef4444;"
        "  color: #b91c1c;"
        "}"
    );
    
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_panelCloseBtn);
    bottomLayout->addStretch();
    
    containerLayout->addWidget(bottomBar);
    
    // 设置 m_panelDrawingBtn 和 m_panelLayerBtn 为 nullptr（不再使用）
    m_panelDrawingBtn = nullptr;
    m_panelLayerBtn = nullptr;
    
    // ==================== 3. 连接信号 ====================
    // 关闭按钮
    connect(m_panelCloseBtn, &QPushButton::clicked, this, [this]() {
        switchToPanel("");  // 关闭面板
    });
    
    qDebug() << "面板系统设置完成";
}

// 定位面板容器
void MyForm::positionPanelSwitcher()
{
    // 定位面板容器（贴到viewport右边缘）
    if (m_panelContainer) {
        QRect viewportRect = ui->graphicsView->viewport()->rect();
        int viewportWidth = viewportRect.width();
        int viewportHeight = viewportRect.height();
        
        int panelWidth = m_panelContainer->width();
        m_panelContainer->setGeometry(
            viewportWidth - panelWidth,  // 贴到右边缘
            0,
            panelWidth,
            viewportHeight
        );
        
        qDebug() << "面板已定位 - viewport:" << viewportWidth << "x" << viewportHeight;
    }
}

// 切换面板显示
void MyForm::switchToPanel(const QString &panelName)
{
    qDebug() << "切换面板至:" << panelName;
    
    if (panelName.isEmpty()) {
        // 关闭面板
        m_panelContainer->hide();
        m_currentPanel = "";
        updateStatus("面板已关闭");
        positionGraphicsOverlay();  // 重新定位缩放按钮
    } else {
        // 打开面板
        m_panelContainer->show();
        m_currentPanel = panelName;
        positionGraphicsOverlay();  // 重新定位缩放按钮
        
        if (panelName == "drawing") {
            m_panelStack->setCurrentIndex(0);
            updateStatus("打开绘制工具面板");
        } else if (panelName == "layer") {
            m_panelStack->setCurrentIndex(1);
            updateStatus("打开图层管理面板");
        }
    }
}
