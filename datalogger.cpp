#include "datalogger.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

DataLogger::DataLogger(QObject *parent)
    : QObject(parent)
    , m_logFilePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                    + QStringLiteral("/env_history.csv"))
{
}

void DataLogger::setLogFilePath(const QString &filePath)
{
    m_logFilePath = filePath;
}

QString DataLogger::logFilePath() const
{
    return m_logFilePath;
}

bool DataLogger::log(const SensorData &data, const AlarmStatus &alarm)
{
    if (!ensureHeader()) {
        return false;
    }

    QFile file(m_logFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << data.timestamp.toString(Qt::ISODate) << ','
        << data.temperature << ','
        << data.humidity << ','
        << data.light << ','
        << data.pressure << ','
        << data.altitude << ','
        << data.gasConcentration << ','
        << (alarm.active ? QStringLiteral("1") : QStringLiteral("0")) << ','
        << '"' << alarm.message << '"' << '\n';

    return true;
}

bool DataLogger::ensureHeader()
{
    QFile file(m_logFilePath);
    if (file.exists()) {
        return true;
    }

    const int slashPos = m_logFilePath.lastIndexOf('/');
    if (slashPos > 0) {
        const QString dirPath = m_logFilePath.left(slashPos);
        QDir().mkpath(dirPath);
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "timestamp,temperature,humidity,light,pressure,altitude,gas,alarm_active,alarm_message\n";
    return true;
}
