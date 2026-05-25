#include "alarmsystem.h"
#include "hardwareinterface.h"

AlarmSystem::AlarmSystem(HardwareInterface *hardware, QObject *parent)
    : QObject(parent)
    , m_hardware(hardware)
    , m_alarmActive(false)
{
}

AlarmStatus AlarmSystem::evaluate(const SensorData &data)
{
    AlarmStatus status;

    if (data.temperature > 35.0) {
        status.active = true;
        status.message = QStringLiteral("温度过高报警");
    } else if (data.humidity > 85.0) {
        status.active = true;
        status.message = QStringLiteral("湿度过高报警");
    } else if (data.pressure < 960.0 || data.pressure > 1040.0) {
        status.active = true;
        status.message = QStringLiteral("气压异常报警");
    } else if (data.gasConcentration > 300.0) {
        status.active = true;
        status.message = QStringLiteral("可燃气浓度超限报警");
    }

    m_hardware->setBuzzer(status.active);

    if (status.active && !m_alarmActive) {
        emit alarmTriggered(status.message);
    } else if (!status.active && m_alarmActive) {
        emit alarmCleared();
    }

    m_alarmActive = status.active;
    return status;
}
