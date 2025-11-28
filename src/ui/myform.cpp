#include "myform.h"
#include "ui_myform.h"
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
{
    logMessage("=== MyForm constructor started ===");
    ui->setupUi(this);
    
    // 设置功能区
    setupFunctionalArea();
    
    logMessage("=== MyForm constructor finished (UI setup only) ===");
    
    // 异步初始化地图区域（延迟100ms，让窗口先显示）
    QTimer::singleShot(100, this, [this]() {
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
    QTimer::singleShot(800, this, [this]() {
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
    
    // 地图控制按钮已移除：Zoom In/Out 与 Pan 改由 graphics 浮层提供
    
    // 初始化进度条
    progressBar = ui->progressBar;
    progressBar->setVisible(false);  // 初始时隐藏进度条
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    
    // 连接区域下载进度信号（在tileMapManager创建后再连接）
    // 初始化状态
    updateStatus("Ready");

    // 添加菜单栏，将常用按钮动作放入菜单
    QMenuBar *menuBar = new QMenuBar(ui->functionalArea);
    menuBar->setNativeMenuBar(false);
    menuBar->setObjectName("mainMenuBar");
    QMenu *menu = new QMenu(tr("文件"), menuBar);
    menu->setObjectName("fileMenu");

    QAction *actNew = new QAction(tr("新建"), menu);
    QAction *actOpen = new QAction(tr("打开"), menu);
    QAction *actSave = new QAction(tr("保存"), menu);
    QAction *actSaveAs = new QAction(tr("另存为"), menu);
    QAction *actUndo = new QAction(tr("撤销"), menu);
    QAction *actRedo = new QAction(tr("重做"), menu);
    QAction *actMapMgr = new QAction(tr("地图管理"), menu);

    actNew->setShortcut(QKeySequence::New);
    actOpen->setShortcut(QKeySequence::Open);
    actSave->setShortcut(QKeySequence::Save);
    actSaveAs->setShortcut(QKeySequence::SaveAs);
    actUndo->setShortcut(QKeySequence::Undo);
    actRedo->setShortcut(QKeySequence::Redo);

    // 预估列宽：图标列固定，文本列和快捷键列取最大值，确保完整显示
    QVector<QAction*> allActs{actNew, actOpen, actSave, actSaveAs, actUndo, actRedo};
    QFontMetrics fm(menu->font());
    int maxTextW = 0;
    int maxScW = 0;
    for (QAction *a : allActs) {
        // 使用 boundingRect 宽度，避免 advance 低估字形外沿
        maxTextW = qMax(maxTextW, fm.boundingRect(a->text()).width());
        maxScW = qMax(maxScW, fm.boundingRect(a->shortcut().toString(QKeySequence::NativeText)).width());
    }
    const int iconColW = 20;        // 图标列宽（更紧凑）
    const int gap = 6;              // 列间距（更紧凑）
    const int paddingLR = 8;        // 行内左右内边距（视感留白）
    int panel = menu->style()->pixelMetric(QStyle::PM_MenuPanelWidth, nullptr, menu);
    int hmargin = menu->style()->pixelMetric(QStyle::PM_MenuHMargin, nullptr, menu);
    QMargins cm = menu->contentsMargins();
    // 额外边距：样式面板+左右边距+控件自身内容边距+样式表边框(2px) + 额外保护像素
    int extra = 2 * (panel + hmargin) + cm.left() + cm.right() + 2 + 2;
    int computedMenuW = iconColW + gap + maxTextW + gap + maxScW + (2 * paddingLR) + extra; // 内容+内边距+系统边距
    menu->setMinimumWidth(computedMenuW);

    // 自定义 QWidgetAction 项（图标左、文本中左对齐、快捷键右）
    auto addMenuItem = [&](QAction *act, const std::function<void()> &onClick) {
        QWidgetAction *wa = new QWidgetAction(menu);
        QPushButton *btn = new QPushButton; // 使用按钮承载，便于 hover/press 效果和点击信号
        btn->setFlat(true);
        btn->setMouseTracking(true);
        btn->setAttribute(Qt::WA_Hover, true);
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        btn->setStyleSheet(
            QString("QPushButton{background:#ffffff; border:0; padding:10px %1; color:#222; font-weight:500; text-align:left;}")
                .arg(paddingLR) +
            // 更明显的悬浮/按下效果（不更改鼠标样式）
            "QPushButton:hover{background:#eef2ff;}"
            "QPushButton:pressed{background:#dde7ff;}"
        );
        btn->setMinimumWidth(computedMenuW);

        QWidget *inner = new QWidget(btn);
        inner->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 让 hover 事件作用于按钮本体
        QHBoxLayout *h = new QHBoxLayout(inner);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(gap);

        QLabel *iconLabel = new QLabel(inner);
        QIcon ico = act->icon();
        if (ico.isNull()) {
            // 兜底使用文件图标，避免为空不显示
            ico = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
        }
        iconLabel->setPixmap(ico.pixmap(18, 18));
        iconLabel->setFixedWidth(iconColW);
        iconLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        h->addWidget(iconLabel);

        QLabel *textLabel = new QLabel(act->text(), inner);
        textLabel->setStyleSheet("color:#222; font-weight:500;");
        textLabel->setFixedWidth(maxTextW); // 文本列固定宽度，保证后续列左对齐起点一致
        textLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        h->addWidget(textLabel);

        QString sc = act->shortcut().toString(QKeySequence::NativeText);
        QLabel *shortcutLabel = new QLabel(sc, inner);
        shortcutLabel->setStyleSheet("color:#666;");
        shortcutLabel->setFixedWidth(maxScW); // 快捷键列起点固定，左对齐展示
        shortcutLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        shortcutLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        h->addWidget(shortcutLabel);

        QHBoxLayout *btnLayout = new QHBoxLayout(btn);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->addWidget(inner);

        connect(btn, &QPushButton::clicked, this, [this, menu, onClick]() {
            if (menu) menu->hide();
            onClick();
        });

        wa->setDefaultWidget(btn);
        menu->addAction(wa);
    };

    // 为各动作设置标准图标（提前设置，确保自定义项能取到 icon）
    // 设置一级菜单图标为“文件系统/目录”图标
    QIcon menuIcon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
    menu->setIcon(menuIcon);

    // 为各动作设置标准图标
    actNew->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
    actOpen->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
    actSave->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    actSaveAs->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    actUndo->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowBack));
    actRedo->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowForward));

    // 确保快捷键生效：把动作注册到窗口
    this->addAction(actNew);
    this->addAction(actOpen);
    this->addAction(actSave);
    this->addAction(actSaveAs);
    this->addAction(actUndo);
    this->addAction(actRedo);

    addMenuItem(actNew, [this]() { handleNewButtonClicked(); });
    addMenuItem(actOpen, [this]() { handleOpenButtonClicked(); });
    // 分隔符用空白项模拟
    menu->addSeparator();
    addMenuItem(actSave, [this]() { handleSaveButtonClicked(); });
    addMenuItem(actSaveAs, [this]() { handleSaveAsButtonClicked(); });
    menu->addSeparator();
    addMenuItem(actUndo, [this]() { handleUndoButtonClicked(); });
    addMenuItem(actRedo, [this]() { handleRedoButtonClicked(); });
    menu->addSeparator();
    // 地图管理入口采用普通 QAction（非 QWidgetAction），直接触发对话框
    menu->addAction(actMapMgr);

    menuBar->addMenu(menu);

    // 基本样式优化（可迁移到全局 qss）
    // 样式：顶部菜单栏透明；二级菜单白底，悬浮浅灰；文本居中由 QPushButton 样式控制
    QString menuStyle =
        "QMenuBar{background-color: transparent; border:0;}"
        "QMenuBar::item{padding:8px 14px; margin:0 6px; color:#ffffff; font-weight:600;}"
        "QMenuBar::item:selected{background-color: rgba(255,122,24,0.20); border-radius:6px;}"
        "QMenuBar::item:pressed{background-color: rgba(255,122,24,0.30);}" 
        "QMenu{background-color:#ffffff; border:1px solid #dddddd; color:#222222; padding:0px; margin:0px;}"
        "QMenu::separator{height:1px; background:#e6e6e6; margin:6px 10px;}";
    menuBar->setStyleSheet(menuStyle);
    connect(actMapMgr, &QAction::triggered, this, [this]() {
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
            // 读取对话框设置（当前区域使用默认中国范围；后续可改为表单读取）
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

    // 放入功能区布局顶部
    if (auto layout = ui->functionalArea->layout()) {
        // 若是 QVBoxLayout，可使用 setMenuBar；否则插入到顶部
        if (auto vbl = qobject_cast<QVBoxLayout*>(layout)) {
            vbl->setMenuBar(menuBar);
        } else {
            layout->setMenuBar(menuBar);
        }
    } else {
        auto vbl = new QVBoxLayout(ui->functionalArea);
        vbl->setContentsMargins(0, 0, 0, 0);
        vbl->setSpacing(0);
        vbl->setMenuBar(menuBar);
    }

    // 将菜单动作连接到已有按钮的槽函数，行为保持一致
    connect(actNew, &QAction::triggered, this, &MyForm::handleNewButtonClicked);
    connect(actOpen, &QAction::triggered, this, &MyForm::handleOpenButtonClicked);
    connect(actSave, &QAction::triggered, this, &MyForm::handleSaveButtonClicked);
    connect(actSaveAs, &QAction::triggered, this, &MyForm::handleSaveAsButtonClicked);
    connect(actUndo, &QAction::triggered, this, &MyForm::handleUndoButtonClicked);
    connect(actRedo, &QAction::triggered, this, &MyForm::handleRedoButtonClicked);
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
        // 隐藏进度条
        isDownloading = false;
        progressBar->setVisible(false);
        progressBar->setValue(0);
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
    ui->statusLabel->setText(message);
    qDebug() << "Status:" << message;
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
    
    // 显示进度条
    if (!isDownloading) {
        isDownloading = true;
        progressBar->setVisible(true);
    }
    
    if (total > 0) {
        int progress = (current * 100) / total;
        qDebug() << "Download progress:" << current << "/" << total << "(" << progress << "%) at zoom level" << zoom;
        
        // 更新进度条
        progressBar->setValue(progress);
        progressBar->setFormat(QString("Downloading zoom level %1: %2% (%3/%4)").arg(zoom).arg(progress).arg(current).arg(total));
        
        // 更新状态标签
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
    
    // 将地图中心移动到测试数据区域（北京天安门）
    if (tileMapManager) {
        qDebug() << "[Pipeline] Moving map to Beijing (test data area)...";
        tileMapManager->setCenter(39.91, 116.40);
        
        // 如果当前缩放级别太小，放大到合适级别
        if (currentZoomLevel < 13) {
            qDebug() << "[Pipeline] Zoom level too low (" << currentZoomLevel << "), setting to 15";
            currentZoomLevel = 15;
            tileMapManager->setZoom(currentZoomLevel);
            
            // 同步到管网渲染器
            if (m_layerManager) {
                m_layerManager->setZoom(currentZoomLevel);
            }
        }
        
        // 等待地图加载完成
        QTimer::singleShot(200, this, [this, geoBounds]() {
            qDebug() << "[Pipeline] Map repositioned, now loading pipeline data...";
            
            // 设置可视范围
            m_layerManager->setVisibleBounds(geoBounds);
            qDebug() << "[Pipeline] Visible bounds set to:" << geoBounds;
            
            // 刷新所有可见图层
            m_layerManager->refreshAllLayers();
            qDebug() << "[Pipeline] refreshAllLayers() called";
            
            checkPipelineRenderResult();
        });
        return;
    }
    
    // 如果没有 tileMapManager，直接加载
    m_layerManager->setVisibleBounds(geoBounds);
    m_layerManager->refreshAllLayers();
    
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
