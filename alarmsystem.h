#ifndef ALARMSYSTEM_H
#define ALARMSYSTEM_H

#include <QObject>

#include "sensormanager.h"

class HardwareInterface;

struct AlarmStatus {
    bool active = false;
    QString message;
};

struct AlarmThresholds {
    double highTemperature = 35.0;
    double highHumidity = 85.0;
    double lowPressure = 960.0;
    double highPressure = 1040.0;
    double highGas = 300.0;
};

class AlarmSystem : public QObject
{
    Q_OBJECT
public:
    explicit AlarmSystem(HardwareInterface *hardware, QObject *parent = nullptr);

    // 根据多参数阈值进行报警判定并控制蜂鸣器
    AlarmStatus evaluate(const SensorData &data);
    void setThresholds(const AlarmThresholds &thresholds);
    AlarmThresholds thresholds() const;

signals:
    void alarmTriggered(const QString &message);
    void alarmCleared();

private:
    HardwareInterface *m_hardware;
    bool m_alarmActive;
    AlarmThresholds m_thresholds;
};

#endif // ALARMSYSTEM_H
