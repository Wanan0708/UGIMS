#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <QObject>
#include <QGraphicsScene>
#include <QHash>
#include <QString>

class PipelineRenderer;
class FacilityRenderer;
class AnnotationRenderer;
class TileMapManager;

/**
 * @brief 图层管理器
 * 管理所有渲染图层的显示、隐藏和更新
 */
class LayerManager : public QObject
{
    Q_OBJECT

public:
    // 图层类型枚举
    enum LayerType {
        BaseMap = 0,        // 底图层
        WaterPipeline,      // 给水管线
        SewagePipeline,     // 排水管线
        GasPipeline,        // 燃气管线
        ElectricPipeline,   // 电力电缆
        TelecomPipeline,    // 通信光缆
        HeatPipeline,       // 供热管线
        Facilities,         // 设施点层
        Labels              // 标注层
    };

    explicit LayerManager(QGraphicsScene *scene, QObject *parent = nullptr);
    ~LayerManager();

    // 设置场景
    void setScene(QGraphicsScene *scene);
    
    // 设置瓦片地图管理器（用于坐标转换）
    void setTileMapManager(TileMapManager *tileMapManager);

    // 图层可见性控制
    void setLayerVisible(LayerType type, bool visible);
    bool isLayerVisible(LayerType type) const;
    void toggleLayer(LayerType type);

    // 获取所有图层类型
    QList<LayerType> getAllLayerTypes() const;
    QString getLayerName(LayerType type) const;

    // 刷新图层（重新加载和渲染数据）
    void refreshLayer(LayerType type);
    void refreshAllLayers();

    // 清空图层
    void clearLayer(LayerType type);
    void clearAllLayers();

    // 获取渲染器
    PipelineRenderer* getPipelineRenderer() const { return m_pipelineRenderer; }
    FacilityRenderer* getFacilityRenderer() const { return m_facilityRenderer; }
    AnnotationRenderer* getAnnotationRenderer() const { return m_annotationRenderer; }

    // 设置可视区域（用于按需加载）
    void setVisibleBounds(const QRectF &bounds);
    QRectF getVisibleBounds() const { return m_visibleBounds; }
    
    // 设置缩放级别（同步到所有渲染器）
    void setZoom(int zoom);
    void setTileSize(int tileSize);

signals:
    // 图层可见性改变信号
    void layerVisibilityChanged(LayerType type, bool visible);
    
    // 图层刷新信号
    void layerRefreshed(LayerType type);
    
    // 数据加载进度信号
    void loadProgress(int current, int total);

public slots:
    // 响应数据变化
    void onDataChanged();

private:
    QGraphicsScene *m_scene;
    TileMapManager *m_tileMapManager;
    
    // 渲染器
    PipelineRenderer *m_pipelineRenderer;
    FacilityRenderer *m_facilityRenderer;
    AnnotationRenderer *m_annotationRenderer;
    
    // 图层可见性状态
    QHash<LayerType, bool> m_layerVisibility;
    
    // 当前可视区域
    QRectF m_visibleBounds;
    
    // 初始化图层
    void initializeLayers();
    
    // 图层名称映射
    static QHash<LayerType, QString> s_layerNames;
};

#endif // LAYERMANAGER_H

