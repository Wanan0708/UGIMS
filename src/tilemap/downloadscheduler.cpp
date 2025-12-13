#include "downloadscheduler.h"
#include <QtGlobal>
#include <QtMath>
#include "tilemapmanager.h"

DownloadScheduler::DownloadScheduler(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &DownloadScheduler::onTick);
}

void DownloadScheduler::configure(const MapManagerSettings &settings)
{
    m_settings = settings;
    int intervalMs = qMax(50, 1000 / qMax(1, settings.rateLimitPerSec));
    m_timer.setInterval(intervalMs);
}

void DownloadScheduler::setManifest(ManifestStore *store)
{
    m_store = store;
}

void DownloadScheduler::setTileManager(TileMapManager *mgr)
{
    m_mgr = mgr;
    if (m_mgr) {
        QObject::connect(m_mgr, &TileMapManager::tileCached,
                         this, &DownloadScheduler::onTileCached);
    }
}

void DownloadScheduler::start()
{
    if (!m_timer.isActive()) m_timer.start();
}

void DownloadScheduler::pause()
{
    m_timer.stop();
}

void DownloadScheduler::enqueueTask(const DownloadTask &task)
{
    if (!m_store) return;
    m_store->upsertTask(task);
    m_store->save();
}

void DownloadScheduler::pauseTask(const QString &taskId)
{
    if (!m_store) return;
    
    // 从队列中移除该任务的所有job
    QQueue<TileJob> newQueue;
    while (!m_queue.isEmpty()) {
        TileJob job = m_queue.dequeue();
        if (job.taskId != taskId) {
            newQueue.enqueue(job);
        }
    }
    m_queue = newQueue;
    
    // 更新任务状态
    m_store->setStatus(taskId, "paused");
    m_store->save();
    
    emit taskStatusChanged(taskId, "paused");
}

void DownloadScheduler::resumeTask(const QString &taskId)
{
    if (!m_store) return;
    
    // 检查任务是否存在且状态为paused
    DownloadTask task = m_store->getTask(taskId);
    if (task.id.isEmpty() || task.status != "paused") {
        return;  // 任务不存在或不是暂停状态
    }
    
    // 将状态改为pending
    m_store->setStatus(taskId, "pending");
    m_store->save();
    
    // 重新构建队列（包含该任务）
    m_queueBuilt = false;
    
    emit taskStatusChanged(taskId, "pending");
}

void DownloadScheduler::cancelTask(const QString &taskId)
{
    if (!m_store) return;
    
    // 从队列中移除该任务的所有job
    QQueue<TileJob> newQueue;
    while (!m_queue.isEmpty()) {
        TileJob job = m_queue.dequeue();
        if (job.taskId != taskId) {
            newQueue.enqueue(job);
        }
    }
    m_queue = newQueue;
    
    // 从outstanding中移除该任务的job（如果正在下载）
    QHash<quint64, QString> newOutstanding;
    for (auto it = m_outstanding.begin(); it != m_outstanding.end(); ++it) {
        if (it.value() != taskId) {
            newOutstanding.insert(it.key(), it.value());
        } else {
            // 减少正在下载的数量
            m_inflight = qMax(0, m_inflight - 1);
        }
    }
    m_outstanding = newOutstanding;
    
    // 更新任务状态
    m_store->setStatus(taskId, "cancelled");
    m_store->save();
    
    emit taskStatusChanged(taskId, "cancelled");
}

void DownloadScheduler::onTick()
{
    if (!m_store || !m_mgr) return;
    if (!m_queueBuilt) {
        buildQueueFromTasks();
        m_queueBuilt = true;
        // 标记任务为 downloading
        for (const auto &t : m_store->tasks()) {
            if (t.status != "downloading" && t.status != "completed") {
                const_cast<ManifestStore*>(m_store)->setStatus(t.id, "downloading");
            }
        }
        const_cast<ManifestStore*>(m_store)->save();
    }
    // 简易并发控制
    if (m_inflight >= m_settings.maxConcurrent) return;
    if (m_queue.isEmpty()) {
        emit allTasksFinished();
        return;
    }
    auto job = m_queue.dequeue();
    m_inflight++;
    // 先登记映射，避免本地命中时回调不会匹配的问题
    m_outstanding.insert(packKey(job.x,job.y,job.z), job.taskId);
    m_mgr->enqueueDownload(job.x, job.y, job.z);
}

static void clampTileRange(int z, int &minX, int &maxX, int &minY, int &maxY)
{
    int n = 1 << z;
    minX = qMax(0, minX); maxX = qMin(n - 1, maxX);
    minY = qMax(0, minY); maxY = qMin(n - 1, maxY);
}

void DownloadScheduler::buildQueueFromTasks()
{
    m_queue.clear();
    if (!m_store) return;
    auto tasks = m_store->tasks();
    for (const auto &t : tasks) {
        // 只处理pending和downloading状态的任务
        if (t.status != "pending" && t.status != "downloading") {
            continue;
        }
        
        int taskTotal = 0;
        for (int z = t.minZoom; z <= t.maxZoom; ++z) {
            int n = 1 << z;
            int minX = int((t.minLon + 180.0) / 360.0 * n);
            int maxX = int((t.maxLon + 180.0) / 360.0 * n);
            auto lat2y = [n](double lat){ double r = lat * M_PI / 180.0; return int((1.0 - log(tan(r) + 1.0 / cos(r)) / M_PI) / 2.0 * n); };
            int minY = lat2y(t.maxLat); // 注意: 瓦片 Y 轴向下
            int maxY = lat2y(t.minLat);
            clampTileRange(z, minX, maxX, minY, maxY);
            for (int x = minX; x <= maxX; ++x) {
                for (int y = minY; y <= maxY; ++y) {
                    m_queue.enqueue({t.id, x, y, z});
                    taskTotal++;
                }
            }
        }
        if (m_store) const_cast<ManifestStore*>(m_store)->setTotalTiles(t.id, taskTotal);
    }
    if (m_store) const_cast<ManifestStore*>(m_store)->save();
}

void DownloadScheduler::onTileCached(int x, int y, int z, bool success)
{
    Q_UNUSED(success);
    auto key = packKey(x,y,z);
    if (m_outstanding.contains(key)) {
        QString taskId = m_outstanding.take(key);
        m_inflight = qMax(0, m_inflight - 1);
        if (m_store) {
            const_cast<ManifestStore*>(m_store)->updateProgress(taskId, success ? 1 : 0, success ? 0 : 1);
            const_cast<ManifestStore*>(m_store)->save();
            // 触发进度信号（读一遍任务得到总数与完成数）
            auto t = m_store->getTask(taskId);
            emit taskProgress(taskId, t.completedTiles, t.totalTiles);
        }
        // emit taskProgress(taskId, ...); // 后续补充具体值
    }
}


