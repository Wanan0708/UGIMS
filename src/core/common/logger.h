#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

/**
 * @brief 日志工具类
 * 线程安全的日志记录器，支持多种日志级别
 */
class Logger
{
public:
    enum LogLevel {
        Debug = 0,
        Info = 1,
        Warning = 2,
        Error = 3,
        Critical = 4
    };

    // 获取单例实例
    static Logger& instance();

    // 初始化日志系统
    void initialize(const QString &logFilePath, LogLevel level = Info);

    // 日志记录方法
    void debug(const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    void critical(const QString &message);

    // 设置日志级别
    void setLogLevel(LogLevel level);

    // 禁用拷贝构造和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    void log(LogLevel level, const QString &message);
    QString levelToString(LogLevel level) const;

    QMutex m_mutex;
    QFile m_logFile;
    QTextStream m_logStream;
    LogLevel m_logLevel;
    bool m_initialized;
};

// 便捷宏定义
#define LOG_DEBUG(msg)    Logger::instance().debug(msg)
#define LOG_INFO(msg)     Logger::instance().info(msg)
#define LOG_WARNING(msg)  Logger::instance().warning(msg)
#define LOG_ERROR(msg)    Logger::instance().error(msg)
#define LOG_CRITICAL(msg) Logger::instance().critical(msg)

#endif // LOGGER_H

