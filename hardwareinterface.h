#ifndef HARDWAREINTERFACE_H
#define HARDWAREINTERFACE_H

#include <QObject>

class HardwareInterface : public QObject
{
    Q_OBJECT
public:
    explicit HardwareInterface(QObject *parent = nullptr);

    // 读取环境传感器数据（当前为模拟实现，方便在开发环境运行）
    double readTemperature() const;
    double readHumidity() const;
    double readLight() const;
    double readPressure() const;
    double readAltitude() const;
    double readGasConcentration() const;

    // 执行器控制接口（可在GEC6818上替换为真实GPIO/PWM）
    void setLedBrightness(int brightness);
    int ledBrightness() const;

    void setBuzzer(bool enabled);
    bool buzzerEnabled() const;

private:
    mutable quint32 m_seed;
    int m_ledBrightness;
    bool m_buzzerEnabled;

    double nextValue(double min, double max) const;
};

#endif // HARDWAREINTERFACE_H
