#include "myform.h"
#include "ui_myform.h"
#include "core/common/version.h"  // 添加版本信息头文件
#include "tilemap/tilemapmanager.h"  // 添加瓦片地图管理器头文件
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QWheelEvent>
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
#include <cmath>
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
    , isRightClickDragging(false)  // 初始化右键拖拽状态
    , tileMapManager(nullptr)  // 初始化瓦片地图管理器
    , m_layerManager(nullptr)  // 初始化图层管理器
    , progressBar(nullptr)  // 初始化进度条
    , isDownloading(false)  // 初始化下载状态
    , viewUpdateTimer(nullptr)  // 初始化更新定时器
    , deviceTreeModel(nullptr)  // 初始化设备树模型
{
    logMessage("=== MyForm constructor started ===");
    ui->setupUi(this);
    
    // 设置功能区
    setupFunctionalArea();
    
    // 设置设备树
    setupDeviceTree();
    
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
    QTimer::singleShot(300, this, [this]() {
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
    // 确保浮动工具条创建并定位（放到下一个事件循环，确保viewport已布局）
    QTimer::singleShot(0, this, [this]() {
        if (mapScene && !gvOverlay) createGraphicsOverlay();
        if (mapScene) positionGraphicsOverlay();
    });
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
    // 重新定位浮动状态栏
    positionFloatingStatusBar();
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
    ui->newButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    ui->openButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->saveButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->saveAsButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
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
    setToolButtonStyle(ui->newButton);
    setToolButtonStyle(ui->openButton);
    setToolButtonStyle(ui->saveButton);
    setToolButtonStyle(ui->saveAsButton);
    setToolButtonStyle(ui->undoButton);
    setToolButtonStyle(ui->redoButton);

    // 连接工具栏按钮
    connect(ui->newButton, &QToolButton::clicked, this, &MyForm::handleNewButtonClicked);
    connect(ui->openButton, &QToolButton::clicked, this, &MyForm::handleOpenButtonClicked);
    connect(ui->saveButton, &QToolButton::clicked, this, &MyForm::handleSaveButtonClicked);
    connect(ui->saveAsButton, &QToolButton::clicked, this, &MyForm::handleSaveAsButtonClicked);
    connect(ui->undoButton, &QToolButton::clicked, this, &MyForm::handleUndoButtonClicked);
    connect(ui->redoButton, &QToolButton::clicked, this, &MyForm::handleRedoButtonClicked);

    // 设置快捷键
    ui->newButton->setShortcut(QKeySequence::New);
    ui->openButton->setShortcut(QKeySequence::Open);
    ui->saveButton->setShortcut(QKeySequence::Save);
    ui->saveAsButton->setShortcut(QKeySequence::SaveAs);
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
    });
    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, [this]() {
        if (tileMapManager && !isDownloading) {
            viewUpdateTimer->start();  // 重启定时器，实现防抖更新
        }
        positionGraphicsOverlay();
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

    vl->addWidget(btnZoomIn);
    vl->addWidget(btnZoomOut);
    vl->addWidget(btnPanToggle);

    // 明确设置容器尺寸，避免 sizeHint 为 0 导致不可见
    int w = 28;
    int h = 28 * 3 + 8 * 2; // 三个按钮 + 两个间距
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
    int x = vp.width() - sz.width() - margin;
    int y = margin;
    gvOverlay->setGeometry(x, y, sz.width(), sz.height());
    qDebug() << "Overlay positioned at:" << gvOverlay->geometry() << "viewport:" << vp;
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
                int newZoomLevel = currentZoomLevel;
                
                if (wheelEvent->angleDelta().y() > 0) {
                    // 放大：增加层级
                    newZoomLevel = qMin(currentZoomLevel + 1, MAX_ZOOM_LEVEL);
                } else {
                    // 缩小：减少层级
                    newZoomLevel = qMax(currentZoomLevel - 1, MIN_ZOOM_LEVEL);
                }
                
                // 如果层级发生变化，更新瓦片地图
                if (newZoomLevel != currentZoomLevel) {
                    // 获取鼠标在视口中的位置
                    QPoint mouseViewportPos = wheelEvent->position().toPoint();
                    QPointF mouseScenePos = ui->graphicsView->mapToScene(mouseViewportPos);
                    
                    qDebug() << "Zoom from" << currentZoomLevel << "to" << newZoomLevel 
                             << "at viewport:" << mouseViewportPos 
                             << "scene:" << mouseScenePos;
                    
                    // 更新缩放级别
                    currentZoomLevel = newZoomLevel;
                    
                    // 关键：以鼠标位置为中心进行缩放
                    // 传递鼠标在视口中的位置（像素坐标）
                    tileMapManager->setZoomAtMousePosition(
                        currentZoomLevel, 
                        mouseScenePos.x(), 
                        mouseScenePos.y(),
                        mouseViewportPos.x(),
                        mouseViewportPos.y(),
                        ui->graphicsView->viewport()->width(),
                        ui->graphicsView->viewport()->height()
                    );
                // 缩放后将视图中心与新中心像素对齐，保持锚点一致
                QPointF centerScene = tileMapManager->getCenterScenePos();
                ui->graphicsView->centerOn(centerScene);
                // 用对齐后的真实中心触发一次立即更新，避免缩放中心与加载中心不一致
                tileMapManager->updateTilesForViewImmediate(centerScene.x(), centerScene.y());
                    
                    // 同步缩放级别到管网渲染器
                    if (m_layerManager) {
                        m_layerManager->setZoom(currentZoomLevel);
                        qDebug() << "[Zoom] Renderer zoom updated to:" << currentZoomLevel;
                    }
                    
                    updateStatus(QString("Tile Map Zoom Level: %1/%2").arg(currentZoomLevel).arg(MAX_ZOOM_LEVEL));
                }
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
            // 只有当鼠标在QGraphicsView区域时，右键按下才启用拖拽模式
            if (mouseEvent->button() == Qt::RightButton) {
                // 检查鼠标是否在graphicsView区域内
                QPoint mousePos = ui->graphicsView->mapFromGlobal(QCursor::pos());
                QRect viewRect = ui->graphicsView->rect();
                if (viewRect.contains(mousePos)) {
                    lastRightClickPos = mouseEvent->pos();
                    lastRightClickScenePos = ui->graphicsView->mapToScene(lastRightClickPos);
                    isRightClickDragging = true;
                    if (tileMapManager) tileMapManager->setDragging(true);
                    ui->graphicsView->setCursor(Qt::ClosedHandCursor);
                    return true; // 事件已处理
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
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

void MyForm::handleNewButtonClicked()
{
    qDebug() << "New button clicked";
    currentFile.clear();
    // 这里可以初始化一个新的文档
    updateStatus("New document created");
    isModified = false;
}

void MyForm::handleOpenButtonClicked()
{
    qDebug() << "Open button clicked";
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", 
        tr("Text Files (*.txt);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            // 这里可以将文件内容加载到编辑区域
            QString content = in.readAll();
            // 假设我们有一个文本编辑器来显示内容
            // ui->textEdit->setPlainText(content);
            
            currentFile = fileName;
            updateStatus("Opened: " + fileName);
            isModified = false;
            file.close();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open file %1").arg(fileName));
        }
    }
}

void MyForm::handleSaveButtonClicked()
{
    qDebug() << "Save button clicked";
    
    if (currentFile.isEmpty()) {
        // 如果没有当前文件，调用另存为
        handleSaveAsButtonClicked();
    } else {
        QFile file(currentFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // 这里可以保存编辑区域的内容
            // out << ui->textEdit->toPlainText();
            
            updateStatus("Saved: " + currentFile);
            isModified = false;
            file.close();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Cannot save file %1").arg(currentFile));
        }
    }
}

void MyForm::handleSaveAsButtonClicked()
{
    qDebug() << "Save As button clicked";
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "", 
        tr("Text Files (*.txt);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // 这里可以保存编辑区域的内容
            // out << ui->textEdit->toPlainText();
            
            currentFile = fileName;
            updateStatus("Saved as: " + fileName);
            isModified = false;
            file.close();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Cannot save file %1").arg(fileName));
        }
    }
}

void MyForm::handleUndoButtonClicked()
{
    qDebug() << "Undo button clicked";
    updateStatus("Undo action performed");
    // 这里可以实现撤销功能
    // ui->textEdit->undo();
}

void MyForm::handleRedoButtonClicked()
{
    qDebug() << "Redo button clicked";
    updateStatus("Redo action performed");
    // 这里可以实现重做功能
    // ui->textEdit->redo();
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
            viewportRect.height()
        );
        // 与滚轮缩放保持一致：对齐视图中心并立即刷新可见瓦片
        QPointF centerScene = tileMapManager->getCenterScenePos();
        ui->graphicsView->centerOn(centerScene);
        tileMapManager->updateTilesForViewImmediate(centerScene.x(), centerScene.y());
        
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
            viewportRect.height()
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
            throw std::runtime_error("App config file not found");
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
        
        // 6. 延迟加载初始数据（再延迟1秒，让界面完全稳定）
        qDebug() << "[Pipeline] Scheduling data load in 1 second...";
        QTimer::singleShot(1000, this, &MyForm::loadPipelineData);
        
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
    
    // 使用测试数据的坐标范围（北京天安门区域）
    // 测试数据范围: 经度 116.39-116.42, 纬度 39.90-39.92
    QRectF geoBounds(
        116.390,  // 最小经度
        39.900,   // 最小纬度
        0.030,    // 宽度（经度跨度）
        0.020     // 高度（纬度跨度）
    );
    
    // 只用于数据库空间查询的可视范围，不改变当前地图视图和缩放
    m_layerManager->setVisibleBounds(geoBounds);
    qDebug() << "[Pipeline] Visible bounds set to:" << geoBounds;
    
    // 刷新所有可见图层（不会修改 tileMapManager 的 center/zoom）
    m_layerManager->refreshAllLayers();
    qDebug() << "[Pipeline] refreshAllLayers() called";
    
    LOG_INFO("Pipeline layers refreshed");
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
    updateStatus("功能开发中：数据导入...");
    QMessageBox::information(this, "提示", "数据导入功能开发中");
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
    updateStatus("功能开发中：爆管影响分析...");
    QMessageBox::information(this, "提示", "爆管影响分析功能开发中");
}

void MyForm::onConnectivityAnalysisButtonClicked()
{
    qDebug() << "[UI] Connectivity Analysis button clicked";
    updateStatus("功能开发中：连通性分析...");
    QMessageBox::information(this, "提示", "连通性分析功能开发中");
}

// 工单与资产模块
void MyForm::onWorkOrderButtonClicked()
{
    qDebug() << "[UI] Work Order button clicked";
    updateStatus("功能开发中：工单管理...");
    QMessageBox::information(this, "提示", "工单管理功能开发中");
}

void MyForm::onAssetManagementButtonClicked()
{
    qDebug() << "[UI] Asset Management button clicked";
    updateStatus("功能开发中：资产管理...");
    QMessageBox::information(this, "提示", "资产管理功能开发中");
}

// 工具模块
void MyForm::onSettingsButtonClicked()
{
    qDebug() << "[UI] Settings button clicked";
    updateStatus("功能开发中：系统设置...");
    QMessageBox::information(this, "提示", "系统设置功能开发中");
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
    
    // TODO: 打开设备详情对话框
    QMessageBox::information(this, "设备详情", 
                             "设备名称: " + name + "\n\n" +
                             "详细信息功能开发中...");
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
