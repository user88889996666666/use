#include "gy39sensor.h"

#include <QDebug>

GY39Sensor::GY39Sensor(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
    , m_isConnected(false)
{
    connect(m_serial, &QSerialPort::readyRead, this, &GY39Sensor::onSerialDataReady);
    connect(m_serial, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &GY39Sensor::onSerialError);
}

GY39Sensor::~GY39Sensor()
{
    closePort();
}

bool GY39Sensor::openPort(const QString &portName, int baudRate)
{
    if (m_isConnected) {
        return true;
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(QString("Failed to open port %1: %2").arg(portName, m_serial->errorString()));
        return false;
    }

    m_isConnected = true;
    emit connectionStatusChanged(true);

    // 配置输出模式：启用BME和光强输出，上电自动输出
    configureOutput(true, true, true);

    return true;
}

void GY39Sensor::closePort()
{
    if (m_isConnected) {
        m_serial->close();
        m_isConnected = false;
        emit connectionStatusChanged(false);
    }
}

bool GY39Sensor::isConnected() const
{
    return m_isConnected;
}

GY39Data GY39Sensor::getCurrentData() const
{
    return m_currentData;
}

void GY39Sensor::configureOutput(bool autoOutput, bool enableBME, bool enableLight)
{
    if (!m_isConnected) {
        return;
    }

    // 构建配置字节
    // Bit0: AUTO (1 = 上电按照上次配置输出)
    // Bit1: BME (1 = 输出温度/气压/湿度/海拔)
    // Bit2: MAX (1 = 输出光强)
    quint8 config = 0x00;
    
    if (autoOutput) {
        config |= 0x01;  // AUTO
    }
    if (enableBME) {
        config |= 0x02;  // BME
    }
    if (enableLight) {
        config |= 0x04;  // MAX
    }

    // 命令格式: 0xA5 + 指令 + 校验和
    // 示例: 0xA5 0x87 0x2C (0x87启用AUTO/BME/MAX，校验和=(0xA5+0x87)&0xFF=0x2C)
    quint8 command = 0x80 | config;
    sendCommand(command);
}

void GY39Sensor::sendCommand(quint8 command)
{
    QByteArray cmd;
    cmd.append(static_cast<char>(0xA5));
    cmd.append(static_cast<char>(command));
    
    // 计算校验和
    quint8 checksum = 0xA5 + command;
    cmd.append(static_cast<char>(checksum));

    if (m_serial->write(cmd) == -1) {
        emit errorOccurred(QString("Failed to send command: %1").arg(m_serial->errorString()));
    }
}

void GY39Sensor::onSerialDataReady()
{
    m_receiveBuffer.append(m_serial->readAll());

    // 查找帧头 0x5A 0x5A
    while (m_receiveBuffer.length() >= 2) {
        int headerPos = -1;
        
        // 查找第一个0x5A
        for (int i = 0; i < m_receiveBuffer.length() - 1; ++i) {
            if (static_cast<unsigned char>(m_receiveBuffer[i]) == 0x5A &&
                static_cast<unsigned char>(m_receiveBuffer[i + 1]) == 0x5A) {
                headerPos = i;
                break;
            }
        }

        if (headerPos == -1) {
            // 没有找到帧头，清空缓冲区
            m_receiveBuffer.clear();
            break;
        }

        if (headerPos > 0) {
            // 移除帧头前的数据
            m_receiveBuffer.remove(0, headerPos);
        }

        // 需要至少3个字节来确定帧长度（帧头 + 类型）
        if (m_receiveBuffer.length() < 3) {
            break;
        }

        quint8 dataType = static_cast<unsigned char>(m_receiveBuffer[2]);
        int frameLength = 0;

        // 根据数据类型确定帧长度
        if (dataType == 0x15) {
            // 光强数据：0x5A 0x5A 0x15 0x04 + 4字节数据 + 校验和 = 9字节
            frameLength = 9;
        } else if (dataType == 0x45) {
            // 温度/气压/湿度/海拔数据
            // 0x5A 0x5A 0x45 0x0A + 10字节数据 + 校验和 = 15字节
            frameLength = 15;
        } else if (dataType == 0x55) {
            // IIC地址数据：0x5A 0x5A 0x55 0x01 + 1字节数据 + 校验和 = 6字节
            frameLength = 6;
        } else {
            // 未知类型，跳过这个字节
            m_receiveBuffer.remove(0, 1);
            continue;
        }

        // 检查是否有完整的帧
        if (m_receiveBuffer.length() < frameLength) {
            break;
        }

        // 提取一个完整的帧
        QByteArray frame = m_receiveBuffer.mid(0, frameLength);
        
        // 解析帧
        if (parseGY39Frame(frame)) {
            emit dataReceived(m_currentData);
        }

        // 移除已处理的帧
        m_receiveBuffer.remove(0, frameLength);
    }
}

bool GY39Sensor::parseGY39Frame(const QByteArray &frame)
{
    if (frame.length() < 3) {
        return false;
    }

    // 检查帧头
    if (static_cast<unsigned char>(frame[0]) != 0x5A ||
        static_cast<unsigned char>(frame[1]) != 0x5A) {
        return false;
    }

    quint8 dataType = static_cast<unsigned char>(frame[2]);

    switch (dataType) {
        case 0x15:
            return parseLightFrame(frame);
        case 0x45:
            return parseMultiSensorFrame(frame);
        case 0x55:
            // IIC地址，暂不处理
            return true;
        default:
            return false;
    }
}

bool GY39Sensor::parseLightFrame(const QByteArray &frame)
{
    if (frame.length() != 9) {
        return false;
    }

    // 验证数据长度字段
    if (static_cast<unsigned char>(frame[3]) != 0x04) {
        return false;
    }

    // 计算校验和
    unsigned char checksum = 0;
    for (int i = 0; i < 8; ++i) {
        checksum += static_cast<unsigned char>(frame[i]);
    }
    checksum = checksum & 0xFF;

    if (checksum != static_cast<unsigned char>(frame[8])) {
        emit errorOccurred("Light frame checksum error");
        return false;
    }

    // 解析光强数据
    // Lux = (Byte4<<24) | (Byte5<<16) | (Byte6<<8) | Byte7
    // 实际值 = Lux / 100
    quint32 lux = (static_cast<unsigned char>(frame[4]) << 24) |
                  (static_cast<unsigned char>(frame[5]) << 16) |
                  (static_cast<unsigned char>(frame[6]) << 8) |
                  static_cast<unsigned char>(frame[7]);

    m_currentData.light = static_cast<double>(lux) / 100.0;
    m_currentData.isValid = true;

    return true;
}

bool GY39Sensor::parseMultiSensorFrame(const QByteArray &frame)
{
    if (frame.length() != 15) {
        return false;
    }

    // 验证数据长度字段
    if (static_cast<unsigned char>(frame[3]) != 0x0A) {
        return false;
    }

    // 计算校验和
    unsigned char checksum = 0;
    for (int i = 0; i < 14; ++i) {
        checksum += static_cast<unsigned char>(frame[i]);
    }
    checksum = checksum & 0xFF;

    if (checksum != static_cast<unsigned char>(frame[14])) {
        emit errorOccurred("Multi-sensor frame checksum error");
        return false;
    }

    // 解析温度（Byte4-5）
    // T = (Byte4<<8) | Byte5, actual value = T / 100
    // Use unsigned intermediate to properly handle two's complement representation
    quint16 tempRawUnsigned = (static_cast<unsigned char>(frame[4]) << 8) |
                              static_cast<unsigned char>(frame[5]);
    qint16 tempRaw = static_cast<qint16>(tempRawUnsigned);
    m_currentData.temperature = static_cast<double>(tempRaw) / 100.0;

    // 解析气压（Byte6-9）
    // P = (Byte6<<24) | (Byte7<<16) | (Byte8<<8) | Byte9, 实际值 = P / 100
    quint32 pressureRaw = (static_cast<unsigned char>(frame[6]) << 24) |
                          (static_cast<unsigned char>(frame[7]) << 16) |
                          (static_cast<unsigned char>(frame[8]) << 8) |
                          static_cast<unsigned char>(frame[9]);
    m_currentData.pressure = static_cast<double>(pressureRaw) / 100.0;

    // 解析湿度（Byte10-11）
    // Hum = (Byte10<<8) | Byte11, 实际值 = Hum / 100
    quint16 humRaw = (static_cast<unsigned char>(frame[10]) << 8) |
                     static_cast<unsigned char>(frame[11]);
    m_currentData.humidity = static_cast<double>(humRaw) / 100.0;

    // 解析海拔（Byte12-13）
    // H = (Byte12<<8) | Byte13, unit = meters (no scaling)
    // Use unsigned intermediate to properly handle two's complement representation (supports below sea level)
    quint16 altitudeRawUnsigned = (static_cast<unsigned char>(frame[12]) << 8) |
                                  static_cast<unsigned char>(frame[13]);
    qint16 altitudeRaw = static_cast<qint16>(altitudeRawUnsigned);
    m_currentData.altitude = static_cast<double>(altitudeRaw);

    m_currentData.isValid = true;

    return true;
}

void GY39Sensor::onSerialError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        emit errorOccurred(QString("Serial error: %1").arg(m_serial->errorString()));
        if (error == QSerialPort::ResourceError) {
            closePort();
        }
    }
}

