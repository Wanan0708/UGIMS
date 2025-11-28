#include <QCoreApplication>
#include <QDebug>
#include "src/core/common/logger.h"
#include "src/core/common/config.h"
#include "src/core/database/databasemanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "========================================";
    qDebug() << "数据库连接测试";
    qDebug() << "========================================";
    
    // 初始化日志
    try {
        Logger::instance().initialize("logs/test_db.log", Logger::Info);
        LOG_INFO("Logger initialized");
    } catch (...) {
        qDebug() << "日志初始化失败";
    }
    
    // 加载配置
    qDebug() << "\n[1/4] 加载配置文件...";
    try {
        if (!Config::instance().initialize("config/app.ini")) {
            qDebug() << "❌ 加载app.ini失败";
            return 1;
        }
        qDebug() << "✅ app.ini加载成功";
        
        if (!Config::instance().loadDatabaseConfig("config/database.ini")) {
            qDebug() << "❌ 加载database.ini失败";
            return 1;
        }
        qDebug() << "✅ database.ini加载成功";
        
    } catch (const std::exception &e) {
        qDebug() << "❌ 配置加载异常:" << e.what();
        return 1;
    }
    
    // 初始化数据库管理器
    qDebug() << "\n[2/4] 初始化数据库管理器...";
    try {
        DatabaseManager &db = DatabaseManager::instance();
        if (!db.initialize()) {
            qDebug() << "❌ 数据库管理器初始化失败";
            return 1;
        }
        qDebug() << "✅ 数据库管理器初始化成功";
        
    } catch (const std::exception &e) {
        qDebug() << "❌ 初始化异常:" << e.what();
        return 1;
    }
    
    // 连接数据库
    qDebug() << "\n[3/4] 连接数据库...";
    qDebug() << "数据库类型:" << Config::instance().getDatabaseType();
    qDebug() << "主机:" << Config::instance().getDatabaseHost();
    qDebug() << "端口:" << Config::instance().getDatabasePort();
    qDebug() << "数据库名:" << Config::instance().getDatabaseName();
    qDebug() << "用户名:" << Config::instance().getDatabaseUsername();
    
    try {
        DatabaseManager &db = DatabaseManager::instance();
        if (!db.connect()) {
            qDebug() << "❌ 数据库连接失败:" << db.lastError();
            return 1;
        }
        qDebug() << "✅ 数据库连接成功！";
        
    } catch (const std::exception &e) {
        qDebug() << "❌ 连接异常:" << e.what();
        return 1;
    }
    
    // 测试查询
    qDebug() << "\n[4/4] 测试数据查询...";
    try {
        DatabaseManager &db = DatabaseManager::instance();
        
        // 查询管线数据
        QString sql = "SELECT pipeline_type, COUNT(*) as count FROM pipelines GROUP BY pipeline_type";
        QSqlQuery query = db.executeQuery(sql);
        
        qDebug() << "\n=== 管线统计 ===";
        while (query.next()) {
            QString type = query.value(0).toString();
            int count = query.value(1).toInt();
            qDebug() << QString("%1: %2条").arg(type).arg(count);
        }
        
        // 查询设施数据
        sql = "SELECT facility_type, COUNT(*) as count FROM facilities GROUP BY facility_type";
        query = db.executeQuery(sql);
        
        qDebug() << "\n=== 设施统计 ===";
        while (query.next()) {
            QString type = query.value(0).toString();
            int count = query.value(1).toInt();
            qDebug() << QString("%1: %2个").arg(type).arg(count);
        }
        
        qDebug() << "\n✅ 数据查询成功！";
        
    } catch (const std::exception &e) {
        qDebug() << "❌ 查询异常:" << e.what();
        return 1;
    }
    
    qDebug() << "\n========================================";
    qDebug() << "数据库连接测试完成 ✅";
    qDebug() << "========================================";
    
    return 0;
}

