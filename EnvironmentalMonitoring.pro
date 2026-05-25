QT       += core gui widgets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EnvironmentalMonitoring
TEMPLATE = app

CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    sensormanager.cpp \
    alarmsystem.cpp \
    datalogger.cpp \
    hardwareinterface.cpp \
    gy39sensor.cpp

HEADERS += \
    mainwindow.h \
    sensormanager.h \
    alarmsystem.h \
    datalogger.h \
    hardwareinterface.h \
    gy39sensor.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
