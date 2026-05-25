#ifndef GY39SENSOR_H
#define GY39SENSOR_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>

struct GY39Data {
    double temperature = 0.0;      // 温度，单位°C
    double humidity = 0.0;         // 湿度，单位%RH
    double pressure = 0.0;         // 气压，单位Pa
    double light = 0.0;            // 光强，单位lux
    double altitude = 0.0;         // 海拔，单位m
    bool isValid = false;
};

class GY39Sensor : public QObject
{
    Q_OBJECT
public:
    explicit GY39Sensor(QObject *parent = nullptr);
    ~GY39Sensor();

    // 打开/关闭串口连接
    // portName: 串口名称 (e.g., "COM3", "/dev/ttyUSB0")
    // baudRate: 波特率 (9600 或 115200)
    bool openPort(const QString &portName = "COM3", int baudRate = 9600);
    void closePort();
    bool isConnected() const;

    // 获取最新的传感器数据
    GY39Data getCurrentData() const;

    // 配置输出模式
    // autoOutput: 上电自动输出
    // enableBME: 输出温度/气压/湿度/海拔
    // enableLight: 输出光强
    void configureOutput(bool autoOutput = true, bool enableBME = true, bool enableLight = true);
    void requestLightData();
    void requestEnvironmentData();
    void requestAllData();

signals:
    void dataReceived(const GY39Data &data);
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString &error);

private slots:
    void onSerialDataReady();
    void onSerialError(QSerialPort::SerialPortError error);

private:
    bool parseGY39Frame(const QByteArray &frame);
    bool parseLightFrame(const QByteArray &frame);
    bool parseMultiSensorFrame(const QByteArray &frame);
    void sendCommand(quint8 command);

    QSerialPort *m_serial;
    QByteArray m_receiveBuffer;
    GY39Data m_currentData;
    bool m_isConnected;
};

#endif // GY39SENSOR_H
