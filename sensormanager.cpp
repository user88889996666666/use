#include "sensormanager.h"
#include "hardwareinterface.h"

#include <QTimer>

SensorManager::SensorManager(HardwareInterface *hardware, QObject *parent)
    : QObject(parent)
    , m_hardware(hardware)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &SensorManager::collectData);
}

void SensorManager::start(int intervalMs)
{
    m_timer->start(intervalMs);
    collectData();
}

void SensorManager::stop()
{
    m_timer->stop();
}

void SensorManager::collectData()
{
    SensorData data;
    data.temperature = m_hardware->readTemperature();
    data.humidity = m_hardware->readHumidity();
    data.light = m_hardware->readLight();
    data.pressure = m_hardware->readPressure();
    data.altitude = m_hardware->readAltitude();
    data.gasConcentration = m_hardware->readGasConcentration();
    data.timestamp = QDateTime::currentDateTime();

    emit sensorDataUpdated(data);
}
