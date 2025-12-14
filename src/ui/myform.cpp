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
#include "widgets/helpdialog.h"  // 帮助对话框
#include "widgets/entityviewdialog.h"  // 实体属性查看对话框
#include "widgets/messagedialog.h"  // 消息对话框
#include "widgets/profiledialog.h"  // 个人信息对话框
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
#include <QClipboard>
#include <QMenu>
#include <QStringConverter>
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
    , m_deviceTreeMenuActive(false)  // 初始化右键菜单标志
    , m_deviceTreeDialogActive(false)  // 初始化对话框标志
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
    , m_messageDialog(nullptr)  // 初始化消息对话框
    , m_selectedItem(nullptr)  // 初始化选中项
    , m_nextPipelineId(1)  // 从ID=1开始
    , m_copiedItem(nullptr)  // 初始化复制项
    , m_copiedLineWidth(3)  // 默认线宽
    , m_hasStyleCopied(false)  // 初始化样式复制标志
    , m_undoStack(nullptr)  // 初始化撤销栈
    , m_currentConnectivityType(0)  // 初始化为Upstream (0)
    , m_hasUnsavedChanges(false)  // 初始化未保存变更标志
    , m_distanceMeasureMode(false)  // 初始化距离量算模式
    , m_distancePreviewLine(nullptr)  // 初始化距离预览线
    , m_areaMeasureMode(false)  // 初始化面积量算模式
    , m_areaPreviewLine(nullptr)  // 初始化面积预览线
{
    logMessage("=== MyForm constructor started ===");
    ui->setupUi(this);
    
    // 创建撤销栈
    m_undoStack = new QUndoStack(this);
    m_undoStack->setUndoLimit(50);  // 限制最多50步撤销
    
    // 连接撤销栈信号，自动更新按钮状态
    connect(m_undoStack, &QUndoStack::canUndoChanged, this, [this](bool canUndo) {
        if (ui->undoButton) {
            ui->undoButton->setEnabled(canUndo);
        }
    });
    connect(m_undoStack, &QUndoStack::canRedoChanged, this, [this](bool canRedo) {
        if (ui->redoButton) {
            ui->redoButton->setEnabled(canRedo);
        }
    });
    connect(m_undoStack, &QUndoStack::indexChanged, this, [this](int index) {
        // 更新按钮状态和提示文本
        updateUndoRedoButtonStates();
    });
    
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
    // Delete 键删除选中的实体或测量线条
    if (event->key() == Qt::Key_Delete) {
        if (m_selectedItem) {
            QString itemType = m_selectedItem->data(0).toString();
            // 检查是否是测量线条
            bool isMeasureItem = (itemType == "distance_measure_line" || 
                                  itemType == "distance_measure_label" ||
                                  itemType == "distance_measure_marker" ||
                                  itemType == "area_measure_polygon" ||
                                  itemType == "area_measure_label" ||
                                  itemType == "area_measure_marker");
            
            if (isMeasureItem) {
                // 删除测量线条
                if (itemType.startsWith("distance_")) {
                    deleteDistanceMeasure(m_selectedItem);
                } else if (itemType.startsWith("area_")) {
                    deleteAreaMeasure(m_selectedItem);
                }
                clearSelection();
            } else {
                // 删除实体
                onDeleteSelectedEntity();
            }
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
    
    // Esc 退出距离量算模式（取消当前测量，清除当前正在绘制的点）
    if (m_distanceMeasureMode && event->key() == Qt::Key_Escape) {
        m_distanceMeasureMode = false;
        // 只清除当前正在绘制的点，不清除已完成的测量结果
        clearDistancePreview();
        if (mapScene) {
            for (QGraphicsItem *item : m_currentDistanceMarkers) {
                mapScene->removeItem(item);
                delete item;
            }
            m_currentDistanceMarkers.clear();
            if (m_currentDistanceLine) {
                mapScene->removeItem(m_currentDistanceLine);
                delete m_currentDistanceLine;
                m_currentDistanceLine = nullptr;
            }
        }
        m_currentDistancePoints.clear();
        ui->graphicsView->setCursor(Qt::ArrowCursor);
        updateStatus("已取消距离量算（已完成的测量结果保留）");
        return;
    }
    
    // Esc 退出面积量算模式（取消当前测量，清除当前正在绘制的点）
    if (m_areaMeasureMode && event->key() == Qt::Key_Escape) {
        m_areaMeasureMode = false;
        // 只清除当前正在绘制的点，不清除已完成的测量结果
        clearAreaPreview();
        if (mapScene) {
            for (QGraphicsItem *item : m_currentAreaMarkers) {
                mapScene->removeItem(item);
                delete item;
            }
            m_currentAreaMarkers.clear();
            if (m_currentAreaPolygon) {
                mapScene->removeItem(m_currentAreaPolygon);
                delete m_currentAreaPolygon;
                m_currentAreaPolygon = nullptr;
            }
        }
        m_currentAreaPoints.clear();
        ui->graphicsView->setCursor(Qt::ArrowCursor);
        updateStatus("已取消面积量算（已完成的测量结果保留）");
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
    // 使用自定义图标：消息按钮（默认无消息状态）
    ui->messageButton->setIcon(QIcon(":/new/prefix1/images/NoMessage.png"));
    // 使用自定义图标：个人信息按钮
    ui->profileButton->setIcon(QIcon(":/new/prefix1/images/Account.png"));

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
    setToolButtonStyle(ui->messageButton);
    setToolButtonStyle(ui->profileButton);

    // 连接工具栏按钮
    connect(ui->refreshButton, &QToolButton::clicked, this, &MyForm::handleRefreshButtonClicked);
    connect(ui->saveButton, &QToolButton::clicked, this, &MyForm::handleSaveButtonClicked);
    connect(ui->undoButton, &QToolButton::clicked, this, &MyForm::handleUndoButtonClicked);
    connect(ui->redoButton, &QToolButton::clicked, this, &MyForm::handleRedoButtonClicked);
    connect(ui->messageButton, &QToolButton::clicked, this, &MyForm::handleMessageButtonClicked);
    connect(ui->profileButton, &QToolButton::clicked, this, &MyForm::handleProfileButtonClicked);

    // 设置快捷键
    ui->refreshButton->setShortcut(QKeySequence(Qt::Key_F5));
    ui->saveButton->setShortcut(QKeySequence::Save);
    ui->undoButton->setShortcut(QKeySequence::Undo);
    ui->redoButton->setShortcut(QKeySequence::Redo);
    ui->messageButton->setShortcut(QKeySequence(Qt::ALT | Qt::Key_M));
    ui->profileButton->setShortcut(QKeySequence(Qt::ALT | Qt::Key_P));
    
    // 初始化按钮状态
    updateUndoRedoButtonStates();

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
    if (auto distanceMeasureBtn = ui->functionalArea->findChild<QPushButton*>("distanceMeasureButton")) {
        connect(distanceMeasureBtn, &QPushButton::clicked, this, &MyForm::onDistanceMeasureButtonClicked);
    }
    if (auto areaMeasureBtn = ui->functionalArea->findChild<QPushButton*>("areaMeasureButton")) {
        connect(areaMeasureBtn, &QPushButton::clicked, this, &MyForm::onAreaMeasureButtonClicked);
    }
    if (auto clearMeasureBtn = ui->functionalArea->findChild<QPushButton*>("clearMeasureButton")) {
        connect(clearMeasureBtn, &QPushButton::clicked, this, &MyForm::onClearMeasureButtonClicked);
    }
    
    // 信息模块
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
    
    // 禁用默认的选中样式（虚线框），使用自定义高亮
    ui->graphicsView->setStyleSheet(
        "QGraphicsView {"
        "  selection-background-color: transparent;"
        "}"
    );
    
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
    
    // 连接场景选择变化信号，完全禁用Qt默认的虚线框选择样式
    connect(mapScene, &QGraphicsScene::selectionChanged, this, [this]() {
        // 当场景选择状态改变时，立即取消所有项的默认选择状态
        // 只使用自定义高亮，不使用Qt默认的虚线框
        QList<QGraphicsItem*> selectedItems = mapScene->selectedItems();
        for (QGraphicsItem *item : selectedItems) {
            // 如果是实体项，取消默认选择，使用自定义高亮
            if (isEntityItem(item)) {
                item->setSelected(false);
                // 如果这个项不是当前选中的项，则选中它（使用自定义高亮）
                if (item != m_selectedItem) {
                    onEntityClicked(item);
                }
            } else {
                // 非实体项也取消默认选择
                item->setSelected(false);
            }
        }
    });
    
    // 检查本地是否有已下载的瓦片，如果有则自动显示
    logMessage("Checking for local tiles...");
    updateStatus("Checking for local tiles...");
    tileMapManager->checkLocalTiles();
    logMessage("Local tiles check completed");
    
    // 显示本地瓦片信息
    tileMapManager->getLocalTilesInfo();
    
    // 初始化视图大小
    QTimer::singleShot(50, this, [this]() {
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
            
            // 距离量算模式：左键添加点，右键完成（但如果右键点击已完成的测量线条，显示菜单）
            if (m_distanceMeasureMode) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                    if (tileMapManager) {
                        QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                        handleDistanceMeasurePoint(QPointF(geo.x(), geo.y()));
                    }
                } else if (mouseEvent->button() == Qt::RightButton) {
                    // 如果正在测量过程中（有未完成的点），右键完成测量
                    if (!m_currentDistancePoints.isEmpty()) {
                        cancelDistanceMeasureMode();
                    } else {
                        // 没有正在测量，检查是否点击了已完成的距离测量线条
                        QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                        const qreal screenPixelTolerance = 2.0;
                        QPointF screenDelta(screenPixelTolerance, 0);
                        QPointF sceneDelta = ui->graphicsView->mapToScene(screenDelta.toPoint()) - 
                                             ui->graphicsView->mapToScene(QPoint(0, 0));
                        qreal searchRadius = qAbs(sceneDelta.x());
                        if (searchRadius <= 0 || searchRadius > 1000) {
                            searchRadius = 2.0;
                        }
                        QRectF searchRect(scenePos.x() - searchRadius, 
                                         scenePos.y() - searchRadius, 
                                         searchRadius * 2, 
                                         searchRadius * 2);
                        QList<QGraphicsItem*> items = mapScene->items(searchRect, Qt::IntersectsItemShape, 
                                                                       Qt::DescendingOrder, 
                                                                       ui->graphicsView->transform());
                        if (items.isEmpty()) {
                            items = mapScene->items(scenePos, Qt::IntersectsItemShape, 
                                                   Qt::DescendingOrder, 
                                                   ui->graphicsView->transform());
                        }
                        // 查找已完成的距离测量线条
                        QGraphicsItem *measureItem = nullptr;
                        for (QGraphicsItem *candidate : items) {
                            QString itemType = candidate->data(0).toString();
                            if (itemType == "distance_measure_line" || 
                                itemType == "distance_measure_label" ||
                                itemType == "distance_measure_marker") {
                                measureItem = candidate;
                                break;
                            }
                        }
                        // 如果点击的是已完成的测量线条，显示右键菜单
                        if (measureItem) {
                            if (measureItem != m_selectedItem) {
                                onEntityClicked(measureItem);
                            }
                            onShowContextMenu(mouseEvent->pos());
                        }
                        // 如果点击空白处，不做任何事（保持测量模式）
                    }
                }
                return true; // 拦截其他操作
            }
            
            // 面积量算模式：左键添加点，右键完成（但如果右键点击已完成的测量线条，显示菜单）
            if (m_areaMeasureMode) {
                if (mouseEvent->button() == Qt::LeftButton) {
                    QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                    if (tileMapManager) {
                        QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                        handleAreaMeasurePoint(QPointF(geo.x(), geo.y()));
                    }
                } else if (mouseEvent->button() == Qt::RightButton) {
                    // 如果正在测量过程中（有未完成的点），右键完成测量
                    if (!m_currentAreaPoints.isEmpty()) {
                        cancelAreaMeasureMode();
                    } else {
                        // 没有正在测量，检查是否点击了已完成的面积测量线条
                        QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                        const qreal screenPixelTolerance = 2.0;
                        QPointF screenDelta(screenPixelTolerance, 0);
                        QPointF sceneDelta = ui->graphicsView->mapToScene(screenDelta.toPoint()) - 
                                             ui->graphicsView->mapToScene(QPoint(0, 0));
                        qreal searchRadius = qAbs(sceneDelta.x());
                        if (searchRadius <= 0 || searchRadius > 1000) {
                            searchRadius = 2.0;
                        }
                        QRectF searchRect(scenePos.x() - searchRadius, 
                                         scenePos.y() - searchRadius, 
                                         searchRadius * 2, 
                                         searchRadius * 2);
                        QList<QGraphicsItem*> items = mapScene->items(searchRect, Qt::IntersectsItemShape, 
                                                                       Qt::DescendingOrder, 
                                                                       ui->graphicsView->transform());
                        if (items.isEmpty()) {
                            items = mapScene->items(scenePos, Qt::IntersectsItemShape, 
                                                   Qt::DescendingOrder, 
                                                   ui->graphicsView->transform());
                        }
                        // 查找已完成的面积测量线条
                        QGraphicsItem *measureItem = nullptr;
                        for (QGraphicsItem *candidate : items) {
                            QString itemType = candidate->data(0).toString();
                            if (itemType == "area_measure_polygon" || 
                                itemType == "area_measure_label" ||
                                itemType == "area_measure_marker") {
                                measureItem = candidate;
                                break;
                            }
                        }
                        // 如果点击的是已完成的测量线条，显示右键菜单
                        if (measureItem) {
                            if (measureItem != m_selectedItem) {
                                onEntityClicked(measureItem);
                            }
                            onShowContextMenu(mouseEvent->pos());
                        }
                        // 如果点击空白处，不做任何事（保持测量模式）
                    }
                }
                return true; // 拦截其他操作
            }

            // 非绘制模式：处理实体选中和右键菜单
            // 注意：如果处于爆管分析、连通性分析等特殊模式，这些模式已经拦截了事件，不会执行到这里
            if (!m_drawingManager || !m_drawingManager->isDrawing()) {
                // 如果处于爆管分析模式，不应该处理实体选中（虽然理论上不会执行到这里）
                if (m_burstSelectionMode) {
                    return false; // 让事件继续传播，但不会处理实体选中
                }
                
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
                
                // 在鼠标按下时立即取消所有项的选择状态，避免显示虚线框
                // 这可以防止Qt默认选择机制在鼠标按下时触发
                for (QGraphicsItem *candidate : items) {
                    if (isEntityItem(candidate)) {
                        candidate->setSelected(false);
                    }
                }
                
                // 收集所有实体项和测量线条
                QList<QGraphicsItem*> facilityItems;
                QList<QGraphicsItem*> pipelineItems;
                QGraphicsItem *measureItem = nullptr;
                
                for (QGraphicsItem *candidate : items) {
                    QString itemType = candidate->data(0).toString();
                    // 检查是否是测量线条（优先）
                    if ((itemType == "distance_measure_line" || 
                         itemType == "distance_measure_label" ||
                         itemType == "area_measure_polygon" ||
                         itemType == "area_measure_label") && !measureItem) {
                        measureItem = candidate;
                    }
                    // 如果点击到标注，尝试找到关联的设施（通过data(10)存储的图形项指针）
                    if (itemType == "annotation" && candidate->data(1).toString() == "facility") {
                        QVariant itemPtr = candidate->data(10);
                        if (itemPtr.isValid()) {
                            QGraphicsItem *linkedItem = reinterpret_cast<QGraphicsItem*>(itemPtr.toULongLong());
                            if (linkedItem && isEntityItem(linkedItem) && linkedItem->data(0).toString() == "facility") {
                                // 将关联的设施添加到列表中
                                if (!facilityItems.contains(linkedItem)) {
                                    facilityItems.append(linkedItem);
                                    qDebug() << "[Click] Found facility via annotation link";
                                }
                            }
                        }
                    }
                    // 检查是否是实体
                    if (isEntityItem(candidate)) {
                        QString entityType = candidate->data(0).toString();
                        if (entityType == "facility") {
                            if (!facilityItems.contains(candidate)) {
                                facilityItems.append(candidate);
                            }
                        } else if (entityType == "pipeline") {
                            pipelineItems.append(candidate);
                        }
                    }
                }
                
                // 优先选择测量线条，然后选择设施，最后选择管线
                QGraphicsItem *item = nullptr;
                
                if (measureItem) {
                    item = measureItem;
                } else if (!facilityItems.isEmpty()) {
                    // 对于设施，也需要检查距离（虽然设施是点，但也要确保在2像素范围内）
                    qreal minDistance = std::numeric_limits<qreal>::max();
                    QGraphicsItem *nearestFacility = nullptr;
                    
                    // 使用与searchRadius相同的计算方法
                    qreal maxAllowedDistance = searchRadius;
                    
                    for (QGraphicsItem *facility : facilityItems) {
                        // 获取设施的位置（椭圆的中心点）
                        QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(facility);
                        QPointF facilityPos;
                        if (ellipseItem) {
                            // 计算椭圆的中心点
                            QRectF rect = ellipseItem->rect();
                            QPointF center = rect.center();
                            facilityPos = facility->mapToScene(center);
                        } else {
                            // 如果不是椭圆，使用场景位置
                            facilityPos = facility->scenePos();
                        }
                        
                        qreal dist = QLineF(scenePos, facilityPos).length();
                        
                        // 只考虑距离在允许范围内的设施（增加容差，因为设施是点）
                        qreal facilityTolerance = maxAllowedDistance * 2; // 设施使用更大的容差
                        if (dist <= facilityTolerance && dist < minDistance) {
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
                    // 左键点击：选中实体或测量线条
                    if (item) {
                        QString itemType = item->data(0).toString();
                        if (isEntityItem(item) || 
                            itemType.startsWith("distance_measure_") ||
                            itemType.startsWith("area_measure_")) {
                            // 立即取消Qt默认选择，避免显示虚线框
                            item->setSelected(false);
                            onEntityClicked(item);
                            return true;
                        }
                    }
                    // 点击空白处，清除选中
                    clearSelection();
                } else if (mouseEvent->button() == Qt::RightButton) {
                    // 右键点击：显示菜单或拖拽
                    if (item) {
                        QString itemType = item->data(0).toString();
                        if (isEntityItem(item) || 
                            itemType.startsWith("distance_measure_") ||
                            itemType.startsWith("area_measure_")) {
                            // 如果点击的是实体或测量线条，显示菜单
                            if (item != m_selectedItem) {
                                onEntityClicked(item);  // 先选中
                            }
                            onShowContextMenu(mouseEvent->pos());
                            return true;
                        }
                    }
                    // 点击空白处，启用拖拽
                    if (true) {
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
            
            // 距离量算模式：双击完成测量
            if (m_distanceMeasureMode && mouseEvent->button() == Qt::LeftButton) {
                cancelDistanceMeasureMode();
                return true; // 拦截事件
            }
            
            // 面积量算模式：双击完成测量
            if (m_areaMeasureMode && mouseEvent->button() == Qt::LeftButton) {
                cancelAreaMeasureMode();
                return true; // 拦截事件
            }
            
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
                
                // 收集所有实体项和测量线条
                QList<QGraphicsItem*> facilityItems;
                QList<QGraphicsItem*> pipelineItems;
                QGraphicsItem *measureItem = nullptr;
                
                for (QGraphicsItem *candidate : items) {
                    QString itemType = candidate->data(0).toString();
                    // 检查是否是测量线条（优先）
                    if ((itemType == "distance_measure_line" || 
                         itemType == "distance_measure_label" ||
                         itemType == "area_measure_polygon" ||
                         itemType == "area_measure_label") && !measureItem) {
                        measureItem = candidate;
                    }
                    // 如果点击到标注，尝试找到关联的设施（通过data(10)存储的图形项指针）
                    if (itemType == "annotation" && candidate->data(1).toString() == "facility") {
                        QVariant itemPtr = candidate->data(10);
                        if (itemPtr.isValid()) {
                            QGraphicsItem *linkedItem = reinterpret_cast<QGraphicsItem*>(itemPtr.toULongLong());
                            if (linkedItem && isEntityItem(linkedItem) && linkedItem->data(0).toString() == "facility") {
                                // 将关联的设施添加到列表中
                                if (!facilityItems.contains(linkedItem)) {
                                    facilityItems.append(linkedItem);
                                    qDebug() << "[DoubleClick] Found facility via annotation link";
                                }
                            }
                        }
                    }
                    // 检查是否是实体
                    if (isEntityItem(candidate)) {
                        QString entityType = candidate->data(0).toString();
                        if (entityType == "facility") {
                            if (!facilityItems.contains(candidate)) {
                                facilityItems.append(candidate);
                            }
                        } else if (entityType == "pipeline") {
                            pipelineItems.append(candidate);
                        }
                    }
                }
                
                // 优先选择测量线条，然后选择设施，最后选择管线
                QGraphicsItem *item = nullptr;
                
                if (measureItem) {
                    item = measureItem;
                } else if (!facilityItems.isEmpty()) {
                    // 对于设施，也需要检查距离（虽然设施是点，但也要确保在2像素范围内）
                    qreal minDistance = std::numeric_limits<qreal>::max();
                    QGraphicsItem *nearestFacility = nullptr;
                    
                    // 使用与searchRadius相同的计算方法
                    qreal maxAllowedDistance = searchRadius;
                    
                    for (QGraphicsItem *facility : facilityItems) {
                        // 获取设施的位置（椭圆的中心点）
                        QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(facility);
                        QPointF facilityPos;
                        if (ellipseItem) {
                            // 计算椭圆的中心点
                            QRectF rect = ellipseItem->rect();
                            QPointF center = rect.center();
                            facilityPos = facility->mapToScene(center);
                        } else {
                            // 如果不是椭圆，使用场景位置
                            facilityPos = facility->scenePos();
                        }
                        
                        qreal dist = QLineF(scenePos, facilityPos).length();
                        
                        // 只考虑距离在允许范围内的设施（增加容差，因为设施是点）
                        qreal facilityTolerance = maxAllowedDistance * 2; // 设施使用更大的容差
                        if (dist <= facilityTolerance && dist < minDistance) {
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
            
            // 距离量算预览线更新
            if (m_distanceMeasureMode && tileMapManager) {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                updateDistancePreview(QPointF(geo.x(), geo.y()));
            }
            
            // 面积量算预览线更新
            if (m_areaMeasureMode && tileMapManager) {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                QPointF geo = tileMapManager->sceneToGeo(scenePos, currentZoomLevel);
                updateAreaPreview(QPointF(geo.x(), geo.y()));
            }
            
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
        
        // 延迟刷新标注图层，确保场景中的图形项已经渲染完成
        if (m_layerManager && m_layerManager->isLayerVisible(LayerManager::Labels)) {
            QTimer::singleShot(50, this, [this]() {
                if (m_layerManager) {
                    qDebug() << "[Refresh] Refreshing annotation layer after data refresh";
                    m_layerManager->refreshLayer(LayerManager::Labels);
                }
            });
        }
        
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
        
        // 撤销后处理待保存变更列表
        handleUndoForPendingChanges();
        
        // 撤销后刷新标注层，确保标注与图形项同步
        QTimer::singleShot(50, this, [this]() {
            if (m_layerManager) {
                m_layerManager->refreshLayer(LayerManager::Labels);
            }
        });
    } else {
        updateStatus("已撤销到初始状态，无可撤销的操作");
    }
}

// 处理撤销操作对待保存变更列表的影响
void MyForm::handleUndoForPendingChanges()
{
    if (!mapScene) {
        return;
    }
    
    // 获取场景中所有图形项
    QList<QGraphicsItem*> allSceneItems = mapScene->items();
    QSet<QGraphicsItem*> sceneItemSet;
    for (QGraphicsItem *item : allSceneItems) {
        sceneItemSet.insert(item);
    }
    
    bool hasChanges = false;
    
    // 检查待保存变更列表中的 ChangeAdded 记录
    // 如果图形项不在场景中，说明被撤销了，需要从待保存列表中移除
    for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
        PendingChange &change = m_pendingChanges[i];
        
        // 只处理 ChangeAdded 类型的变更
        if (change.type == ChangeAdded && change.graphicsItem) {
            // 检查图形项是否还在场景中
            if (!sceneItemSet.contains(change.graphicsItem)) {
                // 图形项不在场景中，说明被撤销了
                QString entityType = change.entityType;
                QString entityId;
                
                if (entityType == "pipeline") {
                    Pipeline pipeline = change.data.value<Pipeline>();
                    entityId = pipeline.pipelineId();
                } else if (entityType == "facility") {
                    Facility facility = change.data.value<Facility>();
                    entityId = facility.facilityId();
                }
                
                qDebug() << "[Undo] Removing ChangeAdded from pending changes:"
                         << entityType << entityId;
                m_pendingChanges.removeAt(i);
                hasChanges = true;
            }
        }
    }
    
    // 检查已保存到数据库的实体（通过检查图形项的数据库ID）
    // 如果图形项不在场景中，但数据库ID存在，说明已经保存到数据库，需要标记为待删除
    QList<QGraphicsItem*> itemsToCheck;
    
    // 检查管线
    for (auto it = m_drawnPipelines.constBegin(); it != m_drawnPipelines.constEnd(); ++it) {
        QGraphicsItem *item = it.key();
        if (!sceneItemSet.contains(item)) {
            // 这个管线项不在场景中，可能被撤销了
            Pipeline pipeline = it.value();
            
            // 检查是否已经在待保存列表中
            bool foundInPending = false;
            for (const PendingChange &change : m_pendingChanges) {
                if (change.graphicsItem == item) {
                    foundInPending = true;
                    break;
                }
            }
            
            // 如果不在待保存列表中，且数据库ID存在，说明已经保存到数据库
            // 需要添加 ChangeDeleted 记录
            if (!foundInPending && pipeline.id() > 0) {
                // 检查是否已经有 ChangeDeleted 记录
                bool hasDeleteRecord = false;
                for (const PendingChange &change : m_pendingChanges) {
                    if (change.type == ChangeDeleted && 
                        change.entityType == "pipeline" &&
                        change.originalId == pipeline.id()) {
                        hasDeleteRecord = true;
                        break;
                    }
                }
                
                if (!hasDeleteRecord) {
                    PendingChange change;
                    change.type = ChangeDeleted;
                    change.entityType = "pipeline";
                    change.data = QVariant::fromValue(pipeline);
                    change.originalId = pipeline.id();
                    change.graphicsItem = nullptr;  // 项已不在场景中
                    m_pendingChanges.append(change);
                    markAsModified();
                    hasChanges = true;
                    qDebug() << "[Undo] Added ChangeDeleted for pipeline:" 
                             << pipeline.pipelineId() << "id:" << pipeline.id();
                }
            }
        }
    }
    
    // 检查设施：处理已保存设施的撤销删除操作
    // 场景：用户删除了已保存的设施，保存后，然后撤销删除操作
    // 需求：撤销删除后，再保存时应该恢复设施到数据库（添加 ChangeAdded 记录）
    // 1. 如果设施在场景中，且有 ChangeDeleted 记录，说明撤销了删除操作
    //    移除 ChangeDeleted 记录，然后添加 ChangeAdded 记录（这样保存时会恢复设施）
    // 2. 如果设施在场景中，且没有 ChangeDeleted 记录，但已保存到数据库（或已从数据库删除）
    //    应该添加 ChangeAdded 记录（用于恢复）
    
    // 首先，检查场景中的设施，如果有 ChangeDeleted 记录，移除它，然后添加 ChangeAdded 记录
    for (QGraphicsItem *item : allSceneItems) {
        if (item->data(0).toString() == "facility") {
            QString facilityId = item->data(1).toString();
            
            // 检查是否有 ChangeDeleted 记录（通过 facilityId 或 graphicsItem 匹配）
            bool hadDeleteRecord = false;
            Facility deletedFacility;
            for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
                PendingChange &change = m_pendingChanges[i];
                if (change.type == ChangeDeleted && 
                    change.entityType == "facility" &&
                    (change.graphicsItem == item || 
                     change.data.value<Facility>().facilityId() == facilityId)) {
                    // 设施在场景中，且有 ChangeDeleted 记录，说明撤销了删除操作
                    // 保存设施信息，然后移除 ChangeDeleted 记录
                    deletedFacility = change.data.value<Facility>();
                    hadDeleteRecord = true;
                    qDebug() << "[Undo] Removing ChangeDeleted for facility (undo delete):" 
                             << deletedFacility.facilityId();
                    m_pendingChanges.removeAt(i);
                    markAsModified();
                    hasChanges = true;
                    break;
                }
            }
            
            // 如果设施在场景中，且设施ID不是临时ID，需要检查是否应该添加 ChangeAdded 记录
            // 两种情况需要添加：
            // 1. 撤销了删除操作（hadDeleteRecord = true）- 有 ChangeDeleted 记录
            // 2. 设施不在数据库中（已删除），但之前保存过（有数据库ID）
            if (!facilityId.isEmpty() && !facilityId.startsWith("TEMP_")) {
                // 检查设施是否在数据库中
                FacilityDAO dao;
                Facility existingFacility = dao.findByFacilityId(facilityId);
                bool facilityExistsInDb = existingFacility.isValid() && existingFacility.id() > 0;
                
                // 只有当撤销了删除操作，或者设施不在数据库中时，才考虑添加 ChangeAdded
                bool shouldAdd = hadDeleteRecord || !facilityExistsInDb;
                
                if (shouldAdd && !facilityExistsInDb) {
                // 检查是否已经有 ChangeAdded 记录（避免重复）
                bool hasAddedRecord = false;
                for (const PendingChange &change : m_pendingChanges) {
                    if (change.type == ChangeAdded && 
                        change.entityType == "facility" &&
                        change.graphicsItem == item) {
                        hasAddedRecord = true;
                        break;
                    }
                }
                
                if (!hasAddedRecord) {
                    Facility facility;
                    
                    // 如果有之前删除时保存的设施信息，优先使用它（包含数据库ID和完整信息）
                    if (hadDeleteRecord && deletedFacility.isValid()) {
                        facility = deletedFacility;
                        qDebug() << "[Undo] Using facility data from ChangeDeleted record, id:" << facility.id();
                        
                        // 确保从图形项获取最新的名称（如果图形项中有）
                        QString facilityName = item->data(3).toString();
                        if (!facilityName.isEmpty() && facility.facilityName().isEmpty()) {
                            facility.setFacilityName(facilityName);
                            qDebug() << "[Undo] Updated facility name from graphics item:" << facilityName;
                        }
                    } else {
                        // 否则尝试从数据库查询完整信息（如果设施还在数据库中）
                        FacilityDAO dao;
                        Facility dbFacility = dao.findByFacilityId(facilityId);
                        if (dbFacility.isValid() && dbFacility.id() > 0) {
                            facility = dbFacility;
                            qDebug() << "[Undo] Got facility data from database, id:" << facility.id();
                        } else {
                            // 如果数据库中没有，说明已经删除了，需要从图形项重建完整信息
                            QString facilityName = item->data(3).toString();
                            QString facilityType = item->data(2).toString();
                            
                            facility.setFacilityId(facilityId);
                            facility.setFacilityName(facilityName);
                            facility.setFacilityType(facilityType);
                            
                            // 尝试从图形项获取数据库ID
                            QVariant dbIdVariant = item->data(10);  // 优先从 data(10) 获取
                            if (!dbIdVariant.isValid() || dbIdVariant.toInt() <= 0) {
                                dbIdVariant = item->data(1);  // 如果 data(10) 没有，尝试 data(1)
                            }
                            
                            if (dbIdVariant.isValid() && dbIdVariant.toInt() > 0) {
                                facility.setId(dbIdVariant.toInt());
                                qDebug() << "[Undo] Got facility database ID from item data:" << facility.id();
                            }
                            
                            // 从图形项中获取几何信息
                            QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
                            if (ellipseItem) {
                                QRectF rect = ellipseItem->rect();
                                QPointF centerScene = ellipseItem->pos() + rect.center();
                                
                                // 转换为地理坐标
                                QPointF geoCoord;
                                if (tileMapManager) {
                                    geoCoord = tileMapManager->sceneToGeo(centerScene);
                                } else {
                                    geoCoord = centerScene;
                                }
                                
                                // 生成 WKT 格式
                                QString wkt = QString("POINT(%1 %2)")
                                                .arg(geoCoord.x(), 0, 'f', 8)
                                                .arg(geoCoord.y(), 0, 'f', 8);
                                
                                facility.setCoordinate(geoCoord);
                                facility.setGeomWkt(wkt);
                            }
                            
                            qDebug() << "[Undo] Facility not found in database, rebuilding from graphics item, name:" << facilityName;
                        }
                    }
                    
                    PendingChange change;
                    change.type = ChangeAdded;
                    change.entityType = "facility";
                    change.data = QVariant::fromValue(facility);
                    change.originalId = facility.id() > 0 ? facility.id() : -1;
                    change.graphicsItem = item;  // 关联图形项
                    m_pendingChanges.append(change);
                    markAsModified();
                    hasChanges = true;
                    qDebug() << "[Undo] Added ChangeAdded for facility (will restore on save):" 
                             << facility.facilityId() << "id:" << facility.id();
                }
                }  // 结束 if (shouldAdd && !facilityExistsInDb)
            }  // 结束 if (!facilityId.isEmpty() && !facilityId.startsWith("TEMP_"))
        }
    }
    
    // 然后，检查待保存列表中的 ChangeDeleted 记录
    // 如果对应的设施在场景中，说明撤销了删除操作，应该移除并添加 ChangeAdded 记录
    for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
        PendingChange &change = m_pendingChanges[i];
        if (change.type == ChangeDeleted && change.entityType == "facility") {
            // 如果 ChangeDeleted 记录有 graphicsItem，检查它是否在场景中
            if (change.graphicsItem && sceneItemSet.contains(change.graphicsItem)) {
                // 设施在场景中，说明撤销了删除操作
                // 移除 ChangeDeleted 记录，然后添加 ChangeAdded 记录（这样保存时会恢复设施）
                Facility facility = change.data.value<Facility>();
                QString facilityId = facility.facilityId();
                
                qDebug() << "[Undo] Removing ChangeDeleted for facility (item restored):" 
                         << facilityId;
                m_pendingChanges.removeAt(i);
                markAsModified();
                hasChanges = true;
                
                // 检查是否已经有 ChangeAdded 记录（避免重复）
                bool hasAddedRecord = false;
                for (const PendingChange &ch : m_pendingChanges) {
                    if (ch.type == ChangeAdded && 
                        ch.entityType == "facility" &&
                        ch.graphicsItem == change.graphicsItem) {
                        hasAddedRecord = true;
                        break;
                    }
                }
                
                if (!hasAddedRecord) {
                    // 确保设施信息完整：从图形项获取最新的名称（如果图形项中有）
                    if (change.graphicsItem) {
                        QString facilityName = change.graphicsItem->data(3).toString();
                        if (!facilityName.isEmpty() && facility.facilityName().isEmpty()) {
                            facility.setFacilityName(facilityName);
                            qDebug() << "[Undo] Updated facility name from graphics item:" << facilityName;
                        }
                    }
                    
                    // 检查设施是否还在数据库中（如果还在，不应该添加 ChangeAdded，否则会重复）
                    FacilityDAO dao;
                    Facility existingFacility = dao.findByFacilityId(facilityId);
                    if (existingFacility.isValid() && existingFacility.id() > 0) {
                        // 设施还在数据库中，不应该添加 ChangeAdded 记录
                        qDebug() << "[Undo] Facility still exists in database, skipping ChangeAdded:" << facilityId;
                    } else {
                        // 设施已从数据库删除，添加 ChangeAdded 记录以恢复
                        PendingChange newChange;
                        newChange.type = ChangeAdded;
                        newChange.entityType = "facility";
                        newChange.data = QVariant::fromValue(facility);
                        newChange.originalId = facility.id() > 0 ? facility.id() : -1;
                        newChange.graphicsItem = change.graphicsItem;  // 保持关联
                        m_pendingChanges.append(newChange);
                        markAsModified();
                        hasChanges = true;
                        qDebug() << "[Undo] Added ChangeAdded for facility (will restore on save):" 
                                 << facilityId << "id:" << facility.id() << "name:" << facility.facilityName();
                    }
                }
            }
        }
    }
    
    if (hasChanges) {
        qDebug() << "[Undo] Updated pending changes list, count:" << m_pendingChanges.size();
        updateStatus(QString("撤销完成（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
    }
}

// 处理重做操作对待保存变更列表的影响
void MyForm::handleRedoForPendingChanges()
{
    if (!mapScene) {
        return;
    }
    
    // 获取场景中所有图形项
    QList<QGraphicsItem*> allSceneItems = mapScene->items();
    QSet<QGraphicsItem*> sceneItemSet;
    for (QGraphicsItem *item : allSceneItems) {
        sceneItemSet.insert(item);
    }
    
    bool hasChanges = false;
    
    // 检查 m_drawnPipelines 中的管线项
    // 如果图形项在场景中，但不在待保存列表中，说明是重做的未保存实体，需要添加 ChangeAdded 记录
    for (auto it = m_drawnPipelines.constBegin(); it != m_drawnPipelines.constEnd(); ++it) {
        QGraphicsItem *item = it.key();
        Pipeline pipeline = it.value();
        
        // 如果图形项在场景中
        if (sceneItemSet.contains(item)) {
            // 检查是否已经在待保存列表中
            bool foundInPending = false;
            int pendingIndex = -1;
            for (int i = 0; i < m_pendingChanges.size(); ++i) {
                const PendingChange &change = m_pendingChanges[i];
                if (change.graphicsItem == item) {
                    foundInPending = true;
                    pendingIndex = i;
                    break;
                }
            }
            
            // 如果不在待保存列表中，且数据库ID不存在或为0，说明是未保存的实体被重做了
            // 需要添加 ChangeAdded 记录
            if (!foundInPending && pipeline.id() <= 0) {
                PendingChange change;
                change.type = ChangeAdded;
                change.entityType = "pipeline";
                change.data = QVariant::fromValue(pipeline);
                change.originalId = -1;
                change.graphicsItem = item;
                m_pendingChanges.append(change);
                markAsModified();
                hasChanges = true;
                qDebug() << "[Redo] Added ChangeAdded for pipeline:" << pipeline.pipelineId();
            }
            
            // 如果图形项在场景中，且有 ChangeDeleted 记录，说明是已保存的实体被重做了
            // 需要移除 ChangeDeleted 记录
            if (foundInPending && pendingIndex >= 0) {
                PendingChange &change = m_pendingChanges[pendingIndex];
                if (change.type == ChangeDeleted && change.entityType == "pipeline") {
                    qDebug() << "[Redo] Removing ChangeDeleted for pipeline:" << pipeline.pipelineId();
                    m_pendingChanges.removeAt(pendingIndex);
                    markAsModified();
                    hasChanges = true;
                }
            } else if (!foundInPending && pipeline.id() > 0) {
                // 检查是否有 ChangeDeleted 记录（通过 originalId 匹配）
                for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
                    PendingChange &change = m_pendingChanges[i];
                    if (change.type == ChangeDeleted && 
                        change.entityType == "pipeline" &&
                        change.originalId == pipeline.id()) {
                        qDebug() << "[Redo] Removing ChangeDeleted for pipeline:" << pipeline.pipelineId();
                        m_pendingChanges.removeAt(i);
                        markAsModified();
                        hasChanges = true;
                        break;
                    }
                }
            }
        }
    }
    
    // 检查场景中的设施项
    // 如果设施项在场景中，但不在待保存列表中，可能是重做的未保存实体
    for (QGraphicsItem *item : allSceneItems) {
        if (item->data(0).toString() == "facility") {
            QString facilityId = item->data(1).toString();
            
            // 检查是否已经在待保存列表中
            bool foundInPending = false;
            int pendingIndex = -1;
            for (int i = 0; i < m_pendingChanges.size(); ++i) {
                const PendingChange &change = m_pendingChanges[i];
                if (change.graphicsItem == item) {
                    foundInPending = true;
                    pendingIndex = i;
                    break;
                }
            }
            
            // 如果不在待保存列表中，检查是否为未保存的实体被重做了
            // 判断条件：
            // 1. 设施ID是临时ID（以TEMP_开头）
            // 2. 或者图形项的实体状态为 Added
            // 3. 或者设施ID不在数据库中（通过查询数据库确认）
            if (!foundInPending && !facilityId.isEmpty()) {
                // 检查实体状态
                QVariant stateVariant = item->data(100);
                EntityState entityState = EntityState::Detached;
                if (stateVariant.isValid()) {
                    entityState = static_cast<EntityState>(stateVariant.toInt());
                }
                
                bool isUnsavedEntity = false;
                
                // 条件1：设施ID是临时ID
                if (facilityId.startsWith("TEMP_")) {
                    isUnsavedEntity = true;
                }
                // 条件2：实体状态为 Added
                else if (entityState == EntityState::Added) {
                    isUnsavedEntity = true;
                }
                // 条件3：查询数据库确认设施ID不存在
                else {
                    FacilityDAO dao;
                    Facility facility = dao.findByFacilityId(facilityId);
                    if (!facility.isValid() || facility.id() <= 0) {
                        isUnsavedEntity = true;
                    }
                }
                
                if (isUnsavedEntity) {
                    // 从图形项的data中获取设施信息
                    QString facilityName = item->data(3).toString();
                    QString facilityType = item->data(2).toString();
                    
                    Facility facility;
                    facility.setFacilityId(facilityId);
                    facility.setFacilityName(facilityName);
                    facility.setFacilityType(facilityType);
                    
                    // 从图形项中获取几何信息
                    QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
                    if (ellipseItem) {
                        // 获取图形项的中心点（场景坐标）
                        QRectF rect = ellipseItem->rect();
                        QPointF centerScene = ellipseItem->pos() + rect.center();
                        
                        // 转换为地理坐标
                        QPointF geoCoord;
                        if (tileMapManager) {
                            geoCoord = tileMapManager->sceneToGeo(centerScene);
                        } else {
                            // 如果没有瓦片管理器，使用原始坐标（降级方案）
                            geoCoord = centerScene;
                        }
                        
                        // 生成 WKT 格式
                        QString wkt = QString("POINT(%1 %2)")
                                        .arg(geoCoord.x(), 0, 'f', 8)
                                        .arg(geoCoord.y(), 0, 'f', 8);
                        
                        // 设置几何信息
                        facility.setCoordinate(geoCoord);
                        facility.setGeomWkt(wkt);
                        
                        qDebug() << "[Redo] Facility geometry restored from graphics item:" << wkt;
                    }
                    
                    PendingChange change;
                    change.type = ChangeAdded;
                    change.entityType = "facility";
                    change.data = QVariant::fromValue(facility);
                    change.originalId = -1;
                    change.graphicsItem = item;
                    m_pendingChanges.append(change);
                    markAsModified();
                    hasChanges = true;
                    qDebug() << "[Redo] Added ChangeAdded for facility:" << facilityId 
                             << "(state:" << static_cast<int>(entityState) << ")";
                }
            }
            
            // 如果设施项在场景中，且有 ChangeDeleted 记录，说明是已保存的实体被重做了
            // 需要移除 ChangeDeleted 记录
            if (foundInPending && pendingIndex >= 0) {
                PendingChange &change = m_pendingChanges[pendingIndex];
                if (change.type == ChangeDeleted && change.entityType == "facility") {
                    Facility facility = change.data.value<Facility>();
                    qDebug() << "[Redo] Removing ChangeDeleted for facility:" << facility.facilityId();
                    m_pendingChanges.removeAt(pendingIndex);
                    markAsModified();
                    hasChanges = true;
                }
            } else if (!foundInPending && !facilityId.isEmpty() && !facilityId.startsWith("TEMP_")) {
                // 检查是否有 ChangeDeleted 记录（通过 facilityId 匹配）
                // 需要查询数据库获取ID
                FacilityDAO dao;
                Facility facility = dao.findByFacilityId(facilityId);
                if (facility.isValid() && facility.id() > 0) {
                    for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
                        PendingChange &change = m_pendingChanges[i];
                        if (change.type == ChangeDeleted && 
                            change.entityType == "facility" &&
                            change.originalId == facility.id()) {
                            qDebug() << "[Redo] Removing ChangeDeleted for facility:" << facilityId;
                            m_pendingChanges.removeAt(i);
                            markAsModified();
                            hasChanges = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if (hasChanges) {
        qDebug() << "[Redo] Updated pending changes list, count:" << m_pendingChanges.size();
        updateStatus(QString("重做完成（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
    }
}

void MyForm::handleRedoButtonClicked()
{
    if (m_undoStack && m_undoStack->canRedo()) {
        QString text = m_undoStack->redoText();
        m_undoStack->redo();
        qDebug() << "↪️ Redo:" << text;
        updateStatus("重做: " + text);
        
        // 重做后处理待保存变更列表
        handleRedoForPendingChanges();
        
        // 重做后刷新标注层，确保标注与图形项同步
        QTimer::singleShot(50, this, [this]() {
            if (m_layerManager) {
                m_layerManager->refreshLayer(LayerManager::Labels);
            }
        });
    } else {
        updateStatus("已重做到最新状态，无可重做的操作");
    }
}

void MyForm::handleMessageButtonClicked()
{
    qDebug() << "Message button clicked";
    updateStatus("打开消息中心");
    
    // 创建或显示消息对话框
    if (!m_messageDialog) {
        m_messageDialog = new MessageDialog(this);
        connect(m_messageDialog, &MessageDialog::unreadCountChanged, 
                this, &MyForm::onUnreadCountChanged);
    }
    
    m_messageDialog->refreshMessages();
    m_messageDialog->show();
    m_messageDialog->raise();
    m_messageDialog->activateWindow();
}

void MyForm::handleProfileButtonClicked()
{
    qDebug() << "Profile button clicked";
    updateStatus("打开个人信息");
    
    // 创建并显示个人信息对话框
    ProfileDialog *dialog = new ProfileDialog(this);
    dialog->exec();
    delete dialog;
}

void MyForm::onUnreadCountChanged(int count)
{
    qDebug() << "Unread message count changed:" << count;
    // 更新消息按钮图标
    updateMessageButtonIcon(count > 0);
}

// 更新撤销/重做按钮状态
void MyForm::updateUndoRedoButtonStates()
{
    if (!m_undoStack) {
        return;
    }
    
    // 更新按钮启用状态
    if (ui->undoButton) {
        bool canUndo = m_undoStack->canUndo();
        ui->undoButton->setEnabled(canUndo);
        
        // 更新工具提示
        if (canUndo) {
            QString undoText = m_undoStack->undoText();
            ui->undoButton->setToolTip(QString("撤销: %1 (Ctrl+Z)").arg(undoText));
        } else {
            ui->undoButton->setToolTip("撤销 (Ctrl+Z) - 已撤销到初始状态");
        }
    }
    
    if (ui->redoButton) {
        bool canRedo = m_undoStack->canRedo();
        ui->redoButton->setEnabled(canRedo);
        
        // 更新工具提示
        if (canRedo) {
            QString redoText = m_undoStack->redoText();
            ui->redoButton->setToolTip(QString("重做: %1 (Ctrl+Y)").arg(redoText));
        } else {
            ui->redoButton->setToolTip("重做 (Ctrl+Y) - 已重做到最新状态");
        }
    }
}

void MyForm::updateMessageButtonIcon(bool hasMessage)
{
    if (ui->messageButton) {
        if (hasMessage) {
            ui->messageButton->setIcon(QIcon(":/new/prefix1/images/Message.png"));
        } else {
            ui->messageButton->setIcon(QIcon(":/new/prefix1/images/NoMessage.png"));
        }
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
    // 清除高亮管线
    for (auto *item : m_burstHighlights) {
        if (mapScene) mapScene->removeItem(item);
        delete item;
    }
    m_burstHighlights.clear();
    
    // 清除红色虚线区域框
    if (m_burstAreaItem) {
        if (mapScene) mapScene->removeItem(m_burstAreaItem);
        delete m_burstAreaItem;
        m_burstAreaItem = nullptr;
    }
    
    // 清除爆管点标记和标签
    if (m_burstMarker) {
        if (mapScene) mapScene->removeItem(m_burstMarker);
        delete m_burstMarker;
        m_burstMarker = nullptr;
    }
    if (m_burstLabel) {
        if (mapScene) mapScene->removeItem(m_burstLabel);
        delete m_burstLabel;
        m_burstLabel = nullptr;
    }
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
    
    // 清除当前的选中状态，避免影响爆管分析
    clearSelection();
    
    updateStatus("爆管分析：请选择爆管点（左键确认，右键/ESC 取消）");
    ui->graphicsView->setCursor(Qt::CrossCursor);
    setUiEnabledDuringBurst(false);
}

void MyForm::cancelBurstSelectionMode()
{
    m_burstSelectionMode = false;
    ui->graphicsView->unsetCursor();
    
    // 清除选中状态和爆管高亮
    clearSelection();
    clearBurstHighlights();
    
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
        QTimer::singleShot(50, this, &MyForm::loadPipelineData);
        
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
    
    // 延迟刷新标注图层，确保场景中的图形项已经渲染完成
    // 标注渲染器需要从场景中查找图形项，所以需要在其他图层渲染完成后再渲染标注
    if (m_layerManager && m_layerManager->isLayerVisible(LayerManager::Labels)) {
        QTimer::singleShot(50, this, [this]() {
            if (m_layerManager) {
                qDebug() << "[Pipeline] Refreshing annotation layer after data load";
                m_layerManager->refreshLayer(LayerManager::Labels);
            }
        });
    }
    
    checkPipelineRenderResult();
}

void MyForm::checkPipelineRenderResult()
{
    // 延迟检查渲染结果，让渲染器有时间完成
    QTimer::singleShot(100, this, [this]() {
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
    
    // 清除选中状态，避免影响后续操作
    clearSelection();
    
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

// 距离量算相关函数
void MyForm::startDistanceMeasureMode()
{
    // 如果已经在量算模式，不清除之前的测量结果，直接开始新的测量
    if (m_distanceMeasureMode) {
        // 重置当前正在绘制的点
        m_currentDistancePoints.clear();
        clearDistancePreview();
        // 清除当前标记（如果有未完成的测量）
        if (mapScene) {
            for (QGraphicsItem *item : m_currentDistanceMarkers) {
                mapScene->removeItem(item);
                delete item;
            }
            m_currentDistanceMarkers.clear();
        }
        updateStatus("距离量算模式：左键点击添加测量点，右键完成测量");
        return;
    }
    
    // 取消其他模式
    if (m_areaMeasureMode) {
        cancelAreaMeasureMode();
    }
    if (m_shortestPathSelectionMode) {
        cancelShortestPathSelectionMode();
    }
    if (m_burstSelectionMode) {
        cancelBurstSelectionMode();
    }
    if (m_connectivitySelectionMode) {
        cancelConnectivitySelectionMode();
    }
    
    m_distanceMeasureMode = true;
    m_currentDistancePoints.clear();
    m_currentDistanceMarkers.clear();
    clearDistancePreview();
    
    ui->graphicsView->setCursor(Qt::CrossCursor);
    updateStatus("距离量算模式：左键点击添加测量点，右键完成测量");
}

void MyForm::cancelDistanceMeasureMode()
{
    if (!m_distanceMeasureMode) {
        return;
    }
    
    m_distanceMeasureMode = false;
    // 完成当前测量（如果有足够的点）
    if (m_currentDistancePoints.size() >= 2) {
        finishCurrentDistanceMeasure();
    } else {
        // 清除未完成的测量
        clearDistancePreview();
        if (mapScene) {
            for (QGraphicsItem *item : m_currentDistanceMarkers) {
                mapScene->removeItem(item);
                delete item;
            }
            m_currentDistanceMarkers.clear();
        }
        m_currentDistancePoints.clear();
    }
    ui->graphicsView->setCursor(Qt::ArrowCursor);
    updateStatus("距离量算完成，结果已显示在地图上（点击清除测量可清除所有，右键线条可删除单条）");
}

void MyForm::handleDistanceMeasurePoint(const QPointF &geoLonLat)
{
    if (!m_distanceMeasureMode || !mapScene || !tileMapManager) {
        return;
    }
    
    m_currentDistancePoints.append(geoLonLat);
    QPointF scenePos = tileMapManager->geoToScene(geoLonLat.x(), geoLonLat.y());
    
    // 添加标记点
    QGraphicsEllipseItem *marker = mapScene->addEllipse(
        scenePos.x() - 6, scenePos.y() - 6, 12, 12,
        QPen(QColor(255, 0, 0), 2), QBrush(QColor(255, 0, 0, 100))
    );
    marker->setZValue(1e5);
    // 设置数据标识，用于右键菜单识别
    marker->setData(0, "distance_measure_marker");
    m_currentDistanceMarkers.append(marker);
    
    // 更新当前距离线
    updateCurrentDistanceLine();
    
    // 更新状态
    if (m_currentDistancePoints.size() >= 2) {
        double totalDistance = 0;
        for (int i = 1; i < m_currentDistancePoints.size(); i++) {
            totalDistance += calculateDistance(m_currentDistancePoints[i-1], m_currentDistancePoints[i]);
        }
        updateStatus(QString("距离量算：已添加 %1 个点，总距离：%2 米（右键完成）")
                     .arg(m_currentDistancePoints.size())
                     .arg(totalDistance, 0, 'f', 2));
    } else {
        updateStatus(QString("距离量算：已添加 %1 个点（至少需要2个点，右键完成）")
                     .arg(m_currentDistancePoints.size()));
    }
}

void MyForm::finishCurrentDistanceMeasure()
{
    if (m_currentDistancePoints.size() < 2 || !mapScene || !tileMapManager) {
        return;
    }
    
    // 创建测量结果
    DistanceMeasureResult result;
    result.points = m_currentDistancePoints;
    result.markers = m_currentDistanceMarkers;
    
    // 计算总距离
    double totalDistance = 0;
    for (int i = 1; i < m_currentDistancePoints.size(); i++) {
        totalDistance += calculateDistance(m_currentDistancePoints[i-1], m_currentDistancePoints[i]);
    }
    result.totalDistance = totalDistance;
    
    // 创建距离线
    QPainterPath path;
    for (int i = 0; i < m_currentDistancePoints.size(); i++) {
        QPointF scenePos = tileMapManager->geoToScene(m_currentDistancePoints[i].x(), m_currentDistancePoints[i].y());
        if (i == 0) {
            path.moveTo(scenePos);
        } else {
            path.lineTo(scenePos);
        }
    }
    
    QGraphicsPathItem *lineItem = new QGraphicsPathItem(path);
    lineItem->setPen(QPen(QColor(255, 0, 0), 2, Qt::DashLine));
    lineItem->setZValue(1e4);
    // 设置数据标识，用于右键菜单识别
    lineItem->setData(0, "distance_measure_line");
    mapScene->addItem(lineItem);
    result.line = lineItem;
    
    // 创建距离标签
    QPointF lastScenePos = tileMapManager->geoToScene(
        m_currentDistancePoints.last().x(), m_currentDistancePoints.last().y()
    );
    QGraphicsTextItem *label = new QGraphicsTextItem(
        QString("总距离: %1 米").arg(totalDistance, 0, 'f', 2)
    );
    label->setPos(lastScenePos.x() + 10, lastScenePos.y() - 20);
    label->setDefaultTextColor(QColor(255, 0, 0));
    label->setZValue(1e5);
    QFont font = label->font();
    font.setBold(true);
    font.setPointSize(10);
    label->setFont(font);
    label->setData(0, "distance_measure_label");
    mapScene->addItem(label);
    result.label = label;
    
    // 保存到结果列表
    m_distanceMeasureResults.append(result);
    
    // 清空当前绘制状态
    // 清除临时线条（如果存在）
    if (m_currentDistanceLine) {
        mapScene->removeItem(m_currentDistanceLine);
        delete m_currentDistanceLine;
        m_currentDistanceLine = nullptr;
    }
    m_currentDistancePoints.clear();
    m_currentDistanceMarkers.clear();
    clearDistancePreview();
}

void MyForm::updateCurrentDistanceLine()
{
    if (!mapScene || !tileMapManager || m_currentDistancePoints.size() < 2) {
        // 清除临时线条
        if (m_currentDistanceLine) {
            mapScene->removeItem(m_currentDistanceLine);
            delete m_currentDistanceLine;
            m_currentDistanceLine = nullptr;
        }
        return;
    }
    
    // 清除旧的临时线条
    if (m_currentDistanceLine) {
        mapScene->removeItem(m_currentDistanceLine);
        delete m_currentDistanceLine;
        m_currentDistanceLine = nullptr;
    }
    
    // 创建当前正在绘制的距离线（临时显示）
    QPainterPath path;
    for (int i = 0; i < m_currentDistancePoints.size(); i++) {
        QPointF scenePos = tileMapManager->geoToScene(m_currentDistancePoints[i].x(), m_currentDistancePoints[i].y());
        if (i == 0) {
            path.moveTo(scenePos);
        } else {
            path.lineTo(scenePos);
        }
    }
    
    QGraphicsPathItem *lineItem = new QGraphicsPathItem(path);
    lineItem->setPen(QPen(QColor(255, 0, 0), 2, Qt::DashLine));
    lineItem->setZValue(1e4);
    mapScene->addItem(lineItem);
    m_currentDistanceLine = lineItem;
}

void MyForm::updateDistancePreview(const QPointF &mouseGeoPos)
{
    if (!m_distanceMeasureMode || !mapScene || !tileMapManager || m_currentDistancePoints.isEmpty()) {
        return;
    }
    
    // 清除旧的预览线
    clearDistancePreview();
    
    // 创建预览线（从最后一个点到鼠标位置）
    QPointF lastScenePos = tileMapManager->geoToScene(
        m_currentDistancePoints.last().x(), m_currentDistancePoints.last().y()
    );
    QPointF mouseScenePos = tileMapManager->geoToScene(mouseGeoPos.x(), mouseGeoPos.y());
    
    QPainterPath previewPath;
    previewPath.moveTo(lastScenePos);
    previewPath.lineTo(mouseScenePos);
    
    QGraphicsPathItem *previewItem = new QGraphicsPathItem(previewPath);
    previewItem->setPen(QPen(QColor(255, 0, 0, 150), 2, Qt::DashDotLine));
    previewItem->setZValue(1e4 + 1); // 比正式线稍高，但低于标记
    mapScene->addItem(previewItem);
    m_distancePreviewLine = previewItem;
}

void MyForm::clearDistancePreview()
{
    if (mapScene && m_distancePreviewLine) {
        mapScene->removeItem(m_distancePreviewLine);
        delete m_distancePreviewLine;
        m_distancePreviewLine = nullptr;
    }
}

void MyForm::clearAllDistanceMeasures()
{
    if (mapScene) {
        // 清除所有已完成的测量结果
        for (DistanceMeasureResult &result : m_distanceMeasureResults) {
            for (QGraphicsItem *item : result.markers) {
                mapScene->removeItem(item);
                delete item;
            }
            if (result.line) {
                mapScene->removeItem(result.line);
                delete result.line;
            }
            if (result.label) {
                mapScene->removeItem(result.label);
                delete result.label;
            }
        }
        m_distanceMeasureResults.clear();
        
        // 清除当前正在绘制的点
        for (QGraphicsItem *item : m_currentDistanceMarkers) {
            mapScene->removeItem(item);
            delete item;
        }
        m_currentDistanceMarkers.clear();
        if (m_currentDistanceLine) {
            mapScene->removeItem(m_currentDistanceLine);
            delete m_currentDistanceLine;
            m_currentDistanceLine = nullptr;
        }
        m_currentDistancePoints.clear();
        
        clearDistancePreview();
    }
}

void MyForm::deleteDistanceMeasure(QGraphicsItem *item)
{
    if (!mapScene || !item) {
        return;
    }
    
    // 查找包含该图形项的测量结果
    for (int i = 0; i < m_distanceMeasureResults.size(); i++) {
        DistanceMeasureResult &result = m_distanceMeasureResults[i];
        
        // 检查是否是这条线的标记、线或标签
        bool isThisMeasure = false;
        if (result.line == item || result.label == item) {
            isThisMeasure = true;
        } else {
            for (QGraphicsItem *marker : result.markers) {
                if (marker == item) {
                    isThisMeasure = true;
                    break;
                }
            }
        }
        
        if (isThisMeasure) {
            // 删除这条测量的所有图形项
            // 先删除标记点
            for (QGraphicsItem *marker : result.markers) {
                if (marker) {
                    // removeItem会检查项是否在scene中，如果不在则不做任何事
                    mapScene->removeItem(marker);
                    delete marker;
                }
            }
            result.markers.clear();
            
            // 删除线条
            if (result.line) {
                mapScene->removeItem(result.line);
                delete result.line;
                result.line = nullptr;
            }
            
            // 删除标签
            if (result.label) {
                mapScene->removeItem(result.label);
                delete result.label;
                result.label = nullptr;
            }
            
            // 从列表中移除
            m_distanceMeasureResults.removeAt(i);
            updateStatus("已删除距离测量结果");
            return;
        }
    }
}

double MyForm::calculateDistance(const QPointF &p1, const QPointF &p2)
{
    // 使用Haversine公式计算两点间的大圆距离（米）
    const double R = 6371000.0; // 地球半径（米）
    double lat1 = p1.y() * M_PI / 180.0;
    double lat2 = p2.y() * M_PI / 180.0;
    double dLat = (p2.y() - p1.y()) * M_PI / 180.0;
    double dLon = (p2.x() - p1.x()) * M_PI / 180.0;
    
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat1) * cos(lat2) *
               sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return R * c;
}

// 面积量算相关函数
void MyForm::startAreaMeasureMode()
{
    // 如果已经在量算模式，不清除之前的测量结果，直接开始新的测量
    if (m_areaMeasureMode) {
        // 重置当前正在绘制的点
        m_currentAreaPoints.clear();
        clearAreaPreview();
        // 清除当前标记（如果有未完成的测量）
        if (mapScene) {
            for (QGraphicsItem *item : m_currentAreaMarkers) {
                mapScene->removeItem(item);
                delete item;
            }
            m_currentAreaMarkers.clear();
        }
        updateStatus("面积量算模式：左键点击添加测量点（至少3个点），右键完成测量");
        return;
    }
    
    // 取消其他模式
    if (m_distanceMeasureMode) {
        cancelDistanceMeasureMode();
    }
    if (m_shortestPathSelectionMode) {
        cancelShortestPathSelectionMode();
    }
    if (m_burstSelectionMode) {
        cancelBurstSelectionMode();
    }
    if (m_connectivitySelectionMode) {
        cancelConnectivitySelectionMode();
    }
    
    m_areaMeasureMode = true;
    m_currentAreaPoints.clear();
    m_currentAreaMarkers.clear();
    clearAreaPreview();
    
    ui->graphicsView->setCursor(Qt::CrossCursor);
    updateStatus("面积量算模式：左键点击添加测量点（至少3个点），右键完成测量");
}

void MyForm::cancelAreaMeasureMode()
{
    if (!m_areaMeasureMode) {
        return;
    }
    
    m_areaMeasureMode = false;
    // 完成当前测量（如果有足够的点）
    if (m_currentAreaPoints.size() >= 3) {
        finishCurrentAreaMeasure();
    } else {
        // 清除未完成的测量
        clearAreaPreview();
        if (mapScene) {
            for (QGraphicsItem *item : m_currentAreaMarkers) {
                mapScene->removeItem(item);
                delete item;
            }
            m_currentAreaMarkers.clear();
        }
        m_currentAreaPoints.clear();
    }
    ui->graphicsView->setCursor(Qt::ArrowCursor);
    updateStatus("面积量算完成，结果已显示在地图上（点击清除测量可清除所有，右键线条可删除单条）");
}

void MyForm::handleAreaMeasurePoint(const QPointF &geoLonLat)
{
    if (!m_areaMeasureMode || !mapScene || !tileMapManager) {
        return;
    }
    
    m_currentAreaPoints.append(geoLonLat);
    QPointF scenePos = tileMapManager->geoToScene(geoLonLat.x(), geoLonLat.y());
    
    // 添加标记点
    QGraphicsEllipseItem *marker = mapScene->addEllipse(
        scenePos.x() - 6, scenePos.y() - 6, 12, 12,
        QPen(QColor(0, 0, 255), 2), QBrush(QColor(0, 0, 255, 100))
    );
    marker->setZValue(1e5);
    // 设置数据标识，用于右键菜单识别
    marker->setData(0, "area_measure_marker");
    m_currentAreaMarkers.append(marker);
    
    // 更新当前面积多边形
    updateCurrentAreaPolygon();
    
    // 更新状态（现在支持2个点就显示预览）
    if (m_currentAreaPoints.size() >= 3) {
        double area = calculateArea(m_currentAreaPoints);
        updateStatus(QString("面积量算：已添加 %1 个点，面积：%2 平方米（%3 亩）（右键完成）")
                     .arg(m_currentAreaPoints.size())
                     .arg(area, 0, 'f', 2)
                     .arg(area / 666.67, 0, 'f', 2));
    } else if (m_currentAreaPoints.size() >= 2) {
        updateStatus(QString("面积量算：已添加 %1 个点（至少需要3个点计算面积，右键完成）")
                     .arg(m_currentAreaPoints.size()));
    } else {
        updateStatus(QString("面积量算：已添加 %1 个点（至少需要3个点，右键完成）")
                     .arg(m_currentAreaPoints.size()));
    }
}

void MyForm::finishCurrentAreaMeasure()
{
    if (m_currentAreaPoints.size() < 3 || !mapScene || !tileMapManager) {
        return;
    }
    
    // 创建测量结果
    AreaMeasureResult result;
    result.points = m_currentAreaPoints;
    result.markers = m_currentAreaMarkers;
    
    // 计算面积
    double area = calculateArea(m_currentAreaPoints);
    result.area = area;
    
    // 创建多边形
    QPolygonF polygon;
    for (const QPointF &geo : m_currentAreaPoints) {
        QPointF scenePos = tileMapManager->geoToScene(geo.x(), geo.y());
        polygon.append(scenePos);
    }
    // 闭合多边形
    if (!polygon.isEmpty()) {
        polygon.append(polygon.first());
    }
    
    QGraphicsPolygonItem *polyItem = new QGraphicsPolygonItem(polygon);
    polyItem->setPen(QPen(QColor(0, 0, 255), 2, Qt::DashLine));
    polyItem->setBrush(QBrush(QColor(0, 0, 255, 50)));
    polyItem->setZValue(1e4);
    // 设置数据标识，用于右键菜单识别
    polyItem->setData(0, "area_measure_polygon");
    mapScene->addItem(polyItem);
    result.polygon = polyItem;
    
    // 创建面积标签
    QPointF center(0, 0);
    for (const QPointF &geo : m_currentAreaPoints) {
        QPointF scenePos = tileMapManager->geoToScene(geo.x(), geo.y());
        center += scenePos;
    }
    center /= m_currentAreaPoints.size();
    
    QGraphicsTextItem *label = new QGraphicsTextItem(
        QString("面积: %1 平方米\n(%2 亩)")
        .arg(area, 0, 'f', 2)
        .arg(area / 666.67, 0, 'f', 2)
    );
    label->setPos(center);
    label->setDefaultTextColor(QColor(0, 0, 255));
    label->setZValue(1e5);
    QFont font = label->font();
    font.setBold(true);
    font.setPointSize(10);
    label->setFont(font);
    label->setData(0, "area_measure_label");
    mapScene->addItem(label);
    result.label = label;
    
    // 保存到结果列表
    m_areaMeasureResults.append(result);
    
    // 清空当前绘制状态
    // 清除临时多边形（如果存在）
    if (m_currentAreaPolygon) {
        mapScene->removeItem(m_currentAreaPolygon);
        delete m_currentAreaPolygon;
        m_currentAreaPolygon = nullptr;
    }
    m_currentAreaPoints.clear();
    m_currentAreaMarkers.clear();
    clearAreaPreview();
}

void MyForm::updateCurrentAreaPolygon()
{
    if (!mapScene || !tileMapManager || m_currentAreaPoints.size() < 3) {
        // 清除临时多边形
        if (m_currentAreaPolygon) {
            mapScene->removeItem(m_currentAreaPolygon);
            delete m_currentAreaPolygon;
            m_currentAreaPolygon = nullptr;
        }
        return;
    }
    
    // 清除旧的临时多边形
    if (m_currentAreaPolygon) {
        mapScene->removeItem(m_currentAreaPolygon);
        delete m_currentAreaPolygon;
        m_currentAreaPolygon = nullptr;
    }
    
    // 创建当前正在绘制的多边形（临时显示）
    QPolygonF polygon;
    for (const QPointF &geo : m_currentAreaPoints) {
        QPointF scenePos = tileMapManager->geoToScene(geo.x(), geo.y());
        polygon.append(scenePos);
    }
    // 闭合多边形
    if (!polygon.isEmpty()) {
        polygon.append(polygon.first());
    }
    
    QGraphicsPolygonItem *polyItem = new QGraphicsPolygonItem(polygon);
    polyItem->setPen(QPen(QColor(0, 0, 255), 2, Qt::DashLine));
    polyItem->setBrush(QBrush(QColor(0, 0, 255, 50)));
    polyItem->setZValue(1e4);
    mapScene->addItem(polyItem);
    m_currentAreaPolygon = polyItem;
}

void MyForm::updateAreaPreview(const QPointF &mouseGeoPos)
{
    if (!m_areaMeasureMode || !mapScene || !tileMapManager || m_currentAreaPoints.isEmpty()) {
        return;
    }
    
    // 清除旧的预览线
    clearAreaPreview();
    
    // 如果有至少1个点，显示预览
    if (m_currentAreaPoints.size() >= 1) {
        QPointF mouseScenePos = tileMapManager->geoToScene(mouseGeoPos.x(), mouseGeoPos.y());
        
        // 如果只有1个点，只显示从该点到鼠标位置的线
        if (m_currentAreaPoints.size() == 1) {
            QPointF firstScenePos = tileMapManager->geoToScene(
                m_currentAreaPoints.first().x(), m_currentAreaPoints.first().y()
            );
            QPainterPath previewPath;
            previewPath.moveTo(firstScenePos);
            previewPath.lineTo(mouseScenePos);
            
            QGraphicsPathItem *previewItem = new QGraphicsPathItem(previewPath);
            previewItem->setPen(QPen(QColor(0, 0, 255, 150), 2, Qt::DashDotLine));
            previewItem->setZValue(1e4 + 1);
            mapScene->addItem(previewItem);
            m_areaPreviewLine = previewItem;
        }
        // 如果有2个或更多点，显示完整的多边形预览（包括所有已有点和鼠标位置）
        else if (m_currentAreaPoints.size() >= 2) {
            QPainterPath previewPath;
            
            // 先绘制所有已确定的点之间的连线
            for (int i = 0; i < m_currentAreaPoints.size(); i++) {
                QPointF scenePos = tileMapManager->geoToScene(
                    m_currentAreaPoints[i].x(), m_currentAreaPoints[i].y()
                );
                if (i == 0) {
                    previewPath.moveTo(scenePos);
                } else {
                    previewPath.lineTo(scenePos);
                }
            }
            
            // 从最后一个点到鼠标位置
            previewPath.lineTo(mouseScenePos);
            
            // 从鼠标位置回到第一个点，形成闭合预览
            QPointF firstScenePos = tileMapManager->geoToScene(
                m_currentAreaPoints.first().x(), m_currentAreaPoints.first().y()
            );
            previewPath.lineTo(firstScenePos);
            
            // 创建预览多边形（使用PathItem，可以同时显示边线和填充）
            QGraphicsPathItem *previewItem = new QGraphicsPathItem(previewPath);
            previewItem->setPen(QPen(QColor(0, 0, 255, 150), 2, Qt::DashDotLine));
            previewItem->setBrush(QBrush(QColor(0, 0, 255, 30))); // 半透明填充
            previewItem->setZValue(1e4 + 1); // 比正式多边形稍高，但低于标记
            mapScene->addItem(previewItem);
            m_areaPreviewLine = previewItem;
        }
    }
}

void MyForm::clearAreaPreview()
{
    if (mapScene && m_areaPreviewLine) {
        mapScene->removeItem(m_areaPreviewLine);
        delete m_areaPreviewLine;
        m_areaPreviewLine = nullptr;
    }
}

void MyForm::clearAllAreaMeasures()
{
    if (mapScene) {
        // 清除所有已完成的测量结果
        for (AreaMeasureResult &result : m_areaMeasureResults) {
            for (QGraphicsItem *item : result.markers) {
                mapScene->removeItem(item);
                delete item;
            }
            if (result.polygon) {
                mapScene->removeItem(result.polygon);
                delete result.polygon;
            }
            if (result.label) {
                mapScene->removeItem(result.label);
                delete result.label;
            }
        }
        m_areaMeasureResults.clear();
        
        // 清除当前正在绘制的点
        for (QGraphicsItem *item : m_currentAreaMarkers) {
            mapScene->removeItem(item);
            delete item;
        }
        m_currentAreaMarkers.clear();
        if (m_currentAreaPolygon) {
            mapScene->removeItem(m_currentAreaPolygon);
            delete m_currentAreaPolygon;
            m_currentAreaPolygon = nullptr;
        }
        m_currentAreaPoints.clear();
        
        clearAreaPreview();
    }
}

void MyForm::deleteAreaMeasure(QGraphicsItem *item)
{
    if (!mapScene || !item) {
        return;
    }
    
    // 查找包含该图形项的测量结果
    for (int i = 0; i < m_areaMeasureResults.size(); i++) {
        AreaMeasureResult &result = m_areaMeasureResults[i];
        
        // 检查是否是这条测量的标记、多边形或标签
        bool isThisMeasure = false;
        if (result.polygon == item || result.label == item) {
            isThisMeasure = true;
        } else {
            for (QGraphicsItem *marker : result.markers) {
                if (marker == item) {
                    isThisMeasure = true;
                    break;
                }
            }
        }
        
        if (isThisMeasure) {
            // 删除这条测量的所有图形项
            // 先删除标记点
            for (QGraphicsItem *marker : result.markers) {
                if (marker) {
                    // removeItem会检查项是否在scene中，如果不在则不做任何事
                    mapScene->removeItem(marker);
                    delete marker;
                }
            }
            result.markers.clear();
            
            // 删除多边形
            if (result.polygon) {
                mapScene->removeItem(result.polygon);
                delete result.polygon;
                result.polygon = nullptr;
            }
            
            // 删除标签
            if (result.label) {
                mapScene->removeItem(result.label);
                delete result.label;
                result.label = nullptr;
            }
            
            // 从列表中移除
            m_areaMeasureResults.removeAt(i);
            updateStatus("已删除面积测量结果");
            return;
        }
    }
}

double MyForm::calculateArea(const QList<QPointF> &points)
{
    if (points.size() < 3) {
        return 0.0;
    }
    
    // 使用球面多边形面积公式（Shoelace公式的球面版本）
    // 这里使用简化的方法：将经纬度坐标转换为平面坐标后计算
    // 对于小范围区域，可以使用平面近似
    
    double area = 0.0;
    const double R = 6371000.0; // 地球半径（米）
    
    // 使用球面多边形面积公式
    for (int i = 0; i < points.size(); i++) {
        int j = (i + 1) % points.size();
        double lat1 = points[i].y() * M_PI / 180.0;
        double lat2 = points[j].y() * M_PI / 180.0;
        double lon1 = points[i].x() * M_PI / 180.0;
        double lon2 = points[j].x() * M_PI / 180.0;
        
        area += (lon2 - lon1) * (2 + sin(lat1) + sin(lat2));
    }
    
    area = qAbs(area) * R * R / 2.0;
    
    return area;
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
    
    // 连接删除信号，删除后立即刷新地图
    // 使用防抖机制，避免多次删除时重复刷新
    QTimer *refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(true);
    refreshTimer->setInterval(200);  // 200ms 防抖延迟
    
    auto scheduleMapRefresh = [this, refreshTimer]() {
        if (refreshTimer->isActive()) {
            refreshTimer->stop();
        }
        refreshTimer->start();
    };
    
    connect(refreshTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "[Asset Delete] Refreshing map after asset deletion";
        // 刷新所有图层，确保地图显示最新数据
        if (m_layerManager) {
            m_layerManager->refreshAllLayers();
            // 重新加载用户绘制的数据
            if (mapScene) {
                DrawingDatabaseManager::loadFromDatabase(
                    mapScene,
                    m_drawnPipelines,
                    m_nextPipelineId
                );
            }
            // 延迟刷新标注层
            QTimer::singleShot(50, this, [this]() {
                if (m_layerManager && m_layerManager->isLayerVisible(LayerManager::Labels)) {
                    m_layerManager->refreshLayer(LayerManager::Labels);
                }
            });
        }
        // 刷新设备树
        setupDeviceTree();
    });
    
    connect(dialog, &AssetManagerDialog::pipelineDeleted, this, [this, scheduleMapRefresh](int id, const QString &pipelineId) {
        qDebug() << "[Asset Delete] Pipeline deleted:" << pipelineId;
        // 从场景中移除对应的管线图形项
        if (mapScene) {
            QList<QGraphicsItem*> allItems = mapScene->items();
            for (QGraphicsItem *item : allItems) {
                if (item->data(0).toString() == "pipeline" && 
                    item->data(1).toString() == pipelineId) {
                    qDebug() << "[Asset Delete] Removing pipeline graphics item from scene:" << pipelineId;
                    mapScene->removeItem(item);
                    delete item;
                    break;
                }
            }
        }
        scheduleMapRefresh();
    });
    
    connect(dialog, &AssetManagerDialog::facilityDeleted, this, [this, scheduleMapRefresh](int id, const QString &facilityId) {
        qDebug() << "[Asset Delete] Facility deleted:" << facilityId;
        // 从场景中移除对应的设施图形项
        if (mapScene) {
            QList<QGraphicsItem*> allItems = mapScene->items();
            for (QGraphicsItem *item : allItems) {
                if (item->data(0).toString() == "facility" && 
                    item->data(1).toString() == facilityId) {
                    qDebug() << "[Asset Delete] Removing facility graphics item from scene:" << facilityId;
                    mapScene->removeItem(item);
                    delete item;
                    break;
                }
            }
        }
        scheduleMapRefresh();
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
// 工具模块
void MyForm::onDistanceMeasureButtonClicked()
{
    qDebug() << "[UI] Distance Measure button clicked";
    startDistanceMeasureMode();
}

void MyForm::onAreaMeasureButtonClicked()
{
    qDebug() << "[UI] Area Measure button clicked";
    startAreaMeasureMode();
}

void MyForm::onClearMeasureButtonClicked()
{
    qDebug() << "[UI] Clear Measure button clicked";
    clearAllDistanceMeasures();
    clearAllAreaMeasures();
    updateStatus("已清除所有测量结果");
}

// 信息模块
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
    
    HelpDialog dialog(this);
    dialog.exec();
    
    updateStatus("帮助文档已关闭");
}

// 设备树设置 - 从数据库加载真实数据
// 新结构：一级节点=管线和设施，二级节点=各种类型，三级节点=具体设备
void MyForm::setupDeviceTree()
{
    qDebug() << "Setting up device tree from database...";
    
    // 创建模型
    deviceTreeModel = new QStandardItemModel(this);
    
    // 设置模型（不显示表头）
    ui->deviceTreeView->setModel(deviceTreeModel);
    
    // 连接信号
    connect(ui->deviceTreeView, &QTreeView::clicked, this, &MyForm::onDeviceTreeItemClicked);
    connect(ui->deviceTreeView, &QTreeView::doubleClicked, this, &MyForm::onDeviceTreeItemDoubleClicked);
    
    // 启用右键菜单
    ui->deviceTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->deviceTreeView, &QTreeView::customContextMenuRequested, 
            this, &MyForm::onDeviceTreeContextMenuRequested, Qt::UniqueConnection);
    
    // 检查数据库连接
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.isConnected()) {
        qWarning() << "Database not connected, using empty device tree";
        updateStatus("设备树初始化失败 - 数据库未连接");
        return;
    }
    
    // 从数据库加载管线和设施数据
    PipelineDAO pipelineDao;
    FacilityDAO facilityDao;
    
    // 加载所有管线（限制1000条以避免性能问题）
    QVector<Pipeline> allPipelines = pipelineDao.findAll(1000);
    qDebug() << "[DeviceTree] Loaded" << allPipelines.size() << "pipelines from database";
    
    // 加载所有设施（限制1000个）
    QVector<Facility> allFacilities = facilityDao.findAll(1000);
    qDebug() << "[DeviceTree] Loaded" << allFacilities.size() << "facilities from database";
    
    // 定义管线类型映射（数据库类型 -> 显示名称）
    QMap<QString, QString> pipelineTypeMap = {
        {"water_supply", "📘 给水管"},
        {"sewage", "📗 排水管"},
        {"gas", "📙 燃气管"},
        {"electric", "📕 电力电缆"},
        {"telecom", "📒 通信光缆"},
        {"heat", "📓 供热管"}
    };
    
    // 定义设施类型映射（数据库类型 -> 显示名称）
    QMap<QString, QString> facilityTypeMap = {
        {"valve", "🚰 阀门井"},
        {"manhole", "🚪 检查井"},
        {"pump_station", "⚙️ 泵站"},
        {"transformer", "⚡ 变压器"},
        {"regulator", "🔧 调压站"},
        {"junction_box", "📦 接线盒"}
    };
    
    // 按管线类型分组
    QMap<QString, QVector<Pipeline>> pipelinesByType;
    for (const Pipeline &pipeline : allPipelines) {
        QString type = pipeline.pipelineType();
        if (!type.isEmpty()) {
            pipelinesByType[type].append(pipeline);
        }
    }
    
    // 按设施类型分组
    QMap<QString, QVector<Facility>> facilitiesByType;
    for (const Facility &facility : allFacilities) {
        QString type = facility.facilityType();
        if (!type.isEmpty()) {
            facilitiesByType[type].append(facility);
        }
    }
    
    // ========== 创建一级节点：管线和设施 ==========
    
    // 一级节点：管线
    QStandardItem *pipelineRoot = new QStandardItem("🔧 管线");
    pipelineRoot->setEditable(false);
    pipelineRoot->setData("pipeline_root", Qt::UserRole);  // 标记为管线根节点
    deviceTreeModel->appendRow(pipelineRoot);
    
    // 一级节点：设施
    QStandardItem *facilityRoot = new QStandardItem("🏗️ 设施");
    facilityRoot->setEditable(false);
    facilityRoot->setData("facility_root", Qt::UserRole);  // 标记为设施根节点
    deviceTreeModel->appendRow(facilityRoot);
    
    // ========== 创建二级节点：各种管线类型 ==========
    int totalPipelines = 0;
    QStringList pipelineTypeOrder = {"water_supply", "sewage", "gas", "electric", "telecom", "heat"};
    
    for (const QString &type : pipelineTypeOrder) {
        if (!pipelinesByType.contains(type) || pipelinesByType[type].isEmpty()) {
            continue;  // 跳过没有数据的类型
        }
        
        QString typeDisplayName = pipelineTypeMap.value(type, "🔧 " + type);
        QStandardItem *typeItem = new QStandardItem(typeDisplayName);
        typeItem->setEditable(false);
        typeItem->setData(type, Qt::UserRole);  // 存储管线类型
        typeItem->setData("pipeline_type", Qt::UserRole + 1);  // 标记为管线类型节点
        pipelineRoot->appendRow(typeItem);
        
        // ========== 创建三级节点：具体管线 ==========
        for (const Pipeline &pipeline : pipelinesByType[type]) {
            QString displayName = formatPipelineDisplayName(pipeline);
            QStandardItem *pipelineItem = new QStandardItem(displayName);
            pipelineItem->setEditable(false);
            pipelineItem->setData(pipeline.pipelineId(), Qt::UserRole);  // 存储管线ID
            pipelineItem->setData("pipeline", Qt::UserRole + 1);  // 标记为管线
            typeItem->appendRow(pipelineItem);
            totalPipelines++;
        }
    }
    
    // ========== 创建二级节点：各种设施类型 ==========
    int totalFacilities = 0;
    QStringList facilityTypeOrder = {"valve", "manhole", "pump_station", "transformer", "regulator", "junction_box"};
    
    for (const QString &type : facilityTypeOrder) {
        if (!facilitiesByType.contains(type) || facilitiesByType[type].isEmpty()) {
            continue;  // 跳过没有数据的类型
        }
        
        QString typeDisplayName = facilityTypeMap.value(type, "🔧 " + type);
        QStandardItem *typeItem = new QStandardItem(typeDisplayName);
        typeItem->setEditable(false);
        typeItem->setData(type, Qt::UserRole);  // 存储设施类型
        typeItem->setData("facility_type", Qt::UserRole + 1);  // 标记为设施类型节点
        facilityRoot->appendRow(typeItem);
        
        // ========== 创建三级节点：具体设施 ==========
        for (const Facility &facility : facilitiesByType[type]) {
            QString displayName = formatFacilityDisplayName(facility);
            QStandardItem *facilityItem = new QStandardItem(displayName);
            facilityItem->setEditable(false);
            facilityItem->setData(facility.facilityId(), Qt::UserRole);  // 存储设施ID
            facilityItem->setData("facility", Qt::UserRole + 1);  // 标记为设施
            typeItem->appendRow(facilityItem);
            totalFacilities++;
        }
    }
    
    // 处理未映射的设施类型
    for (auto it = facilitiesByType.begin(); it != facilitiesByType.end(); ++it) {
        if (!facilityTypeOrder.contains(it.key())) {
            QString typeDisplayName = "🔧 " + it.key();
            QStandardItem *typeItem = new QStandardItem(typeDisplayName);
            typeItem->setEditable(false);
            typeItem->setData(it.key(), Qt::UserRole);
            typeItem->setData("facility_type", Qt::UserRole + 1);
            facilityRoot->appendRow(typeItem);
            
            for (const Facility &facility : it.value()) {
                QString displayName = formatFacilityDisplayName(facility);
                QStandardItem *facilityItem = new QStandardItem(displayName);
                facilityItem->setEditable(false);
                facilityItem->setData(facility.facilityId(), Qt::UserRole);
                facilityItem->setData("facility", Qt::UserRole + 1);
                typeItem->appendRow(facilityItem);
                totalFacilities++;
            }
        }
    }
    
    // 默认展开第一层（管线和设施）
    if (deviceTreeModel->rowCount() > 0) {
        ui->deviceTreeView->expand(deviceTreeModel->index(0, 0));
        if (deviceTreeModel->rowCount() > 1) {
            ui->deviceTreeView->expand(deviceTreeModel->index(1, 0));
        }
    }
    
    qDebug() << "[DeviceTree] ========== Device Tree Summary ==========";
    qDebug() << "[DeviceTree] Total pipelines loaded:" << allPipelines.size();
    qDebug() << "[DeviceTree] Total facilities loaded:" << allFacilities.size();
    qDebug() << "[DeviceTree] Pipelines in tree:" << totalPipelines;
    qDebug() << "[DeviceTree] Facilities in tree:" << totalFacilities;
    
    if (totalPipelines < allPipelines.size()) {
        qWarning() << "[DeviceTree] ⚠️  Warning: Some pipelines not shown in tree!";
        qWarning() << "[DeviceTree]   Loaded:" << allPipelines.size() << "Displayed:" << totalPipelines;
    }
    if (totalFacilities < allFacilities.size()) {
        qWarning() << "[DeviceTree] ⚠️  Warning: Some facilities not shown in tree!";
        qWarning() << "[DeviceTree]   Loaded:" << allFacilities.size() << "Displayed:" << totalFacilities;
    }
    
    QString statusMsg = QString("设备树加载完成 - %1条管线，%2个设施").arg(totalPipelines).arg(totalFacilities);
    qDebug() << "[DeviceTree]" << statusMsg;
    qDebug() << "[DeviceTree] ==========================================";
    updateStatus(statusMsg);
}

// 格式化管线显示名称
QString MyForm::formatPipelineDisplayName(const Pipeline &pipeline)
{
    QString name = pipeline.pipelineName();
    if (name.isEmpty()) {
        name = pipeline.pipelineId();
    }
    
    // 添加管径信息
    QString diameterInfo;
    if (pipeline.diameterMm() > 0) {
        diameterInfo = QString("DN%1").arg(pipeline.diameterMm());
    }
    
    // 添加状态图标
    QString statusIcon = getStatusIcon(pipeline.status(), pipeline.healthScore());
    
    // 组合显示名称
    QString displayName = "  ";
    if (!diameterInfo.isEmpty()) {
        displayName += diameterInfo + " ";
    }
    displayName += name;
    if (!pipeline.pipelineId().isEmpty()) {
        displayName += "-" + pipeline.pipelineId();
    }
    displayName += " " + statusIcon;
    
    return displayName;
}

// 格式化设施显示名称
QString MyForm::formatFacilityDisplayName(const Facility &facility)
{
    QString name = facility.facilityName();
    if (name.isEmpty()) {
        name = facility.facilityId();
    }
    
    // 添加状态图标
    QString statusIcon = getStatusIcon(facility.status(), facility.healthScore());
    
    // 组合显示名称
    QString displayName = "  " + name;
    if (!facility.facilityId().isEmpty()) {
        displayName += "-" + facility.facilityId();
    }
    displayName += " " + statusIcon;
    
    return displayName;
}

// 获取状态图标
QString MyForm::getStatusIcon(const QString &status, int healthScore)
{
    // 根据状态和健康度返回图标
    QString statusLower = status.toLower();
    
    if (statusLower.contains("active") || statusLower.contains("normal") || statusLower.contains("运行") || statusLower.contains("正常")) {
        if (healthScore >= 80) {
            return "🟢运行中";
        } else if (healthScore >= 60) {
            return "🟡正常";
        } else {
            return "🟠需关注";
        }
    } else if (statusLower.contains("maintenance") || statusLower.contains("维护")) {
        return "🟡维护中";
    } else if (statusLower.contains("closed") || statusLower.contains("关闭") || statusLower.contains("停用")) {
        return "🔴关闭";
    } else if (statusLower.contains("offline") || statusLower.contains("离线")) {
        return "⚫离线";
    } else {
        // 默认根据健康度判断
        if (healthScore >= 80) {
            return "🟢正常";
        } else if (healthScore >= 60) {
            return "🟡一般";
        } else {
            return "🔴异常";
        }
    }
}

// 设备树点击事件（适配新结构）
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
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    switch(level) {
        case 0: 
            levelName = "一级分类";
            if (itemType == "pipeline_root") levelName = "管线分类";
            else if (itemType == "facility_root") levelName = "设施分类";
            break;
        case 1: 
            levelName = "类型";
            if (itemType == "pipeline_type") levelName = "管线类型";
            else if (itemType == "facility_type") levelName = "设施类型";
            break;
        case 2: 
            levelName = "具体设备";
            if (itemType == "pipeline") levelName = "管线";
            else if (itemType == "facility") levelName = "设施";
            break;
        default: 
            levelName = "其他"; 
            break;
    }
    
    qDebug() << "[DeviceTree] Item clicked - Level:" << level << "(" << levelName << ")" << "Text:" << text;
    updateStatus("选中[" + levelName + "]: " + text);
}

// 设备树双击事件（适配新结构）
// 一级和二级节点：展开/收起，三级节点：打开详情
void MyForm::onDeviceTreeItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    // 如果对话框正在显示，忽略双击事件（防止重复触发）
    if (m_deviceTreeDialogActive) {
        qDebug() << "[DeviceTree] Dialog is already open, ignoring double-click";
        return;
    }
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(index);
    if (!item) return;
    
    QString name = item->text();
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    // 判断层级
    int level = 0;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        level++;
        parent = parent.parent();
    }
    
    // 一级节点（管线和设施）和二级节点（类型）：展开/收起
    if (level == 0 || level == 1) {
        // 检查节点是否有子节点
        if (item->rowCount() > 0) {
            // 切换展开/收起状态
            bool isExpanded = ui->deviceTreeView->isExpanded(index);
            if (isExpanded) {
                ui->deviceTreeView->collapse(index);
                qDebug() << "[DeviceTree] Collapsed:" << name;
                updateStatus("收起: " + name);
            } else {
                ui->deviceTreeView->expand(index);
                qDebug() << "[DeviceTree] Expanded:" << name;
                updateStatus("展开: " + name);
            }
        } else {
            // 没有子节点，提示用户
            QString levelName = (level == 0) ? "一级分类" : "类型";
            updateStatus(levelName + " \"" + name + "\" 下暂无设备");
        }
        return;
    }
    
    // 三级节点（具体设备）：打开详情对话框
    if (level == 2 && (itemType == "pipeline" || itemType == "facility")) {
        qDebug() << "[DeviceTree] Opening device details:" << name << "Type:" << itemType;
        updateStatus("打开设备详情: " + name);
        
        // 标记对话框正在显示
        m_deviceTreeDialogActive = true;
        
        // 直接从数据中获取设备ID（新结构）
        QString deviceId = item->data(Qt::UserRole).toString();
        
        // 如果ID为空，尝试从设备名称中提取ID（兼容旧格式）
        if (deviceId.isEmpty()) {
            QStringList parts = name.split("-");
            if (parts.size() >= 2) {
                deviceId = parts.last().split(" ").first(); // 提取 "GS001"
            }
        }
        
        // 尝试查找对应的管线或设施
        if (!deviceId.isEmpty()) {
            if (itemType == "pipeline") {
                PipelineDAO pipelineDao;
                Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
                
                if (pipeline.isValid()) {
                    // 显示管线详情
                    PipelineEditDialog dialog(this);
                    dialog.loadPipeline(pipeline);
                    dialog.exec();
                    // 对话框关闭后重置标志
                    m_deviceTreeDialogActive = false;
                    return;
                }
            } else if (itemType == "facility") {
                FacilityDAO facilityDao;
                Facility facility = facilityDao.findByFacilityId(deviceId);
                
                if (facility.isValid()) {
                    // 显示设施详情
                    FacilityEditDialog dialog(this, facility);
                    dialog.exec();
                    // 对话框关闭后重置标志
                    m_deviceTreeDialogActive = false;
                    return;
                }
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
        
        // 消息框关闭后重置标志
        m_deviceTreeDialogActive = false;
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

// 设备树右键菜单
void MyForm::onDeviceTreeContextMenuRequested(const QPoint &pos)
{
    // 如果菜单正在显示，忽略新的请求（防止重复触发）
    if (m_deviceTreeMenuActive) {
        return;
    }
    
    QModelIndex index = ui->deviceTreeView->indexAt(pos);
    if (!index.isValid()) {
        return;  // 点击空白处，不显示菜单
    }
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(index);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    // 判断层级
    int level = 0;
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        level++;
        parent = parent.parent();
    }
    
    // 标记菜单正在显示
    m_deviceTreeMenuActive = true;
    
    // 创建右键菜单（使用局部变量，确保菜单关闭后不会再次触发）
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
    
    // 存储当前选中的索引，供菜单操作使用
    m_currentDeviceTreeIndex = index;
    
    // 所有层级都有的菜单项
    QAction *refreshAction = contextMenu.addAction("🔄 刷新");
    connect(refreshAction, &QAction::triggered, this, &MyForm::onDeviceTreeRefresh);
    
    contextMenu.addSeparator();
    
    if (level == 0 || level == 1) {
        // 一级和二级节点（分类节点）的菜单
        QAction *expandAllAction = contextMenu.addAction("📂 展开全部");
        QAction *collapseAllAction = contextMenu.addAction("📁 折叠全部");
        
        contextMenu.addSeparator();
        
        QAction *exportAction = contextMenu.addAction("📤 导出数据");
        QAction *statisticsAction = contextMenu.addAction("📊 统计信息");
        
        connect(expandAllAction, &QAction::triggered, this, &MyForm::onDeviceTreeExpandAll);
        connect(collapseAllAction, &QAction::triggered, this, &MyForm::onDeviceTreeCollapseAll);
        connect(exportAction, &QAction::triggered, this, &MyForm::onDeviceTreeExport);
        connect(statisticsAction, &QAction::triggered, this, &MyForm::onDeviceTreeStatistics);
        
    } else if (level == 2 && (itemType == "pipeline" || itemType == "facility")) {
        // 三级节点（具体设备）的菜单
        QAction *viewInfoAction = contextMenu.addAction("📋 查看信息");
        QAction *editAction = contextMenu.addAction("✏️ 编辑");
        
        contextMenu.addSeparator();
        
        QAction *copyAction = contextMenu.addAction("📋 复制");
        QAction *locateAction = contextMenu.addAction("📍 在地图上定位");
        QAction *viewRelatedAction = contextMenu.addAction("🔗 查看关联设备");
        
        contextMenu.addSeparator();
        
        QAction *reportAction = contextMenu.addAction("📄 生成报告");
        QAction *historyAction = contextMenu.addAction("📜 查看历史记录");
        QAction *printLabelAction = contextMenu.addAction("🏷️ 打印标签");
        
        contextMenu.addSeparator();
        
        QAction *deleteAction = contextMenu.addAction("🗑️ 删除设备");
        QFont deleteFont = deleteAction->font();
        deleteFont.setBold(true);
        deleteAction->setFont(deleteFont);
        
        connect(viewInfoAction, &QAction::triggered, this, &MyForm::onDeviceTreeViewInfo);
        connect(editAction, &QAction::triggered, this, &MyForm::onDeviceTreeEdit);
        connect(copyAction, &QAction::triggered, this, &MyForm::onDeviceTreeCopy);
        connect(locateAction, &QAction::triggered, this, &MyForm::onDeviceTreeLocateOnMap);
        connect(viewRelatedAction, &QAction::triggered, this, &MyForm::onDeviceTreeViewRelated);
        connect(reportAction, &QAction::triggered, this, &MyForm::onDeviceTreeGenerateReport);
        connect(historyAction, &QAction::triggered, this, &MyForm::onDeviceTreeViewHistory);
        connect(printLabelAction, &QAction::triggered, this, &MyForm::onDeviceTreePrintLabel);
        connect(deleteAction, &QAction::triggered, this, &MyForm::onDeviceTreeDelete);
    }
    
    // 显示菜单并获取选中的操作
    // 使用exec()显示菜单，菜单会在用户选择或点击外部时自动关闭
    QAction *selectedAction = contextMenu.exec(ui->deviceTreeView->mapToGlobal(pos));
    
    // 菜单已关闭，重置标志
    // 使用单次定时器延迟重置，避免菜单关闭瞬间的事件传播导致再次触发
    QTimer::singleShot(50, this, [this]() {
        m_deviceTreeMenuActive = false;
    });
    
    // 如果用户选择了菜单项，执行相应的操作（操作已在connect中处理）
    // 菜单会自动关闭，不会再次弹出
}

// 过滤设备树（适配新结构：一级=管线和设施，二级=类型，三级=具体设备）
void MyForm::filterDeviceTree(const QString &searchText)
{
    if (searchText.isEmpty()) {
        // 搜索框为空，显示所有节点
        for (int i = 0; i < deviceTreeModel->rowCount(); ++i) {
            QStandardItem *item = deviceTreeModel->item(i);
            setItemVisibility(item, true);
        }
        // 折叠所有节点，只展开第一层（管线和设施）
        ui->deviceTreeView->collapseAll();
        if (deviceTreeModel->rowCount() > 0) {
            ui->deviceTreeView->expand(deviceTreeModel->index(0, 0));
            if (deviceTreeModel->rowCount() > 1) {
                ui->deviceTreeView->expand(deviceTreeModel->index(1, 0));
            }
        }
    } else {
        // 有搜索文本，过滤节点
        QString lowerSearchText = searchText.toLower();
        for (int i = 0; i < deviceTreeModel->rowCount(); ++i) {
            QStandardItem *item = deviceTreeModel->item(i);
            bool hasMatch = filterItem(item, lowerSearchText);
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
// 适配新结构：一级=管线和设施，二级=类型，三级=具体设备
bool MyForm::filterItem(QStandardItem *item, const QString &searchText)
{
    if (!item) return false;
    
    QString itemText = item->text().toLower();
    bool currentMatch = itemText.contains(searchText);
    
    // 对于三级节点（具体设备），还可以搜索设备ID
    QString itemType = item->data(Qt::UserRole + 1).toString();
    if (itemType == "pipeline" || itemType == "facility") {
        QString deviceId = item->data(Qt::UserRole).toString().toLower();
        if (deviceId.contains(searchText)) {
            currentMatch = true;
        }
    }
    
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
    bool shouldShow = currentMatch || hasMatchingChild;
    
    // 设置当前节点可见性
    QModelIndex index = item->index();
    ui->deviceTreeView->setRowHidden(index.row(), index.parent(), !shouldShow);
    
    return shouldShow;
}

// ========== 设备树右键菜单功能实现 ==========

// 刷新设备树
void MyForm::onDeviceTreeRefresh()
{
    qDebug() << "[DeviceTree] Refreshing device tree...";
    setupDeviceTree();
    updateStatus("设备树已刷新");
}

// 展开全部
void MyForm::onDeviceTreeExpandAll()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    // 递归展开所有子节点
    QModelIndex index = m_currentDeviceTreeIndex;
    while (index.isValid()) {
        ui->deviceTreeView->expand(index);
        index = deviceTreeModel->index(0, 0, index);  // 获取第一个子节点
    }
    
    // 展开所有子节点
    expandAllChildren(m_currentDeviceTreeIndex);
    updateStatus("已展开全部子节点");
}

// 折叠全部
void MyForm::onDeviceTreeCollapseAll()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    // 递归折叠所有子节点
    collapseAllChildren(m_currentDeviceTreeIndex);
    updateStatus("已折叠全部子节点");
}

// 导出设备数据
void MyForm::onDeviceTreeExport()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出设备数据", 
                                                    QDir::homePath() + "/设备数据.csv",
                                                    "CSV文件 (*.csv);;所有文件 (*)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建文件: " + fileName);
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // 收集该节点下的所有设备
    QList<Pipeline> pipelines;
    QList<Facility> facilities;
    collectDevicesFromItem(item, pipelines, facilities);
    
    // 写入CSV头部
    if (!pipelines.isEmpty() && !facilities.isEmpty()) {
        // 混合导出：先管线后设施
        out << "类型,设备编号,设备名称,类型,长度(m),管径(mm),材质,状态,健康度,建设日期,负责人\n";
        
        // 导出管线
        PipelineDAO pipelineDao;
        for (const Pipeline &p : pipelines) {
            Pipeline fullPipeline = pipelineDao.findByPipelineId(p.pipelineId());
            if (fullPipeline.isValid()) {
                out << "管线," 
                    << "\"" << fullPipeline.pipelineId() << "\","
                    << "\"" << fullPipeline.pipelineName() << "\","
                    << "\"" << fullPipeline.getTypeDisplayName() << "\","
                    << fullPipeline.lengthM() << ","
                    << fullPipeline.diameterMm() << ","
                    << "\"" << fullPipeline.material() << "\","
                    << "\"" << fullPipeline.status() << "\","
                    << fullPipeline.healthScore() << ","
                    << fullPipeline.buildDate().toString("yyyy-MM-dd") << ","
                    << "\"" << fullPipeline.owner() << "\"\n";
            }
        }
        
        // 导出设施
        FacilityDAO facilityDao;
        for (const Facility &f : facilities) {
            Facility fullFacility = facilityDao.findByFacilityId(f.facilityId());
            if (fullFacility.isValid()) {
                out << "设施,"
                    << "\"" << fullFacility.facilityId() << "\","
                    << "\"" << fullFacility.facilityName() << "\","
                    << "\"" << fullFacility.getTypeDisplayName() << "\","
                    << ",,"  // 长度和管径为空
                    << "\"" << fullFacility.material() << "\","
                    << "\"" << fullFacility.status() << "\","
                    << fullFacility.healthScore() << ","
                    << fullFacility.buildDate().toString("yyyy-MM-dd") << ","
                    << "\"" << fullFacility.owner() << "\"\n";
            }
        }
    } else if (!pipelines.isEmpty()) {
        // 只导出管线
        out << "设备编号,设备名称,类型,长度(m),管径(mm),材质,状态,健康度,建设日期,负责人\n";
        PipelineDAO pipelineDao;
        for (const Pipeline &p : pipelines) {
            Pipeline fullPipeline = pipelineDao.findByPipelineId(p.pipelineId());
            if (fullPipeline.isValid()) {
                out << "\"" << fullPipeline.pipelineId() << "\","
                    << "\"" << fullPipeline.pipelineName() << "\","
                    << "\"" << fullPipeline.getTypeDisplayName() << "\","
                    << fullPipeline.lengthM() << ","
                    << fullPipeline.diameterMm() << ","
                    << "\"" << fullPipeline.material() << "\","
                    << "\"" << fullPipeline.status() << "\","
                    << fullPipeline.healthScore() << ","
                    << fullPipeline.buildDate().toString("yyyy-MM-dd") << ","
                    << "\"" << fullPipeline.owner() << "\"\n";
            }
        }
    } else if (!facilities.isEmpty()) {
        // 只导出设施
        out << "设备编号,设备名称,类型,规格,材质,状态,健康度,建设日期,负责人\n";
        FacilityDAO facilityDao;
        for (const Facility &f : facilities) {
            Facility fullFacility = facilityDao.findByFacilityId(f.facilityId());
            if (fullFacility.isValid()) {
                out << "\"" << fullFacility.facilityId() << "\","
                    << "\"" << fullFacility.facilityName() << "\","
                    << "\"" << fullFacility.getTypeDisplayName() << "\","
                    << "\"" << fullFacility.spec() << "\","
                    << "\"" << fullFacility.material() << "\","
                    << "\"" << fullFacility.status() << "\","
                    << fullFacility.healthScore() << ","
                    << fullFacility.buildDate().toString("yyyy-MM-dd") << ","
                    << "\"" << fullFacility.owner() << "\"\n";
            }
        }
    }
    
    file.close();
    
    QString msg = QString("已导出 %1 条管线，%2 个设施到: %3")
                  .arg(pipelines.size())
                  .arg(facilities.size())
                  .arg(fileName);
    QMessageBox::information(this, "导出成功", msg);
    updateStatus(QString("导出完成: %1条管线, %2个设施").arg(pipelines.size()).arg(facilities.size()));
}

// 统计信息
void MyForm::onDeviceTreeStatistics()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    // 获取节点类型（一级节点存储在Qt::UserRole，二级和三级节点存储在Qt::UserRole + 1）
    QString rootType = item->data(Qt::UserRole).toString();
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    // 判断是管线类型还是设施类型
    bool isPipelineType = (rootType == "pipeline_root" || itemType == "pipeline_type" || itemType == "pipeline");
    bool isFacilityType = (rootType == "facility_root" || itemType == "facility_type" || itemType == "facility");
    
    QString itemName = item->text();
    QString stats;
    
    if (isPipelineType) {
        // 管线类型节点：只统计管线数量
        int pipelineCount = 0;
        int facilityCount = 0;  // 不需要，但countDevices需要这个参数
        countDevices(item, pipelineCount, facilityCount);
        
        stats = QString("统计信息 - %1\n\n管线数量: %2")
                        .arg(itemName)
                        .arg(pipelineCount);
        
        QMessageBox::information(this, "统计信息", stats);
        updateStatus(QString("统计: %1条管线").arg(pipelineCount));
    } else if (isFacilityType) {
        // 设施类型节点：只统计设施数量
        int pipelineCount = 0;  // 不需要，但countDevices需要这个参数
        int facilityCount = 0;
        countDevices(item, pipelineCount, facilityCount);
        
        stats = QString("统计信息 - %1\n\n设施数量: %2")
                        .arg(itemName)
                        .arg(facilityCount);
        
        QMessageBox::information(this, "统计信息", stats);
        updateStatus(QString("统计: %1个设施").arg(facilityCount));
    } else {
        // 未知类型：显示两种统计（兼容处理）
        int pipelineCount = 0;
        int facilityCount = 0;
        countDevices(item, pipelineCount, facilityCount);
        
        stats = QString("统计信息 - %1\n\n管线数量: %2\n设施数量: %3")
                        .arg(itemName)
                        .arg(pipelineCount)
                        .arg(facilityCount);
        
        QMessageBox::information(this, "统计信息", stats);
        updateStatus(QString("统计: %1条管线, %2个设施").arg(pipelineCount).arg(facilityCount));
    }
}

// 查看设备信息（只读）
void MyForm::onDeviceTreeViewInfo()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    // 如果对话框正在显示，忽略请求（防止重复触发）
    if (m_deviceTreeDialogActive) {
        return;
    }
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    if (itemType != "pipeline" && itemType != "facility") {
        QMessageBox::information(this, "提示", "只有具体设备才能查看信息。");
        return;
    }
    
    // 标记对话框正在显示
    m_deviceTreeDialogActive = true;
    
    if (itemType == "pipeline") {
        PipelineDAO pipelineDao;
        Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
        if (pipeline.isValid()) {
            PipelineEditDialog dialog(this);
            dialog.loadPipeline(pipeline);
            // 设置为只读模式（如果对话框支持）
            dialog.setWindowTitle("查看管线信息");
            dialog.exec();
        } else {
            QMessageBox::warning(this, "错误", "未找到该管线");
        }
    } else if (itemType == "facility") {
        FacilityDAO facilityDao;
        Facility facility = facilityDao.findByFacilityId(deviceId);
        if (facility.isValid()) {
            FacilityEditDialog dialog(this, facility);
            dialog.setWindowTitle("查看设施信息");
            dialog.exec();
        } else {
            QMessageBox::warning(this, "错误", "未找到该设施");
        }
    }
    
    // 对话框关闭后重置标志
    m_deviceTreeDialogActive = false;
}

// 编辑设备
void MyForm::onDeviceTreeEdit()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    // 如果对话框正在显示，忽略请求（防止重复触发）
    if (m_deviceTreeDialogActive) {
        return;
    }
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    if (itemType != "pipeline" && itemType != "facility") {
        QMessageBox::information(this, "提示", "只有具体设备才能编辑。");
        return;
    }
    
    // 标记对话框正在显示
    m_deviceTreeDialogActive = true;
    
    // 查找地图上的图形项
    QGraphicsItem *graphicsItem = nullptr;
    if (mapScene) {
        QList<QGraphicsItem*> allItems = mapScene->items();
        for (QGraphicsItem *sceneItem : allItems) {
            QString sceneItemType = sceneItem->data(0).toString();
            QString sceneItemId = sceneItem->data(1).toString();
            
            if ((itemType == "pipeline" && sceneItemType == "pipeline" && sceneItemId == deviceId) ||
                (itemType == "facility" && sceneItemType == "facility" && sceneItemId == deviceId)) {
                graphicsItem = sceneItem;
                break;
            }
        }
    }
    
    if (itemType == "pipeline") {
        PipelineDAO pipelineDao;
        Pipeline originalPipeline = pipelineDao.findByPipelineId(deviceId);
        if (originalPipeline.isValid()) {
            PipelineEditDialog dialog(this);
            dialog.loadPipeline(originalPipeline);
            dialog.setWindowTitle("编辑管线信息");
            
            if (dialog.exec() == QDialog::Accepted) {
                Pipeline updatedPipeline = dialog.getPipeline();
                updatedPipeline.setId(originalPipeline.id()); // 确保ID用于更新
                
                // 检查是否有修改
                bool hasChanges = false;
                if (updatedPipeline.pipelineName() != originalPipeline.pipelineName() ||
                    updatedPipeline.pipelineType() != originalPipeline.pipelineType() ||
                    updatedPipeline.diameterMm() != originalPipeline.diameterMm() ||
                    updatedPipeline.material() != originalPipeline.material() ||
                    updatedPipeline.lengthM() != originalPipeline.lengthM() ||
                    updatedPipeline.status() != originalPipeline.status() ||
                    updatedPipeline.healthScore() != originalPipeline.healthScore()) {
                    hasChanges = true;
                }
                
                if (hasChanges) {
                    // 获取数据库ID
                    int databaseId = originalPipeline.id();
                    
                    // 检查是否已经在待保存列表中
                    bool found = false;
                    for (int i = 0; i < m_pendingChanges.size(); ++i) {
                        PendingChange &change = m_pendingChanges[i];
                        if (change.entityType == "pipeline" && 
                            change.originalId == databaseId &&
                            change.type == ChangeModified) {
                            // 更新现有的修改记录
                            change.data = QVariant::fromValue(updatedPipeline);
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        // 添加到待保存变更列表
                        PendingChange change;
                        change.type = ChangeModified;
                        change.entityType = "pipeline";
                        change.data = QVariant::fromValue(updatedPipeline);
                        change.originalId = databaseId;
                        change.graphicsItem = graphicsItem;
                        m_pendingChanges.append(change);
                    }
                    
                    // 更新图形项的状态
                    if (graphicsItem) {
                        QVariant stateVariant = graphicsItem->data(100);
                        EntityState currentState = EntityState::Detached;
                        if (stateVariant.isValid()) {
                            currentState = static_cast<EntityState>(stateVariant.toInt());
                        }
                        // 只有未变更状态才标记为修改
                        if (currentState == EntityState::Unchanged) {
                            graphicsItem->setData(100, static_cast<int>(EntityState::Modified));
                        }
                    }
                    
                    markAsModified();
                    
                    QMessageBox::information(this, "提示", 
                        QString("管线 %1 已标记为待保存\n\n请点击主窗口的保存按钮或按 Ctrl+S 保存到数据库")
                        .arg(updatedPipeline.pipelineId()));
                    updateStatus(QString("管线已修改（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
                }
            }
        } else {
            QMessageBox::warning(this, "错误", "未找到该管线");
        }
    } else if (itemType == "facility") {
        FacilityDAO facilityDao;
        Facility originalFacility = facilityDao.findByFacilityId(deviceId);
        if (originalFacility.isValid()) {
            FacilityEditDialog dialog(this, originalFacility);
            dialog.setWindowTitle("编辑设施信息");
            
            if (dialog.exec() == QDialog::Accepted) {
                Facility updatedFacility = dialog.resultFacility();
                updatedFacility.setId(originalFacility.id()); // 确保ID用于更新
                
                // 检查是否有修改
                bool hasChanges = false;
                if (updatedFacility.facilityName() != originalFacility.facilityName() ||
                    updatedFacility.facilityType() != originalFacility.facilityType() ||
                    updatedFacility.spec() != originalFacility.spec() ||
                    updatedFacility.material() != originalFacility.material() ||
                    updatedFacility.status() != originalFacility.status() ||
                    updatedFacility.healthScore() != originalFacility.healthScore()) {
                    hasChanges = true;
                }
                
                if (hasChanges) {
                    // 获取数据库ID
                    int databaseId = originalFacility.id();
                    
                    // 检查是否已经在待保存列表中
                    bool found = false;
                    for (int i = 0; i < m_pendingChanges.size(); ++i) {
                        PendingChange &change = m_pendingChanges[i];
                        if (change.entityType == "facility" && 
                            change.originalId == databaseId &&
                            change.type == ChangeModified) {
                            // 更新现有的修改记录
                            change.data = QVariant::fromValue(updatedFacility);
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        // 添加到待保存变更列表
                        PendingChange change;
                        change.type = ChangeModified;
                        change.entityType = "facility";
                        change.data = QVariant::fromValue(updatedFacility);
                        change.originalId = databaseId;
                        change.graphicsItem = graphicsItem;
                        m_pendingChanges.append(change);
                    }
                    
                    // 更新图形项的状态
                    if (graphicsItem) {
                        QVariant stateVariant = graphicsItem->data(100);
                        EntityState currentState = EntityState::Detached;
                        if (stateVariant.isValid()) {
                            currentState = static_cast<EntityState>(stateVariant.toInt());
                        }
                        // 只有未变更状态才标记为修改
                        if (currentState == EntityState::Unchanged) {
                            graphicsItem->setData(100, static_cast<int>(EntityState::Modified));
                        }
                    }
                    
                    markAsModified();
                    
                    QMessageBox::information(this, "提示", 
                        QString("设施 %1 已标记为待保存\n\n请点击主窗口的保存按钮或按 Ctrl+S 保存到数据库")
                        .arg(updatedFacility.facilityId()));
                    updateStatus(QString("设施已修改（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
                }
            }
        } else {
            QMessageBox::warning(this, "错误", "未找到该设施");
        }
    }
    
    // 对话框关闭后重置标志
    m_deviceTreeDialogActive = false;
}

// 删除设备
void MyForm::onDeviceTreeDelete()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemName = item->text();
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    if (itemType != "pipeline" && itemType != "facility") {
        QMessageBox::information(this, "提示", "只有具体设备才能删除。");
        return;
    }
    
    int ret = QMessageBox::warning(this, "确认删除", 
                                   QString("确定要删除设备 \"%1\" 吗？\n\n删除后需要点击保存按钮才会真正从数据库删除。")
                                   .arg(itemName),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    // 查找地图上的图形项
    QGraphicsItem *graphicsItem = nullptr;
    int databaseId = -1;
    EntityState entityState = EntityState::Detached;
    
    if (mapScene) {
        QList<QGraphicsItem*> allItems = mapScene->items();
        for (QGraphicsItem *sceneItem : allItems) {
            QString sceneItemType = sceneItem->data(0).toString();
            QString sceneItemId = sceneItem->data(1).toString();
            
            if ((itemType == "pipeline" && sceneItemType == "pipeline" && sceneItemId == deviceId) ||
                (itemType == "facility" && sceneItemType == "facility" && sceneItemId == deviceId)) {
                graphicsItem = sceneItem;
                
                // 获取实体状态
                QVariant stateVariant = sceneItem->data(100);
                if (stateVariant.isValid()) {
                    entityState = static_cast<EntityState>(stateVariant.toInt());
                }
                
                // 获取数据库ID
                QVariant dbIdVariant = sceneItem->data(10);
                if (!dbIdVariant.isValid() || dbIdVariant.toInt() <= 0) {
                    dbIdVariant = sceneItem->data(1);
                }
                
                if (dbIdVariant.isValid() && dbIdVariant.toInt() > 0) {
                    databaseId = dbIdVariant.toInt();
                } else {
                    // 通过设备ID查询数据库
                    if (itemType == "pipeline") {
                        PipelineDAO dao;
                        Pipeline pipeline = dao.findByPipelineId(deviceId);
                        if (pipeline.isValid()) {
                            databaseId = pipeline.id();
                        }
                    } else if (itemType == "facility") {
                        FacilityDAO dao;
                        Facility facility = dao.findByFacilityId(deviceId);
                        if (facility.isValid()) {
                            databaseId = facility.id();
                        }
                    }
                }
                break;
            }
        }
    }
    
    // 如果没有找到图形项，尝试通过设备ID查询数据库获取ID
    if (databaseId <= 0) {
        if (itemType == "pipeline") {
            PipelineDAO dao;
            Pipeline pipeline = dao.findByPipelineId(deviceId);
            if (pipeline.isValid()) {
                databaseId = pipeline.id();
            }
        } else if (itemType == "facility") {
            FacilityDAO dao;
            Facility facility = dao.findByFacilityId(deviceId);
            if (facility.isValid()) {
                databaseId = facility.id();
            }
        }
    }
    
    // 处理待保存变更列表
    if (entityState == EntityState::Added) {
        // 如果是新添加的实体，从待保存列表中移除对应的 ChangeAdded 记录
        for (int i = m_pendingChanges.size() - 1; i >= 0; i--) {
            const PendingChange &change = m_pendingChanges[i];
            if (change.entityType == itemType && 
                change.graphicsItem == graphicsItem &&
                change.type == ChangeAdded) {
                m_pendingChanges.removeAt(i);
                qDebug() << "[DeviceTree] Removed ChangeAdded from pending changes";
                break;
            }
        }
    } else if (entityState == EntityState::Unchanged && databaseId > 0) {
        // 如果是已存在的实体，添加到待保存列表作为 ChangeDeleted
        PendingChange change;
        change.type = ChangeDeleted;
        change.entityType = itemType;
        if (itemType == "pipeline") {
            Pipeline pipeline;
            pipeline.setId(databaseId);
            pipeline.setPipelineId(deviceId);
            change.data = QVariant::fromValue(pipeline);
        } else if (itemType == "facility") {
            Facility facility;
            facility.setId(databaseId);
            facility.setFacilityId(deviceId);
            change.data = QVariant::fromValue(facility);
        }
        change.originalId = databaseId;
        change.graphicsItem = graphicsItem;
        m_pendingChanges.append(change);
        markAsModified();
        qDebug() << "[DeviceTree] Added ChangeDeleted to pending changes, databaseId:" << databaseId;
    }
    
    // 如果找到了图形项，更新实体状态为已删除
    if (graphicsItem) {
        graphicsItem->setData(100, static_cast<int>(EntityState::Deleted));
        
        // 使用命令模式删除（支持撤销）- 立即从界面移除
        DeleteEntityCommand *cmd = new DeleteEntityCommand(
            mapScene,
            graphicsItem,
            &m_drawnPipelines
        );
        
        if (m_undoStack) {
            m_undoStack->push(cmd);
        }
        
        // 删除后立即刷新标注层，确保标注与图形项同步
        QTimer::singleShot(100, this, [this]() {
            if (m_layerManager) {
                qDebug() << "[DeviceTree Delete] Refreshing annotation layer after entity deletion";
                m_layerManager->refreshLayer(LayerManager::Labels);
            }
        });
    }
    
    // 从设备树中移除
    QStandardItem *parent = item->parent();
    if (parent) {
        parent->removeRow(item->row());
    } else {
        deviceTreeModel->removeRow(item->row());
    }
    
    updateStatus(QString("已删除%1（待保存，共 %2 项待保存）")
                 .arg(itemType == "pipeline" ? "管线" : "设施")
                 .arg(m_pendingChanges.size()));
    qDebug() << "[DeviceTree] Device deleted (pending save):" << itemName;
}

// 复制设备
void MyForm::onDeviceTreeCopy()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString deviceId = item->data(Qt::UserRole).toString();
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    // 复制设备ID到剪贴板
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(deviceId);
    
    updateStatus("已复制设备ID: " + deviceId);
}

// 在地图上定位
void MyForm::onDeviceTreeLocateOnMap()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    if (!tileMapManager || !mapScene) {
        QMessageBox::warning(this, "定位失败", "地图未初始化，请先加载地图");
        return;
    }
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    if (itemType == "pipeline") {
        PipelineDAO pipelineDao;
        Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
        if (pipeline.isValid() && !pipeline.coordinates().isEmpty()) {
            // 定位到管线中心点
            QPointF center = calculateCenter(pipeline.coordinates());
            
            // 设置地图中心点
            tileMapManager->setCenter(center.y(), center.x());
            
            // 确保合适的缩放级别
            if (currentZoomLevel < 8) {
                currentZoomLevel = 8;  // 设置为较高级别以便查看细节
                tileMapManager->setZoom(currentZoomLevel);
            }
            
            // 高亮显示管线
            clearBurstHighlights();  // 清除之前的高亮
            highlightPipelineById(deviceId);
            
            // 更新视图
            QPointF centerScene = tileMapManager->geoToScene(center.x(), center.y());
            ui->graphicsView->centerOn(centerScene);
            tileMapManager->updateTilesForViewImmediate(centerScene.x(), centerScene.y());
            
            updateStatus("已定位到管线: " + pipeline.pipelineName());
            qDebug() << "[DeviceTree] Located pipeline:" << deviceId << "at" << center;
        } else {
            QMessageBox::warning(this, "定位失败", "管线数据无效或缺少坐标信息");
        }
    } else if (itemType == "facility") {
        FacilityDAO facilityDao;
        Facility facility = facilityDao.findByFacilityId(deviceId);
        if (facility.isValid() && !facility.coordinate().isNull()) {
            QPointF coord = facility.coordinate();
            
            // 设置地图中心点
            tileMapManager->setCenter(coord.y(), coord.x());
            
            // 确保合适的缩放级别
            if (currentZoomLevel < 9) {
                currentZoomLevel = 9;  // 设置为更高级别以便查看设施
                tileMapManager->setZoom(currentZoomLevel);
            }
            
            // 高亮显示设施
            clearBurstHighlights();  // 清除之前的高亮
            highlightFacilityById(deviceId);
            
            // 更新视图
            QPointF centerScene = tileMapManager->geoToScene(coord.x(), coord.y());
            ui->graphicsView->centerOn(centerScene);
            tileMapManager->updateTilesForViewImmediate(centerScene.x(), centerScene.y());
            
            updateStatus("已定位到设施: " + facility.facilityName());
            qDebug() << "[DeviceTree] Located facility:" << deviceId << "at" << coord;
        } else {
            QMessageBox::warning(this, "定位失败", "设施数据无效或缺少坐标信息");
        }
    }
}

// 查看关联设备
void MyForm::onDeviceTreeViewRelated()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    QStringList relatedInfo;
    
    if (itemType == "pipeline") {
        // 查找关联的设施
        FacilityDAO facilityDao;
        QVector<Facility> facilities = facilityDao.findByPipelineId(deviceId, 100);
        
        if (facilities.isEmpty()) {
            QMessageBox::information(this, "关联设备", 
                                     QString("管线 \"%1\" 没有关联的设施。")
                                     .arg(item->text()));
            return;
        }
        
        relatedInfo.append(QString("管线 \"%1\" 关联的设施 (%2 个):\n")
                          .arg(item->text())
                          .arg(facilities.size()));
        
        for (const Facility &facility : facilities) {
            relatedInfo.append(QString("  • %1 (%2) - %3")
                              .arg(facility.facilityName())
                              .arg(facility.facilityId())
                              .arg(facility.getTypeDisplayName()));
        }
        
    } else if (itemType == "facility") {
        // 查找关联的管线
        FacilityDAO facilityDao;
        Facility facility = facilityDao.findByFacilityId(deviceId);
        
        if (facility.isValid() && !facility.pipelineId().isEmpty()) {
            PipelineDAO pipelineDao;
            Pipeline pipeline = pipelineDao.findByPipelineId(facility.pipelineId());
            
            if (pipeline.isValid()) {
                relatedInfo.append(QString("设施 \"%1\" 关联的管线:\n")
                                  .arg(item->text()));
                relatedInfo.append(QString("  • %1 (%2) - %3")
                                  .arg(pipeline.pipelineName())
                                  .arg(pipeline.pipelineId())
                                  .arg(pipeline.getTypeDisplayName()));
            } else {
                relatedInfo.append(QString("设施 \"%1\" 关联的管线ID \"%2\" 不存在。")
                                  .arg(item->text())
                                  .arg(facility.pipelineId()));
            }
        } else {
            QMessageBox::information(this, "关联设备", 
                                     QString("设施 \"%1\" 没有关联的管线。")
                                     .arg(item->text()));
            return;
        }
    } else {
        QMessageBox::information(this, "关联设备", "该节点类型不支持查看关联设备。");
        return;
    }
    
    QMessageBox::information(this, "关联设备", relatedInfo.join("\n"));
    updateStatus("已显示关联设备信息");
}

// 生成报告
void MyForm::onDeviceTreeGenerateReport()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString fileName = QFileDialog::getSaveFileName(this, "生成设备报告", 
                                                    QDir::homePath() + "/设备报告.txt",
                                                    "文本文件 (*.txt);;所有文件 (*)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "生成失败", "无法创建文件: " + fileName);
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    QString itemName = item->text();
    QString itemType = item->data(Qt::UserRole + 1).toString();
    
    // 写入报告头部
    out << "========================================\n";
    out << "设备报告\n";
    out << "生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "========================================\n\n";
    
    if (itemType == "pipeline_root" || itemType == "facility_root") {
        // 一级节点：生成分类统计报告
        int pipelineCount = 0;
        int facilityCount = 0;
        countDevices(item, pipelineCount, facilityCount);
        
        out << "分类: " << itemName << "\n";
        out << "管线数量: " << pipelineCount << "\n";
        out << "设施数量: " << facilityCount << "\n";
        out << "总计: " << (pipelineCount + facilityCount) << "\n\n";
        
        // 收集详细信息
        QList<Pipeline> pipelines;
        QList<Facility> facilities;
        collectDevicesFromItem(item, pipelines, facilities);
        
        if (!pipelines.isEmpty()) {
            out << "管线明细:\n";
            out << "----------------------------------------\n";
            PipelineDAO pipelineDao;
            for (const Pipeline &p : pipelines) {
                Pipeline fullPipeline = pipelineDao.findByPipelineId(p.pipelineId());
                if (fullPipeline.isValid()) {
                    out << "编号: " << fullPipeline.pipelineId() << "\n";
                    out << "名称: " << fullPipeline.pipelineName() << "\n";
                    out << "类型: " << fullPipeline.getTypeDisplayName() << "\n";
                    out << "长度: " << fullPipeline.lengthM() << " 米\n";
                    out << "管径: " << fullPipeline.diameterMm() << " 毫米\n";
                    out << "状态: " << fullPipeline.status() << "\n";
                    out << "健康度: " << fullPipeline.healthScore() << " 分\n";
                    out << "----------------------------------------\n";
                }
            }
        }
        
        if (!facilities.isEmpty()) {
            out << "\n设施明细:\n";
            out << "----------------------------------------\n";
            FacilityDAO facilityDao;
            for (const Facility &f : facilities) {
                Facility fullFacility = facilityDao.findByFacilityId(f.facilityId());
                if (fullFacility.isValid()) {
                    out << "编号: " << fullFacility.facilityId() << "\n";
                    out << "名称: " << fullFacility.facilityName() << "\n";
                    out << "类型: " << fullFacility.getTypeDisplayName() << "\n";
                    out << "规格: " << fullFacility.spec() << "\n";
                    out << "状态: " << fullFacility.status() << "\n";
                    out << "健康度: " << fullFacility.healthScore() << " 分\n";
                    out << "----------------------------------------\n";
                }
            }
        }
        
    } else if (itemType == "pipeline_type" || itemType == "facility_type") {
        // 二级节点：生成类型统计报告
        int pipelineCount = 0;
        int facilityCount = 0;
        countDevices(item, pipelineCount, facilityCount);
        
        out << "类型: " << itemName << "\n";
        out << "设备数量: " << (pipelineCount + facilityCount) << "\n\n";
        
        QList<Pipeline> pipelines;
        QList<Facility> facilities;
        collectDevicesFromItem(item, pipelines, facilities);
        
        if (!pipelines.isEmpty()) {
            out << "管线列表:\n";
            PipelineDAO pipelineDao;
            for (const Pipeline &p : pipelines) {
                Pipeline fullPipeline = pipelineDao.findByPipelineId(p.pipelineId());
                if (fullPipeline.isValid()) {
                    out << "  • " << fullPipeline.pipelineId() << " - " 
                        << fullPipeline.pipelineName() << " (" 
                        << fullPipeline.healthScore() << "分)\n";
                }
            }
        }
        
        if (!facilities.isEmpty()) {
            out << "\n设施列表:\n";
            FacilityDAO facilityDao;
            for (const Facility &f : facilities) {
                Facility fullFacility = facilityDao.findByFacilityId(f.facilityId());
                if (fullFacility.isValid()) {
                    out << "  • " << fullFacility.facilityId() << " - " 
                        << fullFacility.facilityName() << " (" 
                        << fullFacility.healthScore() << "分)\n";
                }
            }
        }
        
    } else if (itemType == "pipeline" || itemType == "facility") {
        // 三级节点：生成单个设备详细报告
        QString deviceId = item->data(Qt::UserRole).toString();
        
        if (itemType == "pipeline") {
            PipelineDAO pipelineDao;
            Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
            if (pipeline.isValid()) {
                out << "设备类型: 管线\n";
                out << "设备编号: " << pipeline.pipelineId() << "\n";
                out << "设备名称: " << pipeline.pipelineName() << "\n";
                out << "管线类型: " << pipeline.getTypeDisplayName() << "\n";
                out << "长度: " << pipeline.lengthM() << " 米\n";
                out << "管径: " << pipeline.diameterMm() << " 毫米\n";
                out << "材质: " << pipeline.material() << "\n";
                out << "压力等级: " << pipeline.pressureClass() << "\n";
                out << "埋深: " << pipeline.depthM() << " 米\n";
                out << "状态: " << pipeline.status() << "\n";
                out << "健康度: " << pipeline.healthScore() << " 分\n";
                out << "建设日期: " << pipeline.buildDate().toString("yyyy-MM-dd") << "\n";
                out << "建设单位: " << pipeline.builder() << "\n";
                out << "管理单位: " << pipeline.owner() << "\n";
                out << "维护单位: " << pipeline.maintenanceUnit() << "\n";
                out << "最后检查: " << pipeline.lastInspection().toString("yyyy-MM-dd") << "\n";
                out << "备注: " << pipeline.remarks() << "\n";
            }
        } else if (itemType == "facility") {
            FacilityDAO facilityDao;
            Facility facility = facilityDao.findByFacilityId(deviceId);
            if (facility.isValid()) {
                out << "设备类型: 设施\n";
                out << "设备编号: " << facility.facilityId() << "\n";
                out << "设备名称: " << facility.facilityName() << "\n";
                out << "设施类型: " << facility.getTypeDisplayName() << "\n";
                out << "规格: " << facility.spec() << "\n";
                out << "尺寸: " << facility.size() << "\n";
                out << "材质: " << facility.material() << "\n";
                out << "状态: " << facility.status() << "\n";
                out << "健康度: " << facility.healthScore() << " 分\n";
                out << "关联管线: " << facility.pipelineId() << "\n";
                out << "建设日期: " << (facility.buildDate().isValid() ? facility.buildDate().toString("yyyy-MM-dd") : "未知") << "\n";
                out << "建设单位: " << facility.builder() << "\n";
                out << "管理单位: " << facility.owner() << "\n";
                out << "维护单位: " << facility.maintenanceUnit() << "\n";
                out << "最后维护: " << (facility.lastMaintenance().isValid() ? facility.lastMaintenance().toString("yyyy-MM-dd") : "未维护") << "\n";
                out << "下次维护: " << (facility.nextMaintenance().isValid() ? facility.nextMaintenance().toString("yyyy-MM-dd") : "未计划") << "\n";
                out << "资产价值: " << facility.assetValue() << " 元\n";
                out << "备注: " << facility.remarks() << "\n";
            }
        }
    }
    
    file.close();
    
    QMessageBox::information(this, "报告生成成功", 
                            QString("设备报告已保存到:\n%1").arg(fileName));
    updateStatus("报告已生成: " + fileName);
}

// 查看历史记录
void MyForm::onDeviceTreeViewHistory()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    if (itemType != "pipeline" && itemType != "facility") {
        QMessageBox::information(this, "历史记录", "只有具体设备才有历史记录。");
        return;
    }
    
    QStringList history;
    history.append("设备历史记录\n");
    history.append("========================================\n\n");
    
    if (itemType == "pipeline") {
        PipelineDAO pipelineDao;
        Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
        if (pipeline.isValid()) {
            history.append(QString("设备编号: %1\n").arg(pipeline.pipelineId()));
            history.append(QString("设备名称: %1\n\n").arg(pipeline.pipelineName()));
            
            history.append("基本信息:\n");
            history.append(QString("  创建时间: %1\n")
                          .arg(pipeline.createdAt().isValid() ? 
                               pipeline.createdAt().toString("yyyy-MM-dd hh:mm:ss") : "未知"));
            history.append(QString("  更新时间: %1\n")
                          .arg(pipeline.updatedAt().isValid() ? 
                               pipeline.updatedAt().toString("yyyy-MM-dd hh:mm:ss") : "未知"));
            QString createdBy = pipeline.createdBy();
            QString updatedBy = pipeline.updatedBy();
            if (!createdBy.isEmpty()) {
                history.append(QString("  创建人: %1\n").arg(createdBy));
            }
            if (!updatedBy.isEmpty()) {
                history.append(QString("  更新人: %1\n").arg(updatedBy));
            }
            history.append("\n");
            
            history.append("维护记录:\n");
            history.append(QString("  建设日期: %1\n")
                          .arg(pipeline.buildDate().toString("yyyy-MM-dd")));
            history.append(QString("  最后检查: %1\n")
                          .arg(pipeline.lastInspection().isValid() ? 
                               pipeline.lastInspection().toString("yyyy-MM-dd") : "未检查"));
            history.append(QString("  检查周期: %1 天\n")
                          .arg(pipeline.inspectionCycle()));
            history.append(QString("  当前状态: %1\n")
                          .arg(pipeline.status()));
            history.append(QString("  健康度: %1 分\n")
                          .arg(pipeline.healthScore()));
            
            if (!pipeline.remarks().isEmpty()) {
                history.append(QString("\n备注:\n  %1\n").arg(pipeline.remarks()));
            }
        } else {
            history.append("未找到设备数据。");
        }
    } else if (itemType == "facility") {
        FacilityDAO facilityDao;
        Facility facility = facilityDao.findByFacilityId(deviceId);
        if (facility.isValid()) {
            history.append(QString("设备编号: %1\n").arg(facility.facilityId()));
            history.append(QString("设备名称: %1\n\n").arg(facility.facilityName()));
            
            history.append("基本信息:\n");
            history.append(QString("  创建时间: %1\n")
                          .arg(facility.createdAt().isValid() ? 
                               facility.createdAt().toString("yyyy-MM-dd hh:mm:ss") : "未知"));
            history.append(QString("  更新时间: %1\n")
                          .arg(facility.updatedAt().isValid() ? 
                               facility.updatedAt().toString("yyyy-MM-dd hh:mm:ss") : "未知"));
            QString createdBy = facility.createdBy();
            QString updatedBy = facility.updatedBy();
            if (!createdBy.isEmpty()) {
                history.append(QString("  创建人: %1\n").arg(createdBy));
            }
            if (!updatedBy.isEmpty()) {
                history.append(QString("  更新人: %1\n").arg(updatedBy));
            }
            history.append("\n");
            
            history.append("维护记录:\n");
            history.append(QString("  建设日期: %1\n")
                          .arg(facility.buildDate().toString("yyyy-MM-dd")));
            history.append(QString("  最后维护: %1\n")
                          .arg(facility.lastMaintenance().isValid() ? 
                               facility.lastMaintenance().toString("yyyy-MM-dd") : "未维护"));
            history.append(QString("  下次维护: %1\n")
                          .arg(facility.nextMaintenance().isValid() ? 
                               facility.nextMaintenance().toString("yyyy-MM-dd") : "未计划"));
            history.append(QString("  维护单位: %1\n")
                          .arg(facility.maintenanceUnit()));
            history.append(QString("  当前状态: %1\n")
                          .arg(facility.status()));
            history.append(QString("  健康度: %1 分\n")
                          .arg(facility.healthScore()));
            
            if (!facility.remarks().isEmpty()) {
                history.append(QString("\n备注:\n  %1\n").arg(facility.remarks()));
            }
        } else {
            history.append("未找到设备数据。");
        }
    }
    
    QMessageBox::information(this, "历史记录", history.join(""));
    updateStatus("已显示设备历史记录");
}

// 打印标签
void MyForm::onDeviceTreePrintLabel()
{
    if (!m_currentDeviceTreeIndex.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(m_currentDeviceTreeIndex);
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    if (itemType != "pipeline" && itemType != "facility") {
        QMessageBox::information(this, "打印标签", "只有具体设备才能打印标签。");
        return;
    }
    
    // 生成标签内容
    QString deviceName = item->text();
    QString qrCodeUrl;
    QString deviceType;
    
    if (itemType == "pipeline") {
        PipelineDAO pipelineDao;
        Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
        if (pipeline.isValid()) {
            deviceName = pipeline.pipelineName();
            deviceType = pipeline.getTypeDisplayName();
            qrCodeUrl = QString("https://ugims.com/pipeline/%1").arg(deviceId);
        }
    } else if (itemType == "facility") {
        FacilityDAO facilityDao;
        Facility facility = facilityDao.findByFacilityId(deviceId);
        if (facility.isValid()) {
            deviceName = facility.facilityName();
            deviceType = facility.getTypeDisplayName();
            qrCodeUrl = facility.qrcodeUrl();
            if (qrCodeUrl.isEmpty()) {
                qrCodeUrl = QString("https://ugims.com/facility/%1").arg(deviceId);
            }
        }
    }
    
    // 生成标签文本内容（可以用于打印）
    QString labelContent = QString(
        "========================================\n"
        "设备标签\n"
        "========================================\n"
        "设备编号: %1\n"
        "设备名称: %2\n"
        "设备类型: %3\n"
        "二维码: %4\n"
        "生成时间: %5\n"
        "========================================\n"
    ).arg(deviceId)
     .arg(deviceName)
     .arg(deviceType)
     .arg(qrCodeUrl)
     .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    
    // 询问用户是否保存标签文件
    int ret = QMessageBox::question(this, "打印标签", 
                                   "标签内容已生成。\n\n" + labelContent + "\n\n是否保存到文件？",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QString fileName = QFileDialog::getSaveFileName(this, "保存标签", 
                                                        QDir::homePath() + "/" + deviceId + "_标签.txt",
                                                        "文本文件 (*.txt);;所有文件 (*)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out.setEncoding(QStringConverter::Utf8);
                out << labelContent;
                file.close();
                QMessageBox::information(this, "保存成功", "标签已保存到: " + fileName);
                updateStatus("标签已保存: " + fileName);
            } else {
                QMessageBox::warning(this, "保存失败", "无法创建文件: " + fileName);
            }
        }
    } else {
        // 复制到剪贴板
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(labelContent);
        updateStatus("标签内容已复制到剪贴板");
    }
}

// 辅助函数：递归展开所有子节点
void MyForm::expandAllChildren(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    ui->deviceTreeView->expand(index);
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(index);
    if (item) {
        for (int i = 0; i < item->rowCount(); ++i) {
            QModelIndex childIndex = deviceTreeModel->index(i, 0, index);
            expandAllChildren(childIndex);
        }
    }
}

// 辅助函数：递归折叠所有子节点
void MyForm::collapseAllChildren(const QModelIndex &index)
{
    if (!index.isValid()) return;
    
    QStandardItem *item = deviceTreeModel->itemFromIndex(index);
    if (item) {
        for (int i = 0; i < item->rowCount(); ++i) {
            QModelIndex childIndex = deviceTreeModel->index(i, 0, index);
            collapseAllChildren(childIndex);
        }
    }
    
    ui->deviceTreeView->collapse(index);
}

// 辅助函数：统计设备数量
void MyForm::countDevices(QStandardItem *item, int &pipelineCount, int &facilityCount)
{
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    if (itemType == "pipeline") {
        pipelineCount++;
    } else if (itemType == "facility") {
        facilityCount++;
    }
    
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem *child = item->child(i);
        if (child) {
            countDevices(child, pipelineCount, facilityCount);
        }
    }
}

// 辅助函数：收集设备数据
void MyForm::collectDevicesFromItem(QStandardItem *item, QList<Pipeline> &pipelines, QList<Facility> &facilities)
{
    if (!item) return;
    
    QString itemType = item->data(Qt::UserRole + 1).toString();
    QString deviceId = item->data(Qt::UserRole).toString();
    
    // 如果是具体设备，添加到列表
    if (itemType == "pipeline" && !deviceId.isEmpty()) {
        PipelineDAO pipelineDao;
        Pipeline pipeline = pipelineDao.findByPipelineId(deviceId);
        if (pipeline.isValid()) {
            pipelines.append(pipeline);
        }
    } else if (itemType == "facility" && !deviceId.isEmpty()) {
        FacilityDAO facilityDao;
        Facility facility = facilityDao.findByFacilityId(deviceId);
        if (facility.isValid()) {
            facilities.append(facility);
        }
    }
    
    // 递归处理子节点
    for (int i = 0; i < item->rowCount(); ++i) {
        QStandardItem *child = item->child(i);
        if (child) {
            collectDevicesFromItem(child, pipelines, facilities);
        }
    }
}

// 辅助函数：计算管线中心点
QPointF MyForm::calculateCenter(const QVector<QPointF> &coordinates)
{
    if (coordinates.isEmpty()) return QPointF();
    
    double sumX = 0, sumY = 0;
    for (const QPointF &point : coordinates) {
        sumX += point.x();
        sumY += point.y();
    }
    
    return QPointF(sumX / coordinates.size(), sumY / coordinates.size());
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

void MyForm::onPipelineDrawingFinished(const QString &pipelineType, const QString &wkt, const QVector<QPointF> &scenePoints, const QVector<QString> &connectedFacilityIds)
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
            item->setData(3, pipeline.pipelineName());  // 管线名称（用于标注显示）
            item->setData(100, static_cast<int>(EntityState::Added));  // 新增：设置实体状态为Added
            
            // 关键：保存管线对象到hash表，用于后续编辑
            m_drawnPipelines[item] = pipeline;
            
            // 更新连接的设施的 pipeline_id 字段
            if (!connectedFacilityIds.isEmpty()) {
                FacilityDAO facilityDao;
                for (const QString &facilityId : connectedFacilityIds) {
                    if (!facilityId.isEmpty()) {
                        Facility facility = facilityDao.findByFacilityId(facilityId);
                        if (facility.isValid()) {
                            facility.setPipelineId(pipeline.pipelineId());
                            // 添加到待保存变更列表
                            PendingChange facilityChange;
                            facilityChange.type = ChangeModified;
                            facilityChange.entityType = "facility";
                            facilityChange.data = QVariant::fromValue(facility);
                            facilityChange.originalId = facility.id();
                            facilityChange.graphicsItem = nullptr;  // 设施图形项需要单独查找
                            m_pendingChanges.append(facilityChange);
                            qDebug() << "Connected facility" << facilityId << "to pipeline" << pipeline.pipelineId();
                        }
                    }
                }
            }
            
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
            
            // 延迟刷新标注图层，让新绘制的设备也显示标注
            // 注意：即使标注层当前不可见，也要刷新，这样当用户显示标注层时就能看到标注
            QTimer::singleShot(100, this, [this]() {
                if (m_layerManager) {
                    qDebug() << "[Pipeline Drawing] Refreshing annotation layer after new pipeline";
                    m_layerManager->refreshLayer(LayerManager::Labels);
                }
            });
        } else {
            qDebug() << "❌ Cannot draw pipeline: mapScene=" << mapScene 
                     << "scenePoints.size()=" << scenePoints.size();
            // 只有失败时才弹窗
            QMessageBox::warning(this, "错误", "管线创建失败，请检查场景是否已初始化");
        }
        
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
        
        // 刷新设备树以显示最新数据
        setupDeviceTree();
        updateStatus("设备树已刷新");
        qDebug() << "[Save] Device tree refreshed after save";
        
        // 保存后刷新标注层，确保标注与数据库数据同步
        QTimer::singleShot(50, this, [this]() {
            if (m_layerManager) {
                m_layerManager->refreshLayer(LayerManager::Labels);
            }
        });
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
                // 如果 Facility 对象没有几何信息，从图形项中获取
                if (facility.geomWkt().isEmpty() && change.graphicsItem) {
                    QGraphicsEllipseItem *ellipseItem = qgraphicsitem_cast<QGraphicsEllipseItem*>(change.graphicsItem);
                    if (ellipseItem) {
                        // 获取图形项的中心点（场景坐标）
                        QRectF rect = ellipseItem->rect();
                        QPointF centerScene = ellipseItem->pos() + rect.center();
                        
                        // 转换为地理坐标
                        QPointF geoCoord;
                        if (tileMapManager) {
                            geoCoord = tileMapManager->sceneToGeo(centerScene);
                        } else {
                            // 如果没有瓦片管理器，使用原始坐标（降级方案）
                            geoCoord = centerScene;
                        }
                        
                        // 生成 WKT 格式
                        QString wkt = QString("POINT(%1 %2)")
                                        .arg(geoCoord.x(), 0, 'f', 8)
                                        .arg(geoCoord.y(), 0, 'f', 8);
                        
                        // 设置几何信息
                        facility.setCoordinate(geoCoord);
                        facility.setGeomWkt(wkt);
                        
                        qDebug() << QString("[Save] Facility geometry restored from graphics item: %1").arg(wkt);
                    }
                }
                
                // 检查编号是否存在，如果存在则自动生成新编号
                QString originalId = facility.facilityId();
                QString finalId = originalId;
                
                // 如果编号为空或者是临时ID（以TEMP_开头），自动生成
                if (originalId.isEmpty() || originalId.startsWith("TEMP_")) {
                    finalId = IdGenerator::generateFacilityId(facility.facilityType());
                    qDebug() << QString("[Save] Facility ID is empty or temporary (%1), auto-generated: %2")
                                .arg(originalId).arg(finalId);
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
        
        // 如果设施ID为空，自动生成一个临时ID（用于标注显示，保存时会重新生成）
        if (facility.facilityId().isEmpty() && !facility.facilityType().isEmpty()) {
            QString tempId = IdGenerator::generateFacilityId(facility.facilityType());
            // 添加临时标记，保存时会重新生成
            tempId = "TEMP_" + tempId;
            facility.setFacilityId(tempId);
            qDebug() << "[Facility Drawing] Auto-generated temporary ID:" << tempId;
        }
        
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
            ellipseItem->setData(1, facility.facilityId());  // 设施ID（现在保证不为空）
            ellipseItem->setData(2, facilityTypeStr);  // 设施类型
            ellipseItem->setData(3, facility.facilityName());  // 设施名称（用于标注显示）
            ellipseItem->setData(100, static_cast<int>(EntityState::Added));  // 新增：设置实体状态为Added
            
            // 设置可选中和可交互标志（重要：使设施可以被点击选中）
            ellipseItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            ellipseItem->setFlag(QGraphicsItem::ItemIsFocusable, true);
            ellipseItem->setAcceptHoverEvents(true);
            
            // 禁用Qt默认的选中样式（虚线框），使用自定义高亮
            // 确保不显示默认的虚线框，只使用自定义高亮
            ellipseItem->setSelected(false);
            
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
            
            // 延迟刷新标注图层，让新绘制的设备也显示标注
            // 注意：即使标注层当前不可见，也要刷新，这样当用户显示标注层时就能看到标注
            QTimer::singleShot(100, this, [this]() {
                if (m_layerManager) {
                    qDebug() << "[Facility Drawing] Refreshing annotation layer after new facility";
                    m_layerManager->refreshLayer(LayerManager::Labels);
                }
            });
            
            updateStatus(QString("设施创建成功（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
        } else {
            qDebug() << "❌ Cannot create facility: mapScene is null";
            // 只有失败时才弹窗
            QMessageBox::warning(this, "错误", "设施创建失败，请检查场景是否已初始化");
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
    
    // 检查是否是测量线条
    QString itemType = m_selectedItem->data(0).toString();
    bool isMeasureItem = (itemType == "distance_measure_line" || 
                          itemType == "distance_measure_label" ||
                          itemType == "distance_measure_marker" ||
                          itemType == "area_measure_polygon" ||
                          itemType == "area_measure_label" ||
                          itemType == "area_measure_marker");
    
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
    
    if (isMeasureItem) {
        // 测量线条的右键菜单：只显示删除选项
        QAction *deleteAction = contextMenu.addAction("🗑️ 删除测量");
        QFont deleteFont = deleteAction->font();
        deleteFont.setBold(true);
        deleteAction->setFont(deleteFont);
        
        connect(deleteAction, &QAction::triggered, this, [this, itemType]() {
            if (itemType.startsWith("distance_")) {
                deleteDistanceMeasure(m_selectedItem);
            } else if (itemType.startsWith("area_")) {
                deleteAreaMeasure(m_selectedItem);
            }
            clearSelection();
        });
    } else {
        // 实体（管线/设施）的右键菜单：显示完整操作
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
    }
    
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
                    // 保存完整的设施信息，而不仅仅是ID和编号
                    Facility facility;
                    // 尝试从数据库查询完整信息
                    FacilityDAO dao;
                    Facility dbFacility = dao.findByFacilityId(entityId);
                    if (dbFacility.isValid() && dbFacility.id() == databaseId) {
                        // 使用数据库中的完整信息
                        facility = dbFacility;
                        qDebug() << "[Delete] Using complete facility data from database for ChangeDeleted";
                    } else {
                        // 如果数据库查询失败，至少保存ID和编号
                        facility.setId(databaseId);
                        facility.setFacilityId(entityId);
                        // 尝试从图形项获取名称（如果存在）
                        QString facilityName = m_selectedItem->data(3).toString();
                        if (!facilityName.isEmpty()) {
                            facility.setFacilityName(facilityName);
                        }
                        QString facilityType = m_selectedItem->data(2).toString();
                        if (!facilityType.isEmpty()) {
                            facility.setFacilityType(facilityType);
                        }
                        qDebug() << "[Delete] Using partial facility data from graphics item for ChangeDeleted";
                    }
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
        
        // 删除后立即刷新标注层，确保标注与图形项同步
        QTimer::singleShot(100, this, [this]() {
            if (m_layerManager) {
                qDebug() << "[Delete] Refreshing annotation layer after entity deletion";
                m_layerManager->refreshLayer(LayerManager::Labels);
            }
        });
        
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
    
    if (entityType == "pipeline") {
        // 编辑管线
        Pipeline pipeline;
        bool found = false;
        
        // 优先检查是否是绘制的管线
        if (m_drawnPipelines.contains(m_selectedItem)) {
            pipeline = m_drawnPipelines[m_selectedItem];
            found = true;
        } else {
            // 尝试从数据库查询（使用管线编号）
            QString pipelineId = m_selectedItem->data(1).toString();
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
        
        PipelineEditDialog *dialog = new PipelineEditDialog(this);
        dialog->loadPipeline(pipeline);
        dialog->setWindowTitle("编辑管线信息");
        
        if (dialog->exec() == QDialog::Accepted) {
            Pipeline updatedPipeline = dialog->getPipeline();
            updatedPipeline.setId(pipeline.id()); // 确保ID用于更新
            
            // 检查是否有修改
            bool hasChanges = false;
            if (updatedPipeline.pipelineName() != pipeline.pipelineName() ||
                updatedPipeline.pipelineType() != pipeline.pipelineType() ||
                updatedPipeline.diameterMm() != pipeline.diameterMm() ||
                updatedPipeline.material() != pipeline.material() ||
                updatedPipeline.lengthM() != pipeline.lengthM() ||
                updatedPipeline.status() != pipeline.status() ||
                updatedPipeline.healthScore() != pipeline.healthScore()) {
                hasChanges = true;
            }
            
            if (hasChanges) {
                // 获取数据库ID
                int databaseId = pipeline.id();
                
                // 检查是否已经在待保存列表中
                bool found = false;
                for (int i = 0; i < m_pendingChanges.size(); ++i) {
                    PendingChange &change = m_pendingChanges[i];
                    if (change.entityType == "pipeline" && 
                        change.graphicsItem == m_selectedItem) {
                        // 如果是新绘制的管线（ChangeAdded），直接更新原记录
                        if (change.type == ChangeAdded) {
                            change.data = QVariant::fromValue(updatedPipeline);
                            // 更新图形项上的管线名称
                            QString newPipelineName = updatedPipeline.pipelineName();
                            m_selectedItem->setData(3, newPipelineName);
                            // 更新 m_drawnPipelines
                            m_drawnPipelines[m_selectedItem] = updatedPipeline;
                            found = true;
                            qDebug() << "[Edit Pipeline] Updated ChangeAdded record for new pipeline";
                            break;
                        } else if (change.type == ChangeModified && change.originalId == databaseId) {
                            // 如果是已存在的管线（ChangeModified），更新修改记录
                            change.data = QVariant::fromValue(updatedPipeline);
                            // 更新图形项上的管线名称
                            QString newPipelineName = updatedPipeline.pipelineName();
                            m_selectedItem->setData(3, newPipelineName);
                            // 更新 m_drawnPipelines（如果存在）
                            if (m_drawnPipelines.contains(m_selectedItem)) {
                                m_drawnPipelines[m_selectedItem] = updatedPipeline;
                            }
                            found = true;
                            qDebug() << "[Edit Pipeline] Updated ChangeModified record for existing pipeline";
                            break;
                        }
                    }
                }
                
                if (!found) {
                    // 添加到待保存变更列表（这种情况应该是编辑已保存的管线）
                    PendingChange change;
                    change.type = ChangeModified;
                    change.entityType = "pipeline";
                    change.data = QVariant::fromValue(updatedPipeline);
                    change.originalId = databaseId;
                    change.graphicsItem = m_selectedItem;
                    m_pendingChanges.append(change);
                    qDebug() << "[Edit Pipeline] Added new ChangeModified record";
                }
                
                // 更新图形项上的管线名称（无论是否找到现有记录都要更新）
                QString newPipelineName = updatedPipeline.pipelineName();
                m_selectedItem->setData(3, newPipelineName);
                // 更新 m_drawnPipelines（如果存在）
                if (m_drawnPipelines.contains(m_selectedItem)) {
                    m_drawnPipelines[m_selectedItem] = updatedPipeline;
                }
                
                // 更新图形项的状态
                QVariant stateVariant = m_selectedItem->data(100);
                EntityState currentState = EntityState::Detached;
                if (stateVariant.isValid()) {
                    currentState = static_cast<EntityState>(stateVariant.toInt());
                }
                // 只有未变更状态才标记为修改
                if (currentState == EntityState::Unchanged) {
                    m_selectedItem->setData(100, static_cast<int>(EntityState::Modified));
                }
                
                markAsModified();
                
                // 刷新标注层，使名称变化立即显示
                QTimer::singleShot(50, this, [this]() {
                    if (m_layerManager) {
                        m_layerManager->refreshLayer(LayerManager::Labels);
                    }
                });
                
                QMessageBox::information(this, "提示", 
                    QString("管线 %1 已标记为待保存\n\n请点击主窗口的保存按钮或按 Ctrl+S 保存到数据库")
                    .arg(updatedPipeline.pipelineId()));
                updateStatus(QString("管线已修改（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
            }
        }
        
        delete dialog;
        
    } else if (entityType == "facility") {
        // 编辑设施
        Facility facility;
        bool found = false;
        
        // 获取设施ID（用于数据库查询和调试）
        QString facilityId = m_selectedItem->data(1).toString();
        if (facilityId.isEmpty()) {
            facilityId = m_selectedItem->data(2).toString();
        }
        
        qDebug() << "[Edit Facility] Searching for facility, graphicsItem:" << m_selectedItem 
                 << "facilityId:" << facilityId 
                 << "pendingChanges count:" << m_pendingChanges.size();
        
        // 优先检查待保存的变更列表（新绘制的设施可能还没有保存到数据库）
        // 先通过图形项匹配（最可靠），如果失败则通过 facilityId 匹配
        for (const PendingChange &change : m_pendingChanges) {
            if (change.entityType == "facility") {
                bool itemMatches = (change.graphicsItem == m_selectedItem);
                bool idMatches = false;
                if (!itemMatches && !facilityId.isEmpty()) {
                    Facility changeFacility = change.data.value<Facility>();
                    // 检查 facilityId 是否匹配（即使 isValid() 返回 false）
                    idMatches = (!changeFacility.facilityId().isEmpty() && 
                                changeFacility.facilityId() == facilityId);
                }
                
                if (itemMatches || idMatches) {
                    if (change.type == ChangeAdded || change.type == ChangeModified) {
                        // 从待保存的变更中获取设施数据
                        facility = change.data.value<Facility>();
                        // 对于新绘制的设施，m_id 为 0，所以不能用 isValid() 判断
                        // 只要 facilityId 和 facilityType 不为空就认为有效
                        if (!facility.facilityId().isEmpty() && !facility.facilityType().isEmpty()) {
                            found = true;
                            qDebug() << "[Edit Facility] Found in pending changes (type=" << change.type 
                                     << ", match=" << (itemMatches ? "graphicsItem" : "facilityId")
                                     << "), facilityId:" << facility.facilityId()
                                     << "facilityName:" << facility.facilityName();
                            break;
                        } else {
                            qDebug() << "[Edit Facility] Found change but facility data incomplete:"
                                     << "facilityId=" << facility.facilityId()
                                     << "facilityType=" << facility.facilityType();
                        }
                    }
                }
            }
        }
        
        // 如果待保存列表中没有，尝试从数据库查询
        if (!found && !facilityId.isEmpty() && DatabaseManager::instance().isConnected()) {
            FacilityDAO dao;
            facility = dao.findByFacilityId(facilityId);
            if (facility.isValid()) {
                found = true;
                qDebug() << "[Edit Facility] Found in database, facilityId:" << facilityId;
            }
        }
        
        if (!found) {
            qDebug() << "[Edit Facility] NOT FOUND - graphicsItem:" << m_selectedItem 
                     << "facilityId:" << facilityId 
                     << "pendingChanges:" << m_pendingChanges.size();
            QMessageBox::warning(this, "错误", 
                QString("未找到设施数据！\n\n设施ID: %1\n\n提示：如果这是新绘制的设施，请先保存到数据库后再编辑。")
                .arg(facilityId.isEmpty() ? "未知" : facilityId));
            return;
        }
        
        FacilityEditDialog *dialog = new FacilityEditDialog(this, facility);
        dialog->setWindowTitle("编辑设施信息");
        
        if (dialog->exec() == QDialog::Accepted) {
            Facility updatedFacility = dialog->resultFacility();
            updatedFacility.setId(facility.id()); // 确保ID用于更新
            
            // 检查是否有修改
            bool hasChanges = false;
            if (updatedFacility.facilityName() != facility.facilityName() ||
                updatedFacility.facilityType() != facility.facilityType() ||
                updatedFacility.spec() != facility.spec() ||
                updatedFacility.material() != facility.material() ||
                updatedFacility.status() != facility.status() ||
                updatedFacility.healthScore() != facility.healthScore()) {
                hasChanges = true;
            }
            
            if (hasChanges) {
                // 获取数据库ID
                int databaseId = facility.id();
                
                // 检查是否已经在待保存列表中
                bool found = false;
                for (int i = 0; i < m_pendingChanges.size(); ++i) {
                    PendingChange &change = m_pendingChanges[i];
                    if (change.entityType == "facility" && 
                        change.graphicsItem == m_selectedItem) {
                        // 如果是新绘制的设施（ChangeAdded），直接更新原记录
                        if (change.type == ChangeAdded) {
                            change.data = QVariant::fromValue(updatedFacility);
                            // 更新图形项上的设施ID和名称（如果改变了）
                            QString newFacilityId = updatedFacility.facilityId();
                            if (!newFacilityId.isEmpty()) {
                                m_selectedItem->setData(1, newFacilityId);
                            }
                            QString newFacilityName = updatedFacility.facilityName();
                            m_selectedItem->setData(3, newFacilityName);  // 更新设施名称
                            // 更新tooltip
                            QString typeName = m_selectedItem->data(2).toString();
                            m_selectedItem->setToolTip(QString("%1\n类型: %2")
                                                       .arg(newFacilityName.isEmpty() ? newFacilityId : newFacilityName)
                                                       .arg(typeName));
                            found = true;
                            qDebug() << "[Edit Facility] Updated ChangeAdded record for new facility";
                            break;
                        } else if (change.type == ChangeModified && change.originalId == databaseId) {
                            // 如果是已存在的设施（ChangeModified），更新修改记录
                            change.data = QVariant::fromValue(updatedFacility);
                            // 更新图形项上的设施名称
                            QString newFacilityName = updatedFacility.facilityName();
                            m_selectedItem->setData(3, newFacilityName);
                            // 更新tooltip
                            QString typeName = m_selectedItem->data(2).toString();
                            QString facilityId = m_selectedItem->data(1).toString();
                            m_selectedItem->setToolTip(QString("%1\n类型: %2")
                                                       .arg(newFacilityName.isEmpty() ? facilityId : newFacilityName)
                                                       .arg(typeName));
                            found = true;
                            qDebug() << "[Edit Facility] Updated ChangeModified record for existing facility";
                            break;
                        }
                    }
                }
                
                if (!found) {
                    // 添加到待保存变更列表（这种情况应该是编辑已保存的设施）
                    PendingChange change;
                    change.type = ChangeModified;
                    change.entityType = "facility";
                    change.data = QVariant::fromValue(updatedFacility);
                    change.originalId = databaseId;
                    change.graphicsItem = m_selectedItem;
                    m_pendingChanges.append(change);
                    qDebug() << "[Edit Facility] Added new ChangeModified record";
                }
                
                // 更新图形项上的设施名称（无论是否找到现有记录都要更新）
                QString newFacilityName = updatedFacility.facilityName();
                m_selectedItem->setData(3, newFacilityName);
                // 更新tooltip
                QString typeName = m_selectedItem->data(2).toString();
                QString facilityId = m_selectedItem->data(1).toString();
                m_selectedItem->setToolTip(QString("%1\n类型: %2")
                                           .arg(newFacilityName.isEmpty() ? facilityId : newFacilityName)
                                           .arg(typeName));
                
                // 更新图形项的状态
                QVariant stateVariant = m_selectedItem->data(100);
                EntityState currentState = EntityState::Detached;
                if (stateVariant.isValid()) {
                    currentState = static_cast<EntityState>(stateVariant.toInt());
                }
                // 只有未变更状态才标记为修改
                if (currentState == EntityState::Unchanged) {
                    m_selectedItem->setData(100, static_cast<int>(EntityState::Modified));
                }
                
                markAsModified();
                
                QMessageBox::information(this, "提示", 
                    QString("设施 %1 已标记为待保存\n\n请点击主窗口的保存按钮或按 Ctrl+S 保存到数据库")
                    .arg(updatedFacility.facilityId()));
                updateStatus(QString("设施已修改（待保存，共 %1 项待保存）").arg(m_pendingChanges.size()));
            }
        }
        
        delete dialog;
    }
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
            // 管线ID存储在data(1)，data(2)存储的是管线类型
            QString pipelineId = m_selectedItem->data(1).toString();
            if (!pipelineId.isEmpty() && DatabaseManager::instance().isConnected()) {
                PipelineDAO dao;
                pipeline = dao.findByPipelineId(pipelineId);
                if (pipeline.isValid()) {
                    found = true;
                } else {
                    qDebug() << "[ViewProperties] Pipeline not found in database, pipelineId:" << pipelineId;
                    qDebug() << "[ViewProperties] Item data(0):" << m_selectedItem->data(0);
                    qDebug() << "[ViewProperties] Item data(1):" << m_selectedItem->data(1);
                    qDebug() << "[ViewProperties] Item data(2):" << m_selectedItem->data(2);
                    qDebug() << "[ViewProperties] Item data(10):" << m_selectedItem->data(10);
                }
            } else {
                qDebug() << "[ViewProperties] PipelineId is empty or database not connected";
                qDebug() << "[ViewProperties] Item data(0):" << m_selectedItem->data(0);
                qDebug() << "[ViewProperties] Item data(1):" << m_selectedItem->data(1);
                qDebug() << "[ViewProperties] Item data(2):" << m_selectedItem->data(2);
            }
        }
        
        if (!found) {
            // 如果管线不在数据库中，尝试从图形项中获取基本信息
            QString pipelineId = m_selectedItem->data(1).toString();
            QString pipelineName = m_selectedItem->data(3).toString();
            QString pipelineType = m_selectedItem->data(2).toString();
            
            // 检查实体状态
            QVariant stateVariant = m_selectedItem->data(100);
            EntityState entityState = EntityState::Detached;
            if (stateVariant.isValid()) {
                entityState = static_cast<EntityState>(stateVariant.toInt());
            }
            
            // 如果是新添加的管线（还没保存），显示基本信息
            if (entityState == EntityState::Added && !pipelineId.isEmpty()) {
                // 从图形项构建基本的管线对象
                pipeline.setPipelineId(pipelineId);
                pipeline.setPipelineName(pipelineName);
                pipeline.setPipelineType(pipelineType);
                found = true;
            } else {
                QMessageBox::warning(this, "错误", 
                    QString("未找到管线数据！\n\n管线编号: %1\n\n提示：如果这是新绘制的管线，请先保存到数据库后再查看。")
                    .arg(pipelineId.isEmpty() ? "未知" : pipelineId));
                return;
            }
        }
        
        // 显示属性信息（使用新的查看对话框）
        EntityViewDialog *viewDialog = new EntityViewDialog(this);
        viewDialog->setPipeline(pipeline);
        connect(viewDialog, &EntityViewDialog::editRequested, this, &MyForm::onEditSelectedEntity);
        viewDialog->exec();
    } else if (entityType == "facility") {
        Facility facility;
        bool found = false;
        
        // 获取设施ID（用于数据库查询和调试）
        QString facilityId = m_selectedItem->data(1).toString();
        if (facilityId.isEmpty()) {
            facilityId = m_selectedItem->data(2).toString();
        }
        
        qDebug() << "[View Facility] Searching for facility, graphicsItem:" << m_selectedItem 
                 << "facilityId:" << facilityId 
                 << "pendingChanges count:" << m_pendingChanges.size();
        
        // 优先检查待保存的变更列表（新绘制的设施可能还没有保存到数据库）
        // 先通过图形项匹配（最可靠），如果失败则通过 facilityId 匹配
        for (const PendingChange &change : m_pendingChanges) {
            if (change.entityType == "facility") {
                bool itemMatches = (change.graphicsItem == m_selectedItem);
                bool idMatches = false;
                if (!itemMatches && !facilityId.isEmpty()) {
                    Facility changeFacility = change.data.value<Facility>();
                    // 检查 facilityId 是否匹配（即使 isValid() 返回 false）
                    idMatches = (!changeFacility.facilityId().isEmpty() && 
                                changeFacility.facilityId() == facilityId);
                }
                
                if (itemMatches || idMatches) {
                    if (change.type == ChangeAdded || change.type == ChangeModified) {
                        // 从待保存的变更中获取设施数据
                        facility = change.data.value<Facility>();
                        // 对于新绘制的设施，m_id 为 0，所以不能用 isValid() 判断
                        // 只要 facilityId 和 facilityType 不为空就认为有效
                        if (!facility.facilityId().isEmpty() && !facility.facilityType().isEmpty()) {
                            found = true;
                            qDebug() << "[View Facility] Found in pending changes (type=" << change.type 
                                     << ", match=" << (itemMatches ? "graphicsItem" : "facilityId")
                                     << "), facilityId:" << facility.facilityId()
                                     << "facilityName:" << facility.facilityName();
                            break;
                        } else {
                            qDebug() << "[View Facility] Found change but facility data incomplete:"
                                     << "facilityId=" << facility.facilityId()
                                     << "facilityType=" << facility.facilityType();
                        }
                    }
                }
            }
        }
        
        // 如果待保存列表中没有，尝试从数据库查询
        if (!found && !facilityId.isEmpty() && DatabaseManager::instance().isConnected()) {
            FacilityDAO dao;
            facility = dao.findByFacilityId(facilityId);
            if (facility.isValid()) {
                found = true;
                qDebug() << "[View Facility] Found in database, facilityId:" << facilityId;
            }
        }
        
        if (!found) {
            qDebug() << "[View Facility] NOT FOUND - graphicsItem:" << m_selectedItem 
                     << "facilityId:" << facilityId 
                     << "pendingChanges:" << m_pendingChanges.size();
            QMessageBox::warning(this, "错误", 
                QString("未找到设施数据！\n\n设施ID: %1\n\n提示：如果这是新绘制的设施，请先保存到数据库后再查看。")
                .arg(facilityId.isEmpty() ? "未知" : facilityId));
            return;
        }
        
        // 显示设施属性信息（使用新的查看对话框）
        EntityViewDialog *viewDialog = new EntityViewDialog(this);
        viewDialog->setFacility(facility);
        connect(viewDialog, &EntityViewDialog::editRequested, this, &MyForm::onEditSelectedEntity);
        viewDialog->exec();
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
        
        // 确保不显示Qt默认的选中虚线框
        m_selectedItem->setSelected(false);
        
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
    
    // 禁用Qt默认的选中样式（虚线框），使用自定义高亮
    // 不调用setSelected(true)，避免显示默认的虚线框
    item->setSelected(false);
    
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
        
        // 创建高亮样式（加粗边框 + 高亮填充）
        QPen highlightPen = m_originalPen;
        highlightPen.setWidth(m_originalPen.width() + 3);  // 增加边框宽度，更明显
        highlightPen.setColor(QColor(255, 215, 0));  // 金色边框
        
        // 保持原始颜色但增加亮度，或使用半透明黄色叠加
        QColor originalColor = m_originalBrush.color();
        QColor highlightColor;
        if (originalColor.alpha() > 0 && originalColor != Qt::transparent) {
            // 如果原色有透明度，创建更亮的版本
            highlightColor = QColor(
                qMin(255, originalColor.red() + 60),
                qMin(255, originalColor.green() + 60),
                qMin(255, originalColor.blue() + 60),
                qMin(255, originalColor.alpha() + 50)
            );
        } else {
            // 如果原色不透明，使用半透明黄色叠加，更明显
            highlightColor = QColor(255, 255, 0, 200);  // 更明显的黄色高亮
        }
        
        QBrush highlightBrush = QBrush(highlightColor);
        
        ellipseItem->setPen(highlightPen);
        ellipseItem->setBrush(highlightBrush);
        ellipseItem->setZValue(200);  // 提升层级
        
        // 确保不显示Qt默认的选中虚线框
        ellipseItem->setSelected(false);
        
        qDebug() << "✨ Facility highlighted (no dashed border)";
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
        
        // 确保不显示Qt默认的选中虚线框
        ellipseItem->setSelected(false);
        
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
