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
