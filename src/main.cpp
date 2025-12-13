// main.cpp
#include <QApplication>
#include <QLabel>
#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QDir>
#include <QFileInfo>
#include "widgets/basewindow.h"
#include "ui/myform.h"
#include "widgets/logindialog.h"
#include "core/auth/sessionmanager.h"
#include "core/common/config.h"
#include "core/common/logger.h"
#include "core/database/databasemanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 启用控制台输出
    qSetMessagePattern("[%{time yyyy-MM-dd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}");
    
    qDebug() << "=== Application started ===";

    // 设置应用信息（影响任务栏/dock）
    app.setApplicationName("UGIMS");
    app.setOrganizationName("MyOrg");

    // 设置窗口图标（替换为你的图标路径）
    app.setWindowIcon(QIcon(":/new/prefix1/images/OrangeCat.png")); // 若使用资源文件

    // === 加载 QSS 样式表 ===
    QFile styleFile(":/new/prefix1/styles/style.qss"); // 如果用资源文件
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
        styleFile.close();
    }

    // === 初始化数据库连接（在登录前） ===
    qDebug() << "Initializing database connection...";
    
    // 初始化日志
    try {
        QString logDir = QDir::currentPath() + "/logs";
        if (!QDir(logDir).exists()) {
            QDir().mkpath(logDir);
        }
        Logger::instance().initialize(logDir + "/app.log", Logger::Info);
        LOG_INFO("Logger initialized");
    } catch (...) {
        qDebug() << "Warning: Logger initialization failed";
    }
    
    // 加载配置
    QString appConfigPath = QDir::currentPath() + "/config/app.ini";
    QString dbConfigPath = QDir::currentPath() + "/config/database.ini";
    
    if (QFileInfo::exists(appConfigPath)) {
        Config::instance().initialize(appConfigPath);
        qDebug() << "App config loaded";
    } else {
        qDebug() << "Warning: app.ini not found at" << appConfigPath;
    }
    
    if (QFileInfo::exists(dbConfigPath)) {
        if (Config::instance().loadDatabaseConfig(dbConfigPath)) {
            qDebug() << "Database config loaded";
            
            // 初始化并连接数据库
            DatabaseManager &db = DatabaseManager::instance();
            if (db.initialize()) {
                if (db.connect()) {
                    qDebug() << "Database connected successfully";
                } else {
                    qDebug() << "Warning: Database connection failed:" << db.lastError();
                    qDebug() << "Login will still work, but some features may be unavailable";
                }
            } else {
                qDebug() << "Warning: Database initialization failed";
            }
        } else {
            qDebug() << "Warning: Failed to load database config";
        }
    } else {
        qDebug() << "Warning: database.ini not found at" << dbConfigPath;
        qDebug() << "Login will still work, but database features will be unavailable";
    }

    // 显示登录对话框
    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        qDebug() << "Login cancelled, exiting application";
        return 0;  // 用户取消登录，退出应用
    }
    
    qDebug() << "Login successful, user:" << SessionManager::instance().currentUsername();
    
    BaseWindow window;
    MyForm *form = new MyForm(&window); // parent 设为 window，自动管理内存
    window.setContentWidget(form);
    // 启动时以正常窗口显示
    window.resize(1200, 800);  // 设置初始窗口大小
    
    // 将窗口居中显示在屏幕中间
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - window.width()) / 2;
        int y = (screenGeometry.height() - window.height()) / 2;
        window.move(x, y);
    }
    
    window.show();

    qDebug() << "=== Application window shown ===";
    return app.exec();
}
