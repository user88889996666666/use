#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "datalogger.h"
#include "hardwareinterface.h"
#include "sensormanager.h"

#include <QDateTime>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_hardware(new HardwareInterface(this))
    , m_sensorManager(new SensorManager(m_hardware, this))
    , m_alarmSystem(new AlarmSystem(m_hardware, this))
    , m_dataLogger(new DataLogger(this))
{
    ui->setupUi(this);

    connect(m_sensorManager, &SensorManager::sensorDataUpdated,
            this, &MainWindow::onSensorDataUpdated);
    connect(ui->brightnessSlider, &QSlider::valueChanged,
            this, &MainWindow::onManualBrightnessChanged);
    connect(ui->autoModeCheckBox, &QCheckBox::toggled,
            this, &MainWindow::onAutoModeChanged);
    connect(m_alarmSystem, &AlarmSystem::alarmTriggered, ui->alarmMessageLabel, [this](const QString &message) {
        ui->alarmMessageLabel->setText(message);
        ui->alarmStateLabel->setText(QStringLiteral("报警中"));
    });
    connect(m_alarmSystem, &AlarmSystem::alarmCleared, ui->alarmMessageLabel, [this]() {
        ui->alarmMessageLabel->setText(QStringLiteral("无"));
        ui->alarmStateLabel->setText(QStringLiteral("正常"));
    });

    onAutoModeChanged(ui->autoModeCheckBox->isChecked());
    m_sensorManager->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSensorDataUpdated(const SensorData &data)
{
    updateEnvironmentDisplay(data);
    updateLedControl(data.light);

    const AlarmStatus alarm = m_alarmSystem->evaluate(data);
    m_dataLogger->log(data, alarm);
}

void MainWindow::onManualBrightnessChanged(int value)
{
    if (!ui->autoModeCheckBox->isChecked()) {
        m_hardware->setLedBrightness(value);
        ui->ledBrightnessValueLabel->setText(QString::number(value));
    }
}

void MainWindow::onAutoModeChanged(bool checked)
{
    ui->brightnessSlider->setEnabled(!checked);
    ui->controlModeValueLabel->setText(checked ? QStringLiteral("自动") : QStringLiteral("手动"));

    if (!checked) {
        onManualBrightnessChanged(ui->brightnessSlider->value());
    }
}

void MainWindow::updateEnvironmentDisplay(const SensorData &data)
{
    ui->temperatureValueLabel->setText(QString::number(data.temperature, 'f', 1) + QStringLiteral(" °C"));
    ui->humidityValueLabel->setText(QString::number(data.humidity, 'f', 1) + QStringLiteral(" %"));
    ui->lightValueLabel->setText(QString::number(data.light, 'f', 0) + QStringLiteral(" lux"));
    ui->pressureValueLabel->setText(QString::number(data.pressure, 'f', 1) + QStringLiteral(" hPa"));
    ui->altitudeValueLabel->setText(QString::number(data.altitude, 'f', 1) + QStringLiteral(" m"));
    ui->gasValueLabel->setText(QString::number(data.gasConcentration, 'f', 1) + QStringLiteral(" ppm"));

    const double comfort = comfortIndex(data);
    ui->comfortValueLabel->setText(QString::number(comfort, 'f', 1));
    ui->timestampValueLabel->setText(data.timestamp.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));

    ui->buzzerValueLabel->setText(m_hardware->buzzerEnabled() ? QStringLiteral("开") : QStringLiteral("关"));
    ui->ledBrightnessValueLabel->setText(QString::number(m_hardware->ledBrightness()));
    ui->logPathValueLabel->setText(m_dataLogger->logFilePath());
}

void MainWindow::updateLedControl(double lightIntensity)
{
    if (!ui->autoModeCheckBox->isChecked()) {
        return;
    }

    const int autoBrightness = qBound(0, 100 - static_cast<int>(lightIntensity / 10.0), 100);
    m_hardware->setLedBrightness(autoBrightness);
    ui->brightnessSlider->setValue(autoBrightness);
    ui->ledBrightnessValueLabel->setText(QString::number(autoBrightness));
}

double MainWindow::comfortIndex(const SensorData &data) const
{
    const double temperatureScore = 100.0 - qAbs(data.temperature - 24.0) * 3.0;
    const double humidityScore = 100.0 - qAbs(data.humidity - 55.0) * 1.5;
    return qMax(0.0, (temperatureScore + humidityScore) / 2.0);
}
