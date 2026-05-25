#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "datalogger.h"
#include "hardwareinterface.h"
#include "sensormanager.h"

#include <QDateTime>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QStatusBar>
#include <QtGlobal>

namespace {
// 与UI中亮度滑条最大值保持一致（0-100）。
constexpr int kMaxBrightness = 100;
constexpr double kAutoBrightnessLightScale = 10.0;
constexpr double kComfortIdealTemperature = 24.0;
constexpr double kComfortIdealHumidity = 55.0;
constexpr double kTemperaturePenaltyFactor = 3.0;
constexpr double kHumidityPenaltyFactor = 1.5;
constexpr double kThresholdCompareEpsilon = 0.0001;
const QStringList kGY39CandidatePorts = {
    QStringLiteral("/dev/ttyUSB0"),
    QStringLiteral("/dev/ttySAC1"),
    QStringLiteral("/dev/ttyS1"),
    QStringLiteral("COM3")
};
}

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
    connect(m_alarmSystem, &AlarmSystem::alarmTriggered, this, [this](const QString &message) {
        ui->alarmMessageLabel->setText(message);
        ui->alarmStateLabel->setText(QStringLiteral("报警中"));
    });
    connect(m_alarmSystem, &AlarmSystem::alarmCleared, this, [this]() {
        ui->alarmMessageLabel->setText(QStringLiteral("无"));
        ui->alarmStateLabel->setText(QStringLiteral("正常"));
    });
    connect(m_hardware, &HardwareInterface::gy39Connected, this, [this]() {
        updateGy39Status(true);
    });
    connect(m_hardware, &HardwareInterface::gy39Disconnected, this, [this]() {
        updateGy39Status(false);
    });
    connect(ui->tempThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAlarmThresholdChanged);
    connect(ui->humidityThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAlarmThresholdChanged);
    connect(ui->pressureLowThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAlarmThresholdChanged);
    connect(ui->pressureHighThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAlarmThresholdChanged);
    connect(ui->gasThresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAlarmThresholdChanged);
    connect(ui->resetThresholdsButton, &QPushButton::clicked,
            this, &MainWindow::onResetThresholdsClicked);

    initializeGy39();
    loadThresholdsToUi();
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

void MainWindow::onAlarmThresholdChanged()
{
    applyThresholdsFromUi();
}

void MainWindow::onResetThresholdsClicked()
{
    m_alarmSystem->setThresholds(AlarmThresholds{});
    loadThresholdsToUi();
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

    const int calculatedBrightness = kMaxBrightness - static_cast<int>(lightIntensity / kAutoBrightnessLightScale);
    const int autoBrightness = qBound(0, calculatedBrightness, kMaxBrightness);
    m_hardware->setLedBrightness(autoBrightness);
    ui->brightnessSlider->setValue(autoBrightness);
    ui->ledBrightnessValueLabel->setText(QString::number(autoBrightness));
}

double MainWindow::comfortIndex(const SensorData &data) const
{
    const double temperatureScore = qMax(0.0, 100.0 - qAbs(data.temperature - kComfortIdealTemperature) * kTemperaturePenaltyFactor);
    const double humidityScore = qMax(0.0, 100.0 - qAbs(data.humidity - kComfortIdealHumidity) * kHumidityPenaltyFactor);
    return (temperatureScore + humidityScore) / 2.0;
}

void MainWindow::applyThresholdsFromUi()
{
    AlarmThresholds thresholds;
    thresholds.highTemperature = ui->tempThresholdSpinBox->value();
    thresholds.highHumidity = ui->humidityThresholdSpinBox->value();
    thresholds.lowPressure = ui->pressureLowThresholdSpinBox->value();
    thresholds.highPressure = ui->pressureHighThresholdSpinBox->value();
    thresholds.highGas = ui->gasThresholdSpinBox->value();

    m_alarmSystem->setThresholds(thresholds);
    const AlarmThresholds adjustedThresholds = m_alarmSystem->thresholds();
    const auto differs = [](double lhs, double rhs) {
        return qAbs(lhs - rhs) > kThresholdCompareEpsilon;
    };
    if (differs(adjustedThresholds.highTemperature, thresholds.highTemperature) ||
        differs(adjustedThresholds.highHumidity, thresholds.highHumidity) ||
        differs(adjustedThresholds.lowPressure, thresholds.lowPressure) ||
        differs(adjustedThresholds.highPressure, thresholds.highPressure) ||
        differs(adjustedThresholds.highGas, thresholds.highGas)) {
        loadThresholdsToUi();
    }
}

void MainWindow::loadThresholdsToUi()
{
    const AlarmThresholds thresholds = m_alarmSystem->thresholds();
    const auto setSpinBoxValueSilently = [](QDoubleSpinBox *spinBox, double value) {
        spinBox->blockSignals(true);
        spinBox->setValue(value);
        spinBox->blockSignals(false);
    };

    setSpinBoxValueSilently(ui->tempThresholdSpinBox, thresholds.highTemperature);
    setSpinBoxValueSilently(ui->humidityThresholdSpinBox, thresholds.highHumidity);
    setSpinBoxValueSilently(ui->pressureLowThresholdSpinBox, thresholds.lowPressure);
    setSpinBoxValueSilently(ui->pressureHighThresholdSpinBox, thresholds.highPressure);
    setSpinBoxValueSilently(ui->gasThresholdSpinBox, thresholds.highGas);
}

void MainWindow::initializeGy39()
{
    for (const QString &portName : kGY39CandidatePorts) {
        if (m_hardware->initializeGY39(portName, 9600)) {
            updateGy39Status(true, portName);
            statusBar()->showMessage(QStringLiteral("GY-39 已连接: %1 (UART 9600 8N1)").arg(portName), 5000);
            return;
        }
    }

    updateGy39Status(false);
    statusBar()->showMessage(QStringLiteral("GY-39 未连接，当前使用模拟数据"), 5000);
}

void MainWindow::updateGy39Status(bool connected, const QString &portName)
{
    if (connected) {
        if (portName.isEmpty()) {
            ui->gy39StatusValueLabel->setText(QStringLiteral("已连接（UART）"));
        } else {
            ui->gy39StatusValueLabel->setText(QStringLiteral("已连接（%1）").arg(portName));
        }
    } else {
        ui->gy39StatusValueLabel->setText(QStringLiteral("未连接"));
    }
}
