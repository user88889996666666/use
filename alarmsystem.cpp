#include "alarmsystem.h"
#include "hardwareinterface.h"

namespace {
constexpr double kHighTemperatureThreshold = 35.0;
constexpr double kHighHumidityThreshold = 85.0;
constexpr double kLowPressureThreshold = 960.0;
constexpr double kHighPressureThreshold = 1040.0;
constexpr double kHighGasThreshold = 300.0;
}

AlarmSystem::AlarmSystem(HardwareInterface *hardware, QObject *parent)
    : QObject(parent)
    , m_hardware(hardware)
    , m_alarmActive(false)
{
}

AlarmStatus AlarmSystem::evaluate(const SensorData &data)
{
    AlarmStatus status;

    if (data.temperature > kHighTemperatureThreshold) {
        status.active = true;
        status.message = tr("温度过高报警");
    } else if (data.humidity > kHighHumidityThreshold) {
        status.active = true;
        status.message = tr("湿度过高报警");
    } else if (data.pressure < kLowPressureThreshold || data.pressure > kHighPressureThreshold) {
        status.active = true;
        status.message = tr("气压异常报警");
    } else if (data.gasConcentration > kHighGasThreshold) {
        status.active = true;
        status.message = tr("可燃气浓度超限报警");
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
