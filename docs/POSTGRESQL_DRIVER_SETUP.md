# PostgreSQL 驱动设置指南

## 问题：Driver not loaded

如果遇到 "Driver not loaded" 或 "QPSQL driver not available" 错误，说明Qt的PostgreSQL驱动未正确安装。

## 解决方案

### 方法1：检查Qt是否包含PostgreSQL驱动

在代码中添加以下检查：

```cpp
qDebug() << "Available drivers:" << QSqlDatabase::drivers();
```

如果输出中没有 "QPSQL"，说明驱动未安装。

### 方法2：使用Qt Maintenance Tool安装驱动

1. 打开 Qt Maintenance Tool
2. 选择 "Add or remove components"
3. 展开你的Qt版本
4. 找到 "Qt SQL Drivers"
5. 勾选 "PostgreSQL"
6. 完成安装

### 方法3：手动编译PostgreSQL驱动

如果使用源码编译的Qt，需要手动编译驱动：

```bash
cd $QTDIR/src/plugins/sqldrivers/psql
qmake
make
```

然后将生成的 `qsqlpsql.dll` (Windows) 或 `libqsqlpsql.so` (Linux) 复制到：
- Windows: `Qt安装目录/plugins/sqldrivers/`
- Linux: `Qt安装目录/plugins/sqldrivers/`

### 方法4：使用SQLite作为临时替代

如果无法安装PostgreSQL驱动，可以临时使用SQLite：

修改 `config/database.ini`:

```ini
[database]
type=sqlite
sqlite_path=data/ugims.db
```

注意：SQLite不支持PostGIS空间功能，某些功能可能受限。

## 验证驱动安装

运行以下代码验证：

```cpp
#include <QSqlDatabase>
#include <QDebug>

qDebug() << "Available SQL drivers:";
foreach(QString driver, QSqlDatabase::drivers()) {
    qDebug() << "  -" << driver;
}

if (QSqlDatabase::isDriverAvailable("QPSQL")) {
    qDebug() << "PostgreSQL driver is available";
} else {
    qDebug() << "PostgreSQL driver is NOT available";
}
```

## Windows特定说明

在Windows上，PostgreSQL驱动需要：
1. PostgreSQL客户端库（libpq.dll）
2. Qt PostgreSQL插件（qsqlpsql.dll）

确保这两个文件都在正确的位置：
- `libpq.dll` 应该在系统PATH或应用目录
- `qsqlpsql.dll` 应该在 `plugins/sqldrivers/` 目录

## 快速检查命令

在应用启动时，检查日志输出中的 "Available drivers" 信息，确认是否包含 "QPSQL"。

