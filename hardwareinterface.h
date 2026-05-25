#ifndef HARDWAREINTERFACE_H
#define HARDWAREINTERFACE_H

#include <QObject>

class GY39Sensor;

class HardwareInterface : public QObject
{
    Q_OBJECT
public:
    explicit HardwareInterface(QObject *parent = nullptr);
    ~HardwareInterface();

    // 初始化GY-39传感器
    bool initializeGY39(const QString &portName = "COM3", int baudRate = 9600);
    bool isGY39Connected() const;

    // 读取环境传感器数据（优先从GY-39读取，否则使用模拟数据）
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

signals:
    void gy39Connected();
    void gy39Disconnected();

private:
    GY39Sensor *m_gy39Sensor;
    mutable quint32 m_seed;
    int m_ledBrightness;
    bool m_buzzerEnabled;

    double nextValue(double min, double max) const;
};

#endif // HARDWAREINTERFACE_H
