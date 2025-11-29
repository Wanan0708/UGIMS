// main.cpp
#include <QApplication>
#include <QLabel>
#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include "widgets/basewindow.h"
#include "ui/myform.h"

// class MyWindow : public BaseWindow {
// public:
//     MyWindow() : BaseWindow() {
//         auto label = new QLabel("Hello! This is a fully custom title bar window.\n"
//                                 "Try resizing, dragging, maximizing, and minimizing!");
//         label->setAlignment(Qt::AlignCenter);
//         label->setStyleSheet("font-size: 16px;");
//         setContentWidget(label); // 使用 setContentWidget，不是 setCentralWidget
//     }
// };

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 启用控制台输出
    qSetMessagePattern("[%{time yyyy-MM-dd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}");
    
    qDebug() << "=== Application started ===";

    // 设置应用信息（影响任务栏/dock）
    app.setApplicationName("CustomTitleBarApp");
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