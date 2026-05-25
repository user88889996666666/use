#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <QObject>
#include <QDateTime>

class HardwareInterface;
class QTimer;

struct SensorData {
    double temperature = 0.0;
    double humidity = 0.0;
    double light = 0.0;
    double pressure = 0.0;
    double altitude = 0.0;
    double gasConcentration = 0.0;
    QDateTime timestamp;
};

class SensorManager : public QObject
{
    Q_OBJECT
public:
    explicit SensorManager(HardwareInterface *hardware, QObject *parent = nullptr);

    // 启动/停止周期采样（含可燃气浓度定时更新）
    void start(int intervalMs = 1000);
    void stop();

signals:
    void sensorDataUpdated(const SensorData &data);

private slots:
    void collectData();

private:
    HardwareInterface *m_hardware;
    QTimer *m_timer;
};

#endif // SENSORMANAGER_H
