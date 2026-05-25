#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

#include "alarmsystem.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class HardwareInterface;
class SensorManager;
class DataLogger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSensorDataUpdated(const SensorData &data);
    void onManualBrightnessChanged(int value);
    void onAutoModeChanged(bool checked);
    void onAlarmThresholdChanged();
    void onResetThresholdsClicked();

private:
    Ui::MainWindow *ui;
    HardwareInterface *m_hardware;
    SensorManager *m_sensorManager;
    AlarmSystem *m_alarmSystem;
    DataLogger *m_dataLogger;

    // UI刷新与控制逻辑
    void updateEnvironmentDisplay(const SensorData &data);
    void updateLedControl(double lightIntensity);
    double comfortIndex(const SensorData &data) const;
    void applyThresholdsFromUi();
    void loadThresholdsToUi();
    void initializeGy39();
    void updateGy39Status(bool connected, const QString &portName = QString());
};

#endif // MAINWINDOW_H
