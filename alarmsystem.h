#ifndef ALARMSYSTEM_H
#define ALARMSYSTEM_H

#include <QObject>

#include "sensormanager.h"

class HardwareInterface;

struct AlarmStatus {
    bool active = false;
    QString message;
};

class AlarmSystem : public QObject
{
    Q_OBJECT
public:
    explicit AlarmSystem(HardwareInterface *hardware, QObject *parent = nullptr);

    // 根据多参数阈值进行报警判定并控制蜂鸣器
    AlarmStatus evaluate(const SensorData &data);

signals:
    void alarmTriggered(const QString &message);
    void alarmCleared();

private:
    HardwareInterface *m_hardware;
    bool m_alarmActive;
};

#endif // ALARMSYSTEM_H
