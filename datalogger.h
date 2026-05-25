#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <QObject>

#include "alarmsystem.h"

class DataLogger : public QObject
{
    Q_OBJECT
public:
    explicit DataLogger(QObject *parent = nullptr);

    void setLogFilePath(const QString &filePath);
    QString logFilePath() const;

    // 以CSV格式持久化历史数据
    bool log(const SensorData &data, const AlarmStatus &alarm);

private:
    QString m_logFilePath;
    bool ensureHeader();
};

#endif // DATALOGGER_H
