#include "hardwareinterface.h"
#include "gy39sensor.h"

#include <QDateTime>
#include <QtGlobal>

HardwareInterface::HardwareInterface(QObject *parent)
    : QObject(parent)
    , m_gy39Sensor(new GY39Sensor(this))
    // Mask to positive 31-bit range so the pseudo-random sequence starts from a stable non-negative seed.
    , m_seed(static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() & 0x7fffffff))
    , m_ledBrightness(0)
    , m_buzzerEnabled(false)
{
    connect(m_gy39Sensor, &GY39Sensor::connectionStatusChanged, this, [this](bool connected) {
        if (connected) {
            emit gy39Connected();
        } else {
            emit gy39Disconnected();
        }
    });
}

HardwareInterface::~HardwareInterface()
{
}

bool HardwareInterface::initializeGY39(const QString &portName, int baudRate)
{
    return m_gy39Sensor->openPort(portName, baudRate);
}

bool HardwareInterface::isGY39Connected() const
{
    return m_gy39Sensor->isConnected();
}

void HardwareInterface::refreshGY39Data() const
{
    if (m_gy39Sensor->isConnected()) {
        m_gy39Sensor->requestAllData();
    }
}

double HardwareInterface::readTemperature() const
{
    if (m_gy39Sensor->isConnected()) {
        GY39Data data = m_gy39Sensor->getCurrentData();
        if (data.isValid) {
            return data.temperature;
        }
    }
    return nextValue(16.0, 40.0);
}

double HardwareInterface::readHumidity() const
{
    if (m_gy39Sensor->isConnected()) {
        GY39Data data = m_gy39Sensor->getCurrentData();
        if (data.isValid) {
            return data.humidity;
        }
    }
    return nextValue(25.0, 95.0);
}

double HardwareInterface::readLight() const
{
    if (m_gy39Sensor->isConnected()) {
        GY39Data data = m_gy39Sensor->getCurrentData();
        if (data.isValid) {
            return data.light;
        }
    }
    return nextValue(0.0, 1000.0);
}

double HardwareInterface::readPressure() const
{
    if (m_gy39Sensor->isConnected()) {
        GY39Data data = m_gy39Sensor->getCurrentData();
        if (data.isValid) {
            return data.pressure;
        }
    }
    return nextValue(940.0, 1080.0);
}

double HardwareInterface::readAltitude() const
{
    if (m_gy39Sensor->isConnected()) {
        GY39Data data = m_gy39Sensor->getCurrentData();
        if (data.isValid) {
            return data.altitude;
        }
    }
    // 若GY-39不可用，返回模拟数据
    return nextValue(10.0, 260.0);
}

double HardwareInterface::readGasConcentration() const
{
    // GY-39 传感器不包含可燃气浓度检测，使用模拟数据
    return nextValue(80.0, 450.0);
}

void HardwareInterface::setLedBrightness(int brightness)
{
    m_ledBrightness = qBound(0, brightness, 100);
}

int HardwareInterface::ledBrightness() const
{
    return m_ledBrightness;
}

void HardwareInterface::setBuzzer(bool enabled)
{
    m_buzzerEnabled = enabled;
}

bool HardwareInterface::buzzerEnabled() const
{
    return m_buzzerEnabled;
}

double HardwareInterface::nextValue(double min, double max) const
{
    m_seed = 1103515245u * m_seed + 12345u;
    const double ratio = static_cast<double>(m_seed % 10000u) / 10000.0;
    return min + ratio * (max - min);
}
