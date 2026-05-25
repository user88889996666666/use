#include "alarmsystem.h"
#include "hardwareinterface.h"

namespace {
constexpr double kPressureGap = 1.0;
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

    if (data.temperature > m_thresholds.highTemperature) {
        status.active = true;
        status.message = tr("温度过高报警");
    } else if (data.humidity > m_thresholds.highHumidity) {
        status.active = true;
        status.message = tr("湿度过高报警");
    } else if (data.pressure < m_thresholds.lowPressure || data.pressure > m_thresholds.highPressure) {
        status.active = true;
        status.message = tr("气压异常报警");
    } else if (data.gasConcentration > m_thresholds.highGas) {
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

void AlarmSystem::setThresholds(const AlarmThresholds &thresholds)
{
    m_thresholds = thresholds;
    if (m_thresholds.highPressure <= m_thresholds.lowPressure) {
        m_thresholds.highPressure = m_thresholds.lowPressure + kPressureGap;
    }
}

AlarmThresholds AlarmSystem::thresholds() const
{
    return m_thresholds;
}
