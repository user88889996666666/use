#include "hardwareinterface.h"

#include <QDateTime>
#include <QtGlobal>

HardwareInterface::HardwareInterface(QObject *parent)
    : QObject(parent)
    , m_seed(static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() & 0x7fffffff))
    , m_ledBrightness(0)
    , m_buzzerEnabled(false)
{
}

double HardwareInterface::readTemperature() const
{
    return nextValue(16.0, 40.0);
}

double HardwareInterface::readHumidity() const
{
    return nextValue(25.0, 95.0);
}

double HardwareInterface::readLight() const
{
    return nextValue(0.0, 1000.0);
}

double HardwareInterface::readPressure() const
{
    return nextValue(940.0, 1080.0);
}

double HardwareInterface::readAltitude() const
{
    return nextValue(10.0, 260.0);
}

double HardwareInterface::readGasConcentration() const
{
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
