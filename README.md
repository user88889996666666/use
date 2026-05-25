# use

GEC6818 ARM 开发板 Qt 环境监测报警系统示例工程。

## 构建运行

```bash
qmake EnvironmentalMonitoring.pro
make -j$(nproc)
./EnvironmentalMonitoring
```

## 模块说明

- `mainwindow.* / mainwindow.ui`：UI与主业务流程
- `sensormanager.*`：传感器采集与定时更新
- `alarmsystem.*`：多参数报警规则与蜂鸣器控制
- `datalogger.*`：CSV历史数据持久化
- `hardwareinterface.*`：硬件接口抽象（当前为模拟实现，便于在Linux/GEC6818上替换）
- `gy39sensor.*`：GY-39多功能传感器模块驱动（支持UART通信）

## 已实现控制能力

- 周期采集温湿度、光照、气压、海拔、可燃气浓度
- 基于阈值的蜂鸣器报警联动（温度/湿度/气压/可燃气）
- UI虚拟控件实时调节报警阈值，并支持一键恢复默认阈值
- 自动/手动两种照明控制模式切换（LED亮度控制）

## GY-39传感器集成

GY-39是一款集气压、温湿度、光强度于一身的多功能传感器模块，通过UART接口与系统通信。

### 硬件与通信参数

- **供电**：3V~5V
- **核心引脚**：VCC、GND、CT(TX/SCL)、DR(RX/SDA)、S0、S1
- **默认串口模式（S0悬空）**：UART 9600 8N1
- **MCU-IIC模式（S0接GND）**：IIC地址 `0x5B`
- **纯芯片模式（S1接GND）**：直接芯片访问

### 支持的传感器参数

- **温度**：范围 -10 ~ 60°C，精度 ±0.5°C
- **湿度**：范围 0 ~ 100%RH，精度 ±3%RH  
- **气压**：范围 300 ~ 1100hPa，精度 ±1hPa
- **光强**：范围 0 ~ 40000 lux

### 串口通信配置

- **波特率**：9600 bps（推荐）或 115200 bps
- **数据位**：8
- **停止位**：1
- **校验位**：无

### 协议说明

#### 输出帧格式

1. **光强数据帧（0x15）**：9字节
   ```
   0x5A 0x5A | 0x15 | 0x04 | [4字节光强数据] | [校验和]
   ```

2. **多传感器数据帧（0x45）**：15字节
   ```
   0x5A 0x5A | 0x45 | 0x0A | [温度2B][气压4B][湿度2B][海拔2B] | [校验和]
   ```

3. **IIC地址帧（0x55）**：6字节
   ```
   0x5A 0x5A | 0x55 | 0x01 | [1字节IIC地址] | [校验和]
   ```

#### 配置命令

命令格式：`0xA5 | [配置字节] | [校验和]`

配置选项：
- **Bit0 - AUTO**（默认1）：上电自动输出
- **Bit1 - BME**（默认1）：输出温度/气压/湿度/海拔
- **Bit2 - MAX**（默认1）：输出光强

#### 主动查询命令

- **查光照**：`0xA5 0x51 0xF6`
- **查温压湿海拔**：`0xA5 0x52 0xF7`
- **换算规则**：温度/湿度/气压/光照数据原始值均需 `÷100`（海拔保持米单位）

### 使用示例

```cpp
// 初始化GY-39传感器
HardwareInterface hw;
bool connected = hw.initializeGY39("/dev/ttyUSB0", 9600);

// 读取传感器数据（优先使用GY-39，否则使用模拟数据）
if (hw.isGY39Connected()) {
    double temp = hw.readTemperature();      // °C
    double humidity = hw.readHumidity();     // %RH
    double pressure = hw.readPressure();     // hPa
    double light = hw.readLight();           // lux
    double altitude = hw.readAltitude();     // m
}
```

### 降级方案

若GY-39传感器未连接或通信失败，系统自动使用模拟数据，确保应用可继续运行。
