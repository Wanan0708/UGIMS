#include "core/common/logger.h"
#include <QDir>
#include <QDebug>
#include <QDateTime>

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
    : m_logLevel(Info)
    , m_initialized(false)
{
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

void Logger::initialize(const QString &logFilePath, LogLevel level)
{
    QMutexLocker locker(&m_mutex);

    if (m_initialized) {
        return;
    }

    // 确保日志目录存在
    QFileInfo fileInfo(logFilePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_logFile.setFileName(logFilePath);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        m_logLevel = level;
        m_initialized = true;

        // 直接写初始化日志，避免递归锁死锁
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString logMessage = QString("[%1] [INFO ] Logger initialized. Log file: %2").arg(timestamp, logFilePath);
        m_logStream << logMessage << "\n";
        m_logStream.flush();
        qDebug().noquote() << logMessage;
    } else {
        qCritical() << "Failed to open log file:" << logFilePath;
    }
}

void Logger::setLogLevel(LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void Logger::debug(const QString &message)
{
    log(Debug, message);
}

void Logger::info(const QString &message)
{
    log(Info, message);
}

void Logger::warning(const QString &message)
{
    log(Warning, message);
}

void Logger::error(const QString &message)
{
    log(Error, message);
}

void Logger::critical(const QString &message)
{
    log(Critical, message);
}

void Logger::log(LogLevel level, const QString &message)
{
    if (level < m_logLevel) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);

    // 写入文件
    if (m_initialized && m_logFile.isOpen()) {
        m_logStream << logMessage << "\n";
        m_logStream.flush();
    }

    // 同时输出到控制台
    qDebug().noquote() << logMessage;
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
    case Debug:    return "DEBUG";
    case Info:     return "INFO ";
    case Warning:  return "WARN ";
    case Error:    return "ERROR";
    case Critical: return "CRIT ";
    default:       return "UNKNOWN";
    }
}

