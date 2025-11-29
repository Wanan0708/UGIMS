#ifndef LAYERCONTROLPANEL_H
#define LAYERCONTROLPANEL_H

#include <QWidget>
#include <QCheckBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QLabel>
#include "widgets/collapsiblegroupbox.h"
#include "map/layermanager.h"

/**
 * @brief 图层控制面板
 * 提供图层显示/隐藏、透明度调整等功能
 */
class LayerControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LayerControlPanel(QWidget *parent = nullptr);
    ~LayerControlPanel();

    // 设置图层管理器
    void setLayerManager(LayerManager *layerManager);

    // 刷新面板状态（同步图层管理器的状态）
    void refresh();

signals:
    // 图层可见性改变信号
    void layerVisibilityChanged(LayerManager::LayerType type, bool visible);
    
    // 请求刷新图层信号
    void refreshLayerRequested(LayerManager::LayerType type);

private slots:
    void onLayerCheckBoxToggled(bool checked);
    void onRefreshAllClicked();

private:
    void setupUI();
    void setupConnections();
    
    // 创建图层控制项
    QWidget* createLayerItem(LayerManager::LayerType type, 
                             const QString &name, 
                             const QColor &color);

private:
    LayerManager *m_layerManager;
    
    // 管线图层复选框
    QCheckBox *m_waterPipelineCheck;
    QCheckBox *m_sewagePipelineCheck;
    QCheckBox *m_gasPipelineCheck;
    QCheckBox *m_electricPipelineCheck;
    QCheckBox *m_telecomPipelineCheck;
    QCheckBox *m_heatPipelineCheck;
    
    // 设施图层复选框
    QCheckBox *m_facilitiesCheck;
    QCheckBox *m_labelsCheck;
    
    // 布局
    QVBoxLayout *m_mainLayout;
    CollapsibleGroupBox *m_pipelineGroup;
    CollapsibleGroupBox *m_facilitiesGroup;
    CollapsibleGroupBox *m_otherGroup;
    
    // 刷新按钮
    QPushButton *m_refreshAllBtn;
    
    // 图层类型映射
    QHash<QCheckBox*, LayerManager::LayerType> m_checkBoxMap;
};

#endif // LAYERCONTROLPANEL_H
